#ifndef _KEYBOARD_H
#define _KEYBOARD_H

//////////////////////////////////////////////////////////////////////////////
//
// v32tools/v32kbd library: "keyboard.h"         File version: 2026/01/28
// ----------------------------------------------------------------------
// This file contains all definitions and functions needed by programs to
// access input from a keyboard-attached gamepad, allowing keyboard input
// in Vircon32 programs.
//
// This includes global v32kbd parameters,  as well as specific functions
// functions to read the state of directions and buttons.
//
// It works by quantizing the byte of an input key from the REAL keyboard
// into individual bits, each bit corresponding to a pressed button state
// of the following Vircon32 gamepad buttons:
//
// PORT: 0x406 -> INP_GamepadButtonStart
//       0x407 -> INP_GamepadButtonA
//       0x408 -> INP_GamepadButtonB
//       0x409 -> INP_GamepadButtonX
//       0x40A -> INP_GamepadButtonY
//       0x40B -> INP_GamepadButtonL
//       0x40C -> INP_GamepadButtonR
//
// It transacts on the lower 7 bits of the byte,  allowing for up to  128
// distinct keys recognized as input.  This is more than enough to handle
// a standard 104-key keyboard.
//
// The aim is for the eventually-obtained key value (keyval) to map to  a
// region of the BIOS font for output,  or just to check for a particular
// key being pressed.
//
// The design of v32kbd is such that allows for multiple instances, if it
// is desired to have more than one keyboard present.
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
// the v32key struct  stores the  integer value of the  keypress,  in the
// v32kbd linked list of input values
//
struct v32key
{
    int     value;  // the key pressed
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
    int     gamepad;   // id of gamepad being used as a v32 keyboard
    int     lastread;  // contains frame of last read (for timing)    
};

//////////////////////////////////////////////////////////////////////////////
//
// API: function prototypes for the functions in this library
//
v32key *v32key_newkey (int     keyval);                // allocate new v32key
v32kbd *v32kbd_init   (int     gamepad);               // initialize keyboard
int     v32kbd_addkey (v32kbd *keyboard, v32key *key); // add new key to list
v32key *v32kbd_getkey (v32kbd *keyboard);              // obtain next key
bool    v32kbd_probe  (v32kbd *keyboard);              // check for input
int     v32kbd_read   (v32kbd *keyboard);              // read next input

