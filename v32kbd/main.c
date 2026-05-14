#include "video.h"
#include "time.h"
#include "keyboard.h"

void main (void)
{
    int     x         = 0;
    int     y         = 0;
    int     key       = 0;
    v32kbd *keyboard  = NULL;

    keyboard          = v32kbd_init (0);

    select_texture (-1);
    while (1)
    {
        key           = v32kbd_read (keyboard);
        if (key      >= 32)
        {
            select_region (key);
            set_drawing_point (x, y);
            draw_region ();

            x         = x + 10;
        }

        if (key      == 13)
        {
            x         = 0;
            y         = y + 20;
        }

        if (key      == 8) // backspace
        {
            x         = x - 10;
            select_region (key);
            set_drawing_point (x, y);
            draw_region ();
        }

        if (key      == 9) // tab
        {
            x         = x + 40;
        }

        end_frame ();
    }
}
