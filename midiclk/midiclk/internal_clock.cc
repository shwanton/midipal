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

#include "midiclk/internal_clock.h"

namespace midiclk {

using namespace avrlib;

/* extern */
InternalClock internal_clock;

/* static */
static InternalClockCallback _callback;

/* static */
void InternalClock::Init() {
  clock.Init();
  Timer<1>::set_mode(0, _BV(WGM12), 3);
  PwmChannel1A::set_frequency(clock.Tick());
}

/* static */
void InternalClock::Start() {
  Timer<1>::StartCompare();
}

/* static */
void InternalClock::Stop() {
  Timer<1>::StopCompare();
}

/* static */
InternalClockCallback InternalClock::set_callback(InternalClockCallback callback) {
  DisableInterrupts disableInterrupts;
  InternalClockCallback prev = _callback;
  _callback = callback;
  return prev;
}

/* global */
ISR(TIMER1_COMPA_vect) {
  PwmChannel1A::set_frequency(clock.Tick());
  if (clock.running()) {
    if (_callback) {
      _callback();
    }
  }
}

}  // namespace midiclk                           `
