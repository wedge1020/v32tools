#include "tusb.h"
#include "v32kbd.h"

// There is no official USB ID for this: these are the IDs TinyUSB uses for
// its examples, which is fine for personal use. Change them if they clash.
#define USB_VID   0xCAFE
#define USB_PID   0x4B32

// ---------------- device descriptor
static tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

uint8_t const* tud_descriptor_device_cb( void )
{
    return (uint8_t const*) &desc_device;
}

// ---------------- HID report descriptor: gamepad with 11 buttons + X/Y
static uint8_t const desc_hid_report[] =
{
    HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     ),
    HID_USAGE      ( HID_USAGE_DESKTOP_GAMEPAD  ),
    HID_COLLECTION ( HID_COLLECTION_APPLICATION ),

      // 11 buttons
      HID_USAGE_PAGE   ( HID_USAGE_PAGE_BUTTON ),
      HID_USAGE_MIN    ( 1                     ),
      HID_USAGE_MAX    ( V32BTN_COUNT          ),
      HID_LOGICAL_MIN  ( 0                     ),
      HID_LOGICAL_MAX  ( 1                     ),
      HID_REPORT_SIZE  ( 1                     ),
      HID_REPORT_COUNT ( V32BTN_COUNT          ),
      HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),

      // padding up to 16 bits
      HID_REPORT_SIZE  ( 16 - V32BTN_COUNT     ),
      HID_REPORT_COUNT ( 1                     ),
      HID_INPUT        ( HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE ),

      // X and Y axes (always centered)
      HID_USAGE_PAGE   ( HID_USAGE_PAGE_DESKTOP ),
      HID_USAGE        ( HID_USAGE_DESKTOP_X    ),
      HID_USAGE        ( HID_USAGE_DESKTOP_Y    ),
      HID_LOGICAL_MIN  ( 0x81                   ),   // -127
      HID_LOGICAL_MAX  ( 0x7F                   ),   //  127
      HID_REPORT_SIZE  ( 8                      ),
      HID_REPORT_COUNT ( 2                      ),
      HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),

    HID_COLLECTION_END
};

uint8_t const* tud_hid_descriptor_report_cb( uint8_t instance )
{
    (void) instance;
    return desc_hid_report;
}

// ---------------- configuration descriptor
#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#define EPNUM_HID         0x81

static uint8_t const desc_configuration[] =
{
    // config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR( 1, 1, 0, CONFIG_TOTAL_LEN, 0x00, 500 ),

    // interface number, string index, protocol, report descriptor len, EP In address, size & polling interval (ms)
    TUD_HID_DESCRIPTOR( 0, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 1 )
};

uint8_t const* tud_descriptor_configuration_cb( uint8_t index )
{
    (void) index;
    return desc_configuration;
}

// ---------------- string descriptors
static char const* const string_desc[] =
{
    (const char[]) { 0x09, 0x04 },  // 0: supported language is English
    "v32tools",                     // 1: Manufacturer
    "v32kbd",                       // 2: Product
    "000001",                       // 3: Serial
};

static uint16_t desc_str[ 32 + 1 ];

uint16_t const* tud_descriptor_string_cb( uint8_t index, uint16_t langid )
{
    (void) langid;
    size_t chr_count;

    if( index == 0 )
    {
        memcpy( &desc_str[1], string_desc[0], 2 );
        chr_count = 1;
    }
    else
    {
        if( index >= sizeof(string_desc) / sizeof(string_desc[0]) )
          return NULL;

        const char* str = string_desc[ index ];
        chr_count = strlen( str );
        if( chr_count > 32 ) chr_count = 32;

        for( size_t i = 0; i < chr_count; i++ )
          desc_str[ 1 + i ] = str[ i ];
    }

    // first word is length (including header) and string type
    desc_str[0] = (uint16_t)( (TUSB_DESC_STRING << 8) | (2 * chr_count + 2) );
    return desc_str;
}
