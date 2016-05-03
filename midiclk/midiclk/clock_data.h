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

#ifndef MIDICLK_CLOCK_DATA_H_
#define MIDICLK_CLOCK_DATA_H_

#include "midiclk/midiclk.h"

namespace midiclk {

template<uint8_t size = 4 * 24>
class ClockData {
 public:
  ClockData() { Init(); }
  
  void Init() {
    next_ = 0;
    full_ = 0;
    sum_ = 0;
  }

  void Add(uint16_t data) {
    if (full_) {
      sum_-= buf_[next_];
    }

    buf_[next_] = data;
    sum_+= data;

    if (++next_ >= numbof(buf_)) {
      full_ = 1;
      next_ = 0; 
    }
  }

  uint8_t capacity() const { return numbof(buf_); }

  uint8_t full() const { return full_; }

  uint32_t sum() const { return sum_; }

  uint16_t data(uint8_t index) const { return buf_[index]; }

  uint16_t average() const { return sum_ / numbof(buf_); }

  uint16_t max_deviation() const {
    uint16_t max_deviation = 0; uint16_t median = average();
    for (uint8_t n = 0; n < numbof(buf_); n++) {
      uint16_t deviation = buf_[n] > median ? buf_[n] - median : median - buf_[n];
      if (deviation > max_deviation) {
        max_deviation = deviation;
      }
    }
    return max_deviation;
  }

 private:

  uint32_t sum_;
  uint16_t buf_[size];
  uint8_t next_;
  uint8_t full_;
};

}  // namespace midiclk

#endif  // MIDICLK_CLOCK_DATA_H_
