# v32kbd

Vircon32 driver/library for jury-rigged keyboard2gamepad USB gadget.

## HOW THE DEVICE WORKS

A `v32kbd` device is seen by the console as a regular gamepad, but its 11
controls are used to report key events instead:

| IOPort  | Control                  | Meaning                               |
| ------- | ------------------------ | ------------------------------------- |
| `0x402` | `INP_GamepadLeft`        | strobe (see below)                    |
| `0x403` | `INP_GamepadRight`       | strobe (see below)                    |
| `0x404` | `INP_GamepadUp`          | the key was pressed                   |
| `0x405` | `INP_GamepadDown`        | the key was released                  |
| `0x406` | `INP_GamepadButtonStart` | key code, bit 0                       |
| `0x407` | `INP_GamepadButtonA`     | key code, bit 1                       |
| `0x408` | `INP_GamepadButtonB`     | key code, bit 2                       |
| `0x409` | `INP_GamepadButtonX`     | key code, bit 3                       |
| `0x40A` | `INP_GamepadButtonY`     | key code, bit 4                       |
| `0x40B` | `INP_GamepadButtonL`     | key code, bit 5                       |
| `0x40C` | `INP_GamepadButtonR`     | key code, bit 6                       |

So the bit number of each key code bit is its port number minus `0x406`.

The device reports at most 1 key event per frame, and it keeps that state
until the  next event.  The **strobe**  is what  tells events  apart: the
first event presses Left, the next  one presses Right, then Left again...
so a  new event has arrived  whenever the pressed side  is different from
the last one seen. This is what  allows the same key to be received twice
in a row. Before the first event, all controls are unpressed.

The  d-pad never  has opposite  directions pressed  at once,  so this  is
always a valid gamepad state and the console itself needs no changes.

### KEY CODES

Key codes  are 7 bits  (1 to 127) and  they identify **keys**,  not typed
characters. Keys that have an ASCII  character report it as typed with no
shift on a US layout: `a` to `z`, `0` to  `9`, space and `` ` - = [ ] \ ;
' ,  . /  ``. Shift is  reported as a  key of  its own, and  this library
applies it (along with caps lock) to obtain the typed character.

The other keys use these codes:

| Code    | Key (`V32KEY_` name)            | Code    | Key (`V32KEY_` name)     |
| ------- | ------------------------------- | ------- | ------------------------ |
| 1       | `UP`                            | 11      | `RCTRL`                  |
| 2       | `DOWN`                          | 12      | `LALT` (left option)     |
| 3       | `LEFT`                          | 13      | `ENTER`                  |
| 4       | `RIGHT`                         | 14 - 25 | `F1` to `F12`            |
| 5       | `CAPSLOCK`                      | 26      | `RALT` (right option)    |
| 6       | `LSHIFT`                        | 27      | `ESCAPE`                 |
| 7       | `RSHIFT`                        | 28      | `LGUI` (left command)    |
| 8       | `BACKSPACE`                     | 29      | `RGUI` (right command)   |
| 9       | `TAB`                           | 30, 31  | unused                   |
| 10      | `LCTRL`                         | 127     | `DELETE`                 |

Code 0  is never reported. Numeric  keypad keys report the  same codes as
their main keyboard equivalents.

## USING IT IN THE EMULATOR

In the modified desktop emulator, open  menu Gamepads, pick a gamepad and
select `v32kbd`. It  can't be selected while any  gamepad uses `Keyboard`
(and vice  versa), since  both need the  host keyboard.  Emulator hotkeys
(Esc, F2, F4, F5  and the Ctrl shortcuts) keep working  and are also sent
to the program.

The test program  (`v32kbd.c`) expects the keyboard in  **Gamepad 2** (id
1), leaving Gamepad 1 free for a regular gamepad.

## IMPLEMENTATION

There are two current thoughts on  how to implement this on Vircon32; and
until  severe limitations  present  themselves, both  approaches will  be
pursued.

In the  end, the developer  will need  to make the  `v32kbd_probe()` call
once on **every** frame. If frames are skipped, key events can be lost.

### BIOS DRIVER

Create  a custom  BIOS, embedding  these  routines therein.  Then, via  C
wrapper functions that call the binary offsets, do the deed.

### LIBRARY

A C-code  includable library  that contains the  routines (much  like the
existing DevTools headers). This is what `keyboard.h` currently is.

## API

Typical use:

```
v32kbd *keyboard  = v32kbd_init (FOURTH_GAMEPAD_PORT);  // port id 3

