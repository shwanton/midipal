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
// Hardware configuration.

#ifndef MIDICLK_HARDWARE_CONFIG_H_
#define MIDICLK_HARDWARE_CONFIG_H_

#include "avrlib/base.h"
#include "avrlib/gpio.h"
#include "avrlib/serial.h"
#include "avrlib/parallel_io.h"
#include "avrlib/devices/switch.h"
#include "avrlib/devices/rotary_encoder.h"
#include "avrlib/devices/external_eeprom.h"

namespace midiclk {

using avrlib::Gpio;
using avrlib::LSB_FIRST;
using avrlib::MSB_FIRST;
using avrlib::PARALLEL_NIBBLE_HIGH;
using avrlib::PARALLEL_NIBBLE_LOW;
using avrlib::DebouncedSwitch;
using avrlib::RotaryEncoder;
using avrlib::ParallelPort;
using avrlib::PortB;
using avrlib::PortC;
using avrlib::PortD;
using avrlib::SerialPort0;
using avrlib::ExternalEeprom;
using avrlib::I2cMaster;

// MIDI
typedef SerialPort0 MidiPort;

// LCD
typedef Gpio<PortD, 3> LcdEnLine;
typedef Gpio<PortD, 2> LcdRsLine;
typedef ParallelPort<PortD, PARALLEL_NIBBLE_HIGH> LcdData;

static const uint8_t kLcdWidth = 8;
static const uint8_t kLcdHeight = 2;

// Encoder
typedef Gpio<PortB, 2> Encoder_A;
typedef Gpio<PortB, 1> Encoder_B;
typedef Gpio<PortB, 0> Encoder_C;

typedef RotaryEncoder<Encoder_A, Encoder_B, Encoder_C> Encoder;

// Switches
typedef DebouncedSwitch< Gpio<PortB, 4> > Switch;

// LEDs
typedef Gpio<PortB, 5> MidiInLed;
typedef Gpio<PortB, 3> MidiOutLed;

// Sync
typedef ParallelPort<PortC, PARALLEL_NIBBLE_LOW> SyncPorts;

// EEPROM
static const uint16_t kBankSize = 8192;
static const uint16_t kMaxNumBanks = 8;

typedef ExternalEeprom<kMaxNumBanks * kBankSize, I2cMaster<8, 64, 400000> > Eeprom;

}  // namespace midiclk

#endif  // MIDICLK_HARDWARE_CONFIG_H_
