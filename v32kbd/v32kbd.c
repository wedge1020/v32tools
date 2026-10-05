#include "video.h"
#include "time.h"
#include "string.h"
#include "keyboard.h"

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd test program: type text on screen.
//
// The keyboard is expected in the SECOND gamepad port (id 1): in the
// emulator, select "v32kbd" in menu Gamepads > Gamepad 2
//
#define TEXT_MAX  600

void main (void)
{
    int             x         = 0;
    int             key       = 0;
    int             last      = 0;
    int             length    = 0;
    int [TEXT_MAX+2] text;
    int [12]        sym;
    v32kbd         *keyboard  = NULL;

    keyboard                  = v32kbd_init (1);
    text[0]                   = 0;

    while (true)
    {
        //////////////////////////////////////////////////////////////////////
        //
        // Check for keyboard activity (do this once on every frame)
        //
        v32kbd_probe (&keyboard);

        //////////////////////////////////////////////////////////////////////
        //
        // Process any key presses received
        //
        key                   = v32kbd_read (&keyboard);
        while (key           >  0)
        {
            last              = key;

            if (key          == V32KEY_BACKSPACE)
            {
                if (length   >  0)
                {
                    length    = length - 1;
                }
            }

            else if (length  <  TEXT_MAX - 4)
            {
                if (key      == V32KEY_ENTER)
                {
                    text[length]  = '\n';
                    length    = length + 1;
                }

                else if (key == V32KEY_TAB)
                {
                    for (x    = 0; x < 4; x = x + 1)
                    {
                        text[length]  = ' ';
                        length  = length + 1;
                    }
                }

                else if ((key >= 32) && (key < 127))
                {
                    text[length]  = key;
                    length    = length + 1;
                }
            }

            key               = v32kbd_read (&keyboard);
        }

        //////////////////////////////////////////////////////////////////////
        //
        // Draw the screen: typed text followed by a cursor
        //
        text[length]          = '_';
        text[length+1]        = 0;

        clear_screen (color_black);
        print_at (0, 0, "TYPE (last key:    )  SHIFT:   CTRL:");
        itoa (last, sym, 10);
        print_at (160, 0, sym);

        if (v32kbd_isdown (&keyboard, V32KEY_LSHIFT) ||
            v32kbd_isdown (&keyboard, V32KEY_RSHIFT))
        {
            print_at (290, 0, "*");
        }

        if (v32kbd_isdown (&keyboard, V32KEY_LCTRL) ||
            v32kbd_isdown (&keyboard, V32KEY_RCTRL))
        {
            print_at (370, 0, "*");
        }

        print_at (0, 40, text);
        end_frame ();
    }
}
