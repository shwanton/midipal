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

#ifndef MIDICLK_SYNC_H_
#define MIDICLK_SYNC_H_

#include "midiclk/midiclk.h"

namespace midiclk {

static const uint8_t kNumSyncPorts = 4;

enum SyncIndex {
  SYNC_24,
  SYNC_STEP,
  SYNC_STEP_COUNT,
  SYNC_START,
};

class Sync {
 public:
  Sync() {}
  static void Init();
  static void Tick();

  static void OnClock() {
    SetStrobe(SYNC_24);
  }

  static void OnStep() {
    SetStrobe(SYNC_STEP);
  }

  static void OnStepCount() {
    SetStrobe(SYNC_STEP_COUNT);
  }

  static void OnStart() {
    SetGate(SYNC_START, 1);
  }

  static void OnContinue() {
    SetGate(SYNC_START, 1);
  }

  static void OnStop() {
    SetGate(SYNC_START, 0);
  }

  static void SetGate(uint8_t index, uint8_t value);
  static void SetStrobe(uint8_t index);
  
  static void SetStrobeWidth(uint8_t width);
  static uint8_t GetStrobeWidth();

  static uint8_t get() { return value_; }
  static void set(uint8_t value) { value_ = value; }

 private:

  static uint8_t value_;
  static uint8_t latched_value_;
  static uint8_t strobe_count_[kNumSyncPorts];
  static uint8_t strobe_ticks_;
  static uint8_t strobe_mask_;
  static uint8_t strobe_inv_;

  static SyncPorts ports_;

  static void Write(uint8_t value) {
    ports_.Write(value ^ strobe_inv_);
    latched_value_ = value_;
  }

  DISALLOW_COPY_AND_ASSIGN(Sync);
};

extern Sync sync;

}  // namespace midiclk

#endif  // MIDICLK_SYNC_H_
