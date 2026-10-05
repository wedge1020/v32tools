#include "led.h"

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

// the RGB LED is a WS2812 on GPIO 16. Pico-PIO-USB uses PIO 0 (and
// in some versions PIO 1) for the keyboard port, so the LED uses PIO 2
#define LED_PIN         PICO_DEFAULT_WS2812_PIN
#define LED_PIO         pio2

// light color, as 0xRRGGBB. Blue is used by default because it looks the
// same whether the LED takes its colors in RGB or in GRB order. Keep the
// values low: these LEDs are very bright
#define LED_COLOR       0x000030

// blink timings, in milliseconds
#define CONNECT_BLINKS  3
#define CONNECT_ON_MS   150
#define CONNECT_OFF_MS  150
#define SETUP_ON_MS     250
#define SETUP_OFF_MS    250

static uint led_sm     = 0;
static bool led_ready  = false;
static bool led_is_on  = false;

static void led_set( bool on )
{
    if( !led_ready || on == led_is_on )
      return;

    // WS2812 takes 24 bits in order green, red, blue
    uint32_t rgb = on? LED_COLOR : 0;
    uint32_t grb = ((rgb & 0x00FF00) << 8) | ((rgb & 0xFF0000) >> 8) | (rgb & 0x0000FF);

    // the FIFO is always empty here (we send 1 word at most every
    // 150 ms), but if it ever was not, just retry on the next call
    if( pio_sm_is_tx_fifo_full( LED_PIO, led_sm ) )
      return;

    pio_sm_put( LED_PIO, led_sm, grb << 8 );
    led_is_on = on;
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

    // force the first update, to turn the LED off
    led_is_on = true;
    led_set( false );
}

void led_task( bool keyboard_online, bool setup_mode )
{
    static bool     was_online    = false;
    static uint32_t connect_start = 0;
    uint32_t now = to_ms_since_boot( get_absolute_time() );

    // a keyboard was just detected: begin the connection blinks
    if( keyboard_online && !was_online )
      connect_start = now;

    was_online = keyboard_online;

    // no keyboard: off
    if( !keyboard_online )
    {
        led_set( false );
        return;
    }

    // connection blinks (they begin by an on period)
    uint32_t elapsed = now - connect_start;
    uint32_t period  = CONNECT_ON_MS + CONNECT_OFF_MS;

    if( elapsed < CONNECT_BLINKS * period )
    {
        led_set( (elapsed % period) < CONNECT_ON_MS );
        return;
    }

    // setup mode: blink continuously
    if( setup_mode )
    {
        led_set( (now % (SETUP_ON_MS + SETUP_OFF_MS)) < SETUP_ON_MS );
        return;
    }

    // normal operation: solid
    led_set( true );
}
