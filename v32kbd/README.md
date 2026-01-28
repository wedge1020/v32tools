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

### `v32kbd_read()`

The primary  transaction of the library:  it checks for new  key entries,
returning the value of the key pressed.

```
int  v32kbd_read (v32kbd *);
```

It takes an instance of `v32kbd`.
