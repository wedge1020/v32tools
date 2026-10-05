#ifndef V32KBD_H_
#define V32KBD_H_

#include <stdint.h>

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd protocol: a keyboard seen by Vircon32 as a gamepad.
//
// The gamepad has 11 buttons, in the same order as the console's INP ports
// (0x402 to 0x40C), so the buttons word is what a program reads from them:
//
//   button  0: Left   \  strobe: every key event switches sides,
//   button  1: Right  /  beginning by Left
//   button  2: Up     -> the key was pressed
//   button  3: Down   -> the key was released
//   button  4: Start  -> key code, bit 0
//   button  5: A      -> key code, bit 1
//   button  6: B      -> key code, bit 2
//   button  7: X      -> key code, bit 3
//   button  8: Y      -> key code, bit 4
//   button  9: L      -> key code, bit 5
//   button 10: R      -> key code, bit 6
//
#define V32BTN_LEFT       (1u << 0)
#define V32BTN_RIGHT      (1u << 1)
#define V32BTN_UP         (1u << 2)
#define V32BTN_DOWN       (1u << 3)
#define V32BTN_CODE_SHIFT 4
#define V32BTN_COUNT      11

// key codes for keys with no ASCII character (same as the emulator)
#define V32KEY_NONE        0
#define V32KEY_UP          1
#define V32KEY_DOWN        2
#define V32KEY_LEFT        3
#define V32KEY_RIGHT       4
#define V32KEY_CAPSLOCK    5
#define V32KEY_LSHIFT      6
#define V32KEY_RSHIFT      7
#define V32KEY_BACKSPACE   8
#define V32KEY_TAB         9
#define V32KEY_LCTRL      10
#define V32KEY_RCTRL      11
#define V32KEY_LALT       12
#define V32KEY_ENTER      13
#define V32KEY_F1         14   // F1 to F12 are 14 to 25
#define V32KEY_RALT       26
#define V32KEY_ESCAPE     27
#define V32KEY_LGUI       28
#define V32KEY_RGUI       29
#define V32KEY_DELETE    127

// converts a USB HID keyboard usage (page 7) into a v32kbd
// key code; returns V32KEY_NONE for keys that are not supported
uint8_t v32kbd_keycode( uint8_t hid_usage );

// report sent to the PC: 11 buttons, plus 2 axes that never move
// (they are only there so that every OS takes this for a gamepad)
typedef struct __attribute__((packed))
{
    uint16_t buttons;
    int8_t   x, y;
}
v32kbd_report_t;

#endif
