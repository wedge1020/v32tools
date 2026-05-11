# v32kbd

Vircon32 driver/library for jury-rigged keyboard2gamepad USB gadget.

## IMPLEMENTATION

There are two current thoughts on  how to implement this on Vircon32; and
until  severe limitations  present  themselves, both  approaches will  be
pursued.

In the end, the developer will need  to make the `v32kbd` calls (once per
frame, or some other minimal timing threshold).

### BIOS DRIVER

Create  a custom  BIOS, embedding  these  routines therein.  Then, via  C
wrapper functions that call the binary offsets, do the deed.

### LIBRARY

A C-code  includable library  that contains the  routines (much  like the
existing DevTools headers).

## API

v32kbd provides the following functions:

### input key transactional unit: `v32key` struct

### keyboard transactional unit: `v32kbd` struct

### generate new key node for list: `v32key_newkey()`

### initialize keyboard instance: `v32kbd_init()`

### add new key to keyboard input list: `v32kbd_addkey()`

### get next key of input from input list: `v32kbd_getkey()`

### probe for new keyboard activity: `v32kbd_probe()`

```
bool    v32kbd_probe  (v32kbd *);
```

Within this function, an in-RAM custom machine code routine is generated,
`CALL`ed, processed for  each desired INP button port, and  then `RET` at
the end:

```
routine[47]            = 0x54000000;        // PUSH R0
routine[46]            = 0x54200000;        // PUSH R1
routine[45]            = 0x54400000;        // PUSH R2
routine[44]            = 0x4E200000;        // MOV R1, 0
routine[43]            = 0x00000000;        // immediate
routine[42]            = 0x4C424000;        // MOV R2, R1
for (index             = 0;
     index            <  7; 
     index             = index + 1)
{
    offset             = 47 - ((index * 5) + 6);
    port               = index + 6;
    shift              = 6 - index;
    
    routine[offset]    = 0x5C000400 | port; // IN  R0, port
    routine[offset-1]  = 0x24040000;        // IGT R0, R2
    routine[offset-2]  = 0x96000000;        // SHL R0, shift
    routine[offset-3]  = shift;             // immediate value
    routine[offset-4]  = 0x88200000;        // OR  R1, R0
}

offset                 = 47 - (index * 5) + 6;
routine[offset]        = 0x4E034000;        // MOV [keyval], R1
routine[offset-1]      = (int) &keyval;
routine[offset-2]      = 0x58400000;        // POP R2
routine[offset-3]      = 0x58200000;        // POP R1
routine[offset-4]      = 0x58000000;        // POP R0
routine[offset-5]      = 0x10000000;        // RET
routine[offset-6]      = 0x00000000;        // HLT (for safety)
```

Here, `routine`  is a  48 element  array (47 for  operation, 1  for `HLT`
safety). It is packed one word at  a time (in reverse, due to being space
on the stack)  with the desired machine code instruction  to perform. The
desired `INP` IOPort  will be generated via a loop,  allowing each button
press (ie bit) to be packed together nicely.

In  the end,  the 7  bits corresponding  to the  button states  is stored
within the `keyval` variable, which can be used in further processing.

### Read the next key: `v32kbd_read()`

The primary  transaction of the library:  it checks for new  key entries,
returning the value (compatible with the  BIOS font region for display of
characters) of the key pressed.

```
int  v32kbd_read (v32kbd *);
```

It takes an instance of `v32kbd`.
