#include "input.h"
#include "math.h"
#include "misc.h"
#include "video.h"
#include "time.h"

#define  BACKGROUND 0
#define  SPRITE     1
#define  PIXEL      2

//////////////////////////////////////////////////////////////////////////////
//
// declare function pointer 'trigfunc'
//
float (float)* trigfunc;

void  main ()
{
	//////////////////////////////////////////////////////////////////////////
	//
	// initialize trigfunc to initially point to the sin() function
	//
	trigfunc   = &sin;

	//////////////////////////////////////////////////////////////////////////
	//
	// define the BACKGROUND region
	//
	select_texture (0);
	select_region  (BACKGROUND);
	define_region  (0,  0,   0,  0,   639, 359);

	//////////////////////////////////////////////////////////////////////////
	//
	// define the SPRITE region
	//
	select_region  (SPRITE);
	define_region  (0,  360, 0,  360, 63,  424);

	//////////////////////////////////////////////////////////////////////////
	//
	// define the PIXEL region
	//
	select_region  (PIXEL);
	define_region  (64, 360, 64, 360, 72,  367);  

	while (1)
	{

		end_frame ();
	}
}
