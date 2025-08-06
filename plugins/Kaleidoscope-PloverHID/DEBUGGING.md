# PloverHID Enumeration Debugging Guide

This guide helps diagnose and fix PloverHID enumeration issues on the Keyboardio Preonic with Kaleidoscope.

## Quick Diagnosis

### 1. Check USB Enumeration
```bash
# Run the debug script
./debug_enumeration.sh

# Or manually check
lsusb -v -d 1209:2301 | grep -A 20 "HID"
```

### 2. Verify HID Descriptor
The device should show:
- **Usage Page**: `0xFF50` (Vendor-defined)
- **Usage**: `0x4C56` (encoding "STN")
- **Report ID**: `0x50`

### 3. Check Serial Output
Look for these messages during device startup:
```
PloverHID: Starting setup
PloverHID: nRF52 detected
PloverHID: Using device-configured HID interfaces
PloverHID: TinyUSB device is mounted and configured
```

## Common Issues and Solutions

### Issue 1: Device Not Enumerating
**Symptoms**: Device doesn't appear in `lsusb` or shows as "Device Descriptor Request Failed"

**Solutions**:
1. **Check TinyUSB Configuration**: Ensure `CFG_TUD_HID >= 3` in `tusb_config.h`
2. **Verify Firmware Upload**: Re-upload firmware and check for compilation errors
3. **Reset USB Stack**: Unplug/replug device or use recovery mode

### Issue 2: HID Interface Missing
**Symptoms**: Device appears but no HID interfaces listed

**Solutions**:
1. **Check HID Descriptor Values**: Verify usage page `0xFF50` and usage `0x4C56`
2. **Ensure TUSBMultiReport Initialization**: Check that `TUSBMultiReport()` is called during setup
3. **Verify Device Configuration**: Ensure Preonic device includes PloverHID support

### Issue 3: PloverHID Interface Not Recognized
**Symptoms**: Device enumerates but Plover doesn't detect PloverHID

**Solutions**:
1. **Check Report ID**: Ensure report ID `0x50` is used consistently
2. **Verify Report Format**: Ensure 8-byte reports are sent
3. **Test with HID Tools**: Use `usbhid-dump` to verify reports are sent

## Platform-Specific Debugging

### Linux
```bash
# Check HID devices
ls -la /sys/class/hidraw/

# Monitor USB events
sudo dmesg -w | grep -i "usb\|hid"

# Test HID reports
sudo usbhid-dump -d 1209:2301
```

### macOS
```bash
# Check system information
system_profiler SPUSBDataType | grep -A 20 "Keyboardio"

# Monitor USB events
log stream --predicate 'process == "kernel"' | grep -i "usb\|hid"
```

### Windows
```powershell
# Check device manager
Get-PnpDevice | Where-Object {$_.FriendlyName -like "*Keyboardio*"}

# Check USB devices
Get-WmiObject Win32_USBHub | Where-Object {$_.Name -like "*Keyboardio*"}
```

## Advanced Debugging

### 1. Enable TinyUSB Debug Logging
Add to your sketch:
```cpp
#define CFG_TUSB_DEBUG 2
```

### 2. Monitor Serial Output
Connect to device serial port and look for:
```
PloverHID: Sending report: 01 00 00 00 00 00 00 00
PloverHID: Sending via TinyUSB
```

### 3. Test HID Reports Manually
Use a HID testing tool to verify reports are being sent correctly.

### 4. Check Endpoint Allocation
Ensure no endpoint conflicts between HID interfaces.

## Verification Steps

### Step 1: Basic Enumeration
1. Upload firmware with PloverHID plugin
2. Check `lsusb` shows device
3. Verify HID interfaces are listed

### Step 2: HID Descriptor Check
1. Use `lsusb -v` to check descriptor values
2. Verify usage page `0xFF50` and usage `0x4C56`
3. Check report ID `0x50` is present

### Step 3: Report Transmission
1. Press PloverHID keys
2. Check serial output for report transmission
3. Verify reports reach host (use HID monitoring tools)

### Step 4: Plover Integration
1. Open Plover
2. Select "Plover HID" as machine type
3. Verify device is detected and reports are received

## Troubleshooting Checklist

- [ ] Device appears in `lsusb`
- [ ] HID interfaces are listed
- [ ] Usage page `0xFF50` is present
- [ ] Usage `0x4C56` is present
- [ ] Report ID `0x50` is used
- [ ] Serial output shows "TinyUSB device is mounted"
- [ ] Reports are sent when keys are pressed
- [ ] Plover detects device as "Plover HID"

## Getting Help

If issues persist:
1. Run `./debug_enumeration.sh` and share output
2. Check serial output for error messages
3. Verify firmware compilation with PloverHID plugin
4. Test with minimal keymap to isolate issues 