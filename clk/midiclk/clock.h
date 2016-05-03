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

#ifndef MIDICLK_CLOCK_H_
#define MIDICLK_CLOCK_H_

#include "avrlib/base.h"

namespace midiclk {

class Clock {
 public:
  static inline void Init() {
    Update(120);
    running_ = false;
  }
  static inline void Reset() {
    clock_ = 0;
  }
  
  static inline void Start() {
    Reset();
    running_ = true;
  }
  
  static inline void Stop() {
    running_ = false;
  }
  
  static inline bool running() {
    return running_;
  }

  static inline uint16_t Tick() {
    clock_ += interval_ + 1;
    return interval_;
  }
  
  static uint16_t interval() {
    return interval_;
  }
  
  static uint32_t value() {
    return clock_;
  }
  
  static void Update(uint16_t bpm) {
    Update(bpm, 0);
  }
  
  static void Update(uint16_t bpm, uint8_t bpm_10th);

 private:
  static bool running_;
  static uint32_t clock_;  // Counts forever
  static uint16_t interval_;
};

extern Clock clock;

}  // namespace midiclk

#endif // MIDICLK_CLOCK_H_
