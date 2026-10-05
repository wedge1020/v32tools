#ifndef _KEYBOARD_H
#define _KEYBOARD_H

//////////////////////////////////////////////////////////////////////////////
//
// v32tools/v32kbd library: "keyboard.h"         File version: 2026/10/05
// ----------------------------------------------------------------------
// This file contains all definitions and functions needed by programs to
// access input from a keyboard-attached gamepad, allowing keyboard input
// in Vircon32 programs.
//
// A v32kbd device looks like a regular gamepad to the console,  but its
// 11 controls are used to report key events instead:
//
// PORT: 0x402 -> INP_GamepadLeft        \  strobe: each new key event
//       0x403 -> INP_GamepadRight       /  switches sides (Left first)
//       0x404 -> INP_GamepadUp          -> key was pressed
//       0x405 -> INP_GamepadDown        -> key was released
//       0x406 -> INP_GamepadButtonStart -> key code, bit 0
//       0x407 -> INP_GamepadButtonA     -> key code, bit 1
//       0x408 -> INP_GamepadButtonB     -> key code, bit 2
//       0x409 -> INP_GamepadButtonX     -> key code, bit 3
//       0x40A -> INP_GamepadButtonY     -> key code, bit 4
//       0x40B -> INP_GamepadButtonL     -> key code, bit 5
//       0x40C -> INP_GamepadButtonR     -> key code, bit 6
//
// The device reports at most 1 key event per frame,  and it keeps  that
// state until the next event.  So a new event is recognized by the side
// of the strobe (Left or Right) being different from the last one seen.
//
// Key codes are 7 bits, and identify KEYS (not characters): keys with an
// ASCII character report it as typed with no shift on a US layout  ('a'
// to 'z', '0' to '9', space and ` - = [ ] \ ; ' , . /).  Other keys use
// the V32KEY_ codes defined below.  Shift and caps lock are applied  by
// this library, to obtain the typed character (symbol) for each key.
//
// The design of v32kbd is such that allows for multiple instances, if it
// is desired to have more than one keyboard present.
//
// IMPORTANT: v32kbd_probe() must be called once on EVERY frame for each
// keyboard. If frames are skipped, key events can be lost.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// include needed Vircon32 DevTools headers
//
#include "input.h"
#include "misc.h"
#include "time.h"

//////////////////////////////////////////////////////////////////////////////
//
// key codes for the keys that have no ASCII character of their own
//
#define V32KEY_NONE         0
#define V32KEY_UP           1
#define V32KEY_DOWN         2
#define V32KEY_LEFT         3
#define V32KEY_RIGHT        4
#define V32KEY_CAPSLOCK     5
#define V32KEY_LSHIFT       6
#define V32KEY_RSHIFT       7
#define V32KEY_BACKSPACE    8
#define V32KEY_TAB          9
#define V32KEY_LCTRL       10
#define V32KEY_RCTRL       11
#define V32KEY_LALT        12  // left option on Mac
#define V32KEY_ENTER       13
#define V32KEY_F1          14  // F1 to F12 are consecutive: 14 to 25
#define V32KEY_F12         25
#define V32KEY_RALT        26  // right option on Mac
#define V32KEY_ESCAPE      27
#define V32KEY_LGUI        28  // left command on Mac, Windows key on PC
#define V32KEY_RGUI        29  // right command on Mac
#define V32KEY_DELETE     127

//////////////////////////////////////////////////////////////////////////////
//
// library parameters
//
#define V32KBD_KEYS       128  // number of possible key codes
#define V32KBD_MAXQUEUE    64  // max. events waiting to be read
#define V32KBD_ROUTINE     68  // size (in words) of the in-RAM routine

//////////////////////////////////////////////////////////////////////////////
//
// the v32key struct stores a single key event,  in the v32kbd linked list
// of input values
//
struct v32key
{
    int     value;    // key code of the key (see V32KEY_ definitions)
    int     symbol;   // typed character (shift applied); same as value
                      // for keys that have no character
    bool    pressed;  // true: key was pressed, false: it was released
    v32key *next;
};

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd is the keyboard instances, bound to a gameport
//
struct v32kbd
{
    v32key *input;     // start of list (next available key input)
    v32key *data;      // end of the list (new keys appended)
    int     count;     // number of key events in the list
    int     gamepad;   // id of gamepad being used as a v32 keyboard
    int     lastread;  // contains frame of last received key event
    int     strobe;    // last strobe side seen (0: none, 1: left, 2: right)
    int     raw;       // the in-RAM routine stores its result here
    bool    capslock;  // caps lock state (toggles when it is pressed)
    bool [V32KBD_KEYS]    held;     // which keys are currently pressed
    int  [V32KBD_ROUTINE] routine;  // in-RAM custom machine code routine
};

