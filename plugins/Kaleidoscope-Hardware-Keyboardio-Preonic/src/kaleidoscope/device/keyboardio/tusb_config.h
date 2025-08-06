/* TinyUSB Configuration for Keyboardio Preonic
 * This file ensures proper HID enumeration for PloverHID and other HID interfaces
 */

#ifndef TUSB_CONFIG_H_
#define TUSB_CONFIG_H_

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUSB_MCU                OPT_MCU_NRF52840
#define CFG_TUSB_RHPORT0_MODE       OPT_MODE_DEVICE
#define CFG_TUSB_OS                 OPT_OS_NONE

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUD_CDC                 1
#define CFG_TUD_MSC                 0
#define CFG_TUD_HID                 3  // Increased to support multiple HID interfaces
#define CFG_TUD_MIDI                0
#define CFG_TUD_VENDOR              0

//--------------------------------------------------------------------
// HID CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUD_HID_EP_BUFSIZE      64
#define CFG_TUD_HID_EP_IN_BUFSIZE   64
#define CFG_TUD_HID_EP_OUT_BUFSIZE  16

//--------------------------------------------------------------------
// CDC CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUD_CDC_RX_BUFSIZE      64
#define CFG_TUD_CDC_TX_BUFSIZE      64

//--------------------------------------------------------------------
// DEBUG CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUSB_DEBUG              2  // Enable debug logging for enumeration issues

//--------------------------------------------------------------------
// CLASS DRIVER CONFIGURATION
//--------------------------------------------------------------------

// HID Report Descriptor
#define CFG_TUD_HID_EPIN           0x81
#define CFG_TUD_HID_EPOUT          0x01

// CDC
#define CFG_TUD_CDC_EPIN           0x82
#define CFG_TUD_CDC_EPOUT          0x02
#define CFG_TUD_CDC_EPIN_NOTIFY    0x83

#endif /* TUSB_CONFIG_H_ */ 