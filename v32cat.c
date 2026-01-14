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

typedef struct byte Byte;
struct byte
{
    int8_t   value;
    uint8_t  flag;
};

typedef struct node Node;
struct node
{
    uint32_t  addr;
    Node     *next;
};

Node *list;
 
////////////////////////////////////////////////////////////////////////////////////////
//
// Function prototypes
//
void  display_offset (int32_t, uint8_t);
void  display_usage  (int8_t *);
Node *add_node       (Node *,  uint32_t);

int32_t  main (int argc, char **argv)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Declare and initialize variables
    //
    FILE     *fptr                 = NULL; // input FILE pointer
    int8_t   *token                = NULL;
    uint8_t   lineflag             = 0;
    uint8_t   flag                 = 0;
    uint8_t   size                 = 0;
    uint8_t   pos                  = 0;
    int32_t   data                 = 0;        // variable holding input
    int32_t   incomplete           = -1;
    uint32_t  lineaddr             = 0;        // start of line address
    uint32_t  offset               = 0;        // count of input bytes
    uint32_t  start                = 0;        // count of input bytes
    uint32_t  bound                = 0;
    int32_t   index                = 0;
    int32_t   opt                  = 0;
    int32_t   wordsize             = 4;        // Vircon32 CPU word size (in bytes)
    uint8_t   linewidth            = wordsize; // how many words per line
    int32_t   this_option_optind   = optind ? optind : 1;
    int32_t   option_index         = 0;
    Byte     *line                 = NULL;     // array for line input
    Node     *tmp                  = NULL;
    int8_t    HEADER[]             = { 'V', '3', '2', '-' };

    list                           = NULL;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // getopt(3) long options and mapping to short options
    //
    struct option long_options[]   = {
       { "address",  required_argument, 0, 'a' },
       { "column",   no_argument,       0, '1' },
       { "range",    required_argument, 0, 'r' },
       { "width",    required_argument, 0, 'W' },
       { "wordsize", required_argument, 0, 'w' },
       { "help",     no_argument,       0, 'h' },
       { "verbose",  no_argument,       0, 'v' },
       { 0,          0,                 0,  0  }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Process command-line arguments, via getopt(3)
    //
    opt                            = getopt_long (argc, argv,
                                                  "a:1r:W:w:hv",
                                                  long_options,
                                                  &option_index);
    while (opt                    != -1)
    {
        switch (opt)
        {
            case '1':
                linewidth          = 1;
                break;

            case 'a':
                offset             = strtol (optarg, NULL, 16);
                tmp                = add_node (tmp, offset);
                break;

            case 'r':
                token              = strtok (optarg, "-");
                offset             = strtol (token, NULL, 16);

                token              = strtok (NULL,   "-");
                bound              = strtol (token, NULL, 16);

                for (index = offset; index <= bound; index++)
                {
                    tmp            = add_node (tmp, index);
                }
                break;

            case 'w':
                wordsize           = atoi (optarg);
                if (wordsize      <  1)
                {
                    fprintf (stderr, "[error] invalid wordsize (%u)!\n", wordsize);
                    exit (4);
                }
                break;

            case 'W':
                linewidth          = atoi (optarg);
                if (linewidth     <  1)
                {
                    fprintf (stderr, "[error] invalid width (%u)!\n", linewidth);
                    exit (5);
                }
                break;

            case 'h':
                display_usage (argv[0]);
                break;

            case 'v':
                break;
        }
        opt                        = getopt_long (argc, argv,
                                                  "a:1r:W:w:hv",
                                                  long_options,
                                                  &option_index);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Allocate space for the line of input
    //
    size                                      = sizeof (Byte) * (wordsize * linewidth);
    line                                      = (Byte *) malloc (size);
    if (line                                 == NULL)
    {
        fprintf (stderr, "[error] Could not allocate space for 'line'\n");
        exit (3);
    }
    
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Open specified file
    //
    fptr                                      = fopen (argv[optind], "r");
    if (fptr                                 == NULL)
    {
        fprintf (stderr, "[error] could not open '%s' for reading!\n", argv[optind]);
        exit (1);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Display offset header
    //
	display_offset (wordsize, linewidth);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Continue until EOF or otherwise interrupted
    //
    offset                                    = start;
    do
    {
        ////////////////////////////////////////////////////////////////////////////////
        //
        // Read and file the line array
        //
        lineaddr                              = offset;
        for (index = 0; index < linewidth * wordsize; index++)
        {
            ////////////////////////////////////////////////////////////////////////////
            //
            // Read a byte of data from the file
            //
            data                              = fgetc (fptr);

            ////////////////////////////////////////////////////////////////////////////
            //
            // Check for EOF, bail out if so
            //
            if (feof (fptr))
            {
                break;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // Not EOF, assign read data value into line array
            //
            (line+index) -> value             = data;

            ////////////////////////////////////////////////////////////////////////////
            //
            // Check for address highlight match (cycle through linked list)
            //
            tmp                               = list;
            while (tmp                       != NULL)
            {
                if (tmp -> addr              == (offset / wordsize))
                {
                    (line+index) -> flag      = 1;
                    lineflag                  = 1;
                    break;
                }
                tmp                           = tmp -> next;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // Byte valid and processing complete, increment offset
            //
            offset                            = offset + 1;
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // If the file has ended, and the current line isn't full: store index
        //
        if (index                            <  (linewidth * wordsize))
        {
            if (feof (fptr))
            {
                incomplete                    = index;
                if (incomplete               == 0)
                {
                    break;
                }
            }
        }
        else
        {
            incomplete                        = -1;
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Check the current line for a V32 HEADER
        //
        data                                  = 0;
        pos                                   = 0;
        for (index = 0; index < (wordsize * linewidth); index++)
        {
            if ((line+index) -> value        == HEADER[pos])
            {
                data                          = data + 1;
                pos                           = pos  + 1;
                
                ////////////////////////////////////////////////////////////////////////
                //
                // If we have a match for a V32 HEADER, set lineflag and bail
                //
                if (data                     == 3)
                {
                    lineflag                  = 8;
                    break;
                }
            }
            else
            {
                data                          = 0;
                pos                           = 0;
            }
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Check for line address highlight: if enabled, do the escape sequence
        //
        if (lineflag                         >  0)
        {
            fprintf (stdout, "\e[1;33m");
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Display left side offset
        //
        fprintf (stdout, "[0x%.8X]  ", lineaddr / wordsize);

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Check for line address highlight: if enabled, wrap the escape sequence
        //
        if (lineflag                         >  0)
        {
            fprintf (stdout, "\e[m");
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Process the line array, displaying (and formatting) line contents
        //
        for (index = 0; index < (linewidth * wordsize); index++)
        {
            ////////////////////////////////////////////////////////////////////////////
            //
            // On incomplete lines, pad spaces to maintain alignment
            //
            if ((incomplete                  >  -1) &&
                (index                       >= incomplete))
            {
                fprintf (stdout, "   ", incomplete);
                if (((index + 1) % wordsize) == 0)
                {
                    fprintf (stdout, " ");
                }
                offset                        = offset + 1;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // Normal line content processing
            //
            else
            {
                ////////////////////////////////////////////////////////////////////////
                //
                // Check for V32 HEADER highlighting
                //
                if (lineflag                 >  1)
                {
                    fprintf (stdout, "\e[1;31m");
                    flag                      = 1;
                }
                
                ////////////////////////////////////////////////////////////////////////
                //
                // Check for address highlight match: enable if confirmed
                //
                if ((line+index) -> flag     == 1)
                {
                    if (flag                 == 0)
                    {
                        fprintf (stdout, "\e[1;32m");
                        flag                  = 1;
                    }
                }
                else
                {
                    flag                      = 0;
                }

                ////////////////////////////////////////////////////////////////////////
                //
                // Display the byte of data
                //
                fprintf (stdout, "%.2hhX ", (line+index) -> value);
                (line+index) -> value         = 0;
                (line+index) -> flag          = 0;

                ////////////////////////////////////////////////////////////////////////
                //
                // If we are on a wordsize boundary, display an extra whitespace
                //
                if (((index + 1) % wordsize) == 0)
                {
                    fprintf (stdout, " ");
                    if (flag                 == 1)
                    {
                        fprintf (stdout, "\e[m");
                        flag                  = 0;
                    }
                }

                ////////////////////////////////////////////////////////////////////////
                //
                // When dealing with V32 HEADERS, lineflag also serves as a counter
                // to highlight bytes; decrement it by one as a byte has just been
                // displayed
                //
                if (lineflag                 >  1)
                {
                    lineflag                  = lineflag - 1;
                }

                ////////////////////////////////////////////////////////////////////////
                //
                // Check for address highlight match: wrap if confirmed
                //
                if ((line+index) -> flag     == 1)
                {
                    if (flag                 == 1)
                    {
                        fprintf (stdout, "\e[m");
                        flag                  = 0;
                    }
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Disable any trailing escape sequence from earlier in the line
        //
        if (flag                             == 1)
        {
            fprintf (stdout, "\e[m");
            flag                              = 0;
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Check for line address highlight: if enabled, do the escape sequence
        //
        if (lineflag                         >  0)
        {
            fprintf (stdout, "\e[1;33m");
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Display right side offset
        //
        fprintf (stdout, "[0x%.8X]\n", (offset - 1) / wordsize);

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Check for line address highlight: if enabled, wrap the escape sequence
        //
        if (lineflag                         >  0)
        {
            fprintf (stdout, "\e[m");
            lineflag                          = 0;
        }

    } while (!feof (fptr));

    display_offset (wordsize, linewidth);

    free (line);

    fclose (fptr);

    return (0);
}

void display_offset (int32_t  wordsize, uint8_t  linewidth)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Declare and initialize variables
    //
    uint8_t  leftpad    = 0;
    uint8_t  rightpad   = 0;
    int32_t  index      = 0;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Calculate and display offset footer
    //
    if ((wordsize % 2) == 0) // even
    {
        leftpad         = (((wordsize / 2) - 1) * 3) + 2;
        rightpad        = (wordsize   / 2) * 3;
    }
    else if (wordsize  == 1) // one
    {
        leftpad         = 0;
        rightpad        = 1;
    }
    else // all other odds
    {
        leftpad         = ((wordsize / 2) * 3) + 1;
        rightpad        = ((wordsize / 2) * 3) + 1;
    }

    fprintf (stdout, "             ");
    for (index = 0; index < linewidth; index++)
    {
        fprintf (stdout, "%*c+%-u%*c", leftpad, ' ', index, rightpad, ' ');
    }
    fprintf (stdout, "\n");
}

void  display_usage (int8_t *argv)
{
    fprintf (stdout, "Usage: %s [OPTION]... FILE\n", argv);
    fprintf (stdout, "Display hex representation of bytes read from FILE.\n\n");
    fprintf (stdout, "Mandatory arguments to long options are mandatory for ");
    fprintf (stdout, "short options too.\n\n");
    fprintf (stdout, "  -a, --address=ADDR         highlight WORD at ADDR\n");
    fprintf (stdout, "  -1, --column               force one WORD column output\n");
    fprintf (stdout, "  -r, --range=ADDR1-ADDR2    highlight WORDs in ADDR range\n");
    fprintf (stdout, "  -W, --width=WIDTH          set line WIDTH (in bytes)\n");
    fprintf (stdout, "  -w, --wordsize=SIZE        set WORD size to SIZE (in bytes)\n");
    fprintf (stdout, "  -v, --verbose              enable operational verbosity\n");
    fprintf (stdout, "  -h, --help                 display this usage information\n\n");
    exit (0);
}

Node *add_node (Node *tmp, uint32_t address)
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

    tmp  -> addr         = address;
    tmp  -> next         = NULL;

    return (tmp);
}

