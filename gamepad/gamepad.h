#ifndef _GAMEPAD_H
#define _GAMEPAD_H

////////////////////////////////////////////////////////////////////////////////////////
//
// gamepad_read()  - function  to  read and  process indicated  gamepad's
// buttons  into a  digitized (0-  not pressed,  1- pressed)  status word
// which is then returned.
//
// Status word layout is as follows:
//
//  * bits 31-11: unused (there are only 11 inputs per gamepad)
//  * bit 10: state of the UP button
//  * bit 9: state of the DOWN button
//  * bit 8: state of the LEFT button
//  * bit 7: state of the RIGHT button
//  * bit 6: state of the START button
//  * bit 5: state of the A button
//  * bit 4: state of the B button
//  * bit 3: state of the X button
//  * bit 2: state of the Y button
//  * bit 1: state of the L button
//  * bit 0: state of the R button
//
// NOTE that the selected gamepad will be altered to the one specified in
// the function parameter as a result of running this function.
//
// In the case that the indicated gamepad is not connected,  a -1 will be
// returned to indicate an error.
//
int  gamepad_read (int gamepad_id)
{
    int  status_word  = -1;

    asm
    {
        mov   R0,                  {gamepad_id}
        out   INP_SelectedGamepad, R0
        in    R0,                  INP_GamepadConnected
        jf    R0,                  _skip

        mov   R0,                  0x4E200000 ; machine code: mov  R1, immediate
        mov   [0x00000000],        R0
        mov   R0,                  0x5C000400 ; immediate:    0x5C000400
        mov   [0x00000001],        R0
        mov   R0,                  0x88200000 ; machine code: or   R1, R0
        mov   [0x00000002],        R0
        mov   R0,                  0x4E034000 ; machine code: mov  [immediate], R1
        mov   [0x00000003],        R0
        mov   R0,                  0x00000005 ; immediate:    0x00000005
        mov   [0x00000004],        R0
        mov   R0,                  0x5C000400 ; machine code: in   R0, INP_port
        mov   [0x00000005],        R0
        mov   R0,                  0x10000000 ; machine code: ret
        mov   [0x00000006],        R0
        mov   R0,                  0x00000000 ; machine code: hlt (for safety)
        mov   [0x00000007],        R0
        _skip:
    }
}

#endif
