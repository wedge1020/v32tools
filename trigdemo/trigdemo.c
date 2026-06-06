#include "math.h"
#include "misc.h"
#include "video.h"
#include "time.h"
#include "gamepad.h"

#define  BACKGROUND   0
#define  SPRITE       1
#define  PIXEL        2
#define  UP           10
#define  DOWN         11
#define  LEFT         12
#define  RIGHT        13
#define  START        14
#define  A            15
#define  B            16
#define  X            17
#define  Y            18
#define  L            19
#define  R            20
#define  UP_PRESS     30
#define  DOWN_PRESS   31
#define  LEFT_PRESS   32
#define  RIGHT_PRESS  33
#define  START_PRESS  34
#define  A_PRESS      35
#define  B_PRESS      36
#define  X_PRESS      37
#define  Y_PRESS      38
#define  L_PRESS      39
#define  R_PRESS      40

//////////////////////////////////////////////////////////////////////////////
//
// declare function pointer 'trigfunc'
//
//float (float)* trigfunc;

void  main ()
{
	int  index  = 0;
	int  pos    = 0;
	int  word   = 0;
	int  x      = 0;

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
	define_region  (564, 360, 564, 360, 572,  367);  

	//////////////////////////////////////////////////////////////////////////
	//
	// define the UP region
	//
	select_region  (UP);
	define_region  (0, 360, 0, 360, 47,  407);  

	//////////////////////////////////////////////////////////////////////////
	//
	// define the UP_PRESS region
	//
	select_region  (UP_PRESS);
	define_region  (0, 408, 0, 408, 47,  454);  

	//////////////////////////////////////////////////////////////////////////
	//
	// define the DOWN region
	//
	select_region  (DOWN);
	define_region  (47, 360, 47, 360, 94,  407);  

	//////////////////////////////////////////////////////////////////////////
	//
	// define the DOWN_PRESS region
	//
	select_region  (DOWN_PRESS);
	define_region  (47, 408, 47, 408, 94,  454);  

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
		clear_screen  (0);
		select_region (0);
		set_drawing_point (0, 0);
		draw_region ();

		word        = gamepad_read (0);

		pos         = UP;
		x           = 0;
		for (index  = 1024;
		     index >= 1;
			 index  = index / 2)
		{
			if (0  <  (word & index))
			{
				select_region (pos + 20);
			}
			else
			{
				select_region (pos);
			}

			set_drawing_point (x, 160);
			draw_region ();
			x       = x + 56;
		}

		end_frame ();
	}
}
