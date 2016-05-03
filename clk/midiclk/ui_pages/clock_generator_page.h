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

#ifndef MIDICLK_UI_PAGES_CLOCK_GENERATOR_PAGE_H_
#define MIDICLK_UI_PAGES_CLOCK_GENERATOR_PAGE_H_

#include "midiclk/ui_pages/ui_page.h"
#include "midiclk/internal_clock.h"
#include "midiclk/midi_out.h"
#include "midiclk/midi_in.h"
#include "midiclk/eemem.h"

namespace midiclk {

class ClockGeneratorPage : public UiPage {
 public:
  ClockGeneratorPage() { }
  
  static void OnInit(PageInfo* pageInfo, UiPageIndex prevPage);
  static void OnQuit(UiPageIndex nextPage);

  static uint8_t OnIncrement(uint8_t id, int8_t value);
  static uint8_t OnClick(uint8_t id, uint8_t value);
  static uint8_t OnSwitch(uint8_t id, uint8_t value);
  static uint8_t OnIdle();

  static void UpdateScreen();
  static void UpdateLeds();
  
  static const prog_EventHandlers event_handlers_;

  static uint16_t tempo() { return tempo_; }
  static void set_tempo(uint16_t tempo) { 
    tempo_ = tempo; UpdateClock(); save_tempo_ = 1; 
  }

  static uint8_t step_leng() { return step_leng_; }
  static void set_step_leng(uint8_t step_leng) { 
    step_leng_ = step_leng;
  }

  static uint8_t step_numb() { return step_numb_; }
  static void set_step_numb(uint8_t step_numb) { 
    step_numb_ = step_numb;
  }

  static uint8_t hold_mode() { return hold_mode_; }
  static void set_hold_mode(uint8_t hold_mode) { 
    hold_mode_ = hold_mode;
  }

  static uint8_t pass_thru() { return pass_thru_; }
  static void set_pass_thru(uint8_t pass_thru) { 
    pass_thru_ = pass_thru;
  }

  static uint8_t round_bpm() { return round_bpm_; }
  static void set_round_bpm(uint8_t round_bpm) { 
    round_bpm_ = round_bpm;
  }

  static uint8_t threshold() { return threshold_; }
  static void set_threshold(uint8_t threshold) { 
    threshold_ = threshold;
  }

  static void SetStepLeng(uint8_t step_leng);
  static void SetStepNumb(uint8_t step_numb);
  static void SetHoldMode(uint8_t hold_mode);
  static void SetPassThru(uint8_t pass_thru);
  static void SetRoundBpm(uint8_t round_bpm);
  static void SetThreshold(uint8_t threshold);
  
 protected:

  enum State {
    STATE_STOPPED,
    STATE_PENDING_START,
    STATE_STARTED,
    STATE_PENDING_STOP,
  };

  static uint16_t tempo_;
  static uint8_t state_;
  static uint8_t save_tempo_;
  static uint8_t started_externally_;
  static uint8_t request_internal_clock_;
  static uint8_t send_internal_clock_;
  static uint8_t input_clock_present_;
  static uint8_t input_clock_stable_;
  static uint8_t input_clock_stable_prev_;
  static uint8_t input_clock_counter_;
  static uint8_t input_clock_counter_last_;
  static uint8_t clock_prescaler_;
  static uint8_t clock_prescaler_counter_;
  static uint8_t step_counter_;
  static uint8_t half_step_;
  static uint32_t last_increment_time_;

  static uint8_t step_leng_;
  static uint8_t step_numb_;
  static uint8_t hold_mode_;
  static uint8_t pass_thru_;
  static uint8_t round_bpm_;
  static uint8_t threshold_;

  static void StartStop();
  static void UpdateClock();
  static void AdvanceClockCounter();
  static void InternalClockCallbackHandler();
  static void MidiInCallbackHandler(uint8_t byte);
  static void UpdateTempo();
  static uint16_t CalcTempo();

  static uint8_t running() {
    return !(state_ == STATE_STOPPED && !started_externally_);
  }

  static void SendNow(uint8_t byte) {
	  midi_out.SendNow(byte);
  }

  static void SendClock();
  static void SendStart();
  static void SendContinue();
  static void SendStop();

  DISALLOW_COPY_AND_ASSIGN(ClockGeneratorPage);
};

}  // namespace midiclk

#endif  // MIDICLK_UI_PAGES_CLOCK_GENERATOR_PAGE_H_
