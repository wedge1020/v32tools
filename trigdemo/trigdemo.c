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
    define_region  (0,   0,   639, 359, 0,   0);

    //////////////////////////////////////////////////////////////////////////
    //
    // define the SPRITE region
    //
    select_region  (SPRITE);
    define_region  (518, 407, 563, 454, 518, 407);

    //////////////////////////////////////////////////////////////////////////
    //
    // define the PIXEL region
    //
    select_region  (PIXEL);
    define_region  (564, 360, 572, 367, 564, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the UP region
    //
    select_region  (UP);
    define_region  (0,   360,  47, 407, 0,   360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the UP_PRESS region
    //
    select_region  (UP_PRESS);
    define_region  (0,   408, 47,  454, 0,   408);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the DOWN region
    //
    select_region  (DOWN);
    define_region  (47,  360, 94, 407, 47, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the DOWN_PRESS region
    //
    select_region  (DOWN_PRESS);
    define_region  (47,  408, 94, 454, 47,  408);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the LEFT region
    //
    select_region  (LEFT);
    define_region  (93, 360, 141, 407, 95, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the LEFT_PRESS region
    //
    select_region  (LEFT_PRESS);
    define_region  (93, 408, 141, 454, 95, 408);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the RIGHT region
    //
    select_region  (RIGHT);
    define_region  (140, 360, 188, 407, 142, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the RIGHT_PRESS region
    //
    select_region  (RIGHT_PRESS);
    define_region  (140, 408, 188, 454, 142, 408);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the START region
    //
    select_region  (START);
    define_region  (188, 360, 234, 407, 188, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the START_PRESS region
    //
    select_region  (START_PRESS);
    define_region  (188, 408, 234, 454, 188, 408);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the A region
    //
    select_region  (A);
    define_region  (234, 360, 282, 407, 234, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the A_PRESS region
    //
    select_region  (A_PRESS);
    define_region  (234, 408, 282, 454, 234, 408);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the B region
    //
    select_region  (B);
    define_region  (282, 360, 328, 407, 282, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the B_PRESS region
    //
    select_region  (B_PRESS);
    define_region  (282, 408, 328, 454, 282, 408);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the X region
    //
    select_region  (X);
    define_region  (328, 360, 376, 407, 328, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the X_PRESS region
    //
    select_region  (X_PRESS);
    define_region  (328, 408, 376, 454, 328, 408);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the Y region
    //
    select_region  (Y);
    define_region  (376, 360, 422, 407, 376, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the Y_PRESS region
    //
    select_region  (Y_PRESS);
    define_region  (376, 408, 422, 454, 376, 408);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the L region
    //
    select_region  (L);
    define_region  (422, 360, 470, 407, 422, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the L_PRESS region
    //
    select_region  (L_PRESS);
    define_region  (422, 408, 470, 454, 422, 408);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the R region
    //
    select_region  (R);
    define_region  (470, 360, 516, 407, 470, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the R_PRESS region
    //
    select_region  (R_PRESS);
    define_region  (470, 408, 516, 454, 470, 408);  

    while (1)
    {
        clear_screen  (color_black);
		select_texture (0);
        select_region (SPRITE);
        set_drawing_point (0, 80);
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
