// Copyright 2013 Peter Kvitek.
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

#include "midiclk/ui.h"

#include "avrlib/op.h"
#include "avrlib/string.h"

#include "midiclk/resources.h"
#include "midiclk/display.h"
#include "midiclk/eemem.h"
#include "midiclk/leds.h"

#include "midiclk/ui_pages/clock_tester_page.h"
#include "midiclk/ui_pages/clock_generator_page.h"
#include "midiclk/ui_pages/step_leng_page.h"
#include "midiclk/ui_pages/step_numb_page.h"
#include "midiclk/ui_pages/hold_mode_page.h"
#include "midiclk/ui_pages/round_bpm_page.h"
#include "midiclk/ui_pages/threshold_page.h"
#include "midiclk/ui_pages/pass_thru_page.h"
#include "midiclk/ui_pages/strobe_width_page.h"
#include "midiclk/ui_pages/dlg_page.h"

namespace midiclk {

using namespace avrlib;

static const uint8_t kDblClickTime = 250;

/* <static> */
Encoder Ui::encoder_;

Switch Ui::switch_;

EventQueue<32> Ui::queue_;

UiPageIndex Ui::active_page_ = NO_PAGE;
UiPageIndex Ui::last_page_;
EventHandlers Ui::event_handlers_;
PageInfo Ui::page_info_;

uint8_t Ui::cycle_;
uint8_t Ui::request_redraw_;
uint8_t Ui::inhibit_shift_raised_;
uint32_t Ui::last_click_time_;
/* </static> */

/* extern */
Ui ui;

const prog_PageInfo page_registry[] PROGMEM = {
  { PAGE_CLOCK_TESTER,
    &ClockTesterPage::event_handlers_,
  },
  { PAGE_CLOCK_GENERATOR,
    &ClockGeneratorPage::event_handlers_,
  },
  { PAGE_STEP_LENG,
    &StepLengPage::event_handlers_,
  },
  { PAGE_STEP_NUMB,
    &StepNumbPage::event_handlers_,
  },
  { PAGE_HOLD_MODE,
    &HoldModePage::event_handlers_,
  },
  { PAGE_ROUND_BPM,
    &RoundBpmPage::event_handlers_,
  },
  { PAGE_THRESHOLD,
    &ThresholdPage::event_handlers_,
  },
  { PAGE_PASS_THRU,
    &PassThruPage::event_handlers_,
  },
  { PAGE_STROBE_WIDTH,
    &StrobeWidthPage::event_handlers_,
  },
  { PAGE_DLG,
    &DlgPage::event_handlers_,
  },
};

/* static */
void Ui::Init() {
  encoder_.Init();
  switch_.Init();
  lcd.Init();
  display.Init();

  Logo();

  InitDeviceMode();
}

/* static */
void Ui::InitDeviceMode() {
  uint8_t device_mode = eemem.device_mode();
  if (switch_.low()) {
    switch (device_mode) {
    case DEVICEMODE_CLOCK_TESTER:
      device_mode = DEVICEMODE_CLOCK_GENERATOR;
      eemem.set_device_mode(device_mode);
      break;
    case DEVICEMODE_CLOCK_GENERATOR:
      device_mode = DEVICEMODE_CLOCK_TESTER;
      eemem.set_device_mode(device_mode);
      break;
    }
  }

  switch (device_mode) {
    case DEVICEMODE_CLOCK_TESTER:
      ShowPage(PAGE_CLOCK_TESTER);
      break;
    case DEVICEMODE_CLOCK_GENERATOR:
      ShowPage(PAGE_CLOCK_GENERATOR);
      break;
  }
}

/* static */
void Ui::Logo() {
  Clear();
  char* line1 = display.line_buffer(0);
  char* line2 = display.line_buffer(1);
  memcpy_P(&line1[0], PSTRN("MIDI Clk"));
  memcpy_P(&line2[1], PSTRN("v" MIDICLK_VERSION));
  RedrawScreen();

  for (uint8_t n = 0; n < 3; n++) {
    MidiInLed::High(); MidiOutLed::High(); _delay_ms(150);
    MidiInLed::Low(); MidiOutLed::Low(); _delay_ms(150);
  }
}

/* static */
void Ui::Poll() {

  // 2.45KHz
  cycle_++;

  PollEncoder();

  if ((cycle_ & 3) == 0) {
    // 612Hz
    PollSwitches();
  }

  // Output
  lcd.Tick();
}

/* static */
void Ui::PollEncoder() {
  int8_t increment = encoder_.Read();
  if (increment) {
    //if (GetShifted()) increment*= 10;
    AddEvent(CONTROL_ENCODER, ENCODER, increment);
  }
  if (encoder_.clicked()) {    
    uint32_t time = milliseconds();
    uint8_t value = ((time - last_click_time_) < kDblClickTime ? DBLCLICK : CLICK) + GetShifted();
    AddEvent(CONTROL_ENCODER_CLICK, ENCODER, value);
    last_click_time_ = time;
  }
}

/* static */
void Ui::PollSwitches() {
  switch_.Read();
  if (switch_.lowered()) {
    AddEvent(CONTROL_SWITCH, SWITCH);
  } else
  if (switch_.raised()) {
    if (inhibit_shift_raised_) {
      inhibit_shift_raised_ = 0;
    } else
      AddEvent(CONTROL_SWITCH, SWITCH_RAISED);
  }
}

/* static */
void Ui::DoEvents() {
  display.Tick();
  
  uint8_t redraw = request_redraw_; request_redraw_ = 0;
  while (queue_.available()) {
    Event e = queue_.PullEvent();
    queue_.Touch();
    HandleEvent(e);
    redraw = 1;
  }
  
  if (queue_.idle_time_ms() > 1000) {
    queue_.Touch();
    if ((*event_handlers_.OnIdle)()) {
      redraw = 1;
    }
  }
  
  if (redraw) {
    display.Clear();
    (*event_handlers_.UpdateScreen)();
  }

  (*event_handlers_.UpdateLeds)();
}

/* static */
void Ui::HandleEvent(const Event& e) {
  switch (e.control_type) {
    case CONTROL_ENCODER:
      if (!(*event_handlers_.OnIncrement)(e.control_id, e.value)) {
        HandleEncoderEvent(e);
      }
      break;
    case CONTROL_ENCODER_CLICK:
      if (!(*event_handlers_.OnClick)(e.control_id, e.value)) {
        HandleEncoderClickEvent(e);
      }
      break;
    case CONTROL_SWITCH:
      if (!(*event_handlers_.OnSwitch)(e.control_id, e.value)) {
        HandleSwitchEvent(e);
      }
      break;
    case CONTROL_REQUEST:
      {
        HandleRequestEvent(e);
      }
      break;
  }
}

/* static */
void Ui::HandleEncoderEvent(const Event& e) {
  // Cycle between primary pages on ENC rotation
  if (e.control_id == ENCODER) {
    uint8_t page = active_page_;
    if ((int8_t)e.value > 0) {
      if (page < LAST_PAGE) {
        page++;
      }
    } else
    if ((int8_t)e.value < 0) {
      if (page > FIRST_PAGE) {
        page--;
      }
    }
    ShowPage(static_cast<UiPageIndex>(page));
  }
}

void Ui::HandleEncoderClickEvent(const Event& e) {
  if (e.control_id == ENCODER) {
    switch (e.value) {
      case CLICK:
        break;
      case CLICK_SHIFTED: 
        break;
      case DBLCLICK: 
        UiPage::set_changing_page(0);
        ShowPage(FIRST_PAGE); 
        break;
      case DBLCLICK_SHIFTED: 
        break;
    }
  }
}

void Ui::HandleSwitchEvent(const Event& e) {
  if (e.control_id == SWITCH) {
  } else
  if (e.control_id == SWITCH_SHIFTED) {
  } else
  if (e.control_id == SWITCH_RAISED) {
  } else
  if (e.control_id == SWITCH_RAISED_SHIFTED) {
  }
}

/* static */
void Ui::HandleRequestEvent(const Event& e) {
  switch (e.control_id) {

    case REQUEST_SHOWPAGE:
      if (active_page_ != e.value) {
        ShowPage(static_cast<UiPageIndex>(e.value));
      } else
        UpdateScreen();
      break;

    case REQUEST_UPDATESCREEN:
      UpdateScreen();
      break;
  }
}

/* static */
void Ui::ShowPage(UiPageIndex page) {
  // Flush the event queue.
  queue_.Flush();
  queue_.Touch();

  // Inform current page if any
  if (*event_handlers_.OnQuit) {
    (*event_handlers_.OnQuit)(page);
  }

#ifdef ENABLE_DEBUG_OUTPUT  
  printf("ShowPage: %d\n", page);
#endif

  // Only remember primary pages
  if (active_page_ >= FIRST_PAGE && active_page_ <= LAST_PAGE) {
    last_page_ = active_page_;
  }

  // Activate new page
  UiPageIndex prevPage = active_page_;
  active_page_ = page;

  // Load the page info structure in RAM.
  ResourcesManager::Load(page_registry, page, &page_info_);

  // Load the event handlers structure in RAM.
  ResourcesManager::Load(page_info_.event_handlers, 0, &event_handlers_);

  (*event_handlers_.OnInit)(&page_info_, prevPage);

  UpdateScreen();
}

/* static */
Dialog::Dialog(const prog_char* text, const prog_char* buttons, uint8_t num_buttons) {
  data_.text = text;
  data_.buttons = buttons;
  data_.num_buttons = num_buttons;
  Ui::ShowDialog(data_);
}

/* static */
void Ui::ShowDialog(DlgData& dlg_data) {
  // Flush the event queue.
  queue_.Flush();
  queue_.Touch();

  // Load the Dlg page info structure in RAM.
  ResourcesManager::Load(page_registry, PAGE_DLG, &page_info_);

  // Load the event handlers structure in RAM.
  ResourcesManager::Load(page_info_.event_handlers, 0, &event_handlers_);

  memcpy(&page_info_.dlg_data, &dlg_data, sizeof(DlgData));

  (*event_handlers_.OnInit)(&page_info_, active_page_);

  UpdateScreen();

  // Run local event loop
  page_info_.dlg_data.result = kNoDlgResult;
  while (page_info_.dlg_data.result == kNoDlgResult) {
    ui.DoEvents();
  }

  // Set result for the caller
  dlg_data.result = page_info_.dlg_data.result;

  // Flush the event queue.
  queue_.Flush();
  queue_.Touch();

  // Load the active page info structure in RAM.
  ResourcesManager::Load(page_registry, active_page_, &page_info_);

  // Load the event handlers structure in RAM.
  ResourcesManager::Load(page_info_.event_handlers, 0, &event_handlers_);

  UpdateScreen();
}

/* static */
void Ui::Clear() {
  display.Clear();
}

/* static */
void Ui::UpdateScreen() {
  display.Clear();
  (*event_handlers_.UpdateScreen)();
}

/* static */
void Ui::RedrawScreen() {
  for (uint8_t n = 0; n < kLcdWidth * kLcdHeight; n++) {
    display.Tick();
  }
}

/* static */
void Ui::PrintChannel(char* buffer, uint8_t channel) {
  *buffer = (channel == 0xff)
      ? ' '
      : ((channel < 9) ? '1' + channel : (channel - 8));
}

/* static */
void Ui::PrintHex(char* buffer, uint8_t value) {
  *buffer++ = NibbleToAscii(U8ShiftRight4(value));
  *buffer = NibbleToAscii(value & 0xf);
}

/* static */
void Ui::PrintNote(char* buffer, uint8_t note) {
  uint8_t octave = 0;
  while (note >= 12) {
    ++octave;
    note -= 12;
  }

  switch (note) {
    case 1: case 3: case 6: case 8: case 10: buffer++; break;
  }

  static const prog_char note_names[] PROGMEM = " CC# DD# E FF# GG# AA# B";
  static const prog_char octaves[] PROGMEM = "-0123456789";

  *buffer++ = ResourcesManager::Lookup<char, uint8_t>(
      note_names, note << 1);
  *buffer++ = ResourcesManager::Lookup<char, uint8_t>(
      note_names, 1 + (note << 1));
  *buffer = ResourcesManager::Lookup<char, uint8_t>(
      octaves, octave);
}

/* static */
void Ui::PrintNN(char* buffer, uint16_t numb, char filler) {
  uint8_t x = 0;
  if (numb < 10) { x = 1; buffer[0] = filler; }
  UnsafeItoa(numb, 2 - x, &buffer[x]);
}

/* static */
void Ui::PrintNNN(char* buffer, uint16_t numb, char filler) {
  uint8_t x = 0;
  if (numb < 10)  { x = 2; buffer[0] = filler; buffer[1] = filler; } else 
  if (numb < 100) { x = 1; buffer[0] = filler; }
  UnsafeItoa(numb, 3 - x, &buffer[x]);
}

/* static */
int16_t Ui::Scale(int16_t x0, int16_t x0min, int16_t x0max, int16_t x1min, int16_t x1max) {
	if (x0max == x0min) 
		return x1max;
	
	int16_t x1 = x1min + ((x0 - x0min) * (x1max - x1min)) / (x0max - x0min);

	if (x1min < x1max) {
		if (x1 < x1min) {
			x1 = x1min;
		} else
		if (x1 > x1max) {
			x1 = x1max;
		}
	} else {
		if (x1 < x1max) {
			x1 = x1max;
		} else
		if (x1 > x1min) {
			x1 = x1min;
		}
	}
	
	return x1;
}

}  // namespace midiclk
