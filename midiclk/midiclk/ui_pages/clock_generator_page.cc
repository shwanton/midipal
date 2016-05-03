// Copyright 2015 Peter Kvitek.
//
// Author: Peter Kvitek (pete@kvitek.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Clock generator page class.

#include "midiclk/ui_pages/clock_generator_page.h"
#include "midiclk/clock_data.h"
#include "midiclk/sync.h"

namespace midiclk {

// Module local declarations

static const uint8_t kInputClockIcon = '>';
static const uint8_t kStableClockIcon = 1;

static const uint8_t kBufferSize = 1 * 24;

ClockData<kBufferSize> clock_data;

static volatile uint8_t overflow_count;

/* <static> */
uint16_t ClockGeneratorPage::tempo_ = kDefTempo;
uint8_t ClockGeneratorPage::state_ = STATE_STOPPED;
uint8_t ClockGeneratorPage::save_tempo_;
uint8_t ClockGeneratorPage::started_externally_;
uint8_t ClockGeneratorPage::request_internal_clock_;
uint8_t ClockGeneratorPage::send_internal_clock_;
uint8_t ClockGeneratorPage::input_clock_present_;
uint8_t ClockGeneratorPage::input_clock_stable_;
uint8_t ClockGeneratorPage::input_clock_stable_prev_;
uint8_t ClockGeneratorPage::input_clock_counter_;
uint8_t ClockGeneratorPage::input_clock_counter_last_;
uint8_t ClockGeneratorPage::clock_prescaler_;
uint8_t ClockGeneratorPage::clock_prescaler_counter_;
uint8_t ClockGeneratorPage::step_counter_;
uint8_t ClockGeneratorPage::half_step_;
uint32_t ClockGeneratorPage::last_increment_time_;

uint8_t ClockGeneratorPage::step_leng_;
uint8_t ClockGeneratorPage::step_numb_;
uint8_t ClockGeneratorPage::hold_mode_;
uint8_t ClockGeneratorPage::pass_thru_;
uint8_t ClockGeneratorPage::round_bpm_;
uint8_t ClockGeneratorPage::threshold_;
/* </static> */

/* static */
const prog_EventHandlers ClockGeneratorPage::event_handlers_ PROGMEM = {
  OnInit,
  OnQuit,
  OnIncrement,
  OnClick,
  OnSwitch,
  OnIdle,
  UpdateScreen,
  UpdateLeds,
};

/* static */
void ClockGeneratorPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
  if (prevPage != NO_PAGE)
    return;

  SetStepLeng(eemem.step_leng());
  SetStepNumb(eemem.step_numb());
  SetHoldMode(eemem.hold_mode());
  SetRoundBpm(eemem.round_bpm());
  SetPassThru(eemem.pass_thru());
  SetThreshold(eemem.threshold());

  clock_data.Init();
  internal_clock.Init();
  internal_clock.set_callback(InternalClockCallbackHandler);
  set_tempo(eemem.tempo()); save_tempo_ = 0;
  internal_clock.Start();
  clock.Start();

  Timer<0>::set_prescaler(4);
  Timer<0>::set_mode(TIMER_NORMAL);
  Timer<0>::Start();

  midi_in.set_callback(MidiInCallbackHandler);

  lcd.SetCustomCharMapRes(chr_res_custom_icons, 1, kStableClockIcon);
}

/* static */
void ClockGeneratorPage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t ClockGeneratorPage::OnIncrement(uint8_t id, int8_t value) {
  if (!changing_page())
    return 0;

  uint32_t time = milliseconds();
  uint16_t time_since_last_increment = time - last_increment_time_;
  last_increment_time_ = time;

  uint16_t base_tempo = tempo();
  if (time_since_last_increment < 50) {
    value*= 100; base_tempo = (base_tempo / 100) * 100;
  } else
  if (time_since_last_increment < 250 || round_bpm_) {
    value*= 10; base_tempo = (base_tempo / 10) * 10;
  }

  set_tempo(Clamp16(base_tempo + value, kMinTempo, kMaxTempo));

  return 1;
}

