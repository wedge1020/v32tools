#include "math.h"
#include "misc.h"
#include "string.h"
#include "video.h"
#include "time.h"
#include "gamepad.h"

#define  BACKGROUND   0
#define  SPRITE       5
#define  PIXEL        6
#define  LEFT         1024
#define  RIGHT        512
#define  UP           256
#define  DOWN         128
#define  START        64
#define  A            32
#define  B            16
#define  X            8
#define  Y            4
#define  L            2
#define  R            1
#define  LEFT_PRESS   2048
#define  RIGHT_PRESS  1536
#define  UP_PRESS     1280
#define  DOWN_PRESS   1152
#define  START_PRESS  1088
#define  A_PRESS      1056
#define  B_PRESS      1040
#define  X_PRESS      1032
#define  Y_PRESS      1028
#define  L_PRESS      1026
#define  R_PRESS      1025

//////////////////////////////////////////////////////////////////////////////
//
// declare function pointer 'trigfunc'
//
//float (float)* trigfunc;

void  main ()
{
    int  index  = 0;
    int  word   = 0;
    int  x      = 0;
	int [10] data;

    //////////////////////////////////////////////////////////////////////////
    //
    // initialize trigfunc to initially point to the sin() function
    //
    //trigfunc   = &sin;

    //////////////////////////////////////////////////////////////////////////
    //
    // define the BACKGROUND region
    //
    select_texture (0);
    select_region  (BACKGROUND);
    define_region  (0,   0,   0,   0,   639, 359);

    //////////////////////////////////////////////////////////////////////////
    //
    // define the SPRITE region
    //
    select_region  (SPRITE);
    define_region  (518, 407, 518, 407, 563, 454);

    //////////////////////////////////////////////////////////////////////////
    //
    // define the PIXEL region
    //
    select_region  (PIXEL);
    define_region  (564, 360, 564, 360, 572, 367);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the UP region
    //
    select_region  (UP);
    define_region  (0,   360, 0,   360,  47, 407);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the UP_PRESS region
    //
    select_region  (UP_PRESS);
    define_region  (0,   408, 0,   408,  47, 454);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the DOWN region
    //
    select_region  (DOWN);
    define_region  (47,  360, 47,  360,  94, 407);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the DOWN_PRESS region
    //
    select_region  (DOWN_PRESS);
    define_region  (47,  408, 47,  408,  94, 454);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the LEFT region
    //
    select_region  (LEFT);
    define_region  (95, 360, 95, 360, 141, 407);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the LEFT_PRESS region
    //
    select_region  (LEFT_PRESS);
    define_region  (95, 408, 95, 408, 141, 454);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the RIGHT region
    //
    select_region  (RIGHT);
    define_region  (142, 360, 142, 360, 188, 407);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the RIGHT_PRESS region
    //
    select_region  (RIGHT_PRESS);
    define_region  (142, 408, 142, 408, 188, 454);  

    while (1)
    {
        clear_screen  (color_black);
		select_texture (0);
        select_region (0);
        set_drawing_point (0, 0);
        draw_region ();

        word                    = gamepad_read (0);
        if (word               == -1)
        {
            continue;
        }

		itoa (word, data, 16);
		print_at (0, 0, data);

		select_texture (0);
        x                       = 0;
        for (index              = 1024;
             index             >= 1;
             index              = index >> 1)
        {
            if ((word & index) >  0)
            {
                select_region (index + 1024);
            }
            else
            {
                select_region (index);
            }

            set_drawing_point (x, 160);
            draw_region ();
            x       = x + 56;
        }

        end_frame ();
    }
}
