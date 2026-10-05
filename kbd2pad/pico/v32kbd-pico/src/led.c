#include "led.h"

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

// the RGB LED is a WS2812 on GPIO 16. Pico-PIO-USB uses PIO 0 (and
// in some versions PIO 1) for the keyboard port, so the LED uses PIO 2
#define LED_PIN         PICO_DEFAULT_WS2812_PIN
#define LED_PIO         pio2

// Order in which the LED takes its colors. WS2812 normally use green,
// red, blue. If red and green show up swapped on your board (the
// "no keyboard" light is green instead of red), change this to 0
#define LED_ORDER_GRB   1

// light colors, as 0xRRGGBB. Keep values low: these LEDs are very bright
#define COLOR_OFF       0x000000
#define COLOR_RED       0x300000
#define COLOR_YELLOW    0x281800
#define COLOR_GREEN     0x003000
#define COLOR_BLUE      0x000030

// blink timings, in milliseconds (on time, off time)
#define BLINK_ON_MS     150     // yellow / green / blue blinks
#define BLINK_OFF_MS    150
#define QUICK_ON_MS      80     // quick blue blinks when entering setup
#define QUICK_OFF_MS     80
#define SETUP_ON_MS     250     // steady blue blinking in setup mode
#define SETUP_OFF_MS    250

#define CONNECT_BLINKS  3       // minimum yellow blinks; also green blinks
#define SETUP_IN_BLINKS 2       // quick blue blinks when entering setup
#define SETUP_OUT_BLINKS 3      // blue blinks when leaving setup

//////////////////////////////////////////////////////////////////////////////
//
// LED output
//
static uint     led_sm    = 0;
static bool     led_ready = false;
static uint32_t led_shown = 0xFFFFFFFF;     // color being shown now

static void led_show( uint32_t rgb )
{
    if( !led_ready || rgb == led_shown )
      return;

    uint32_t r = (rgb >> 16) & 0xFF;
    uint32_t g = (rgb >>  8) & 0xFF;
    uint32_t b =  rgb        & 0xFF;

  #if LED_ORDER_GRB
    uint32_t data = (g << 16) | (r << 8) | b;
  #else
    uint32_t data = (r << 16) | (g << 8) | b;
  #endif

    // the FIFO is always empty here (colors change every 80 ms at
    // most), but if it ever was not, just retry on the next call
    if( pio_sm_is_tx_fifo_full( LED_PIO, led_sm ) )
      return;

    pio_sm_put( LED_PIO, led_sm, data << 8 );
    led_shown = rgb;
}

void led_init( void )
{
    if( !pio_can_add_program( LED_PIO, &ws2812_program ) )
      return;

    int sm = pio_claim_unused_sm( LED_PIO, false );
    if( sm < 0 ) return;

    led_sm = (uint) sm;
    uint offset = (uint) pio_add_program( LED_PIO, &ws2812_program );

    pio_gpio_init( LED_PIO, LED_PIN );
    pio_sm_set_consecutive_pindirs( LED_PIO, led_sm, LED_PIN, 1, true );

    pio_sm_config config = ws2812_program_get_default_config( offset );
    sm_config_set_sideset_pins( &config, LED_PIN );
    sm_config_set_out_shift( &config, false, true, 24 );
    sm_config_set_fifo_join( &config, PIO_FIFO_JOIN_TX );

    // 800 kHz data rate; uses the system clock as it is right now,
    // so this must be called after the system clock has been set
    int cycles_per_bit = ws2812_T1 + ws2812_T2 + ws2812_T3;
    float divider = (float) clock_get_hz( clk_sys ) / (800000.0f * (float) cycles_per_bit);
    sm_config_set_clkdiv( &config, divider );

    pio_sm_init( LED_PIO, led_sm, offset, &config );
    pio_sm_set_enabled( LED_PIO, led_sm, true );
    led_ready = true;

    led_show( COLOR_RED );
}

//////////////////////////////////////////////////////////////////////////////
//
// Light patterns
//

