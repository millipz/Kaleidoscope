/* Kaleidoscope - Firmware for computer input devices
 * Copyright (C) 2013-2025 Keyboard.io, inc.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * Additional Permissions:
 * As an additional permission under Section 7 of the GNU General Public
 * License Version 3, you may link this software against a Vendor-provided
 * Hardware Specific Software Module under the terms of the MCU Vendor
 * Firmware Library Additional Permission Version 1.0.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "kaleidoscope/plugin/PloverHID.h"

#include <Arduino.h>                   // for F, __FlashStringHelper
#include <Kaleidoscope-FocusSerial.h>  // for Focus, FocusSerial
#include <stdint.h>                    // for uint8_t
#include <string.h>                    // for memset

// For HID report ID and architecture-specific HID includes
#ifdef ARDUINO_ARCH_NRF52
#include "kaleidoscope/driver/hid/bluefruit/HIDD.h"
#include "kaleidoscope/driver/hid/tinyusb/MultiReport.h"
#include "Adafruit_TinyUSB.h"
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RP2040)
#include "kaleidoscope/driver/hid/tinyusb/MultiReport.h"
#include "Adafruit_TinyUSB.h"
#else
#include "HID-Settings.h"
#include "HID.h"
#include "MultiReport/PloverHID.h"
#include "KeyboardioHID.h"
#endif

#include "kaleidoscope/KeyEvent.h"              // for KeyEvent
#include "kaleidoscope/Runtime.h"               // for Runtime, Runtime_
#include "kaleidoscope/event_handler_result.h"  // for EventHandlerResult
#include "kaleidoscope/key_defs.h"              // for Key
#include "kaleidoscope/keyswitch_state.h"       // for keyswitch_state_t
#include "kaleidoscope/plugin/LEDControl.h"     // for LEDControl

namespace kaleidoscope {
namespace plugin {

uint8_t PloverHID::report_[8];
bool PloverHID::report_dirty_ = false;

EventHandlerResult PloverHID::onSetup() {
  return EventHandlerResult::OK;
}

EventHandlerResult PloverHID::onNameQuery() {
  return ::Focus.sendName(F("PloverHID"));
}

EventHandlerResult PloverHID::onKeyEvent(KeyEvent &event) {
  // Check if this is a Plover HID key
  if (event.key < plover_hid::START || event.key > plover_hid::END)
    return EventHandlerResult::OK;

  // Calculate bit position (0-63)
  uint8_t key_index = event.key.getRaw() - plover_hid::START;
  uint8_t byte_index = key_index / 8;
  uint8_t bit_index = key_index % 8;

  // EXPERIMENTAL FIX: Reverse bit order within each byte to match Plover expectations
  // Based on user feedback that S_L maps to A, T_L maps to H, etc.
  uint8_t reversed_bit_index = 7 - bit_index;

  // Update the report based on key state
  if (keyToggledOn(event.state)) {
    // Set the bit for key press (using reversed bit index)
    report_[byte_index] |= (1 << reversed_bit_index);
    report_dirty_ = true;
  } else if (keyToggledOff(event.state)) {
    // Clear the bit for key release (using reversed bit index)
    report_[byte_index] &= ~(1 << reversed_bit_index);
    report_dirty_ = true;
  }

  // Don't send immediately - batch reports per scan cycle
  return EventHandlerResult::EVENT_CONSUMED;
}

EventHandlerResult PloverHID::beforeReportingState(const KeyEvent &event) {
  // Send the report once per scan cycle if there were changes
  if (report_dirty_) {
    sendReport();
    report_dirty_ = false;
  }
  return EventHandlerResult::OK;
}

void PloverHID::sendReport() {

#ifdef ARDUINO_ARCH_NRF52
  // For nRF52 devices with Hybrid HID (Preonic), send via both interfaces
  // The Hybrid driver will route to the active connection (USB or Bluetooth)

  // Send via Bluefruit (Bluetooth) interface
  kaleidoscope::driver::hid::bluefruit::blehid.sendInputReport(kaleidoscope::driver::hid::bluefruit::RID_PLOVER_HID, report_, sizeof(report_));

  // Send via TinyUSB (USB) interface using MultiReport
  // Use the same pattern as TUSBConsumerControl and TUSBMouse
  kaleidoscope::driver::hid::tinyusb::TUSBMultiReport().sendReport(kaleidoscope::driver::hid::tinyusb::RID_PLOVER_HID, report_, sizeof(report_));

#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RP2040)
  // For TinyUSB devices (SAMD, RP2040), use TinyUSB MultiReport
  kaleidoscope::driver::hid::tinyusb::TUSBMultiReport().sendReport(kaleidoscope::driver::hid::tinyusb::RID_PLOVER_HID, report_, sizeof(report_));
#else
  // For AVR and other architectures using KeyboardioHID
  // Use the PloverHIDInterface directly
  ::PloverHIDInterface.sendReport(report_, sizeof(report_));
#endif
}

}  // namespace plugin
}  // namespace kaleidoscope

kaleidoscope::plugin::PloverHID PloverHID;
