// Copyright 2015 Peter Kvitek.
//
// Author: Peter Kvitek (pete@kvitek.com)
//
// Based on code by: Olivier Gillet (ol.gillet@gmail.com)
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
// Global clock.

#include "midiclk/clock.h"

#include "midiclk/resources.h"

namespace midiclk {

Clock clock;

/* <static> */
bool Clock::running_;
uint32_t Clock::clock_;
uint16_t Clock::interval_;
/* </static> */

/* static */
void Clock::Update(uint16_t bpm, uint8_t bpm_tenth) {
  interval_ = 7812500 / (static_cast<uint32_t>(bpm) * 10 + bpm_tenth) - 1;
}

}  // namespace midiclk
