# v32pad

Another Vircon32 driver/library for gamepad input

## OVERVIEW

When using the  standard Vircon32 DevTools `input.h`  library, and indeed
even just accessing the gamepad IOPorts,  the typical approach is that of
multiple distinct transactions, one per desired IOPort.

This library (provided  via `gamepad.h`) takes a  different approach: via
the use of  a RAM-based self-modifying machine code  routine, all gamepad
inputs are read  and processed into a single word  (various bit positions
corresponding to the digital nature  of the particular gamepad input), of
the following form:

## STATUS WORD

The `gamepad.h` status word is laid  out as follows (note it is following
the ordering prescribed by the INP IOPort ID values):

  * bits 31-11: unused (there are only 11 inputs per gamepad)
  * bit 10: state of the LEFT button (IOport 0x402)
  * bit 9: state of the RIGHT button (IOport 0x403)
  * bit 8: state of the UP button (IOport 0x404)
  * bit 7: state of the DOWN button (IOport 0x405)
  * bit 6: state of the START button (IOport 0x406)
  * bit 5: state of the A button (IOport 0x407)
  * bit 4: state of the B button (IOport 0x408)
  * bit 3: state of the X button (IOport 0x409)
  * bit 2: state of the Y button (IOport 0x40A)
  * bit 1: state of the L button (IOport 0x40B)
  * bit 0: state of the R button (IOport 0x40C)

The current  implementation of  `gamepad.h` assumes  one status  word per
gamepad. With optimization we could only fit two gamepads per status word
anyway  (`11*2=22`),  coming up  one  input  short  for a  third  gamepad
(`11*3=33`).

## USAGE

With  each  gamepad  button's  IOPort state  recorded  in  the  indicated
position, the  value stored there  will be either  0 (not pressed),  or 1
(pressed). Using bitwise operations, individual  bits can be isolated and
transacted as needed.

Also, this scenario will allow us the convenience of comparing the entire
status word against 0, which if  true, implies NONE of the gamepad inputs
are currently being pressed.

The  core  function at  play  here  is  `gamepad_read()`, which  has  the
following prototype:

```
int  gamepad_read (int);
```

Where the parameter is the desired gamepad to read (0-3).

And  the return  value is  the `status  word` (the  bitpacked int  of the
binary encoded state of the indicated gamepad's buttons).

## INSTALLATION

Either     copy    the     `gamepad.h`    file     into    the     system
`Vircon32/DevTools/include/`  directory, or  into your  project's current
directory for inclusion.

### DEMO

Run  the included  `Make.sh` to  build a  demo showing  the functionality
provided by `gamepad.h` in action.
