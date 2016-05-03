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

#include "midiclk/eemem.h"

#include <avr/eeprom.h>

namespace midiclk {

using namespace avrlib;

/* extern */
Eemem eemem;

static const uint16_t kEememMagic = 0xdef0;

struct EememData {
  uint16_t magic_;
  uint16_t size_;
  uint8_t device_mode_;
  uint8_t display_max_stddev_;
  uint16_t tempo_;
  uint8_t step_leng_;
  uint8_t step_numb_;
  uint8_t hold_mode_;
  uint8_t pass_thru_;
  uint8_t round_bpm_;
  uint8_t threshold_;
  uint8_t strobe_width_;
};

EememData EEMEM eememData;

/* static */
void Eemem::Init() {
  // Check if already initialized
  uint16_t initialized_size = 0;
  if (eeprom_read_word(&eememData.magic_) == kEememMagic) {
    initialized_size = eeprom_read_word(&eememData.size_);
    if (initialized_size == sizeof(EememData))
      return;
  }

  #define FIXSIZE(field) \
    if (initialized_size > sizeof(field)) initialized_size-= sizeof(field); \
    else initialized_size = 0;

  // Init initialization controls
  eeprom_update_word(&eememData.magic_, kEememMagic); FIXSIZE(eememData.magic_);
  eeprom_update_word(&eememData.size_, sizeof(EememData)); FIXSIZE(eememData.size_);

  // Init device mode
  if (initialized_size < sizeof(eememData.device_mode_)) {
    set_device_mode(kDefDeviceMode);
  } FIXSIZE(eememData.device_mode_);

  // Init display max stddev
  if (initialized_size < sizeof(eememData.display_max_stddev_)) {
    set_display_max_stddev(kDefDisplayMaxStdDev);
  } FIXSIZE(eememData.display_max_stddev_);

  // Init tempo
  if (initialized_size < sizeof(eememData.tempo_)) {
    set_tempo(kDefTempo);
  } FIXSIZE(eememData.tempo_);

  // Init step_leng
  if (initialized_size < sizeof(eememData.step_leng_)) {
    set_step_leng(kDefStepLeng);
  } FIXSIZE(eememData.step_leng_);

  // Init step_numb
  if (initialized_size < sizeof(eememData.step_numb_)) {
    set_step_numb(kDefStepNumb);
  } FIXSIZE(eememData.step_numb_);

  // Init hold_mode
  if (initialized_size < sizeof(eememData.hold_mode_)) {
    set_hold_mode(kDefHoldMode);
  } FIXSIZE(eememData.hold_mode_);

  // Init pass_thru
  if (initialized_size < sizeof(eememData.pass_thru_)) {
    set_pass_thru(kDefPassThru);
  } FIXSIZE(eememData.pass_thru_);

  // Init round_bpm
  if (initialized_size < sizeof(eememData.round_bpm_)) {
    set_round_bpm(kDefRoundBpm);
  } FIXSIZE(eememData.round_bpm_);

  // Init threshold
  if (initialized_size < sizeof(eememData.threshold_)) {
    set_threshold(kDefThreshold);
  } FIXSIZE(eememData.threshold_);

  // Init strobe_width
  if (initialized_size < sizeof(eememData.strobe_width_)) {
    set_strobe_width(kDefStrobeWidth);
  } FIXSIZE(eememData.strobe_width_);

  #undef FIXSIZE
}

/* static */
void Eemem::Reset() {
  eeprom_update_word(&eememData.magic_, ~kEememMagic);
  Init();
}

/* static */
uint8_t Eemem::device_mode() {
  uint8_t device_mode = eeprom_read_byte(&eememData.device_mode_);
  if (device_mode >= kDeviceModeCount) device_mode = kDefDeviceMode;
  return device_mode;
}

/* static */
void Eemem::set_device_mode(uint8_t device_mode) {
  eeprom_update_byte(&eememData.device_mode_, device_mode);
}

/* static */
uint8_t Eemem::display_max_stddev() {
  return eeprom_read_byte(&eememData.display_max_stddev_);
}

/* static */
void Eemem::set_display_max_stddev(uint8_t display_max_stddev) {
  eeprom_update_byte(&eememData.display_max_stddev_, display_max_stddev);
}

/* static */
uint16_t Eemem::tempo() {
  uint16_t tempo = eeprom_read_word(&eememData.tempo_);
  if (tempo < kMinTempo || tempo > kMaxTempo) tempo = kDefTempo;
  return tempo;
}

/* static */
void Eemem::set_tempo(uint16_t tempo) {
  eeprom_update_word(&eememData.tempo_, tempo);
}

/* static */
uint8_t Eemem::step_leng() {
  uint8_t step = eeprom_read_byte(&eememData.step_leng_);
  if (step < kMinStepLeng || step > kMaxStepLeng) step = kDefStepLeng;
  return step;
}

/* static */
void Eemem::set_step_leng(uint8_t step_leng) {
  eeprom_update_byte(&eememData.step_leng_, step_leng);
}

/* static */
uint8_t Eemem::step_numb() {
  uint8_t numb = eeprom_read_byte(&eememData.step_numb_);
  if (numb < kMinStepNumb || numb > kMaxStepNumb) numb = kDefStepNumb;
  return numb;
}

/* static */
void Eemem::set_step_numb(uint8_t step_numb) {
  eeprom_update_byte(&eememData.step_numb_, step_numb);
}

/* static */
uint8_t Eemem::hold_mode() {
  uint8_t mode = eeprom_read_byte(&eememData.hold_mode_);
  if (mode < kMinHoldMode || mode > kMaxHoldMode) mode = kDefHoldMode;
  return mode;
}

/* static */
void Eemem::set_hold_mode(uint8_t hold_mode) {
  eeprom_update_byte(&eememData.hold_mode_, hold_mode);
}

/* static */
uint8_t Eemem::pass_thru() {
  uint8_t pass = eeprom_read_byte(&eememData.pass_thru_);
  if (pass > 1) pass = kDefPassThru;
  return pass;
}

/* static */
void Eemem::set_pass_thru(uint8_t pass_thru) {
  eeprom_update_byte(&eememData.pass_thru_, pass_thru);
}

/* static */
uint8_t Eemem::round_bpm() {
  uint8_t round = eeprom_read_byte(&eememData.round_bpm_);
  if (round > 1) round = kDefRoundBpm;
  return round;
}

/* static */
void Eemem::set_round_bpm(uint8_t round_bpm) {
  eeprom_update_byte(&eememData.round_bpm_, round_bpm);
}

/* static */
uint8_t Eemem::threshold() {
  uint8_t threshold = eeprom_read_byte(&eememData.threshold_);
  if (threshold < kMinThreshold || threshold > kMaxThreshold) threshold = kDefThreshold;
  return threshold;
}

/* static */
void Eemem::set_threshold(uint8_t threshold) {
  eeprom_update_byte(&eememData.threshold_, threshold);
}

/* static */
uint8_t Eemem::strobe_width() {
  uint8_t strobe_width = eeprom_read_byte(&eememData.strobe_width_);
  if (strobe_width < kMinStrobeWidth || strobe_width > kMaxStrobeWidth) strobe_width = kDefStrobeWidth;
  return strobe_width;
}

/* static */
void Eemem::set_strobe_width(uint8_t strobe_width) {
  eeprom_update_byte(&eememData.strobe_width_, strobe_width);
}

}  // namespace midiclk

