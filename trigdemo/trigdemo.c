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

#define  PI           3.1415926

//////////////////////////////////////////////////////////////////////////////
//
// declare function pointer 'trigfunc'
//
float (float)* trigfunc;

void  main ()
{
    int  index    = 0;
	int  xoffset  = 0;
	int  waveidx  = 0;
	int  wavex    = 0;
	int  wavey    = 0;
	int  waveflag = 0;
    int  word     = 0;
    int  x        = 295;
    int  y        = 0;
    int [10] data;
    //float  rad  = 0.0;

    //////////////////////////////////////////////////////////////////////////
    //
    // initialize trigfunc to initially point to the sin() function
    //
    trigfunc      = &sin;

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
    // define the DOWN region
    //
    select_region  (DOWN);
    define_region  (47,  360, 94, 407, 47, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the LEFT region
    //
    select_region  (LEFT);
    define_region  (93, 360, 141, 407, 95, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the RIGHT region
    //
    select_region  (RIGHT);
    define_region  (140, 360, 188, 407, 142, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the START region
    //
    select_region  (START);
    define_region  (188, 360, 234, 407, 188, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the A region
    //
    select_region  (A);
    define_region  (234, 360, 282, 407, 234, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the B region
    //
    select_region  (B);
    define_region  (282, 360, 328, 407, 282, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the X region
    //
    select_region  (X);
    define_region  (328, 360, 376, 407, 328, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the Y region
    //
    select_region  (Y);
    define_region  (376, 360, 422, 407, 376, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the L region
    //
    select_region  (L);
    define_region  (422, 360, 470, 407, 422, 360);  

    //////////////////////////////////////////////////////////////////////////
    //
    // define the R region
    //
    select_region  (R);
    define_region  (470, 360, 516, 407, 470, 360);  

    while (1)
    {
        clear_screen  (color_black);
        select_texture (0);
        //select_region (BACKGROUND);
        //set_drawing_point (0, 0);
        //draw_region ();

		waveflag                 = 0;
		xoffset                  = -360;
		for (waveidx             = -360;
		     waveidx            <  540;
			 waveidx             = waveidx + 1)
		{
			wavex                = 90 * cos      (3 * PI/180 * waveidx) + 120;
			wavey                = 90 * trigfunc (3 * PI/180 * waveidx) + 120;
			select_region (PIXEL);
			set_drawing_point (wavex + xoffset + index, wavey);
			draw_region ();
			switch (waveidx)
			{
				case -240:
				case -180:
				case -120:
				case -60:
				case 0:
				case 60:
				case 120:
				case 180:
				case 240:
				case 360:
					xoffset      = xoffset + 180;
					break;
			}

	//		if (xoffset > 640)
		//		xoffset  = 0;
		}

/*
        word                    = gamepad_read (0);
        if (word               == -1)
        {
            continue;
        }
		*/

        // title bounce: y = 8 * sin (3 * PI/180 * framecounter) * amplify

		

        y                   = 90 * trigfunc (3 * PI/180 * index) + 120;
        select_region (SPRITE);
        set_drawing_point (x, y);
        draw_region ();

        /*
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
        */

        index                   = (index + 1) % 360;

        end_frame ();
    }
}
