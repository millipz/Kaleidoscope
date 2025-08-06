#!/bin/bash

# PloverHID Enumeration Debug Script
# This script helps verify if PloverHID is properly enumerated on the host

echo "=== PloverHID Enumeration Debug ==="
echo "Checking for Keyboardio Preonic device..."

# Check if device is connected
if lsusb | grep -q "Keyboardio"; then
    echo "✓ Keyboardio device found"
    
    # Get detailed USB information
    echo ""
    echo "=== USB Device Details ==="
    lsusb -v -d 1209:2301 2>/dev/null | grep -A 20 -B 5 "HID\|Interface\|Endpoint" || echo "Device not found with VID:PID 1209:2301"
    
    # Check for HID interfaces
    echo ""
    echo "=== HID Interfaces ==="
    lsusb -v -d 1209:2301 2>/dev/null | grep -A 10 "HID" || echo "No HID interfaces found"
    
    # Check for PloverHID specific usage page
    echo ""
    echo "=== PloverHID Usage Page Check ==="
    lsusb -v -d 1209:2301 2>/dev/null | grep -A 5 -B 5 "FF50\|4C56" || echo "PloverHID usage page (0xFF50) not found"
    
else
    echo "✗ Keyboardio device not found"
    echo "Available USB devices:"
    lsusb
fi

# Check kernel messages for USB enumeration
echo ""
echo "=== Kernel USB Messages ==="
dmesg | tail -20 | grep -i "usb\|hid\|keyboardio" || echo "No relevant kernel messages found"

# Check if device appears in /sys/class/hidraw
echo ""
echo "=== HID Raw Devices ==="
ls -la /sys/class/hidraw/ 2>/dev/null || echo "No HID raw devices found"

# For macOS
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo ""
    echo "=== macOS System Information ==="
    system_profiler SPUSBDataType | grep -A 10 -B 5 "Keyboardio" || echo "Keyboardio device not found in system profiler"
fi

# For Windows (if running in WSL or similar)
if command -v powershell.exe >/dev/null 2>&1; then
    echo ""
    echo "=== Windows Device Manager Check ==="
    echo "Run in Windows PowerShell: Get-PnpDevice | Where-Object {$_.FriendlyName -like '*Keyboardio*'}"
fi

echo ""
echo "=== Debug Complete ==="
echo "If PloverHID is not enumerated, check:"
echo "1. Device firmware compilation with PloverHID plugin"
echo "2. TinyUSB configuration (CFG_TUD_HID >= 3)"
echo "3. HID descriptor values (Usage Page: 0xFF50, Usage: 0x4C56)"
echo "4. USB re-enumeration after firmware upload" 