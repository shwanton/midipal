// Copyright 2012 Peter Kvitek.
//
// Author: Peter Kvitek (pete@kvitek.com)
//
// Based on Ambika code by Olivier Gillet (ol.gillet@gmail.com)
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
// Base UI page class.

#include "midiclk/ui_pages/ui_page.h"

#include <math.h>

namespace midiclk {

/* static */
uint8_t UiPage::changing_page_;
uint8_t UiPage::reset_counter_;

/* static */
const prog_EventHandlers UiPage::event_handlers_ PROGMEM = {
  OnInit,
  OnQuit,
  OnIncrement,
  OnClick,
  OnSwitch,
  OnIdle,
  UpdateScreen,
  UpdateLeds,
};

/* static */
void UiPage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
}

/* static */
void UiPage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t UiPage::OnIncrement(uint8_t id, int8_t value) {
  return 0;
}

/* static */
uint8_t UiPage::OnClick(uint8_t id, uint8_t value) {
  return 0;
}

/* static */
uint8_t UiPage::OnSwitch(uint8_t id, uint8_t value) {
  return 0;
}

/* static */
uint8_t UiPage::OnIdle() {
  return 0;
}

/* static */
void UiPage::UpdateScreen() {
}

/* static */
void UiPage::UpdateLeds() {
}

///////////////////////////////////////////////////////////////////////////////
// Miscellaneous helpers

/* static */
uint8_t UiPage::CheckIdleShiftAction() {
  if (ui.GetShifted()) {
    if (++reset_counter_ >= 2) {  // @1.225Hz
      reset_counter_ = 0;
      ui.set_inhibit_shift_raised();
      return 1;
    }
  } else
    reset_counter_ = 0;

  return 0;
}

/* static */
void UiPage::DrawSelStr2(char* buffer, uint8_t sel, const prog_char* strings) {
  memcpy_P(buffer, &strings[sel << 1], 2);
}

/* static */
void UiPage::DrawSelStr4(char* buffer, uint8_t sel, const prog_char* strings) {
  memcpy_P(buffer, &strings[sel << 2], 4);
}

/* static */
void UiPage::DrawSelStrN(char* buffer, uint8_t sel, const prog_char* strings, uint8_t width) {
  memcpy_P(buffer, &strings[sel * width], width);
}

/* static */
void UiPage::DrawBrackets(char* buffer, uint8_t max_width) {
  *buffer++ = '[';
  for (uint8_t n = 0; n < max_width; n++, buffer++) {
    if (buffer[1] == ' ') {
      buffer[1] = ']';
      break;
    }
  }
}

/* static */
uint8_t UiPage::Clamp(int16_t value, uint8_t min, uint8_t max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return static_cast<uint8_t>(value);
}

/* static */
int16_t UiPage::Clamp16(int16_t value, int16_t min, int16_t max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

/* static */
int16_t UiPage::Round(double x) {
  double ret, dx = fabs(x - (int)x);

  if (x < 0) {
    ret = dx < 0.5 ? ceil(x) : floor(x);
  } else {
    ret = dx > 0.5 ? ceil(x) : floor(x);
  }

  return ret;
}

/* static */
double UiPage::Scale(double x0, double x0min, double x0max, double x1min, double x1max) {
  if (x0max == x0min) 
    return x1max;

  double x1 = x1min + ((x0 - x0min) * (x1max - x1min)) / (x0max - x0min);

  if (x1 < x1min) {
    x1 = x1min;
  } else
  if (x1 > x1max) {
    x1 = x1max;
  }

  return x1;
}

}  // namespace midiclk