//////////////////////////////////////////////////////////////////////////////
//
// API: function prototypes for the functions in this library
//
v32key *v32key_newkey    (int      keyval);                // allocate new v32key
v32kbd *v32kbd_init      (int      gamepad);               // initialize keyboard
void    v32kbd_free      (v32kbd **keyboard);              // release a keyboard
int     v32kbd_addkey    (v32kbd **keyboard, v32key *key); // add new key to list
v32key *v32kbd_getkey    (v32kbd **keyboard);              // obtain next key
int     v32kbd_scan      (v32kbd  *keyboard);              // run in-RAM routine
int     v32kbd_symbol    (v32kbd  *keyboard, int keyval);  // apply shift to key
bool    v32kbd_probe     (v32kbd **keyboard);              // check for input
int     v32kbd_read      (v32kbd **keyboard);              // read next key press
int     v32kbd_readevent (v32kbd **keyboard, bool *pressed); // read next event
bool    v32kbd_isdown    (v32kbd **keyboard, int keyval);  // is key held now?

//////////////////////////////////////////////////////////////////////////////
//
// v32key_newkey(): allocate a new v32key for the v32kbd input list
//
v32key *v32key_newkey (int  keyval)
{
    v32key *newkey        = NULL;

    newkey                = (v32key *) malloc (sizeof (v32key) * 1);
    if (newkey           != NULL)
    {
        newkey -> value   = keyval;
        newkey -> symbol  = keyval;
        newkey -> pressed = true;
        newkey -> next    = NULL;
    }

    return (newkey);
}

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd_scan(): select the keyboard's gamepad and run its in-RAM routine,
// which reads the 11 gamepad control ports and packs them as bits:
//
//   bit 0: left    bit 1: right    bit 2: up    bit 3: down
//   bits 4 to 10:  key code (start, a, b, x, y, l, r)
//
// The previously selected gamepad is restored afterwards
//
int  v32kbd_scan (v32kbd *keyboard)
{
    int  previous         = 0;
    int  offset           = 0;

    previous              = get_selected_gamepad ();
    select_gamepad (keyboard -> gamepad);

    //////////////////////////////////////////////////////////////////////////
    //
    // Call the routine
    //
    offset                = (int) &(keyboard -> routine[0]);
    asm
    {
        "PUSH  R0"
        "MOV   R0, {offset}"
        "CALL  R0"
        "POP   R0"
    }

    select_gamepad (previous);

    return (keyboard -> raw);
}

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd_init(): initialize new v32kbd instance (associate with gamepad)
//
v32kbd *v32kbd_init (int  gamepad)
{
    v32kbd *keyboard          = NULL;
    int    *routine           = NULL;
    int     index             = 0;
    int     offset            = 0;
    int     port              = 0x000;

    keyboard                  = (v32kbd *) malloc (sizeof (v32kbd) * 1);
    if (keyboard             != NULL)
    {
        keyboard -> input     = NULL;
        keyboard -> data      = NULL;
        keyboard -> count     = 0;
        keyboard -> gamepad   = gamepad;
        keyboard -> lastread  = -1;
        keyboard -> raw       = 0;
        keyboard -> capslock  = false;

        for (index            = 0;
             index           <  V32KBD_KEYS;
             index            = index + 1)
        {
            keyboard -> held[index]  = false;
        }

        //////////////////////////////////////////////////////////////////////
        //
        // Cycle through the gamepad controls via ports in assembly;
        // due to how the compiler processes inline assembly, we're
        // generating an in-RAM custom routine with exactly what is
        // needed to pack all of them as bits of a single value.
        //
        // The routine is generated only once and it is kept within
        // the keyboard instance, so it is released along with it.
        //
        routine               = &(keyboard -> routine[0]);
        routine[0]            = 0x54000000;        // PUSH R0
        routine[1]            = 0x54200000;        // PUSH R1
        routine[2]            = 0x54400000;        // PUSH R2
        routine[3]            = 0x4E200000;        // MOV R1, 0
        routine[4]            = 0x00000000;        // immediate
        routine[5]            = 0x4C424000;        // MOV R2, R1
        for (index            = 0;
             index           <  11;
             index            = index + 1)
        {
            offset            = ((index * 5) + 6);
            port              = index + 2;         // 0x402 to 0x40C

            routine[offset]   = 0x5C000400 | port; // IN  R0, port
            routine[offset+1] = 0x24040000;        // IGT R0, R2
            routine[offset+2] = 0x96000000;        // SHL R0, index
            routine[offset+3] = index;             // immediate value
            routine[offset+4] = 0x88200000;        // OR  R1, R0
        }

        offset                = (index * 5) + 6;
        routine[offset]       = 0x4E034000;        // MOV [raw], R1
        routine[offset+1]     = (int) &(keyboard -> raw);
        routine[offset+2]     = 0x58400000;        // POP R2
        routine[offset+3]     = 0x58200000;        // POP R1
        routine[offset+4]     = 0x58000000;        // POP R0
        routine[offset+5]     = 0x10000000;        // RET
        routine[offset+6]     = 0x00000000;        // HLT (for safety)

        //////////////////////////////////////////////////////////////////////
        //
        // The device keeps the state of its last event (even if the
        // console is reset), so take the current strobe side as our
        // starting point: this way an old event is not read as new
        //
        keyboard -> strobe    = v32kbd_scan (keyboard) & 3;
    }

    return (keyboard);
}

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd_free(): release a keyboard instance and any pending key events
//
void    v32kbd_free (v32kbd **keyboard)
{
    v32key *key               = NULL;

    if (*keyboard            != NULL)
    {
        key                   = v32kbd_getkey (keyboard);
        while (key           != NULL)
        {
            free (key);
            key               = v32kbd_getkey (keyboard);
        }

        free (*keyboard);
        *keyboard             = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd_addkey(): add new key input to keyboard list
//
int     v32kbd_addkey (v32kbd **keyboard, v32key *key)
{
    v32key *tmp                   = NULL;
    int     result                = 0;

    if ((*keyboard               != NULL) &&
        (key                     != NULL))
    {
        key             -> next   = NULL;
        if ((*keyboard) -> input == NULL)
        {
            (*keyboard) -> input  = key;
            (*keyboard) -> data   = key;
        }
        else
        {
            tmp                   = (*keyboard) -> data;
            tmp         -> next   = key;
            (*keyboard) -> data   = key;
        }
        (*keyboard)     -> count  = (*keyboard) -> count + 1;
        result                    = 1;
    }

    return (result);
}

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd_getkey(): obtain next key of input from the keyboard list
// (the caller becomes responsible of calling free() on the key)
//
v32key *v32kbd_getkey (v32kbd **keyboard)
{
    v32key *nextkey                  = NULL;

    if (*keyboard                   != NULL)
    {
        if ((*keyboard) -> input    != NULL)
        {
            nextkey                  = (*keyboard) -> input;
            (*keyboard) -> input     = nextkey     -> next;
            (*keyboard) -> count     = (*keyboard) -> count - 1;
            nextkey  -> next         = NULL;
            if ((*keyboard) -> input == NULL)
            {
                (*keyboard) -> data  = NULL;
            }
        }
    }

    return (nextkey);
}

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd_symbol(): obtain the typed character for a key,  applying  the
// current state of shift and caps lock (US layout).  Keys that have  no
// character return their same key code
//
int  v32kbd_symbol (v32kbd *keyboard, int  keyval)
{
    bool  shift          = false;
    int   symbol         = keyval;

    shift                = keyboard -> held[V32KEY_LSHIFT] ||
                           keyboard -> held[V32KEY_RSHIFT];

    //////////////////////////////////////////////////////////////////////////
    //
    // Letters: caps lock inverts the effect of shift
    //
    if ((keyval         >= 'a') &&
        (keyval         <= 'z'))
    {
        if (shift       != keyboard -> capslock)
        {
            symbol       = keyval - 32;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // Numbers and punctuation: only shift is applied
    //
    else if (shift      == true)
    {
        switch (keyval)
        {
            case '1':  symbol = '!';  break;
            case '2':  symbol = '@';  break;
            case '3':  symbol = '#';  break;
            case '4':  symbol = '$';  break;
            case '5':  symbol = '%';  break;
            case '6':  symbol = '^';  break;
            case '7':  symbol = '&';  break;
            case '8':  symbol = '*';  break;
            case '9':  symbol = '(';  break;
            case '0':  symbol = ')';  break;
            case '-':  symbol = '_';  break;
            case '=':  symbol = '+';  break;
            case '[':  symbol = '{';  break;
            case ']':  symbol = '}';  break;
            case '\\': symbol = '|';  break;
            case ';':  symbol = ':';  break;
            case '\'': symbol = '"';  break;
            case ',':  symbol = '<';  break;
            case '.':  symbol = '>';  break;
            case '/':  symbol = '?';  break;
            case '`':  symbol = '~';  break;
        }
    }

    return (symbol);
}

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd_probe(): check for new input, updating keyboard input list.
// Call it once every frame; it returns true if a key event was received
//
bool v32kbd_probe (v32kbd **keyboard)
{
    //////////////////////////////////////////////////////////////////////////
    //
    // Declare and initialize local variables
    //
    bool     result                = false;
    int      keyval                = 0;
    int      raw                   = 0;
    int      side                  = 0;
    v32kbd  *kbd                   = NULL;
    v32key  *key                   = NULL;

    //////////////////////////////////////////////////////////////////////////
    //
    // Only process for an established keyboard instance
    //
    if (*keyboard                  != NULL)
    {
        kbd                         = *keyboard;

        //////////////////////////////////////////////////////////////////////
        //
        // Read all gamepad controls with the in-RAM routine
        //
        raw                         = v32kbd_scan (kbd);

        //////////////////////////////////////////////////////////////////////
        //
        // There is a new key event when the strobe side has changed
        // (side is 1 for left,  2 for right,  0 before any events)
        //
        side                        = raw & 3;
        if ((side                  != 0) &&
            (side                  != kbd -> strobe))
        {
            kbd -> strobe           = side;
            kbd -> lastread         = get_frame_counter ();
            keyval                  = (raw >> 4) & 127;

            if (keyval             >  0)
            {
                result              = true;

                //////////////////////////////////////////////////////////////
                //
                // Keep track of which keys are held (up: key pressed)
                //
                kbd -> held[keyval] = ((raw & 4) != 0);

                if ((keyval        == V32KEY_CAPSLOCK) &&
                    (kbd -> held[keyval]))
                {
                    kbd -> capslock = !(kbd -> capslock);
                }

                //////////////////////////////////////////////////////////////
                //
                // Queue the event, unless nobody is reading them
                //
                if (kbd -> count   <  V32KBD_MAXQUEUE)
                {
                    key             = v32key_newkey (keyval);
                    if (key        != NULL)
                    {
                        key -> pressed  = kbd -> held[keyval];
                        key -> symbol   = v32kbd_symbol (kbd, keyval);
                        v32kbd_addkey (keyboard, key);
                    }
                }
            }
        }
    }

    return (result);
}

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd_readevent(): obtain next key event from keyboard input list (if
// available). Returns the key code (not the symbol), or 0 if there is no
// event. If pressed is not NULL, it is set to true (press) or false
// (release)
//
int  v32kbd_readevent (v32kbd **keyboard, bool *pressed)
{
    int     keyval                = 0;
    v32key *oldkey                = NULL;

    if (*keyboard                != NULL)
    {
        oldkey                    = v32kbd_getkey (keyboard);
        if (oldkey               != NULL)
        {
            keyval                = oldkey   -> value;
            if (pressed          != NULL)
            {
                *pressed          = oldkey   -> pressed;
            }
            free (oldkey);
            oldkey                = NULL;
        }
    }

    return (keyval);
}

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd_read(): obtain next key PRESS from keyboard input list, skipping
// any key releases. Returns the typed character (compatible with the BIOS
// font regions) or, for keys with no character, their key code. Returns 0
// when there are no more key presses to read
//
int  v32kbd_read  (v32kbd **keyboard)
{
    int     keyval                = 0;
    v32key *oldkey                = NULL;

    if (*keyboard                != NULL)
    {
        oldkey                    = v32kbd_getkey (keyboard);
        while ((oldkey           != NULL) &&
               (keyval           == 0))
        {
            if (oldkey -> pressed)
            {
                keyval            = oldkey   -> symbol;
            }
            free (oldkey);
            oldkey                = NULL;

            if (keyval           == 0)
            {
                oldkey            = v32kbd_getkey (keyboard);
            }
        }
    }

    return (keyval);
}

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd_isdown(): check if a key is currently held (use key codes here,
// like 'a' or V32KEY_LEFT). Useful for modifiers and for game controls
//
bool v32kbd_isdown (v32kbd **keyboard, int  keyval)
{
    bool    result                = false;

    if ((*keyboard               != NULL) &&
        (keyval                  >  0)    &&
        (keyval                  <  V32KBD_KEYS))
    {
        result                    = (*keyboard) -> held[keyval];
    }

    return (result);
}

#endif
