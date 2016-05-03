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

#include "midiclk/midi_in.h"
#include "midiclk/leds.h"

namespace midiclk {

using namespace avrlib;

/* extern */
MidiIn midi_in;

/* static */
static MidiInCallback _callback;

/* static */
void MidiIn::Init() {
  // USART initialization
  // Communication Parameters: 8 Data, 1 Stop, No Parity
  // USART Receiver: On
  // USART Transmitter: Off
  // USART Mode: Asynchronous
  // USART Baud Rate: 31250
  UCSR0A = 0;
  UCSR0B = (1 << RXEN0)  | (1 << RXCIE0) | (1 << TXEN0); // Enable RX interrupt and TX
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 data bits, no parity, 1 stop bit
  UBRR0H = 0;
  UBRR0L = (F_CPU / (31250 * 16LU)) - 1;
}

/* static */
MidiInCallback MidiIn::set_callback(MidiInCallback callback) {
  DisableInterrupts disableInterrupts;
  MidiInCallback prev = _callback;
  _callback = callback;
  return prev;
}

/* global */
ISR(USART_RX_vect) {
  // Check for errors
  if ((UCSR0A & ((1 << FE0) | (1 << UPE0) | (1 << DOR0))) != 0) {
    // receive error, ignore...
  } else {
    leds.FlashMidiIn();
    uint8_t byte = UDR0;
    if (_callback) {
      _callback(byte);
    }
  }
}

}  // namespace midiclk                           `
