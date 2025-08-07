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

// Static member variables
uint8_t PloverHID::report_[8];
uint8_t PloverHID::pending_report_[8];
bool PloverHID::report_pending_ = false;
uint16_t PloverHID::last_report_time_ = 0;
uint16_t PloverHID::report_delay_ms_ = 0;  // No artificial delay for steno reports
bool PloverHID::keys_held_ = false;

EventHandlerResult PloverHID::onSetup() {
  memset(report_, 0, sizeof(report_));
  memset(pending_report_, 0, sizeof(pending_report_));
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

  // Update the pending report based on key state
  if (keyToggledOn(event.state)) {
    // Set the bit for key press
    pending_report_[byte_index] |= (1 << reversed_bit_index);
    keys_held_ = true;
    report_pending_ = true;
    
    // Debug output disabled for BLE
    // Serial.print("PloverHID: Key press - index=");
    // Serial.print(key_index);
    // Serial.print(", byte=");
    // Serial.print(byte_index);
    // Serial.print(", bit=");
    // Serial.println(reversed_bit_index);
    // printPendingReport();
    
  } else if (keyToggledOff(event.state)) {
    // Clear the bit for key release
    pending_report_[byte_index] &= ~(1 << reversed_bit_index);
    
    // Check if any keys are still held
    keys_held_ = false;
    for (uint8_t i = 0; i < 8; i++) {
      if (pending_report_[i] != 0) {
        keys_held_ = true;
        break;
      }
    }
    report_pending_ = true;
    
    // Debug output disabled for BLE
    // Serial.print("PloverHID: Key release - index=");
    // Serial.print(key_index);
    // Serial.print(", byte=");
    // Serial.print(byte_index);
    // Serial.print(", bit=");
    // Serial.print(reversed_bit_index);
    // Serial.print(", keys_held=");
    // Serial.println(keys_held_);
    // printPendingReport();
  }

  // Send immediately if report has changed (like ZMK approach)
  if (report_pending_) {
    uint16_t current_time = millis();
    
    // Check if enough time has passed since last report (respecting minimum delay)
    if (current_time - last_report_time_ >= report_delay_ms_) {
      // Check if the report has actually changed
      bool report_changed = false;
      for (uint8_t i = 0; i < 8; i++) {
        if (report_[i] != pending_report_[i]) {
          report_changed = true;
          report_[i] = pending_report_[i];
        }
      }

      // Send the report if it has changed
      if (report_changed) {
        sendReport();
        last_report_time_ = current_time;
      }
      
      // Clear the pending flag after processing
      report_pending_ = false;
    }
  }

  return EventHandlerResult::EVENT_CONSUMED;
}

EventHandlerResult PloverHID::afterEachCycle() {
  // Handle any delayed reports that couldn't be sent immediately due to timing constraints
  if (!report_pending_) {
    return EventHandlerResult::OK;
  }

  uint16_t current_time = millis();
  
  // Check if enough time has passed since last report
  if (current_time - last_report_time_ < report_delay_ms_) {
    return EventHandlerResult::OK;
  }

  // Check if the report has changed
  bool report_changed = false;
  for (uint8_t i = 0; i < 8; i++) {
    if (report_[i] != pending_report_[i]) {
      report_changed = true;
      report_[i] = pending_report_[i];
    }
  }

  // Send the report if it has changed
  if (report_changed) {
    // Debug output disabled for BLE
    // Serial.print("PloverHID: Sending delayed report - ");
    // printCurrentReport();
    sendReport();
    last_report_time_ = current_time;
  }
  
  // Always clear the pending flag after processing
  report_pending_ = false;

  return EventHandlerResult::OK;
}

void PloverHID::sendReport() {
#ifdef ARDUINO_ARCH_NRF52
  // For nRF52 devices with Hybrid HID (Preonic)
  // The Hybrid driver will route to the active connection automatically
  
  // Send via Bluefruit (Bluetooth) interface
  kaleidoscope::driver::hid::bluefruit::blehid.sendInputReport(
    kaleidoscope::driver::hid::bluefruit::RID_PLOVER_HID, 
    report_, 
    sizeof(report_)
  );

  // Send via TinyUSB (USB) interface using MultiReport
  kaleidoscope::driver::hid::tinyusb::TUSBMultiReport().sendReport(
    kaleidoscope::driver::hid::tinyusb::RID_PLOVER_HID, 
    report_, 
    sizeof(report_)
  );

#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RP2040)
  // For TinyUSB devices
  kaleidoscope::driver::hid::tinyusb::TUSBMultiReport().sendReport(
    kaleidoscope::driver::hid::tinyusb::RID_PLOVER_HID, 
    report_, 
    sizeof(report_)
  );
#else
  // For AVR and other architectures
  ::PloverHIDInterface.sendReport(report_, sizeof(report_));
#endif
}

// Configuration methods
void PloverHID::setReportDelay(uint16_t delay_ms) {
  report_delay_ms_ = delay_ms;
}

uint16_t PloverHID::getReportDelay() {
  return report_delay_ms_;
}

// Debug methods (disabled for BLE)
/*
void PloverHID::printCurrentReport() {
  Serial.print("Current report: ");
  for (uint8_t i = 0; i < 8; i++) {
    Serial.print("0x");
    if (report_[i] < 16) Serial.print("0");
    Serial.print(report_[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void PloverHID::printPendingReport() {
  Serial.print("Pending report: ");
  for (uint8_t i = 0; i < 8; i++) {
    Serial.print("0x");
    if (pending_report_[i] < 16) Serial.print("0");
    Serial.print(pending_report_[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}
*/

}  // namespace plugin
}  // namespace kaleidoscope

kaleidoscope::plugin::PloverHID PloverHID;
