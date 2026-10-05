#include "v32kbd.h"
#include "tusb.h"

// HID keyboard usages have the same values as SDL scancodes,
// so this is the same table used by the emulator's v32kbd device
uint8_t v32kbd_keycode( uint8_t usage )
{
    // ranges of consecutive keys
    if( usage >= HID_KEY_A && usage <= HID_KEY_Z )
      return (uint8_t)( 'a' + (usage - HID_KEY_A) );

    if( usage >= HID_KEY_1 && usage <= HID_KEY_9 )
      return (uint8_t)( '1' + (usage - HID_KEY_1) );

    if( usage >= HID_KEY_F1 && usage <= HID_KEY_F12 )
      return (uint8_t)( V32KEY_F1 + (usage - HID_KEY_F1) );

    if( usage >= HID_KEY_KEYPAD_1 && usage <= HID_KEY_KEYPAD_9 )
      return (uint8_t)( '1' + (usage - HID_KEY_KEYPAD_1) );

    switch( usage )
    {
        // keys with an ASCII character (US layout, no shift)
        case HID_KEY_0:               return '0';
        case HID_KEY_SPACE:           return ' ';
        case HID_KEY_GRAVE:           return '`';
        case HID_KEY_MINUS:           return '-';
        case HID_KEY_EQUAL:           return '=';
        case HID_KEY_BRACKET_LEFT:    return '[';
        case HID_KEY_BRACKET_RIGHT:   return ']';
        case HID_KEY_BACKSLASH:       return '\\';
        case HID_KEY_EUROPE_1:        return '\\';
        case HID_KEY_SEMICOLON:       return ';';
        case HID_KEY_APOSTROPHE:      return '\'';
        case HID_KEY_COMMA:           return ',';
        case HID_KEY_PERIOD:          return '.';
        case HID_KEY_SLASH:           return '/';

        // keys with an ASCII control code
        case HID_KEY_BACKSPACE:       return V32KEY_BACKSPACE;
        case HID_KEY_TAB:             return V32KEY_TAB;
        case HID_KEY_ENTER:           return V32KEY_ENTER;
        case HID_KEY_ESCAPE:          return V32KEY_ESCAPE;
        case HID_KEY_DELETE:          return V32KEY_DELETE;

        // keys with no ASCII code
        case HID_KEY_ARROW_UP:        return V32KEY_UP;
        case HID_KEY_ARROW_DOWN:      return V32KEY_DOWN;
        case HID_KEY_ARROW_LEFT:      return V32KEY_LEFT;
        case HID_KEY_ARROW_RIGHT:     return V32KEY_RIGHT;
        case HID_KEY_CAPS_LOCK:       return V32KEY_CAPSLOCK;
        case HID_KEY_SHIFT_LEFT:      return V32KEY_LSHIFT;
        case HID_KEY_SHIFT_RIGHT:     return V32KEY_RSHIFT;
        case HID_KEY_CONTROL_LEFT:    return V32KEY_LCTRL;
        case HID_KEY_CONTROL_RIGHT:   return V32KEY_RCTRL;
        case HID_KEY_ALT_LEFT:        return V32KEY_LALT;
        case HID_KEY_ALT_RIGHT:       return V32KEY_RALT;
        case HID_KEY_GUI_LEFT:        return V32KEY_LGUI;
        case HID_KEY_GUI_RIGHT:       return V32KEY_RGUI;

        // numeric keypad: same codes as the equivalent main keys
        case HID_KEY_KEYPAD_0:        return '0';
        case HID_KEY_KEYPAD_DECIMAL:  return '.';
        case HID_KEY_KEYPAD_DIVIDE:   return '/';
        case HID_KEY_KEYPAD_SUBTRACT: return '-';
        case HID_KEY_KEYPAD_ENTER:    return V32KEY_ENTER;

        default:                      return V32KEY_NONE;
    }
}
