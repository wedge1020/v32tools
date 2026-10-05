# v32kbd-pico

Hardware v32kbd  adapter: firmware for the  **Waveshare RP2350-USB-A**
that turns a USB keyboard into a v32kbd gamepad.

```
USB keyboard --> [type A]  RP2350-USB-A  [type C] --> computer running
                  (USB host, PIO)        (USB device)   the Vircon32 emulator
```

The computer sees  an ordinary USB gamepad named `v32kbd`,  with 11
buttons. No changes  are needed in the emulator: it is  used through a
regular joystick profile, and Vircon32  programs read it with the same
`keyboard.h` driver used for the emulator's built-in `v32kbd` device.

## HARDWARE NOTES

- The type A port is driven with  PIO on GPIO 12 (D+) and GPIO 13 (D-).
  These are the pins in the Pico SDK board definition for this board.
- **Resistor R13.** Waveshare states  that R13 must be  removed for the
  type A port to support hot-plugging  and low-speed devices as a host,
  and that after removing it  the port can't be used as  a device. Most
  keyboards are  low-speed devices,  so expect to  need this. Try your
  keyboard first: if it is not detected, R13 is the reason.
- The keyboard is powered from the  type A port, which takes its power
  from the  computer through the type  C port. Keyboards with  a lot of
  lighting may draw more than the board can pass through.

## STATUS LIGHT

The board's onboard RGB LED shows the state of the adapter:

| Light                                 | Meaning                              |
| ------------------------------------- | ------------------------------------ |
| solid red                             | powered, no keyboard                 |
| blinking yellow                       | a device was plugged in and is being set up (at least 3 blinks) |
| 3 green blinks, then solid green      | keyboard ready, normal operation     |
| 2 quick blue blinks, then steady blue blinking | setup mode (see below)      |
| 3 blue blinks, then solid green       | setup mode was left                  |

If something that is not a keyboard is plugged in, the light blinks
yellow for a moment and goes back to red.

Colors and blink timings are at the top of `src/led.c`. The LED is
assumed to take its colors in the usual WS2812 order (green, red, blue).
If the "no keyboard" light is green instead of red, set `LED_ORDER_GRB`
to 0 there.

## GAMEPAD PRESENCE

The computer only sees the `v32kbd` gamepad while a keyboard is plugged
into the adapter. With no keyboard, the type C port keeps powering the
board but stays disconnected for data, so for the computer the gamepad is
unplugged. It comes back when a keyboard is detected.

## PROTOCOL

Gamepad buttons follow the order of  the console's INP ports (`0x402` to
`0x40C`):

| Button | Vircon32 control | Meaning                                  |
| ------ | ---------------- | ---------------------------------------- |
| 0      | Left             | strobe (each key event switches sides,   |
| 1      | Right            | beginning by Left)                       |
| 2      | Up               | the key was pressed                      |
| 3      | Down             | the key was released                     |
| 4      | Start            | key code, bit 0                          |
| 5      | A                | key code, bit 1                          |
| 6      | B                | key code, bit 2                          |
| 7      | X                | key code, bit 3                          |
| 8      | Y                | key code, bit 4                          |
| 9      | L                | key code, bit 5                          |
| 10     | R                | key code, bit 6                          |

Key codes are the same as in the emulator's `v32kbd` device (see the
v32kbd library README). The gamepad  also reports X and Y axes that never
move: they only exist so that every system takes it for a gamepad.

### TIMING

The emulator's own `v32kbd` device  delivers exactly 1 event per frame.
A real gamepad can't  know when frames happen, so  the adapter instead
holds every key event long enough to be seen:

1. key code and action are sent (strobe unchanged)
2. 8 ms later, the strobe switches sides
3. that state is held for 34 ms (2 frames) before the next event

So events go out at about 23 per  second (around 11 keystrokes, each
being a press and  a release). Faster bursts wait in a  queue of 128
events and are delivered in order, slightly delayed.

Sending the strobe  after the key code means a program  can never see a
new strobe with an old key code,  even if the computer ever applies the
buttons of a report across 2 frames.

`SETTLE_MS`, `HOLD_MS` and `QUEUE_SIZE` are at the top of `src/main.c`.

## SETTING UP THE EMULATOR

The emulator needs a joystick profile  for the adapter. A keyboard must
be plugged into the adapter for the computer to see the gamepad. Vircon32's
EditControls creates profiles by asking  you to press each control on
its own, which the  v32kbd protocol never does. For  this, the adapter
has a **setup mode**:

1. Press **Scroll Lock** on the keyboard to enter setup mode. The
   status light blinks blue while in this mode.
2. In EditControls, create a profile for the `v32kbd` joystick and
   press these keys when asked for each control:

   | Left | Right | Up | Down | Start | A  | B  | X  | Y  | L   | R   |
   | ---- | ----- | -- | ---- | ----- | -- | -- | -- | -- | --- | --- |
   | F1   | F2    | F3 | F4   | F5    | F6 | F7 | F8 | F9 | F10 | F11 |

   Leave the Command button unmapped.
3. Press **Scroll Lock** again to go back to normal operation (3 blue
   blinks, then solid green). Unplugging the keyboard also leaves
   setup mode.
4. In the emulator, menu Gamepads, select that profile for the gamepad
   your program expects (Gamepad 2 for the v32kbd test program).

The resulting entry in `Config-Controls.xml` should look like this (the
GUID depends on your system, so let EditControls write it):

```
<joystick nickname="v32kbd">
    <guid>...</guid>
    <name>v32tools v32kbd</name>
    <left button="0" />
    <right button="1" />
    <up button="2" />
    <down button="3" />
    <button-start button="4" />
    <button-a button="5" />
    <button-b button="6" />
    <button-x button="7" />
    <button-y button="8" />
    <button-l button="9" />
    <button-r button="10" />
</joystick>
```

Entering or leaving setup mode releases all buttons and restarts the
strobe; `keyboard.h` handles that with no phantom key.

## BUILDING

Needs the Pico SDK (2.1 or later, with its TinyUSB submodule), the
arm-none-eabi GCC toolchain and Pico-PIO-USB:

```
git clone https://github.com/sekigon-gonnoc/Pico-PIO-USB.git
mkdir build
cd build
cmake -DPICO_SDK_PATH=/path/to/pico-sdk ..
make
```

This produces `v32kbd_pico.uf2`. A prebuilt one is included.

## FLASHING

Hold the BOOT button while plugging  the type C port into the computer.
The board shows up as a drive: copy `v32kbd_pico.uf2` into it.

## NOTES

- Keyboards are used in USB boot protocol: up to 6 keys plus modifiers
  held at once.
- The USB IDs (`0xCAFE:0x4B32`) are not officially assigned. They can be
  changed in `src/usb_descriptors.c`.
- Events sent while the emulator is  paused or its window has lost
  focus are not seen by the program.