/* static */
uint8_t ClockGeneratorPage::OnClick(uint8_t id, uint8_t value) {
  switch (value) {
    case CLICK:
    case DBLCLICK:
      ToggleChangingPage();
      if (!changing_page()) {
        input_clock_stable_prev_ = !input_clock_stable_;
      }
      break;
  }
  return 1;
}

/* static */
uint8_t ClockGeneratorPage::OnSwitch(uint8_t id, uint8_t value) {
  if (id == SWITCH) {
    StartStop();
    return 1;
  }

  return 0;
}

/* static */
uint8_t ClockGeneratorPage::OnIdle() {
  if (save_tempo_) {
    eemem.set_tempo(tempo_);
    save_tempo_ = 0;
  }

  input_clock_present_ = input_clock_counter_ > 0 && (input_clock_counter_last_ != input_clock_counter_);
  input_clock_counter_last_ = input_clock_counter_;
  if (!input_clock_present_) started_externally_ = 0;

  if (!changing_page() && input_clock_stable_ && input_clock_present_) {
    uint16_t tempo = ClockGeneratorPage::CalcTempo();
    if (tempo != tempo_) {
      set_tempo(tempo);
    }
  }

  return 1;
}

/* static */
void ClockGeneratorPage::UpdateScreen() {
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);

  // Display input clock marker
  if (input_clock_present_) {
    line1[0] = input_clock_stable_ ? kStableClockIcon : kInputClockIcon;
  }

  // Display bpm
  { uint8_t x = 1; uint16_t tempo_int = tempo_ / 10;
    x+= UnsafeItoaLen(tempo_int, 3, &line1[x]); line1[x++] = '.';
    UnsafeItoa(tempo_ - tempo_int * 10, 1, &line1[x]);
  }

  if (changing_page()) {
    DrawBrackets(&line1[0], 5);
  }

  // Display running indicator
  switch (state_) {
    case STATE_STARTED:
    case STATE_PENDING_STOP:
      line1[kLcdWidth - 1] = 0x7e;
      break;
  }

#if 0
  // Display state
  { uint8_t ch;
    switch (state_) {
      case STATE_STOPPED: ch = 'P'; break;
      case STATE_PENDING_START: ch = 's'; break;
      case STATE_STARTED: ch = 'S'; break;
      case STATE_PENDING_STOP: ch = 'p'; break;
    }
    line1[kLcdWidth - 1] = ch;
  }
#endif

  // Display step counter

  uint8_t brackets;
  switch (state_) {
    case STATE_STOPPED: 
      brackets = 0;
      break;
    case STATE_STARTED:
      brackets = 1;
      break;
    case STATE_PENDING_START:
    case STATE_PENDING_STOP:
      brackets = half_step_;
      break;
  }
  
  uint8_t x = 1;
  if (step_numb_ < 10) {
    x++;
    if (brackets) line2[x - 1] = '[';
    x+= UnsafeItoaLen(1 + step_counter_, 1, &line2[x]);
    line2[x++] = ':';
    x+= UnsafeItoaLen(step_numb_, 1, &line2[x]);
  } else {
    if (brackets) line2[x - 1] = '[';
    ui.PrintNN(&line2[x], 1 + step_counter_); x+= 2;
    line2[x++] = ':';
    x+= UnsafeItoaLen(step_numb_, 2, &line2[x]);
  }

  if (brackets) line2[x++] = ']';
}

/* static */
void ClockGeneratorPage::UpdateLeds() {
  // Check if clock stability changes to stable and start internal clock
  if (input_clock_stable_ != input_clock_stable_prev_) {
    input_clock_stable_prev_ = input_clock_stable_;
    ui.set_request_redraw();
    if (input_clock_present_ && input_clock_stable_ && !changing_page()) {
      UpdateTempo();
    }
  }
}

// Accessor helper routines

