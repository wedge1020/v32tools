# keyboard2gamepad

A ghetto (initially proof-of-concept) keyboard input demultiplexer script
for use in  masquerading a keyboard as  a USB gamepad device,  for use on
the Vircon32 fantasy console.

## OVERVIEW

This script will  be running on that hardware (assuming  a Linux-based OS
on  a system  like  a SBC),  obtaining each  key  pressed, isolating  the
individual  bits of  that  input, and  indicating  which gamepad  buttons
(bits) to activate, to be interpret by something like the `v32kbd` driver
routine running on the Vircon32 system itself.

Keyboard  side will  process  each  key as  it  happens,  queuing up  the
information to  accommodate the  processing environment on  Vircon32 (all
buttons will need to be read, likely only occuring once per frame or so).
Queuing on  keyboard side will  mitigate keystrokes lost on  the Vircon32
side.

Or so is the current plan.

## KEYBOARD INPUT

Once the format of input is identified, it should prove unique enough and
hopefully encodable within 7-bits (the Vircon32 controller has 7 buttons.
11 technically, counting the D-pad).

Assuming the  letter 'a', which  is hex `0x41`,  in binary that  would be
`01000001`. Dropping the leading bit (always a 0 in a 7-bit scheme), that
would be the bit states of each button.

## GAMEPAD OUTPUT

The next  challenge will  be to  figure out how  to influence  the button
state, so that we can set each button ON or OFF as needed.

## PREREQUISITES

### PACKAGES

Make  sure  the   `kbd`  package  is  installed  (to  make   use  of  the
`showkeys(1)` tool).

### HARDWARE

Hardware operating in USB gadget mode acting as a USB gamepad.

The  original thought  was  to use  a  Raspberry Pi  Zero,  since it  can
operating in gadget mode, and is a  minimal enough system where it can be
set up and ignored.

However, it turns  out the `RPi zero`  only has ONE USB data  port. So it
can serve  as a gadget, but  then there's no way  to plug in a  wired USB
keyboard. A  bluetooth-equipped `RPi  zero` could  work with  a bluetooth
keyboard, however  (hopefully bluetooth  keyboard input and  USB keyboard
input both go through the same kernel interface that `showkeys(1)` uses.

Other  options   are  being   explored,  with  the   following  scenarios
potentially identified as viable:

Orange Pi  zero 3: this  SBC DOES have multiple  USB data ports,  and can
operate in  gadget mode.  It remains to  be seen if  it has  distinct USB
ports or if they are all on one HUB.

Raspberry Pi pico: through the PIO port and the TinyUSB library, a second
USB  port  (operating at  USB1.1  speeds,  which  should  be fine  for  a
keyboard) can be  emulated. This would talk to the  keyboard, whereas the
hardware USB would be operating in gadget mode.

The  priority of  development will  be to  test the  pi zero  systems for
viability, and  falling back to  the pico (since more  extensive software
development will  seemingly be  necessary: the  original hope  this would
just be a hacked-together script on a custom-configured system)

Another scheme is  to use a USB decoder board  (used in MAME/hobby arcade
setups).  From the  description,  it  sees to  work  as  the USB  gadget,
presenting  itself as  a gamepad,  and  has GPIO  pins that  map to  each
button. A connection can  be made from the pi GPIO pins  to these pins on
the breakout board. That would also  be a viable approach (and could then
be served by  a Raspberry Pi zero  with a wired USB keyboard,  as well as
any system, since  the requirement of operating in USB  gadget mode is no
longer present).
