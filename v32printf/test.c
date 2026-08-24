#include "video.h"
#include "printf.h"
#include "time.h"

void main ()
{
    int [32] string_data;
    int      player_score  = 175;
    float    health_pct    = 87.413;
    int      val1          = 2026;
    int      val2          = 8;
    int      val3          = 24;

    clear_screen (color_black);

    printf1( 10, 10, "%3s", printf_sarg( "hi" ) );
    printf1( 10, 30, "score: %05d", player_score );
    printf2( 10, 50, "%-8s: %3.1f%%", printf_sarg( "health" ), printf_farg( health_pct ) );
    sprintf3( string_data, "%.4d-%.2d-%.2d", val1, val2, val3 );
    printf1( 10, 70, "%s", printf_sarg( string_data ) );
    printf1( 10, 90, "player_score: 0x%.8X", player_score );

    end_frame ();
}
