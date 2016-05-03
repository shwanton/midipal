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
// Onboard EEPROM interface.

#ifndef MIDICLK_EEMEM_H_
#define MIDICLK_EEMEM_H_

#include "midiclk/midiclk.h"

namespace midiclk {

class Eemem {
 public:
  Eemem() {}
  static void Init();
  static void Reset();

  static uint8_t device_mode();
  static void set_device_mode(uint8_t device_mode);

  static uint8_t display_max_stddev();
  static void set_display_max_stddev(uint8_t display_max_stddev);

  static uint16_t tempo();
  static void set_tempo(uint16_t tempo);

  static uint8_t step_leng();
  static void set_step_leng(uint8_t step_leng);

  static uint8_t step_numb();
  static void set_step_numb(uint8_t step_numb);

  static uint8_t hold_mode();
  static void set_hold_mode(uint8_t hold_mode);

  static uint8_t pass_thru();
  static void set_pass_thru(uint8_t pass_thru);

  static uint8_t round_bpm();
  static void set_round_bpm(uint8_t round_bpm);

  static uint8_t threshold();
  static void set_threshold(uint8_t threshold);

  static uint8_t strobe_width();
  static void set_strobe_width(uint8_t strobe_width);

 private:

  DISALLOW_COPY_AND_ASSIGN(Eemem);
};

extern Eemem eemem;

}  // namespace midiclk

#endif // MIDICLK_EEMEM_H_
