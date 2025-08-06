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
String PloverHID::setup_debug_ = "";

EventHandlerResult PloverHID::onSetup() {
  // Debug output via Serial and store for later
  Serial.println("PloverHID: Starting setup");
  setup_debug_ = "Setup: ";

#ifdef ARDUINO_ARCH_NRF52
  Serial.println("PloverHID: nRF52 detected");
  setup_debug_ += "nRF52 ";

  // For nRF52 devices, we rely on the device's HID configuration
  // The TUSBMultiReport should already be initialized by the device
  Serial.println("PloverHID: Using device-configured HID interfaces");
  setup_debug_ += "DeviceHID ";

  // Check if TinyUSB device is mounted (configured)
  if (TinyUSBDevice.mounted()) {
    Serial.println("PloverHID: TinyUSB device is mounted and configured");
    setup_debug_ += "Mounted ";
  } else {
    Serial.println("PloverHID: TinyUSB device is not yet mounted (normal during setup)");
    setup_debug_ += "NotMounted ";
  }

#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RP2040)
  Serial.println("PloverHID: TinyUSB device detected");
  setup_debug_ += "TinyUSB ";

  // For other TinyUSB devices, same approach
  Serial.println("PloverHID: Using device-configured HID interfaces");
  setup_debug_ += "DeviceHID ";

  if (TinyUSBDevice.mounted()) {
    setup_debug_ += "Mounted ";
  } else {
    setup_debug_ += "NotMounted ";
  }
#else
  Serial.println("PloverHID: AVR device detected");
  setup_debug_ += "AVR ";
#endif

  Serial.println("PloverHID: Setup complete");
  setup_debug_ += "Complete";
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

  // Debug output for troubleshooting mapping issues
  Serial.print("PloverHID: Key ");
  if (event.key == plover_hid::S_L) Serial.print("S_L");
  else if (event.key == plover_hid::T_L) Serial.print("T_L");
  else if (event.key == plover_hid::K_L) Serial.print("K_L");
  else if (event.key == plover_hid::P_L) Serial.print("P_L");
  else if (event.key == plover_hid::W_L) Serial.print("W_L");
  else if (event.key == plover_hid::H_L) Serial.print("H_L");
  else if (event.key == plover_hid::R_L) Serial.print("R_L");
  else if (event.key == plover_hid::A) Serial.print("A");
  else Serial.print("OTHER");
  
  Serial.print(" -> key_index=");
  Serial.print(key_index);
  Serial.print(", byte=");
  Serial.print(byte_index);
  Serial.print(", bit=");
  Serial.print(bit_index);

  // EXPERIMENTAL FIX: Reverse bit order within each byte to match Plover expectations
  // Based on user feedback that S_L maps to A, T_L maps to H, etc.
  uint8_t reversed_bit_index = 7 - bit_index;
  Serial.print(", reversed_bit=");
  Serial.println(reversed_bit_index);

  // Update the report based on key state
  if (keyToggledOn(event.state)) {
    // Set the bit for key press (using reversed bit index)
    report_[byte_index] |= (1 << reversed_bit_index);
  } else if (keyToggledOff(event.state)) {
    // Clear the bit for key release (using reversed bit index)
    report_[byte_index] &= ~(1 << reversed_bit_index);
  }

  // Send the report immediately (real-time key state changes)
  sendReport();

  return EventHandlerResult::EVENT_CONSUMED;
}

void PloverHID::sendReport() {
  // Debug output with setup information and USB state
  Serial.print("PloverHID: Sending report: ");
  for (int i = 0; i < 8; i++) {
    Serial.print(report_[i], HEX);
    Serial.print(" ");
  }
  Serial.print("| ");
  Serial.print(setup_debug_);
  Serial.print(" | USB: ");
  Serial.print(TinyUSBDevice.mounted() ? "Mounted" : "NotMounted");
  Serial.print(" Ready: ");
  Serial.print(TinyUSBDevice.ready() ? "Yes" : "No");
  Serial.print(" Suspended: ");
  Serial.println(TinyUSBDevice.suspended() ? "Yes" : "No");

#ifdef ARDUINO_ARCH_NRF52
  // For nRF52 devices with Hybrid HID (Preonic), send via both interfaces
  // The Hybrid driver will route to the active connection (USB or Bluetooth)

  Serial.println("PloverHID: Sending via Bluefruit");
  // Send via Bluefruit (Bluetooth) interface
  kaleidoscope::driver::hid::bluefruit::blehid.sendInputReport(kaleidoscope::driver::hid::bluefruit::RID_PLOVER_HID, report_, sizeof(report_));

  Serial.println("PloverHID: Sending via TinyUSB");
  // Send via TinyUSB (USB) interface using MultiReport
  // Use the same pattern as TUSBConsumerControl and TUSBMouse
  kaleidoscope::driver::hid::tinyusb::TUSBMultiReport().sendReport(kaleidoscope::driver::hid::tinyusb::RID_PLOVER_HID, report_, sizeof(report_));

#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RP2040)
  Serial.println("PloverHID: Sending via TinyUSB");
  // For TinyUSB devices (SAMD, RP2040), use TinyUSB MultiReport
  kaleidoscope::driver::hid::tinyusb::TUSBMultiReport().sendReport(kaleidoscope::driver::hid::tinyusb::RID_PLOVER_HID, report_, sizeof(report_));
#else
  Serial.println("PloverHID: Sending via KeyboardioHID");
  // For AVR and other architectures using KeyboardioHID
  // Use the PloverHIDInterface directly
  ::PloverHIDInterface.sendReport(report_, sizeof(report_));
#endif
}

}  // namespace plugin
}  // namespace kaleidoscope

kaleidoscope::plugin::PloverHID PloverHID;
