#ifndef _KEYBOARD_H
#define _KEYBOARD_H

struct v32key
{
    int     value;  // the key pressed
    v32key *next;
};

struct keyboard
{
    v32key *input;     // start of list (next available key input)
    v32key *data;      // end of the list (new keys appended)
    int     gamepad;   // id of gamepad being used as a v32 keyboard
	int     lastread;  // contains frame of last read (for timing)	
};

v32key *v32kbd_newkey (void);       // allocate new v32key struct for list
int     v32kbd_addkey (v32key *);   // add new key input to keyboard list
void    v32kbd_read   (keyboard *); // check for input

#endif
