//
// floatify.c - obtain raw hex of IEEE754 floating point value
//
//
//
// if we didn't want to have an extra binary kicking around, but did have
// perl installed, apparently this also works:
//
// perl -e 'print pack("f>", 3.14)' | xxd -p
//
// or, python:
//
// python3 -c "import struct; import sys; sys.stdout.buffer.write(struct.pack('f', 10.75))" | xxd -p
//
// ... although the python variant is endianified
//
////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

union i2f
{
    int32_t  i32;
    float    f32;
};
typedef union i2f i2f_t;

int  main (int  argc, char **argv)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // declare variables
    //
    i2f_t  value;
    
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // obtain the floating point value from the command-line into the float
    // element of the union
    //
    value.f32     = strtof (argv[1], NULL);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // display the result, accessing the integer element of the union to see
    // its raw hex representation
    //
    fprintf (stdout, "0x%.8X\n", value.i32);

    return (0);
}
