// Copyright 2011 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
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
// Ear Training Game plug-in.

#ifndef MIDIPAL_EAR_TRAINING_GAME_H_
#define MIDIPAL_EAR_TRAINING_GAME_H_

#include "midipal/plugin.h"

namespace midipal { namespace plugins {

class EarTrainingGame : public PlugIn {
 public:
  EarTrainingGame() { }

  virtual void OnLoad();
  virtual void OnRawMidiData(
     uint8_t status,
     uint8_t* data,
     uint8_t data_size,
     uint8_t accepted_channel);
  void OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
  
  virtual void SetParameter(uint8_t key, uint8_t value);
  virtual uint8_t GetParameter(uint8_t key);
  virtual void OnInternalClockStep();
  virtual uint8_t OnRedraw();
  virtual uint8_t OnClick();
  virtual uint8_t OnIncrement(int8_t value);
  
 private:
  void GenerateChallenge();
  void StartChallenge();
  
  uint8_t step_counter_;
  uint8_t played_notes_[8];
  uint8_t recorded_notes_[8];
  uint8_t record_ptr_;
  uint8_t play_ptr_;
  uint8_t wait_;
  uint8_t attempts_;
  uint8_t new_challenge_;
  uint8_t show_score_;

  uint8_t level_;
  uint8_t num_notes_;
  uint8_t confirm_reset_;
  uint16_t num_games_;
  uint16_t num_attempts_;
  uint8_t seeded_;
  
  DISALLOW_COPY_AND_ASSIGN(EarTrainingGame);
};

} }  // namespace midipal::plugins

#endif // MIDIPAL_EAR_TRAINING_GAME_H_