while (true)
{
    v32kbd_probe (&keyboard);           // once per frame

    key           = v32kbd_read (&keyboard);
    while (key   >  0)                  // process all key presses
    {
        ...
        key       = v32kbd_read (&keyboard);
    }

    end_frame ();
}
```

v32kbd provides the following:

### input key transactional unit: `v32key` struct

A single key event in the input list: `value` (key code), `symbol` (typed
character, with shift and caps lock applied as they were when the key was
pressed) and `pressed` (true for press, false for release).

### keyboard transactional unit: `v32kbd` struct

A keyboard instance bound to a gamepad. It holds the input list, the last
strobe side seen, the caps lock state, which keys are held and the in-RAM
routine.

### generate new key node for list: `v32key_newkey()`

### initialize keyboard instance: `v32kbd_init()`

```
v32kbd *v32kbd_init (int gamepad);
```

Allocates the  instance and generates  its in-RAM routine. It  also takes
the current strobe side as starting point,  so a state left in the device
from before (the  device is not affected by console  resets) is not taken
as a new key.

### release keyboard instance: `v32kbd_free()`

```
void    v32kbd_free (v32kbd **);
```

Frees the  instance and any  unread key events,  and sets the  pointer to
`NULL`.

### add new key to keyboard input list: `v32kbd_addkey()`

### get next key of input from input list: `v32kbd_getkey()`

The caller becomes responsible of calling `free()` on the returned key.

### read all control ports: `v32kbd_scan()`

```
int     v32kbd_scan (v32kbd *);
```

Selects  the keyboard's  gamepad,  `CALL`s the  in-RAM  routine and  then
restores  the previously  selected gamepad.  It returns  the 11  controls
packed as bits:  bit 0 left, bit 1  right, bit 2 up, bit 3  down, and the
key code in bits 4 to 10.

The  in-RAM  custom machine  code  routine  is  generated just  once,  by
`v32kbd_init()`, and it  is stored in the keyboard instance  (so there is
nothing to release separately). It reads  each INP control port, turns it
into a 0 or 1, and packs them all together:

```
routine[0]            = 0x54000000;        // PUSH R0
routine[1]            = 0x54200000;        // PUSH R1
routine[2]            = 0x54400000;        // PUSH R2
routine[3]            = 0x4E200000;        // MOV R1, 0
routine[4]            = 0x00000000;        // immediate
routine[5]            = 0x4C424000;        // MOV R2, R1
for (index            = 0;
     index           <  11;
     index            = index + 1)
{
    offset            = ((index * 5) + 6);
    port              = index + 2;         // 0x402 to 0x40C

    routine[offset]   = 0x5C000400 | port; // IN  R0, port
    routine[offset+1] = 0x24040000;        // IGT R0, R2
    routine[offset+2] = 0x96000000;        // SHL R0, index
    routine[offset+3] = index;             // immediate value
    routine[offset+4] = 0x88200000;        // OR  R1, R0
}

offset                = (index * 5) + 6;
routine[offset]       = 0x4E034000;        // MOV [raw], R1
routine[offset+1]     = (int) &(keyboard -> raw);
routine[offset+2]     = 0x58400000;        // POP R2
routine[offset+3]     = 0x58200000;        // POP R1
routine[offset+4]     = 0x58000000;        // POP R0
routine[offset+5]     = 0x10000000;        // RET
routine[offset+6]     = 0x00000000;        // HLT (for safety)
```

Here, `routine`  is a  68 element  array (67 for  operation, 1  for `HLT`
safety).  It is  packed one  word at  a time,  in ascending  order: array
elements are always  at increasing addresses, which is also  the order in
which the CPU executes.

### obtain typed character for a key: `v32kbd_symbol()`

```
int     v32kbd_symbol (v32kbd *, int keyval);
```

Applies the current shift and caps lock  state to a key code (US layout).
Keys with no character return their same key code                       .

### probe for new keyboard activity: `v32kbd_probe()`

```
bool    v32kbd_probe  (v32kbd **);
```

Call it once every frame. It scans  the device and, if the strobe changed
side, it adds  the new key event  to the input list and  updates the held
keys and caps lock. Returns true when  a key event was received. Up to 64
events can wait in the list; beyond that, new ones are dropped.

### Read the next key: `v32kbd_read()`

The primary transaction of the library: it returns the next key **press**
(releases are skipped), as its  typed character (compatible with the BIOS
font regions  for display of  characters). Keys with no  character return
their key code, which is always under  32 or 127. Returns 0 when there is
nothing left to read.

```
int     v32kbd_read (v32kbd **);
```

### Read the next key event: `v32kbd_readevent()`

```
int     v32kbd_readevent (v32kbd **, bool *pressed);
```

Like `v32kbd_read()` but it returns every event (presses and releases) as
a  key code,  with no  shift applied.  Use one  or the  other on  a given
keyboard: both take events from the same list.

### Check if a key is held: `v32kbd_isdown()`

```
bool    v32kbd_isdown (v32kbd **, int keyval);
```

For  modifiers  and  game-like   controls,  for  instance:

```
state  = v32kbd_isdown (&keyboard, V32KEY_LCTRL);
```
