//////////////////////////////////////////////////////////////////////////////
//
// v32kbd-pico: hardware v32kbd adapter for Waveshare RP2350-USB-A
// ----------------------------------------------------------------------
// A USB keyboard is plugged into the board's type A port (USB host, done
// with PIO on GPIO 12/13). The type C port is plugged into the computer,
// which sees an 11-button USB gamepad named "v32kbd". Key events from the
// keyboard are reported through those buttons using the v32kbd protocol
// (see v32kbd.h), so Vircon32 programs can read them with keyboard.h
//
//////////////////////////////////////////////////////////////////////////////

#include <string.h>

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "tusb.h"
#include "pio_usb.h"
#include "v32kbd.h"
#include "led.h"

//////////////////////////////////////////////////////////////////////////////
//
// Timing. The emulator applies gamepad changes whenever they arrive, and a
// program reads them once per frame (16.7 ms), so every event has to stay
// unchanged for longer than a frame or it could be missed.
//
// Each key event is sent in 2 steps: first the key code and action, and
// SETTLE_MS later the strobe. That way, even if a report was ever applied
// across 2 frames, the strobe can't be seen before its key code.
//
#define SETTLE_MS       8    // from key code to strobe
#define HOLD_MS        34    // from strobe to the next event (2 frames)
#define QUEUE_SIZE    128    // pending key events (power of 2)

//////////////////////////////////////////////////////////////////////////////
//
// Setup mode, toggled with the Scroll Lock key. In this mode keys F1 to
// F11 are plain buttons 1 to 11, held for as long as the key is. This is
// only meant for creating the joystick profile in Vircon32's EditControls,
// which asks to press each control on its own.
//
static bool setup_mode = false;

// number of keyboards currently plugged in and ready
static int keyboards_online = 0;

//////////////////////////////////////////////////////////////////////////////
//
// Queue of pending key events
//
typedef struct
{
    uint8_t code;
    bool    pressed;
}
key_event_t;

static key_event_t queue[ QUEUE_SIZE ];
static unsigned    queue_head = 0;      // next to read
static unsigned    queue_tail = 0;      // next to write

static void queue_push( uint8_t code, bool pressed )
{
    unsigned next = (queue_tail + 1) & (QUEUE_SIZE - 1);

    // when full, discard the oldest event
    if( next == queue_head )
      queue_head = (queue_head + 1) & (QUEUE_SIZE - 1);

    queue[ queue_tail ].code    = code;
    queue[ queue_tail ].pressed = pressed;
    queue_tail = next;
}

static bool queue_pop( key_event_t* event )
{
    if( queue_head == queue_tail )
      return false;

    *event = queue[ queue_head ];
    queue_head = (queue_head + 1) & (QUEUE_SIZE - 1);
    return true;
}

//////////////////////////////////////////////////////////////////////////////
//
// Gamepad state
//
static uint16_t buttons      = 0;       // state we want the PC to see
static uint16_t buttons_sent = 0;       // state last sent to the PC
static bool     report_sent  = false;   // nothing was sent yet

static uint32_t millis( void )
{
    return to_ms_since_boot( get_absolute_time() );
}

//////////////////////////////////////////////////////////////////////////////
//
// Handling of key changes coming from the keyboard
//
static void key_changed( uint8_t usage, bool pressed )
{
    // scroll lock toggles setup mode
    if( usage == HID_KEY_SCROLL_LOCK )
    {
        if( pressed )
        {
            setup_mode = !setup_mode;

            // leave all buttons released and discard pending events;
            // the strobe restarts from Left, as when just plugged
            buttons = 0;
            queue_head = queue_tail;
        }

        return;
    }

    if( setup_mode )
    {
        if( usage >= HID_KEY_F1 && usage < HID_KEY_F1 + V32BTN_COUNT )
        {
            uint16_t mask = (uint16_t)( 1u << (usage - HID_KEY_F1) );

            if( pressed ) buttons |=  mask;
            else          buttons &= (uint16_t)~mask;
        }

        return;
    }

    uint8_t code = v32kbd_keycode( usage );

    if( code != V32KEY_NONE )
      queue_push( code, pressed );
}

