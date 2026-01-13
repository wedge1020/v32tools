#!/usr/bin/env -S tcc -run
//
// v32cat.c - C script to display hex output of binary file, highlighting
//            recognized sections
//
//     usage: v32cat.c [OPTION...] FILE
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
// Pre-processor directives
//
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

typedef struct node Node;
struct node
{
    uint32_t  addr;
    Node     *next;
};

Node    *list;
 
Node    *add_node        (Node   *,   uint32_t);
void     display_help    (int8_t *);
void     display_version (void);

Node    *add_node (Node *tmp, uint32_t address)
{
    if (list            == NULL)
    {
        list             = (Node *) malloc (sizeof (Node) * 1);
        if (list        == NULL)
        {
            fprintf (stderr, "[add_node] ERROR: Could not allocate for list\n");
            exit (1);
        }

        tmp              = list;
    }
    else
    { 
        tmp -> next      = (Node *) malloc (sizeof (Node) * 1);
        if (tmp -> next == NULL)
        {
            fprintf (stderr, "[add_node] ERROR: Could not allocate for list node\n");
            exit (2);
        }

        tmp              = tmp -> next;
    }

    fprintf (stdout, "[add_node] adding 0x%X to the list\n", address);
    tmp  -> addr         = address;
    tmp  -> next         = NULL;

    return (tmp);
}

int32_t  main (int argc, char **argv)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Declare and initialize variables
    //
    FILE     *fptr                 = NULL; // input FILE pointer
    int8_t   *token                = NULL;
    uint8_t   lineflag             = 0;
    int32_t   byte                 = 0;    // variable holding input
    uint32_t  flag                 = 0;
    uint32_t  offset               = 0;    // count of input bytes
    uint32_t  bound                = 0;
    int32_t   index                = 0;
    int32_t   opt                  = 0;
    int32_t   wordsize             = 4;    // Vircon32 CPU word size
    int32_t   this_option_optind   = optind ? optind : 1;
    int32_t   option_index         = 0;
    Node     *tmp                  = NULL;

    list                           = NULL;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // getopt(3) long options and mapping to short options
    //
    struct option long_options[]   = {
       { "address", required_argument, 0, 'a' },
       { "range",   required_argument, 0, 'r' },
       { "version", no_argument,       0, 'V' },
       { "help",    no_argument,       0, 'h' },
       { "verbose", no_argument,       0, 'v' },
       { 0,         0,                 0,  0  }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Process command-line arguments, via getopt(3)
    //
    opt                            = getopt_long (argc, argv,
                                                  "a:r:hVv",
                                                  long_options,
                                                  &option_index);
    while (opt                    != -1)
    {
        switch (opt)
        {
            case 'a':
                offset             = strtol (optarg, NULL, 16);
                tmp                = add_node (tmp, offset);
                break;

            case 'r':
                fprintf (stdout, "[range]    processing: '%s'\n", optarg);

                token              = strtok (optarg, "-");
                offset             = strtol (token, NULL, 16);

                token              = strtok (NULL,   "-");
                bound              = strtol (token, NULL, 16);

                for (index = offset; index <= bound; index++)
                {
                    tmp            = add_node (tmp, index);
                }
                break;

            case 'h':
                break;

            case 'V':
                break;

            case 'v':
                break;
        }
        opt                        = getopt_long (argc, argv,
                                                  "a:r:hVv",
                                                  long_options,
                                                  &option_index);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Open specified file
    //
    fptr                           = fopen (argv[optind], "r");
    if (fptr                      == NULL)
    {
        fprintf (stderr, "[error] could not open '%s' for reading!\n", argv[1]);
        exit (1);
    }

    tmp                            = list;
    while (tmp                    != NULL)
    {
        fprintf (stdout, "[list] 0x%X\n", tmp -> addr);
        tmp                        = tmp -> next;
    }


    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Display offset header
    //
    fprintf (stdout, "               ");
    for (byte = 0; byte < wordsize; byte++)
    {
        fprintf (stdout, "   +%-6u   ", byte);
    }
    fprintf (stdout, "\n");

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Display initial offset
    //
    offset                         = 0;
    fprintf (stdout, "[0x%.8X]  ", offset / wordsize);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Continue until EOF or otherwise interrupted
    //
    byte                               = fgetc (fptr);
    offset                             = offset + 1;
    flag                               = 0;
    while (!feof (fptr))
    {
        ////////////////////////////////////////////////////////////////////////////////
        //
        // Display the byte
        //
        if (!feof (fptr))
        {
            ////////////////////////////////////////////////////////////////////////////
            //
            // If provided, highlight the matching word when we encounter it
            //
            tmp                        = list;
            while (tmp                != NULL)
            {
                if (tmp -> addr       == ((offset - 1) / wordsize))
                {
                    if (flag          == 0)
                    {
                        fprintf (stdout, "\e[1;32m");
                        flag           = 2;
                        lineflag       = 1;
                    }
                    break;
                }
                else
                {
                    if (flag          == 2)
                    {
                        ////////////////////////////////////////////////////////////////
                        //
                        // Turn off highlighting once we are beyond the matching word
                        //
                        fprintf (stdout, "\e[m");
                        flag           = 0;
                    }
                }

                tmp                    = tmp -> next;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // Display the current byte being processed
            //
            fprintf (stdout, "%.2X ", byte);
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // If we are on a wordsize boundary, display an extra whitespace
        //
        if ((offset % wordsize)       == 0)
        {
            fprintf (stdout, " ");
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // If we are on a line boundary, display the current offset and generate a
        // newline (based on wordsize)
        //
        if ((offset % (wordsize * 4)) == 0)
        {
            if (flag                  == 2)
            {
                fprintf (stdout, "\e[m");
                flag                   = 0;
            }

            if (lineflag              == 1)
            {
                fprintf (stdout, "\e[1;33m");
            }

            fprintf (stdout, "[0x%.8X]\n", ((offset - 1)/ wordsize));

            if (lineflag              == 1)
            {
                fprintf (stdout, "\e[m");
                lineflag               = 0;
            }
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Obtain the byte
        //
        byte                           = fgetc (fptr);

        ////////////////////////////////////////////////////////////////////////////////
        //
        // If there is still more file to go
        //
        if (!feof (fptr))
        {
            ////////////////////////////////////////////////////////////////////////////
            //
            // If we are on a new line: display the current offset (based on wordsize)
            //
            if ((offset % (wordsize * 4)) == 0)
            {
                fprintf (stdout, "[0x%.8X]  ", (offset / wordsize));
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // Update the offset
            //
            offset                         = offset + 1;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Complete any incomplete last line
    //
    if ((offset % (wordsize * 4))         != 0)
    {
        flag                               = offset;
        while ((flag % (wordsize * 4))    != 0)
        {
            fprintf (stdout, "   ");
            if ((flag % wordsize)         == 0)
            {
                fprintf (stdout, " ");
            }
            flag                           = flag + 1;
        }

        fprintf (stdout, " [0x%.8X]\n", (offset / wordsize));
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Display offset footer
    //
    fprintf (stdout, "               ");
    for (byte = 0; byte < wordsize; byte++)
    {
        fprintf (stdout, "   +%-6u   ", byte);
    }
    fprintf (stdout, "\n");

    fclose (fptr);

    return (0);
}
