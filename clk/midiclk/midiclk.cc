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

#include "avrlib/gpio.h"
#include "avrlib/boot.h"
#include "avrlib/time.h"
#include "avrlib/timer.h"
#include "avrlib/serial.h"
#include "avrlib/watchdog_timer.h"

#include "midiclk/midi_out.h"
#include "midiclk/midi_in.h"
#include "midiclk/display.h"
#include "midiclk/eemem.h"
#include "midiclk/sync.h"
#include "midiclk/leds.h"
#include "midiclk/ui.h"

#include <util/delay.h>

#include <math.h>

using namespace avrlib;
using namespace midiclk;

// Sysetem tick timer

ISR(TIMER2_OVF_vect, ISR_NOBLOCK) {
  // 4.9KHz

  sync.Tick();

  // Handle lower priority tasks
  static uint8_t sub_clock;
  ++sub_clock;
  if ((sub_clock & 1) == 0) {
    // 2.45KHz
    ui.Poll();
    if ((sub_clock & 3) == 0) {
      // 1.225KHz
      TickSystemClock();
      leds.Tick();
      if ((sub_clock & 7) == 0) {
        // 306Hz
        display.BlinkCursor();
      }
    }
  }
}

void Init() {
  sei();
  UCSR0B = 0;

  // System tick timer @4.9kHz
  Timer<2>::set_prescaler(2);
  Timer<2>::set_mode(TIMER_PWM_PHASE_CORRECT);
  Timer<2>::Start();

  display.Init();
  eemem.Init();
  leds.Init();
  sync.Init();
  ui.Init();

  midi_out.Init();
  midi_in.Init();
}

int main(void) {
  ResetWatchdog();
  Init();

  while (1) {
    ui.DoEvents();
  }
}
