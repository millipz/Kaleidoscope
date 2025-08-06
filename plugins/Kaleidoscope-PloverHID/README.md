# PloverHID

This plugin implements the Plover HID protocol for stenography, providing an alternative to the existing GeminiPR serial protocol. The Plover HID protocol offers several advantages:

- **Better Bluetooth Support**: Native HID works seamlessly over Bluetooth Low Energy
- **Real-time Key Events**: Sends individual key press/release events instead of complete chords
- **Extended Key Support**: Supports up to 64 keys vs. the limited GeminiPR key set
- **Better Host Integration**: No need for serial port configuration in Plover
- **Future-Proof**: More extensible for features like pressure-sensitive keys

## Protocol Details

The Plover HID protocol uses:
- **Vendor-defined HID Usage Page**: 0xFF50 ("S")
- **Usage**: 0x564C ("TN") 
- Together they spell "STN" for stenography
- **Report ID**: 0x50 (80 decimal)
- **Report Format**: 8 bytes representing 64 key states as a bitmap

## Key Layout

The plugin defines 64 keys following the Plover HID specification:

### Standard Steno Keys (0-24)
```
S_L T_L K_L P_L W_L H_L R_L A O STAR E U F_R R_R P_R B_R L_R G_R T_R S_R D_R Z_R NUM
```

### Extended Keys (25-63)
```
X1 through X39
```

## Using the Plugin

To use the plugin, include the header in your sketch and add PloverHID keys to your keymap:

```c++
#include <Kaleidoscope.h>
#include <Kaleidoscope-PloverHID.h>

// In your keymap, use PH() macro for Plover HID keys:
PH(S_L), PH(T_L), PH(K_L), PH(P_L), PH(W_L), PH(H_L), PH(R_L),
PH(A), PH(O), PH(STAR), PH(E), PH(U),
PH(F_R), PH(R_R), PH(P_R), PH(B_R), PH(L_R), PH(G_R), PH(T_R), PH(S_R), PH(D_R), PH(Z_R), PH(NUM)

KALEIDOSCOPE_INIT_PLUGINS(PloverHID);

void setup() {
  Kaleidoscope.setup();
}
```

## Plover Configuration

In Plover, select "Plover HID" as your machine type. The plugin will be automatically detected when the keyboard is connected.

## Troubleshooting

### Key Mapping Issues

If you experience incorrect key mapping (e.g., pressing S_L results in A in Plover), this is due to a bit ordering issue between the Kaleidoscope implementation and Plover's expectations.

**Symptoms:**
- S_L key produces A output
- T_L key produces H output  
- H_L key produces T output
- Other keys may be similarly mismatched

**Solution:**
The current implementation includes a fix that reverses bit order within each byte of the HID report. This addresses the mapping discrepancy by sending the bits in the order that Plover expects.

**Debug Information:**
Enable debug output by connecting to the Serial Monitor. You'll see output like:
```
PloverHID: Key S_L -> key_index=0, byte=0, bit=0, reversed_bit=7
PloverHID: Sending report: 80 00 00 00 00 00 00 00 | Setup: nRF52 DeviceHID Mounted Complete | USB: Mounted Ready: Yes Suspended: No
```

This debug information helps verify that keys are being mapped correctly.

### Connection Issues

If Plover doesn't detect the keyboard:
1. Ensure the keyboard is properly connected (USB or Bluetooth)
2. Check that PloverHID is included in your KALEIDOSCOPE_INIT_PLUGINS list
3. Verify that "Plover HID" is selected as the machine type in Plover settings
4. Try disconnecting and reconnecting the keyboard

### Performance Issues

For optimal performance:
- Ensure your keyboard has adequate processing power for real-time HID reports
- Consider reducing debug output in production builds
- Test both USB and Bluetooth connections to find the most reliable option

## Compatibility

This plugin is compatible with:
- USB connections
- Bluetooth Low Energy connections  
- All Kaleidoscope-supported hardware platforms

## Comparison with GeminiPR

| Feature | PloverHID | GeminiPR |
|---------|-----------|----------|
| Connection | HID (USB/BLE) | Serial (USB CDC) |
| Key Events | Real-time press/release | Chord on release |
| Key Count | 64 keys | ~43 keys |
| Bluetooth | Native support | Requires custom protocol |
| Host Setup | Automatic | Manual serial port config |
| Latency | Lower | Higher |

Both protocols can coexist in the same firmware if needed.