/* static */
void ClockGeneratorPage::SetStepLeng(uint8_t step_leng) {
  if (step_leng != step_leng_) {
    set_step_leng(step_leng);
    eemem.set_step_leng(step_leng);
    clock_prescaler_ = Duration::GetMidiClockTicks(step_leng);
  }
}

/* static */
void ClockGeneratorPage::SetStepNumb(uint8_t step_numb) {
  if (step_numb != step_numb_) {
    set_step_numb(step_numb);
    eemem.set_step_numb(step_numb);
  }
}

/* static */
void ClockGeneratorPage::SetHoldMode(uint8_t hold_mode) {
  if (hold_mode != hold_mode_) {
    set_hold_mode(hold_mode);
    eemem.set_hold_mode(hold_mode);
  }
}

/* static */
void ClockGeneratorPage::SetPassThru(uint8_t pass_thru) {
  if (pass_thru != pass_thru_) {
    set_pass_thru(pass_thru);
    eemem.set_pass_thru(pass_thru);
  }
}

/* static */
void ClockGeneratorPage::SetRoundBpm(uint8_t round_bpm) {
  if (round_bpm != round_bpm_) {
    set_round_bpm(round_bpm);
    eemem.set_round_bpm(round_bpm);
  }
}

/* static */
void ClockGeneratorPage::SetThreshold(uint8_t threshold) {
  if (threshold != threshold_) {
    set_threshold(threshold);
    eemem.set_threshold(threshold);
  }
}

// Clock related routines

/* static */
void ClockGeneratorPage::SendClock() {
  SendNow(0xf8);
  sync.OnClock();
  AdvanceClockCounter();
}

/* static */
void ClockGeneratorPage::SendStart() {
  SendNow(0xfa);
  sync.OnStart();
}

/* static */
void ClockGeneratorPage::SendContinue() {
  SendNow(0xfb);
  sync.OnContinue();
}

/* static */
void ClockGeneratorPage::SendStop() {
  SendNow(0xfc);
  sync.OnStop();
}

/* static */
void ClockGeneratorPage::StartStop() {
  switch (state_) {
    case STATE_STARTED:
      if (hold_mode_ == HOLDMODE_STARTSTOP && step_counter_ > 0) {
        state_ = STATE_PENDING_STOP;
      } else {
        SendStop();
        state_ = STATE_STOPPED;
      }
      return;
    case STATE_STOPPED:
      if (hold_mode_ >= HOLDMODE_START && step_counter_ > 0) {
        state_ = STATE_PENDING_START;
      } else {
        SendStart();
        state_ = STATE_STARTED;
        clock_prescaler_counter_ = 0;
        step_counter_ = 0;
      }
      return;
    case STATE_PENDING_START:
      state_ = STATE_STOPPED;
      return;
    case STATE_PENDING_STOP:
      state_ = STATE_STARTED;
      return;
  }
}

/* static */
void ClockGeneratorPage::UpdateClock() {
  uint8_t tempo_int = tempo_ / 10;
  clock.Update(tempo_int, (uint8_t)(tempo_ - tempo_int * 10));
}

/* static */
void ClockGeneratorPage::AdvanceClockCounter() {
  if (++clock_prescaler_counter_ >= clock_prescaler_) {
    sync.OnStep();
    clock_prescaler_counter_ = 0;
    if (++step_counter_ >= step_numb_) {
      sync.OnStepCount();
      step_counter_ = 0;

      if (running()) {
        switch (state_) {
          case STATE_PENDING_START:
            SendStart();
            state_ = STATE_STARTED;
            break;
          case STATE_PENDING_STOP: 
            SendStop();
            state_ = STATE_STOPPED;
            break;
        }
      }
    }
    ui.set_request_redraw();
  }

  // Update half step state
  uint8_t half_step;
  if (clock_prescaler_ < 4) {
    static uint8_t counter; 
    half_step = ++counter & 1;
  } else {
    half_step = clock_prescaler_counter_ < clock_prescaler_ / 2;
  }

  if (half_step != half_step_) {
    half_step_ = half_step;
    ui.set_request_redraw();
  }
}

