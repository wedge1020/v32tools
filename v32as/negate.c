#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int32_t  main (int32_t  argc, int8_t **argv)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // value is the hex value passed in
    //
    uint32_t  value  = strtol (argv[1], NULL, 16);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // two's complement
    //
    value            = ~value;
    value            = value + 1;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // display the result
    //
    fprintf (stdout, "0x%.8X\n", value);

    return (0);
}
