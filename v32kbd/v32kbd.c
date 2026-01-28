#include "keyboard.h"

/*
| `IN`  | `0x406` | `INP_GamepadButtonStart` | `Start` button input (ENTER)         |
| `IN`  | `0x407` | `INP_GamepadButtonA`     | `A` button input (X on keyboard)     |
| `IN`  | `0x408` | `INP_GamepadButtonB`     | `B` button input (Z on keyboard)     |
| `IN`  | `0x409` | `INP_GamepadButtonX`     | `X` key input (S on keyboard)        |
| `IN`  | `0x40A` | `INP_GamepadButtonY`     | `Y` key input (A on keyboard)        |
| `IN`  | `0x40B` | `INP_GamepadButtonL`     | `L` key input (Q on keyboard)        |
| `IN`  | `0x40C` | `INP_GamepadButtonR`     | `R` key input (W on keyboard)        |
*/

void  v32kbd_read (v32kbd *keyboard)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Declare and initialize local variables
    //
    int  keyval             = 0;
    int  port               = 0x000;
    int  state              = 0;
    int *time               = NULL;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Only process for an established keyboard instance
    //
    if (keyboard           != NULL)
    {
        ////////////////////////////////////////////////////////////////////////////////
        //
        // Check that enough time has elapsed (current frame must be above that of
        // the last frame the keyboard input was processed)
        //
        state               = get_frame_counter ();
        if (state          >  keyboard -> lastread)
        {
            ////////////////////////////////////////////////////////////////////////////
            //
            // Select the gamepad slot associated with the keyboard
            //
            select_gamepad (keyboard -> gamepad);

            ////////////////////////////////////////////////////////////////////////////
            //
            // Cycle through the gamepad buttons via ports in assembly
            //
            for (port       = 0x406;  // INP_GamepadButtonStart
                 port      <= 0x40C;  // INP_GamepadButtonR
                 port       = port + 1)
            {
                ////////////////////////////////////////////////////////////////////////
                //
                // In assembly, read the current button, storing its value into state
                //
                asm
                {
                    "in    R0,      {port}"
                    "mov   {state}, R0"
                }

                ////////////////////////////////////////////////////////////////////////
                //
                // In assembly, read the current button, storing its value into
                // keyval, which can be looked up to identify the current key
                // input
                //
                if (state  >  0) // positive means pressed
                {
                    keyval  = key | (1 << (port - 0x406));
                }
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // If keyval is positive, a key has been pressed
            //
            if (keyval     >  0)
            {
                v32kbd_addkey (keyboard, keyval);
                time        = keyboard -> lastread;
                time        = get_frame_counter ();
            }
        }
    }
}

/* keyboard.h

struct v32key
{
    int     value;  // the key pressed
    v32key *next;
};

struct v32kbd
{
    v32key *input;     // start of list (next available key input)
    v32key *data;      // end of the list (new keys appended)
    int     gamepad;   // id of gamepad being used as a v32 keyboard
    int     lastread;  // contains frame of last read (for timing)    
};

v32key *v32kbd_newkey (void);     // allocate new v32key struct for list
int     v32kbd_addkey (v32key *); // add new key input to keyboard list
void    v32kbd_read   (v32kbd *); // check for input

*/
