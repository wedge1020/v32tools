#ifndef LED_H_
#define LED_H_

#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////////
//
// Status light, using the board's RGB LED:
//
//   solid red         powered, no keyboard
//   blinking yellow   a device was plugged in and is being set up
//                     (at least 3 blinks are always shown)
//   3 green blinks    the keyboard is ready...
//   solid green       ...and in normal operation
//   2 quick blue blinks, then steady blue blinking
//                     setup mode
//   3 blue blinks, then solid green
//                     setup mode was left
//
typedef enum
{
    LED_LINK_NONE,          // no keyboard
    LED_LINK_CONNECTING,    // a device is being set up
    LED_LINK_ONLINE         // keyboard ready
}
led_link_t;

void led_init( void );

// call continuously from the main loop
void led_task( led_link_t link, bool setup_mode );

#endif
