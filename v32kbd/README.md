# v32kbd

Vircon32 driver/library for jury-rigged keyboard2gamepad USB gadget.

## IMPLEMENTATION

There are  two current  thoughts on  how to  implement this  on Vircon32.
Until  severe limitations  present  themselves, both  approaches will  be
pursued.

In the end,  the developer will need  to make the v32kbd  calls (once per
frame).

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

### Read the next key: `v32kbd_read()`

The primary  transaction of the library:  it checks for new  key entries,
returning the value (compatible with the  BIOS font region for display of
characters) of the key pressed.

```
int  v32kbd_read (v32kbd *);
```

It takes an instance of `v32kbd`.
