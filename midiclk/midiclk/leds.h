// Copyright 2013 Peter Kvitek.
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

#ifndef MIDICLK_LEDS_H_
#define MIDICLK_LEDS_H_

#include "avrlib/devices/led.h"

namespace midiclk {
  
class Leds {
 public:
  Leds() {}
  static void Init();
  static void Tick();

  static void FlashMidiIn();
  static void FlashMidiOut();

 private:

  DISALLOW_COPY_AND_ASSIGN(Leds);
};

extern Leds leds;

}  // namespace midiclk

#endif  // MIDICLK_LEDS_H_