//////////////////////////////////////////////////////////////////////////////
//
// Turns pending key events into gamepad states, one at a time
//
static void protocol_task( void )
{
    enum { IDLE, SETTLING, HOLDING };

    static int      state = IDLE;
    static uint32_t since = 0;
    uint32_t now = millis();

    if( setup_mode )
    {
        state = IDLE;
        return;
    }

    switch( state )
    {
        case IDLE:
        {
            key_event_t event;

            if( !queue_pop( &event ) )
              break;

            // step 1: key code and action, strobe is not changed
            uint16_t strobe = buttons & (V32BTN_LEFT | V32BTN_RIGHT);

            buttons = (uint16_t)( strobe
                    | (event.pressed? V32BTN_UP : V32BTN_DOWN)
                    | ((uint16_t)event.code << V32BTN_CODE_SHIFT) );

            state = SETTLING;
            since = now;
            break;
        }

        case SETTLING:
        {
            // wait until step 1 was actually sent, plus the settle time
            if( buttons != buttons_sent )
            {
                since = now;
                break;
            }

            if( now - since < SETTLE_MS )
              break;

            // step 2: switch the strobe side (first event is Left)
            if( buttons & V32BTN_LEFT )
              buttons = (uint16_t)( (buttons & ~V32BTN_LEFT) | V32BTN_RIGHT );
            else
              buttons = (uint16_t)( (buttons & ~V32BTN_RIGHT) | V32BTN_LEFT );

            state = HOLDING;
            since = now;
            break;
        }

        case HOLDING:
        {
            if( buttons != buttons_sent )
            {
                since = now;
                break;
            }

            if( now - since >= HOLD_MS )
              state = IDLE;

            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Sends the gamepad state to the PC when it has changed
//
static void report_task( void )
{
    if( !tud_mounted() )
    {
        // the PC knows nothing of our state: begin again when it does
        report_sent = false;
        return;
    }

    if( report_sent && buttons == buttons_sent )
      return;

    if( !tud_hid_ready() )
      return;

    v32kbd_report_t report = { .buttons = buttons, .x = 0, .y = 0 };

    if( tud_hid_report( 0, &report, sizeof(report) ) )
    {
        buttons_sent = buttons;
        report_sent  = true;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Main
//
int main( void )
{
    // PIO USB needs a system clock that is a multiple of 12 MHz
    set_sys_clock_khz( 120000, true );

    // native USB port: device (gamepad)
    tusb_rhport_init_t device_init = { .role = TUSB_ROLE_DEVICE, .speed = TUSB_SPEED_AUTO };
    tusb_init( BOARD_TUD_RHPORT, &device_init );

    // PIO USB port: host (keyboard); D+ is GPIO 12 and D- is GPIO 13
    pio_usb_configuration_t pio_config = PIO_USB_DEFAULT_CONFIG;
    pio_config.pin_dp = PICO_DEFAULT_PIO_USB_DP_PIN;
    tuh_configure( BOARD_TUH_RHPORT, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_config );

    tusb_rhport_init_t host_init = { .role = TUSB_ROLE_HOST, .speed = TUSB_SPEED_AUTO };
    tusb_init( BOARD_TUH_RHPORT, &host_init );

    // status light (after the system clock is set)
    led_init();

    while( true )
    {
        tuh_task();
        tud_task();
        protocol_task();
        report_task();
        led_task( keyboards_online > 0, setup_mode );
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// USB device callbacks (gamepad side)
//
uint16_t tud_hid_get_report_cb( uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen )
{
    (void) instance; (void) report_id; (void) report_type;

    v32kbd_report_t report = { .buttons = buttons_sent, .x = 0, .y = 0 };

    if( reqlen < sizeof(report) )
      return 0;

    memcpy( buffer, &report, sizeof(report) );
    return sizeof(report);
}

void tud_hid_set_report_cb( uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize )
{
    (void) instance; (void) report_id; (void) report_type; (void) buffer; (void) bufsize;
}

//////////////////////////////////////////////////////////////////////////////
//
// USB host callbacks (keyboard side)
//

// last report of each keyboard, to find which keys have changed
typedef struct
{
    bool                  used;
    uint8_t               dev_addr, instance;
    hid_keyboard_report_t last;
}
keyboard_t;

#define MAX_KEYBOARDS 4
static keyboard_t keyboards[ MAX_KEYBOARDS ];

static keyboard_t* find_keyboard( uint8_t dev_addr, uint8_t instance, bool create )
{
    for( int i = 0; i < MAX_KEYBOARDS; i++ )
      if( keyboards[i].used && keyboards[i].dev_addr == dev_addr && keyboards[i].instance == instance )
        return &keyboards[i];

    if( !create ) return NULL;

    for( int i = 0; i < MAX_KEYBOARDS; i++ )
      if( !keyboards[i].used )
      {
          memset( &keyboards[i], 0, sizeof(keyboard_t) );
          keyboards[i].used     = true;
          keyboards[i].dev_addr = dev_addr;
          keyboards[i].instance = instance;
          return &keyboards[i];
      }

    return NULL;
}

static bool report_has_key( const hid_keyboard_report_t* report, uint8_t usage )
{
    for( int i = 0; i < 6; i++ )
      if( report->keycode[i] == usage )
        return true;

    return false;
}

static void process_keyboard_report( keyboard_t* keyboard, const hid_keyboard_report_t* report )
{
    // when too many keys are pressed keyboards report an
    // error in all positions: ignore those reports
    if( report->keycode[0] == 1 )
      return;

    const hid_keyboard_report_t* last = &keyboard->last;

    // modifier keys: 8 bits that are usages 0xE0 to 0xE7. Process
    // them first so that shift arrives before the key it applies to
    uint8_t changed = report->modifier ^ last->modifier;

    for( int bit = 0; bit < 8; bit++ )
      if( changed & (1 << bit) )
        key_changed( (uint8_t)( HID_KEY_CONTROL_LEFT + bit ), report->modifier & (1 << bit) );

    // released keys: they were in the last report but not in this one
    for( int i = 0; i < 6; i++ )
    {
        uint8_t usage = last->keycode[i];

        if( usage > 3 && !report_has_key( report, usage ) )
          key_changed( usage, false );
    }

    // pressed keys: they are in this report but not in the last one
    for( int i = 0; i < 6; i++ )
    {
        uint8_t usage = report->keycode[i];

        if( usage > 3 && !report_has_key( last, usage ) )
          key_changed( usage, true );
    }

    keyboard->last = *report;
}

// a HID interface was connected: we only care for keyboards. These
// are used in boot protocol (TinyUSB's default), so their reports
// always have the standard 8-byte format
void tuh_hid_mount_cb( uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len )
{
    (void) desc_report; (void) desc_len;

    if( tuh_hid_interface_protocol( dev_addr, instance ) != HID_ITF_PROTOCOL_KEYBOARD )
      return;

    if( find_keyboard( dev_addr, instance, true ) )
    {
        keyboards_online++;
        tuh_hid_receive_report( dev_addr, instance );
    }
}

// keyboard unplugged: release all the keys it had pressed
void tuh_hid_umount_cb( uint8_t dev_addr, uint8_t instance )
{
    keyboard_t* keyboard = find_keyboard( dev_addr, instance, false );
    if( !keyboard ) return;

    hid_keyboard_report_t empty;
    memset( &empty, 0, sizeof(empty) );
    process_keyboard_report( keyboard, &empty );
    keyboard->used = false;
    keyboards_online--;

    // with no keyboard left there is no way to leave setup mode,
    // so go back to normal operation with all buttons released
    if( keyboards_online <= 0 && setup_mode )
    {
        keyboards_online = 0;
        setup_mode = false;
        buttons = 0;
        queue_head = queue_tail;
    }
}

void tuh_hid_report_received_cb( uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len )
{
    keyboard_t* keyboard = find_keyboard( dev_addr, instance, false );
    if( !keyboard ) return;

    if( len >= sizeof(hid_keyboard_report_t) )
    {
        hid_keyboard_report_t keys;
        memcpy( &keys, report, sizeof(keys) );
        process_keyboard_report( keyboard, &keys );
    }

    // keep receiving reports
    tuh_hid_receive_report( dev_addr, instance );
}
