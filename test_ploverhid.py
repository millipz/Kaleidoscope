#!/usr/bin/env python3
"""
Test script to verify PloverHID interface functionality
"""

import sys
import time

def test_ploverhid():
    """Test if PloverHID interface is accessible"""
    try:
        # Try to find HID devices
        import hid
        
        print("=== PloverHID Interface Test ===")
        print("Searching for HID devices...")
        
        # List all HID devices
        devices = hid.enumerate()
        
        print(f"Found {len(devices)} HID devices:")
        
        plover_found = False
        preonic_devices = []
        
        for device in devices:
            vendor_id = device['vendor_id']
            product_id = device['product_id']
            product_name = device.get('product_string', 'Unknown')
            usage_page = device.get('usage_page', 0)
            usage = device.get('usage', 0)
            interface_number = device.get('interface_number', -1)
            
            # Check if it's a Preonic device
            if vendor_id == 0x3496 and product_id == 0x00a1:
                preonic_devices.append(device)
                print(f"  VID: 0x{vendor_id:04x}, PID: 0x{product_id:04x}")
                print(f"    Name: {product_name}")
                print(f"    Usage Page: 0x{usage_page:04x}, Usage: 0x{usage:04x}")
                print(f"    Interface: {interface_number}")
                print()
                
                # Check if this is the PloverHID interface
                if usage_page == 0xFF50 and usage == 0x4C56:
                    print("  *** FOUND PloverHID INTERFACE! ***")
                    plover_found = True
                    
                    # Try to test the interface
                    try:
                        device_path = device['path']
                        print(f"  Opening device at path: {device_path}")
                        h = hid.device()
                        h.open_path(device_path)
                        
                        # Try to send a test PloverHID report
                        # PloverHID report format: [Report ID (0x50), 8 bytes of data]
                        test_report = [0x50] + [0x00] * 8  # Report ID + 8 bytes of zeros
                        print(f"  Sending test report: {test_report}")
                        h.write(test_report)
                        print("  ✓ Successfully sent PloverHID test report!")
                        
                        h.close()
                        
                    except Exception as e:
                        print(f"  ✗ Failed to test PloverHID interface: {e}")
            else:
                print(f"  VID: 0x{vendor_id:04x}, PID: 0x{product_id:04x}")
                print(f"    Name: {product_name}")
                print(f"    Usage Page: 0x{usage_page:04x}, Usage: 0x{usage:04x}")
                print()
        
        print(f"\nFound {len(preonic_devices)} Preonic HID interfaces")
        
        if not plover_found:
            print("\nNo PloverHID interface found with usage page 0xFF50 and usage 0x4C56")
            
            # Try alternative test - check if any Preonic interface can accept PloverHID reports
            print("\n=== Alternative Test: Check if any Preonic interface accepts PloverHID reports ===")
            
            for device in preonic_devices:
                device_path = device['path']
                usage_page = device.get('usage_page', 0)
                usage = device.get('usage', 0)
                
                try:
                    print(f"Testing interface with Usage Page: 0x{usage_page:04x}, Usage: 0x{usage:04x}")
                    h = hid.device()
                    h.open_path(device_path)
                    
                    # Try to send a PloverHID report
                    test_report = [0x50] + [0x00] * 8  # Report ID 0x50 + 8 bytes
                    print(f"  Sending PloverHID test report: {test_report}")
                    result = h.write(test_report)
                    print(f"  ✓ Write returned: {result} bytes")
                    
                    h.close()
                    print("  ✓ Successfully sent PloverHID report to this interface!")
                    
                except Exception as e:
                    print(f"  ✗ Failed to send PloverHID report: {e}")
                    try:
                        h.close()
                    except:
                        pass
            
            print("\nThis could mean:")
            print("  1. The PloverHID interface is not being enumerated")
            print("  2. The HID descriptor values are incorrect")
            print("  3. The interface is not being registered properly")
            print("\nNext steps:")
            print("  - Check if TinyUSB is properly registering the MultiReport interface")
            print("  - Verify the HID descriptor is being included in the USB configuration")
            print("  - Check if the Report ID 0x50 is being handled correctly")
        
    except ImportError:
        print("hidapi library not installed. Install with: pip install hidapi")
        return False
    except Exception as e:
        print(f"Error: {e}")
        return False
    
    return plover_found

if __name__ == "__main__":
    if test_ploverhid():
        print("\n✓ PloverHID interface test PASSED")
        sys.exit(0)
    else:
        print("\n✗ PloverHID interface test FAILED")
        sys.exit(1)