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
// Hold mode page class.

#include "midiclk/ui_pages/hold_mode_page.h"
#include "midiclk/ui_pages/clock_generator_page.h"

namespace midiclk {

/* static */
const prog_EventHandlers HoldModePage::event_handlers_ PROGMEM = {
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
void HoldModePage::OnInit(PageInfo* pageInfo, UiPageIndex prevPage) {
}

/* static */
void HoldModePage::OnQuit(UiPageIndex nextPage) {
}

/* static */
uint8_t HoldModePage::OnIncrement(uint8_t id, int8_t value) {
  if (!changing_page())
    return 0;

  ClockGeneratorPage::SetHoldMode(Clamp(static_cast<int16_t>(ClockGeneratorPage::hold_mode()) + value, kMinHoldMode, kMaxHoldMode));

  return 1;
}

/* static */
uint8_t HoldModePage::OnClick(uint8_t id, uint8_t value) {
  if (value == CLICK) {
    ToggleChangingPage();
    return 1;
  }

  return 0;
}

/* static */
uint8_t HoldModePage::OnSwitch(uint8_t id, uint8_t value) {
  return ClockGeneratorPage::OnSwitch(id, value);
}

/* static */
uint8_t HoldModePage::OnIdle() {
  return 0;
}

/* static */
void HoldModePage::UpdateScreen() {
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);
  memcpy_P(&line1[0], PSTRN("HoldMode"));

  DrawSelStrN(&line2[1], ClockGeneratorPage::hold_mode(), PSTR("none  start strstp"), 6);

  if (changing_page()) {
      DrawBrackets(&line2[0], 6);
  }
}

/* static */
void HoldModePage::UpdateLeds() {
}

}  // namespace midiclk
