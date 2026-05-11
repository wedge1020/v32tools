//
// gamepad_test.c - program to open, generate, and send USB gamepad "packets" that
//                  correspond with gamepad inputs to send.
//
//           usage: gamepad_test UP DOWN LEFT RIGHT A B X Y L R START SELECT
//
// NOTE: the program expects a `1` or `0` in the particular argument. `0` means it
// is NOT pressed. A `1` in that position means it is.
//
// Multiple inputs can be set in the same packet.
//
// ALL arguments must be provided. If you're only setting one input,  you MUST set
// the others to `0`.
//
////////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#define  BUTTONS 0
#define  PAD     1
#define  XAXIS   2
#define  YAXIS   3

int32_t  main (int32_t  argc, uint8_t **argv)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Declare and initialize variables
    //
    FILE     *gamepad  = fopen ("/dev/hidg0", "w");
    int32_t   index    = 0;
    uint8_t  *packet   = (uint8_t *) malloc (sizeof (uint8_t) * 4);
    uint8_t  *inputs   = (packet+BUTTONS);
    uint8_t  *dpad     = NULL;
    uint8_t   pos      = 7;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Open the gadget device
    //
    if (gamepad       == NULL)
    {
        fprintf (stderr, "[ERROR] COULD NOT OPEN '/dev/hidg0' FOR WRITING!\n");
        gamepad        = fopen ("/dev/null", "w");
        //exit    (1);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Verify that enough arguments were provided
    //
    if (argc          <  13)
    {
        fprintf (stderr, "[ERROR] NOT ENOUGH GAMEPAD INPUTS SPECIFIED!\n\n");
        fprintf (stderr, "usage: %s UP DOWN LEFT RIGHT A B X Y L R START SELECT\n", *(argv+0));
        exit    (2);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Initialize the gadget packet
    //
    *(packet+BUTTONS)  = 0x00;
    *(packet+PAD)      = 0x00;
    *(packet+XAXIS)    = 0x80; // 0-255: 0 is left, 128 is center, 255 is right
    *(packet+YAXIS)    = 0x80; // 0-255: 0 is up,   128 is center, 255 is down

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Process DPad axes
    //
    for (index         = 1;
         index        <= 4;
         index         = index + 1)
    {
        switch (index)
        {
            case 1: // up
            case 2: // down
                dpad   = (packet+YAXIS);
                break;

            case 3: // left
            case 4: // right
                dpad   = (packet+XAXIS);
                break;
        }
        *dpad          = (atoi (*(argv+index))) ? (255 - ((index % 2) * 255)) : *dpad;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Convert arguments into individual button states, and pack the BUTTONS
    // packet byte
    //
    for (index         = 5;
         index        <= 12;
         index         = index + 1)
    {
        *inputs       |= (atoi (*(argv+index)) << pos);
        pos            = pos   - 1;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Display the states of each D-pad axis
    //
    fprintf (stdout,          "[dpad] X-axis: %.2X, Y-axis: %.2X\n",
             *(packet+XAXIS), *(packet+YAXIS));

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Display the states of each button
    //
    fprintf (stdout, "[inputs] A: %u, B: %u, X: %u, Y: %u, L: %u, R: %u, START: %u, SELECT: %u\n",
                     (((*inputs) & 0x80) >> 7),
                     (((*inputs) & 0x40) >> 6),
                     (((*inputs) & 0x20) >> 5),
                     (((*inputs) & 0x10) >> 4),
                     (((*inputs) & 0x08) >> 3),
                     (((*inputs) & 0x04) >> 2),
                     (((*inputs) & 0x02) >> 1),
                     (((*inputs) & 0x01)));

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Send the inputs
    //
    for (index         = 0;
         index        <  10;
         index         = index + 1)
    {
        ////////////////////////////////////////////////////////////////////////////////
        //
        // Write the packet out to the gadget device
        //
        fwrite (packet, sizeof (uint8_t), 4, gamepad);

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Delay for a fraction of a second (tenth)
        //
        usleep (100000);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Close and de-allocate resources
    //
    fclose (gamepad);
    free   (packet);

    return (0);
}
