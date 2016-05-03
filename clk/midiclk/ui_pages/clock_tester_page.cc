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
// Clock tester page class.

#include "midiclk/ui_pages/clock_tester_page.h"
#include "midiclk/midi_out.h"
#include "midiclk/eemem.h"

#include <math.h>

namespace midiclk {

// Module local declarations

static const uint8_t kBufferSize = 4 * 24;

struct Tick16 { uint8_t overflow_count; uint16_t tick_count; };

static Tick16 tick_buffer[kBufferSize];
static volatile uint8_t tick_buffer_ready;
static volatile uint8_t next_buffer_slot;

static uint8_t display_max_stddev;
static double max_stddev;

static volatile uint8_t overflow_count;

/* static */
const prog_EventHandlers ClockTesterPage::event_handlers_ PROGMEM = {
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
void ClockTesterPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
  next_buffer_slot = 0;
  tick_buffer_ready = 0;
  max_stddev = 0.0;
  display_max_stddev = eemem.display_max_stddev();

  Timer<1>::set_prescaler(1);
  Timer<1>::set_mode(TIMER_NORMAL);
  Timer<1>::Start();

  midi_in.set_callback(MidiInCallbackHandler);
}

/* static */
void ClockTesterPage::OnQuit(UiPageIndex nextPage) {
  midi_in.set_callback(0);
  Timer<1>::Stop();
}

/* static */
uint8_t ClockTesterPage::OnIncrement(uint8_t id, int8_t value) {
  return 1;
}

/* static */
uint8_t ClockTesterPage::OnClick(uint8_t id, uint8_t value) {
  switch (value) {
    case CLICK:
      max_stddev = 0.0;
      break;
    case DBLCLICK:
      display_max_stddev = display_max_stddev ? 0 : 1;
      eemem.set_display_max_stddev(display_max_stddev);
      break;
  }
  return 1;
}

/* static */
uint8_t ClockTesterPage::OnSwitch(uint8_t id, uint8_t value) {
  if (id == SWITCH) {
    DisableInterrupts disableInterrupts;
    tick_buffer_ready = 0;
    next_buffer_slot = 0; 
    max_stddev = 0.0;
    return 1;
  }

  return 0;
}

/* static */
uint8_t ClockTesterPage::OnIdle() {
  // Return positive to force screen redraw
  return 1;
}

/* static */
void ClockTesterPage::UpdateScreen() {
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);

  // Check if buffer is not ready yet and bail out
  if (!tick_buffer_ready) {
    memcpy_P(&line1[2], PSTRN("Wait"));
    UnsafeItoa(numbof(tick_buffer) - next_buffer_slot, 5, &line2[3]);
    return;
  }

  // Copy tick buffer to local storage disabling interrupts
  Tick16 tick_buffer2[kBufferSize];
  { DisableInterrupts disableInterrupts;
    memcpy(tick_buffer2, tick_buffer, sizeof(tick_buffer));
  }

  // Convert ticks to bpm * 100
  uint16_t bpm_buffer[kBufferSize];
  for (uint8_t n = 0; n < kBufferSize; n++) {
    uint32_t tick_count = ((uint32_t)tick_buffer2[n].overflow_count << 16) + tick_buffer2[n].tick_count;
//  BPM = (60 * 20,000,000) / (tick_count * 24);
    bpm_buffer[n] = (uint16_t)(5000000000.0l / tick_count);
  }

  // Calculate average bpm
  double ave = 0.0; 
  for (uint8_t n = 0; n < kBufferSize; n++) {
    ave+= bpm_buffer[n];
  }

  ave = ave / kBufferSize;
  uint16_t bpm = Round(ave);

  // Display average bpm
  { uint8_t x = 1; uint16_t bpm_int = bpm / 100;
    x+= UnsafeItoaLen(bpm_int, 3, &line1[x]); line1[x++] = '.';
#if 0
    UnsafeItoa(bpm - bpm_int * 100, 2, &line1[x]);
#else
    uint8_t dec = bpm - bpm_int * 100;
    if (dec % 10 > 5) dec+= 10;
    UnsafeItoa(dec / 10, 1, &line1[x]);
#endif
  }

  // Calculate standard deviation
  double std = 0.0; 
  for (uint8_t n = 0; n < kBufferSize; n++) {
    double dev = (double)bpm_buffer[n] - ave;
    std+= dev * dev;
  }

  std = sqrt(std / kBufferSize);

  double stddev = Round(Scale(std, 0, ave, 0, 100000));

  // Maintain maximum standard deviation

  if (display_max_stddev && (uint16_t)(stddev / 1000.0) < 100) {
    if (stddev > max_stddev) {
      max_stddev = stddev;
    } else {
      stddev = max_stddev;
    }
  }

  // Display standard deviation
  { uint8_t x = 1; uint16_t stddev_int = stddev / 1000;
    if (stddev_int > 100) {
      memcpy_P(&line2[x], PSTRN("> 100%"));
    } else {
      x+= UnsafeItoaLen(stddev_int, 3, &line2[x]); line2[x++] = '.';
      ui.PrintNNN(&line2[x], stddev - stddev_int * 1000); line2[x + 3] = '%';
    }
    if (display_max_stddev) {
      line2[0] = '<';
    }
  }
}

/* static */
void ClockTesterPage::UpdateLeds() {
}

/* static */
void ClockTesterPage::MidiInCallbackHandler(uint8_t byte) {
  // This routine is called with interrupts disabled!
  midi_out.SendNow(byte);

  switch (byte) {
    case 0xf8:  // clock
      // Save reference timer and overflow count state in the circular buffer and reset it
      tick_buffer[next_buffer_slot].overflow_count = overflow_count; overflow_count = 0;
      tick_buffer[next_buffer_slot].tick_count = TCNT1; TCNT1 = 0;

      // Maintain circular buffer 
      if (++next_buffer_slot == numbof(tick_buffer)) {
        tick_buffer_ready = 1;
        next_buffer_slot = 0; 
      }
      break;

/* Starting with 0.93 stats are not reset on Start/Continue

    case 0xfa:  // start
    case 0xfb:  // continue
      TCNT1 = 0;
      overflow_count = 0;
      max_stddev = 0.0;
      break;
    case 0xfc:  // stop
      break;*/
  }
}

/* global */
ISR(TIMER1_OVF_vect) {
  // 20MHz/65535 = 305.1757813Hz
  ++overflow_count;
}

}  // namespace midiclk
