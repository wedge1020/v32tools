#ifndef LED_H_
#define LED_H_

#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////////
//
// Status light, using the board's RGB LED:
//
//   off              no keyboard plugged in
//   3 quick blinks   a keyboard was just detected and is ready
//   solid            keyboard ready, normal operation
//   steady blinking  setup mode
//
void led_init( void );

// call continuously from the main loop
void led_task( bool keyboard_online, bool setup_mode );

#endif
