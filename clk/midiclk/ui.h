// Copyright 2015 Peter Kvitek.
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

#ifndef MIDICLK_UI_H_
#define MIDICLK_UI_H_

#include "avrlib/base.h"
#include "avrlib/ui/event_queue.h"

#include "midiclk/midiclk.h"

namespace midiclk {

using namespace avrlib;

enum EncoderClickValue {
  CLICK,
  CLICK_SHIFTED,
  DBLCLICK,
  DBLCLICK_SHIFTED,
};

enum ControlId {
  ENCODER,
  SWITCH,
  SWITCH_SHIFTED,
  SWITCH_RAISED,
  SWITCH_RAISED_SHIFTED,
};

enum ControlTypeEx {
  CONTROL_REQUEST = 10,
};

enum RequestControlId {
  REQUEST_SHOWPAGE = 1,     // event.value specifies page id
  REQUEST_UPDATESCREEN, 
};

enum UiPageIndex {
  PAGE_CLOCK_TESTER,
  PAGE_CLOCK_GENERATOR,
  PAGE_STEP_LENG,
  PAGE_STEP_NUMB,
  PAGE_HOLD_MODE,
  PAGE_ROUND_BPM,
  PAGE_THRESHOLD,
  PAGE_PASS_THRU,
  PAGE_STROBE_WIDTH,
  FIRST_PAGE = PAGE_CLOCK_GENERATOR,
  LAST_PAGE = PAGE_STROBE_WIDTH,

  PAGE_DLG,
  NO_PAGE = -1
};

struct PageInfo;

struct EventHandlers {
  void (*OnInit)(PageInfo* pageInfo, UiPageIndex prevPage);
  void (*OnQuit)(UiPageIndex nextPage);
  uint8_t (*OnIncrement)(uint8_t, int8_t);
  uint8_t (*OnClick)(uint8_t, uint8_t);
  uint8_t (*OnSwitch)(uint8_t, uint8_t);
  uint8_t (*OnIdle)();
  void (*UpdateScreen)();
  void (*UpdateLeds)();
};

typedef EventHandlers PROGMEM prog_EventHandlers;

static const uint8_t kNoDlgResult = 0xff;

struct DlgData {
  const prog_char* text;
  const prog_char* buttons;
  uint8_t num_buttons;
  uint8_t result;
};

class Dialog {
 public:
  Dialog(const prog_char* text, const prog_char* buttons, uint8_t num_buttons);

  uint8_t result() { return data_.result; }

 protected:
  DlgData data_;
};

struct PageInfo {
  uint8_t index;
  const prog_EventHandlers* event_handlers;
  union {
    uint8_t data[1];
    DlgData dlg_data;
  };
};

typedef PageInfo PROGMEM prog_PageInfo;

class Ui {
 public:
  static void Init();
  static void Logo();
  static void Poll();
  static void DoEvents();
  
  static void ShowPage(UiPageIndex page);
  static void ShowLastPage() { ShowPage(last_page_); }
  static void ShowDialog(DlgData& dlg_data);

  static UiPageIndex active_page() { return active_page_; }
  static UiPageIndex last_page() { return last_page_; }

  static void Clear();

  static void UpdateScreen();
  static void RedrawScreen();

  static void AddEvent(uint8_t control_type, uint8_t id = 0, uint8_t data = 0) {
    queue_.AddEvent(control_type, id, data);
  }

  static void RequestRefresh() { 
    AddEvent(CONTROL_REFRESH, 0, 0);
  }

  static void AddRequest(uint8_t id = 0, uint8_t value = 0) { 
    AddEvent(CONTROL_REQUEST, id, value);
  }

  static void PrintChannel(char* buffer, uint8_t channel);
  static void PrintHex(char* buffer, uint8_t value);
  static void PrintNote(char* buffer, uint8_t note);
  static void PrintNN(char* buffer, uint16_t numb, char filler = '0');
  static void PrintNNN(char* buffer, uint16_t numb, char filler = '0');

  static uint8_t GetShifted() { 
    return switch_.low() ? 1 : 0; 
  }

  static uint8_t GetEncoderLow() {
    return encoder_.low();
  }

  static int16_t Scale(int16_t x0, int16_t x0min, int16_t x0max, int16_t x1min, int16_t x1max);

  // Accessors

  static uint8_t request_redraw() { return request_redraw_; }
  static void set_request_redraw(uint8_t request_redraw = 1) { 
    request_redraw_ = request_redraw;
  }
  
  static uint8_t inhibit_shift_raised() { return inhibit_shift_raised_; }
  static void set_inhibit_shift_raised(uint8_t inhibit = 1) { 
    inhibit_shift_raised_ = inhibit;
  }
  
  // Accessors

 private:
  static Encoder encoder_;
  static Switch switch_;

  static avrlib::EventQueue<32> queue_;

  static UiPageIndex active_page_;
  static UiPageIndex last_page_;
  static EventHandlers event_handlers_;
  static PageInfo page_info_;

  static uint8_t cycle_;
  static uint8_t request_redraw_;
  static uint8_t inhibit_shift_raised_;
  static uint32_t last_click_time_;

  static void PollEncoder();
  static void PollSwitches();

  static void InitDeviceMode();

  static void HandleEvent(const Event& e);
  static void HandleEncoderEvent(const Event& e);
  static void HandleEncoderClickEvent(const Event& e);
  static void HandleSwitchEvent(const Event& e);
  static void HandleRequestEvent(const Event& e);
};

extern Ui ui;

}  // namespace midiclk

#endif // MIDICLK_UI_H_