/* static */
void ClockGeneratorPage::InternalClockCallbackHandler() {
  if (send_internal_clock_ || !input_clock_present_ || changing_page()) {
    SendClock();
  }
}

/* static */
void ClockGeneratorPage::MidiInCallbackHandler(uint8_t byte) {
  // This routine is called with interrupts disabled!
  switch (byte) {

    case 0xf8:  // clock
      ++input_clock_counter_; input_clock_present_ = 1;

      // Add clock tick to the data buffer to maintain statistics
      { uint16_t tick = static_cast<uint16_t>(overflow_count) << 8 | TCNT0;
        overflow_count = 0; TCNT0 = 0;
        clock_data.Add(tick);
      }

      if (!changing_page()) {
        // Check if input clock is stable and pass it through if not
        input_clock_stable_ = clock_data.full() && (clock_data.max_deviation() < (clock_data.average() / 100) * threshold_);
        if (!input_clock_stable_) {
          SendClock();
          send_internal_clock_ = 0;
          request_internal_clock_ = 0;
          ui.set_request_redraw();
        } else {
          // If input clock is stable, handle switch to internal clock request
          if (request_internal_clock_) {
            if (!send_internal_clock_) {
              SendClock();
            }
            request_internal_clock_ = 0;
            TCNT1 = 0;
            send_internal_clock_ = 1;
            ui.set_request_redraw();
          } else
          if (!send_internal_clock_) {
            SendClock();
          }
        }
      }
      break;

    case 0xfa:  // start
      SendStart();
      input_clock_counter_ = 0;
      input_clock_counter_last_ = 0;
      clock_prescaler_counter_ = 0;
      step_counter_ = 0;
      goto Fix;

    case 0xfb:  // continue
      SendContinue();
Fix:  input_clock_present_ = 1;
      started_externally_ = 1;
      state_ = STATE_STARTED;
      ui.set_request_redraw();
      break;

    case 0xfc:  // stop
      SendStop();
      started_externally_ = 0;
      clock_prescaler_counter_ = 0;
      step_counter_ = 0;
      state_ = STATE_STOPPED;
      ui.set_request_redraw();
      break;

    default:    // thru
      if (pass_thru_) {
        SendNow(byte);
      }
      break;
  }
}

/* static */
void ClockGeneratorPage::UpdateTempo() {
  uint16_t tempo = CalcTempo();
  if (!tempo)
    return;
    
  if (tempo != tempo_) {
    set_tempo(tempo);
  }

  // Request switch to internal clock on the next input clock
  request_internal_clock_ = 1;
}

/* static */
uint16_t ClockGeneratorPage::CalcTempo() {
  if (!input_clock_stable_ || !clock_data.full())
    return 0;

  // Copy clock data to a local buffer
  ClockData<kBufferSize> clock_data2;
  { DisableInterrupts di;
    clock_data2 = clock_data;
  }

  // Convert ticks to bpm * 100
  uint16_t bpm_buffer[kBufferSize];
  for (uint8_t n = 0; n < kBufferSize; n++) {
//  BPM = (60 * 20,000,000 / 256) / (tick_count * 24);
    bpm_buffer[n] = 19531250.0l / clock_data2.data(n);
  }

  // Calculate average bpm
  double ave = 0.0; 
  for (uint8_t n = 0; n < kBufferSize; n++) {
    ave+= bpm_buffer[n];
  }

  ave = ave / (kBufferSize * 10);
  uint16_t tempo = Round(ave);

  if (tempo < kMinTempo) {
    tempo = kMinTempo; 
  } else
  if (tempo > kMaxTempo) {
    tempo = kMaxTempo;
  } else
  if (round_bpm_) {
    if (tempo % 10 > 5) {
      tempo = (tempo / 10) * 10 + 10;
    } else
      tempo = (tempo / 10) * 10;
  }

  return tempo;
}

/* global */
ISR(TIMER0_OVF_vect) {
  // 20MHz/256/256 = 305.1757813Hz
  ++overflow_count;
}

}  // namespace midiclk