// a blink sequence that plays once, from a given moment (which can be
// in the future, to let a previous pattern finish first)
typedef struct
{
    bool     active;
    uint32_t start;
    uint32_t color;
    uint32_t blinks, on_ms, off_ms;
}
sequence_t;

static sequence_t sequence = { false, 0, 0, 0, 0, 0 };

static void play_sequence( uint32_t start, uint32_t color, uint32_t blinks, uint32_t on_ms, uint32_t off_ms )
{
    sequence.active = true;
    sequence.start  = start;
    sequence.color  = color;
    sequence.blinks = blinks;
    sequence.on_ms  = on_ms;
    sequence.off_ms = off_ms;
}

// true when "now" is at or after "moment" (safe on timer wrap around)
static bool reached( uint32_t now, uint32_t moment )
{
    return (int32_t)( now - moment ) >= 0;
}

void led_task( led_link_t link, bool setup_mode )
{
    static led_link_t last_link     = LED_LINK_NONE;
    static bool       last_setup    = false;
    static uint32_t   yellow_start  = 0;    // when yellow blinking began
    static uint32_t   steady_start  = 0;    // when the steady pattern began

    uint32_t now = to_ms_since_boot( get_absolute_time() );
    uint32_t blink_period = BLINK_ON_MS + BLINK_OFF_MS;

    //////////////////////////////////////////////////////////////////////////
    //
    // React to state changes
    //
    if( link != last_link )
    {
        if( link == LED_LINK_NONE )
        {
            // keyboard removed: straight to red
            sequence.active = false;
        }

        else if( link == LED_LINK_CONNECTING )
        {
            sequence.active = false;
            yellow_start = now;
        }

        else  // keyboard ready
        {
            // if we never saw it connecting, begin the yellow blinks now
            if( last_link != LED_LINK_CONNECTING )
              yellow_start = now;

            // green blinks begin once the minimum of yellow blinks is done,
            // and always after a complete yellow blink
            uint32_t yellow_blinks = (now - yellow_start + blink_period - 1) / blink_period;
            if( yellow_blinks < CONNECT_BLINKS ) yellow_blinks = CONNECT_BLINKS;

            play_sequence( yellow_start + yellow_blinks * blink_period,
                           COLOR_GREEN, CONNECT_BLINKS, BLINK_ON_MS, BLINK_OFF_MS );
        }

        last_link = link;
    }

    if( setup_mode != last_setup )
    {
        if( link == LED_LINK_ONLINE )
        {
            if( setup_mode )
              play_sequence( now, COLOR_BLUE, SETUP_IN_BLINKS, QUICK_ON_MS, QUICK_OFF_MS );
            else
              play_sequence( now, COLOR_BLUE, SETUP_OUT_BLINKS, BLINK_ON_MS, BLINK_OFF_MS );
        }

        last_setup = setup_mode;
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // Show the light for this moment
    //

    // no keyboard: solid red
    if( link == LED_LINK_NONE )
    {
        led_show( COLOR_RED );
        return;
    }

    // a sequence is being played, or waiting for the yellow blinks to end
    if( sequence.active )
    {
        uint32_t period = sequence.on_ms + sequence.off_ms;

        if( !reached( now, sequence.start ) )
        {
            led_show( ((now - yellow_start) % blink_period) < BLINK_ON_MS? COLOR_YELLOW : COLOR_OFF );
            return;
        }

        uint32_t elapsed = now - sequence.start;

        if( elapsed < sequence.blinks * period )
        {
            led_show( (elapsed % period) < sequence.on_ms? sequence.color : COLOR_OFF );
            return;
        }

        // finished: the steady pattern begins here
        sequence.active = false;
        steady_start = sequence.start + sequence.blinks * period;
    }

    // steady patterns
    if( link == LED_LINK_CONNECTING )
      led_show( ((now - yellow_start) % blink_period) < BLINK_ON_MS? COLOR_YELLOW : COLOR_OFF );

    else if( setup_mode )
      led_show( ((now - steady_start) % (SETUP_ON_MS + SETUP_OFF_MS)) < SETUP_ON_MS? COLOR_BLUE : COLOR_OFF );

    else
      led_show( COLOR_GREEN );
}
