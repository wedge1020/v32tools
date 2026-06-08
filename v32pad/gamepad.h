#ifndef _GAMEPAD_H
#define _GAMEPAD_H

////////////////////////////////////////////////////////////////////////////////////////
//
// routine - the global array that is used to store the RAM-based machine
// code routine for reading input
//
// routine assumes the currently selected (and connected) gamepad
//
int [39] routine;

void gamepad_init ()
{
    // address is in R0
    // R1 is loop variable value (starting at 2)
    // R2 is data
    // R3 is bit position
    // R4 is status word to return
    // R5 is loop start address
    int  index        = 0;

    routine[index++]  = 0x54200000; // push  R1
    routine[index++]  = 0x54400000; // push  R2
    routine[index++]  = 0x54600000; // push  R3
    routine[index++]  = 0x54800000; // push  R4
    routine[index++]  = 0x54A00000; // push  R5
    routine[index++]  = 0x4CA04000; // mov  R5, R0
    routine[index++]  = 0x9AA00000; // iadd R5, immediate
    routine[index++]  = 0x0000000E; // immediate value
    routine[index++]  = 0x4E200000; // mov   R1, immediate
    routine[index++]  = 0x00000002; // immediate value
    routine[index++]  = 0x4E600000; // mov   R3, immediate
    routine[index++]  = 0x0000000A; // immediate value
    routine[index++]  = 0x4E800000; // mov   R4, immediate
    routine[index++]  = 0x00000000; // immediate value
    routine[index++]  = 0x4E400000; // mov   R2, immediate
    routine[index++]  = 0x5C400400; // immediate value (IN R2, 0x400))
    routine[index++]  = 0x88420000; // or    R2, R1
    routine[index++]  = 0x4E05C000; // mov   [R0+immediate], R2
    routine[index++]  = 0x00000013; // immediate value
    index             = index + 1;  // the custom IN instruction
    routine[index++]  = 0x26400000; // igt   R2, immediate
    routine[index++]  = 0x00000000; // immediate value
    routine[index++]  = 0x94460000; // shl   R2, R3
    routine[index++]  = 0x88840000; // or    R4, R2
    routine[index++]  = 0x9E600000; // isub  R3, immediate
    routine[index++]  = 0x00000001; // immediate value
    routine[index++]  = 0x9A200000; // iadd  R1, immediate
    routine[index++]  = 0x00000001; // immediate value
    routine[index++]  = 0x4C424000; // mov   R2, R1
    routine[index++]  = 0x1E400000; // ieq   R2, immediate
    routine[index++]  = 0x0000000D; // immediate value
    routine[index++]  = 0x184A0000; // jf    R2, R5
    routine[index++]  = 0x4C084000; // mov   R0, R4
    routine[index++]  = 0x58A00000; // pop R5
    routine[index++]  = 0x58800000; // pop R4
    routine[index++]  = 0x58600000; // pop R3
    routine[index++]  = 0x58400000; // pop R2
    routine[index++]  = 0x58200000; // pop R1
    routine[index++]  = 0x10000000; // ret
    routine[index++]  = 0x00000000; // hlt
}

//////////////////////////////////////////////////////////////////////////////
//
// gamepad_read()  - function  to  read and  process indicated  gamepad's
// buttons  into a  digitized (0-  not pressed,  1- pressed)  status word
// which is then returned.
//
// Status word layout is as follows:
//
//  * bits 31-11: unused (there are only 11 inputs per gamepad)
//  * bit 10: state of the LEFT button
//  * bit 9: state of the RIGHT button
//  * bit 8: state of the UP button
//  * bit 7: state of the DOWN button
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
    //////////////////////////////////////////////////////////////////////////
    //
    // declare and initialize status_word variable (-1 indicates that the
    // desired gamepad to read is not connected)
    //
    int  status_word  = -1;
    int *addr         = NULL;

    //////////////////////////////////////////////////////////////////////////
    //
    // if machine code routine has not been initialized, do so now
    //
    if (routine[15]  != 0x5C400400)
    {
        gamepad_init ();
    }

    addr              = routine;

    //////////////////////////////////////////////////////////////////////////
    //
    // check to make sure the gamepad is connected, if so: run the
    // routine
    //
    asm
    {
        "push  R0"

        "mov   R0,                  {gamepad_id}"
        "out   INP_SelectedGamepad, R0"
        "in    R0,                  INP_GamepadConnected"
        "jf    R0,                  _skip"
            
        "mov   R0,                  {addr}"       // obtain array address
        "call  R0"                                // call custom RAM routine

        "mov   {status_word},       R0"           // copy bit-packed value

        "_skip:"
        "pop   R0"
    }

    return (status_word);
}

#endif
