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

#ifndef MIDICLK_MIDICLK_H_
#define MIDICLK_MIDICLK_H_

#include "avrlib/base.h"

#include <string.h>

#ifdef ENABLE_DEBUG_OUTPUT
#include <stdlib.h>
#include <stdio.h>
#endif

#include "midiclk/hardware_config.h"
#include "midiclk/resources.h"
#include "midiclk/settings.h"
#include "midiclk/duration.h"

namespace midiclk {

#define MIDICLK_VERSION "0.97"

// Configuration

static const uint8_t kDefDeviceMode = DEVICEMODE_CLOCK_TESTER;

static const uint8_t kDefDisplayMaxStdDev = 0;

static const uint16_t kMinTempo = 250;   // bmp x 10
static const uint16_t kMaxTempo = 2500;  // -- // --
static const uint16_t kDefTempo = 1200;  // -- // --

static const uint8_t kDefHoldMode = HOLDMODE_NONE;
static const uint8_t kMinHoldMode = 0;
static const uint8_t kMaxHoldMode = kHoldModeCount - 1;

static const uint8_t kDefStepLeng = k4thNote;
static const uint8_t kMinStepLeng = 0;
static const uint8_t kMaxStepLeng = kNoteDurationCount - 1;

static const uint8_t kDefStepNumb = 4;
static const uint8_t kMinStepNumb = 1;
static const uint8_t kMaxStepNumb = 96;

static const uint8_t kDefPassThru = 1;

static const uint8_t kDefRoundBpm = 0;

static const uint8_t kDefThreshold = 7;
static const uint8_t kMinThreshold = 0;
static const uint8_t kMaxThreshold = 25;

static const uint8_t kDefStrobeWidth = 0;
static const uint8_t kMinStrobeWidth = 0;
static const uint8_t kMaxStrobeWidth = 4;

// Useful declarations

#define numbof(a)  (sizeof(a)/sizeof(a[0]))
#define lengof(s)  (numbof(s) - 1)

#define max(a,b)  (((a) > (b)) ? (a) : (b))
#define min(a,b)  (((a) < (b)) ? (a) : (b))

#define SETFLAG(f,b)  (f)|= (b)
#define CLRFLAG(f,b)  (f)&=~(b)
#define SETFLAGTO(f,b,c)  if (c) SETFLAG(f,b); else CLRFLAG(f,b);

#define PSTRN(str) PSTR(str), lengof(str)

#ifdef _MSC_VER
#define OFFSETOF(s,m) (uint16_t)offsetof(s,m)
#else
#define OFFSETOF(s,m) (uint16_t)&reinterpret_cast<const volatile char&>((((s *)0)->m))
#endif

#ifndef _MSC_VER
#define assert(expr)  (void)0
#endif

// 50ns @20MHz
static inline void nop() { 
  __asm__ volatile (
    "nop"
    ); 
}

// count*160ns @20MHz, 0=38us
static inline void Delay(uint8_t count) {
  __asm__ volatile (
    "1: dec %0" "\n\t"
    "brne 1b"
    : "=r" (count)
    : "0" (count)
  );
}

// Disable interrupts
class DisableInterrupts {
  uint8_t sreg_;
 public:
   DisableInterrupts() { sreg_= SREG; }
   ~DisableInterrupts() { SREG = sreg_; }
};

// Profiling aids
struct RaiseDebugPort {
  typedef Gpio<PortC, 0> DebugPort;
  RaiseDebugPort() { DebugPort::High(); }
  RaiseDebugPort(uint8_t count) { DebugPort::High(); Delay(count); }
  ~RaiseDebugPort() { DebugPort::Low(); }
};

}  // namespace midiclk

#endif  // MIDICLK_MIDICLK_H_