//////////////////////////////////////////////////////////////////////////////
//
// v32key_newkey(): allocate a new v32key for the v32kbd input list
//
v32key *v32key_newkey (int  keyval)
{
    v32key *newkey       = NULL;

    newkey               = (v32key *) malloc (sizeof (v32key) * 1);
    if (newkey          != NULL)
    {
        newkey -> value  = keyval;
        newkey -> next   = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd_init(): initialize new v32kbd instance (associate with gamepad)
//
v32kbd *v32kbd_init (int  gamepad)
{
    v32kbd *keyboard          = NULL;

    keyboard                  = (v32kbd *) malloc (sizeof (v32kbd) * 1);
    if (keyboard             != NULL)
    {
        keyboard -> input     = NULL;
        keyboard -> data      = NULL;
        keyboard -> gamepad   = gamepad;
        keyboard -> lastread  = -1;
    }

    return (keyboard);
}

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd_addkey(): add new key input to keyboard list
//
int     v32kbd_addkey (v32kbd *keyboard, v32key *key)
{
    v32key *tmp                = NULL;

    if (keyboard              != NULL)
    {
        if (keyboard -> input == NULL)
        {
            keyboard -> input  = key;
            keyboard -> data   = key;
            key      -> next   = NULL;
        }
        else
        {
            tmp                = keyboard -> data;           
            tmp      -> next   = key;
            keyboard -> data   = key;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd_getkey(): obtain next key of input from the keyboard list
//
v32key *v32kbd_getkey (v32kbd *keyboard)
{
    v32key *nextkey               = NULL;

    if (keyboard                 != NULL)
    {
        if (keyboard -> input    != NULL)
        {
            nextkey               = keyboard -> input;
            keyboard -> input     = keyboard -> input -> next;
            nextkey  -> next      = NULL;
            if (nextkey          == keyboard -> data)
            {
                keyboard -> data  = NULL;
            }
        }
    }

    return (nextkey);
}

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd_probe(): check for new input, updating keyboard input list
//
bool v32kbd_probe (v32kbd *keyboard)
{
    //////////////////////////////////////////////////////////////////////////
    //
    // Declare and initialize local variables
    //
    bool     result                = false;
    int      index                 = 0;
    int      keyval                = 0;
    int      port                  = 0x000;
    int      offset                = 0;
    int [48] routine;                  // custom routine
    int      shift                 = 0;
    int      state                 = 0;
    int     *time                  = NULL;
    v32key  *key                   = NULL;

    //////////////////////////////////////////////////////////////////////////
    //
    // Only process for an established keyboard instance
    //
    if (keyboard                  != NULL)
    {
        //////////////////////////////////////////////////////////////////////
        //
        // Check that enough time has elapsed (current frame must be above
        // that of the last frame the keyboard input was processed)
        //
        state                      = get_frame_counter ();
        if (state                 >  keyboard -> lastread)
        {
            //////////////////////////////////////////////////////////////////
            //
            // Select the gamepad slot associated with the keyboard
            //
            select_gamepad (keyboard -> gamepad);

            //////////////////////////////////////////////////////////////////
            //
            // Cycle through the gamepad buttons via ports in assembly;
            // due to how the compiler processes inline assembly, we're
            // generating an in-RAM custom routine with exactly what is
            // needed to generate a byte of data.
            //
            // The routine array elements need to be packed  backwards,
            // due to being on the stack
            //
            routine[0]             = 0x54000000;        // PUSH R0
            routine[1]             = 0x54200000;        // PUSH R1
            routine[2]             = 0x54400000;        // PUSH R2
            routine[3]             = 0x4E200000;        // MOV R1, 0
            routine[4]             = 0x00000000;        // immediate
            routine[5]             = 0x4C424000;        // MOV R2, R1
            for (index             = 0;
                 index            <  7; 
                 index             = index + 1)
            {
                offset             = ((index * 5) + 6);
                port               = index + 6;
                shift              = 6 - index;
                
                routine[offset]    = 0x5C000400 | port; // IN  R0, port
                routine[offset+1]  = 0x24040000;        // IGT R0, R2
                routine[offset+2]  = 0x96000000;        // SHL R0, shift
                routine[offset+3]  = shift;             // immediate value
                routine[offset+4]  = 0x88200000;        // OR  R1, R0
            }

            offset                 = (index * 5) + 6;
            routine[offset]        = 0x4E034000;        // MOV [keyval], R1
            routine[offset+1]      = (int) &keyval;
            routine[offset+2]      = 0x58400000;        // POP R2
            routine[offset+3]      = 0x58200000;        // POP R1
            routine[offset+4]      = 0x58000000;        // POP R0
            routine[offset+5]      = 0x10000000;        // RET
            routine[offset+6]      = 0x00000000;        // HLT (for safety)
                
            //////////////////////////////////////////////////////////////////
            //
            // Call the routine
            //
            offset                 = (int) &routine[0];
            asm
            {
                "PUSH  R0"
                "MOV   R0, {offset}"
                "CALL  R0"
                "POP   R0"
            }
            
            //////////////////////////////////////////////////////////////////
            //
            // If keyval is positive, a key has been pressed
            //
            keyval                 = 0x41; // 'A'
            if (keyval            >  0)
            {
                key                = v32key_newkey (keyval);
                v32kbd_addkey (keyboard, key);
                time               = &(keyboard -> lastread);
                *time              = get_frame_counter ();
                result             = true;
            }
        }
    }

    return (result);
}

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd_read(): obtain input key from keyboard input list (if available)
//
int  v32kbd_read  (v32kbd *keyboard)
{
    bool    check                  = false;
    int     keyval                 = 0;
    v32key *oldkey                 = NULL;

    if (keyboard                  != NULL)
    {
        check                      = v32kbd_probe (keyboard);
        if (check                 == true)
        {
            if (keyboard -> input != NULL)
            {
                oldkey             = v32kbd_getkey (keyboard);
                keyval             = oldkey   -> value;
                free (oldkey);
                oldkey             = NULL;
            }
        }
    }

    return (keyval);
}    

#endif
