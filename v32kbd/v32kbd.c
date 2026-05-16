#include "video.h"
#include "time.h"
#include "string.h"
#include "keyboard.h"

void main (void)
{
    bool    check         = false;
    int     x             = 0;
    int     y             = 0;
    int     key           = 0;
    int [2] sym;
    v32kbd *keyboard      = NULL;

    keyboard              = v32kbd_init (1);

    select_texture (-1);
    clear_screen (color_black);
    print_at (0, 100, "TYPE");
    while (1)
    {
        check             = v32kbd_probe (&keyboard);
        if (check        == true)
        {
            key           = v32kbd_read (&keyboard);
            if (key      >= 32)
            {
                select_region (key);
                set_drawing_point (x, y);
                draw_region ();

                x         = x + 10;
            }

            else if (key == 13)
            {
                x         = 0;
                y         = y + 20;
            }

            else if (key == 8) // backspace
            {
                x         = x - 10;
                select_region (key);
                set_drawing_point (x, y);
                draw_region ();
            }

            else if (key == 9) // tab
            {
                x         = x + 40;
            }
        }
        else
        {
            select_region ('X');
            set_drawing_point (x, y);
            draw_region ();
        }

        itoa (key, sym, 10);
        print_at (100, 100, sym);
        end_frame ();
    }
}
