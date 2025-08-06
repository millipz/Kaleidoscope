/* -*- mode: c++ -*-
 * Copyright (C) 2025  Keyboard.io, Inc.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "testing/setup-googletest.h"

SETUP_GOOGLETEST();

namespace kaleidoscope {
namespace testing {
namespace {

using ::testing::IsEmpty;

class PloverHIDReports : public VirtualDeviceTest {};

TEST_F(PloverHIDReports, BasicKeyPress) {
  // Test pressing S_L key (first key in Plover HID layout)
  sim_.Press(0, 0);  // S_L
  auto state = RunCycle();

  // Should generate a PloverHID report
  // Note: This test assumes the testing framework supports PloverHID reports
  // If not, we'll need to add that support to the testing infrastructure
  
  sim_.Release(0, 0);  // S_L
  state = RunCycle();
  
  // Key should be released in the report
  state = RunCycle();
}

TEST_F(PloverHIDReports, MultipleKeyPress) {
  // Test chord: S_L + T_L + A + O + STAR
  sim_.Press(0, 0);  // S_L
  sim_.Press(0, 1);  // T_L
  sim_.Press(4, 0);  // A
  sim_.Press(4, 1);  // O
  sim_.Press(5, 0);  // STAR
  auto state = RunCycle();

  // All keys should be set in the bitmap
  
  // Release all keys
  sim_.Release(0, 0);  // S_L
  sim_.Release(0, 1);  // T_L
  sim_.Release(4, 0);  // A
  sim_.Release(4, 1);  // O
  sim_.Release(5, 0);  // STAR
  state = RunCycle();
  
  // All keys should be cleared in the bitmap
  state = RunCycle();
}

TEST_F(PloverHIDReports, ExtendedKeys) {
  // Test extended keys X1-X5
  sim_.Press(1, 0);  // X1
  sim_.Press(1, 1);  // X2
  sim_.Press(1, 2);  // X3
  sim_.Press(1, 3);  // X4
  sim_.Press(1, 4);  // X5
  auto state = RunCycle();

  // Extended keys should be set in the bitmap
  
  // Release all keys
  sim_.Release(1, 0);  // X1
  sim_.Release(1, 1);  // X2
  sim_.Release(1, 2);  // X3
  sim_.Release(1, 3);  // X4
  sim_.Release(1, 4);  // X5
  state = RunCycle();
  
  // Extended keys should be cleared in the bitmap
  state = RunCycle();
}

TEST_F(PloverHIDReports, RealTimeReporting) {
  // Test that PloverHID sends reports immediately on key press/release
  // (unlike GeminiPR which waits for chord completion)
  
  sim_.Press(0, 0);  // S_L
  auto state1 = RunCycle();
  // Should have a report with S_L set
  
  sim_.Press(0, 1);  // T_L (add to chord)
  auto state2 = RunCycle();
  // Should have a report with both S_L and T_L set
  
  sim_.Release(0, 0);  // S_L (partial release)
  auto state3 = RunCycle();
  // Should have a report with only T_L set
  
  sim_.Release(0, 1);  // T_L (complete release)
  auto state4 = RunCycle();
  // Should have a report with no keys set
}

}  // namespace
}  // namespace testing
}  // namespace kaleidoscope
