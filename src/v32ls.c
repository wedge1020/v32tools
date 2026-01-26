//
// v32ls.c - tool to list recognized V32 cartridge sections based on header information
//
//     usage: v32ls [OPTION...] FILE
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
// Pre-processor directives
//
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#define  MATCH     0

#define  V32_NONE  0
#define  V32_BIOS  1
#define  V32_CART  2
#define  V32_MEMC  4
#define  V32_VBIN  8
#define  V32_VSND  16
#define  V32_VTEX  32

#define  TRUE      1
#define  FALSE     0

int32_t  incomplete;
uint32_t byteoffset;
uint8_t  headertype;
uint8_t  wordsize;
uint32_t wordoffset;

void     display_usage (int8_t *);
void     get_word      (FILE *, int8_t *);
uint32_t wtoi          (int8_t *);

int32_t  main (int  argc, char **argv)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Declare and initialize local variables
    //
    FILE     *fptr                  = NULL;       // current input FILE pointer
    FILE     *verbose               = NULL;       // /dev/null or dup of stdout
    int32_t   byte                  = 0;          // current byte read
    int32_t   count                 = 0;          // secondary index variable
    int32_t   headercheck           = 0;          // comparison variable
    int32_t   index                 = 0;          // primary loop index variable
    int32_t   incomplete            = 0;          // tracking non-word boundary ends
    uint16_t  vtex_wide             = 0;          // width of current VTEX
    uint16_t  vtex_high             = 0;          // height of current VTEX
    uint32_t  vbinoffset            = 0x00000020; // file-based offset of VBIN
    uint32_t  vtexoffset            = 0;          // file-based offset of VTEX section
    uint32_t  vsndoffset            = 0;          // file-based offset of VSND section
    uint32_t  vtexlength            = 0;          // offset of VTEX section length
    uint32_t  value                 = 0;          // integer representation of word
    int8_t   *word                  = NULL;       // array containing word hex bytes
    int8_t   *argptr                = NULL;       // shorthand for argv array
    uint8_t   verboseflag           = FALSE;      // flag for verbose mode
    int32_t   digit_optind          = 0;
    int32_t   this_option_optind    = 0;
    int32_t   option_index          = 0;
    int32_t   opt                   = 0;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // getopt long arguments and their short option mappings
    //
    struct option long_options[]    = {
        {"verbose", no_argument,       0, 'v' },
        {"help",    no_argument,       0, 'h' },
        {0,         0,                 0,  0  }
    };

    opt                             = getopt_long (argc, argv,
                                                   "lv",
                                                   long_options,
                                                   &option_index);
    while (opt                     != -1)
    {
        this_option_optind          = optind ? optind : 1;

        switch (opt)
        {
            case 'l':
            case 'v':
                verboseflag         = TRUE;
                break;

            case 'h':
                display_usage (argv[0]);
                break;
        }

        opt                         = getopt_long (argc, argv,
                                                   "lv",
                                                   long_options,
                                                   &option_index);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // If verbosity has been enabled: verbose FILE pointer points to STDOUT
    //
    if (verboseflag                == TRUE)
    {
        verbose                     = stdout;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // otherwise NO verbosity, open /dev/null
    //
    else
    {
        verbose                     = fopen ("/dev/null", "w");
        if (verbose                == NULL)
        {
            fprintf (stderr, "[error] could not open '/dev/null' for writing!\n");
            exit (2);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Allocate memory for our 'word' array
    //
    value                           = sizeof (int8_t) * (wordsize + 1);
    word                            = (int8_t *) malloc (value);
    if (word                       == NULL)
    {
        fprintf (stderr, "[error] could not allocate memory for 'word'!\n");
        exit (3);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Process for each specified file
    //
    while (optind                  <  argc)
    {
        ////////////////////////////////////////////////////////////////////////////////////
        //
        // Initialize global variables
        //
        incomplete                  = 0;          // tracking non-word boundary ends
        byteoffset                  = 0;          // tally of bytes read from file
        headertype                  = V32_NONE;   // what section we are in of v32
        wordoffset                  = 0;          // word offset from start
        wordsize                    = 4;          // size (in bytes) of word

        ////////////////////////////////////////////////////////////////////////////////////
        //
        // For each remaining specified file, open each in turn for reading
        //
        if (argc                   >  1)
        {
            argptr                  = argv[optind];
            fptr                    = fopen (argptr, "r");
            if (fptr               == NULL)
            {
                fprintf (stderr, "[error] could not open '%s' for reading!\n", argptr);
                exit (4);
            }
        }

        fprintf (stdout, "%s:\n", argv[optind]);

        ////////////////////////////////////////////////////////////////////////////////////
        //
        // Begin processing loop for current file (read until end of file)
        //
        get_word (fptr, word);                               // get initial word
        wordoffset                  = wordoffset - 1;        // fix wordoffset
        while (!feof (fptr))
        {
            ////////////////////////////////////////////////////////////////////////////////
            //
            // We identify headings via their first four bytes
            //
            headercheck             = strncmp (word, "V32-", 4);
            if (headercheck        == MATCH)
            {
                ////////////////////////////////////////////////////////////////////////////
                //
                // Get next word (containing what type of header we have
                //
                get_word (fptr, word);

                ////////////////////////////////////////////////////////////////////////////
                //
                // Display the encountered header and its offset information
                //
                fprintf (stdout, "  > V32-%s header at offset: ", word);
                fprintf (stdout, "0x%.8X\n", (wordoffset - 1));
                switch (*(word+1))
                {
                    case 'A': // header: V32-CART
                        headertype  = V32_CART;
                        get_word (fptr, word);
                        value       = wtoi (word);
                        fprintf (verbose, "    * Vircon32 version:   %u\n", value);
                        get_word (fptr, word);
                        value       = wtoi (word);
                        fprintf (verbose, "    * Vircon32 revision:  %u\n", value);
                        fprintf (verbose, "    * Cartridge Title:    \"");
                        for (index = 0; index < 16; index++)
                        {
                            get_word (fptr, word);
                            fprintf (verbose, "%s", word);
                        }
                        fprintf (verbose, "\"\n");

                        get_word (fptr, word);
                        value       = wtoi (word);
                        fprintf (verbose, "    * ROM version:        %u\n", value);
                        get_word (fptr, word);
                        value       = wtoi (word);
                        fprintf (verbose, "    * ROM revision:       %u\n", value);
                        get_word (fptr, word);
                        value       = wtoi (word);
                        fprintf (verbose, "    * # of VTEX:          %u\n", value);
                        get_word (fptr, word);
                        value       = wtoi (word);
                        fprintf (verbose, "    * # of VSND:          %u\n", value);
                        get_word (fptr, word);
                        value       = wtoi (word) / wordsize;
                        fprintf (verbose, "    * Program ROM offset: 0x%.8X\n", value);
                        get_word (fptr, word);
                        value       = wtoi (word);
                        fprintf (verbose, "    * Program ROM size:   0x%.8X\n", value);
                        get_word (fptr, word);
                        vtexoffset  = wtoi (word) / wordsize;
                        fprintf (verbose, "    * VTEX Offset:        0x%.8X\n", vtexoffset);
                        get_word (fptr, word);
                        value       = wtoi (word) / wordsize;
                        fprintf (verbose, "    * VTEX size:          0x%.8X\n", value);
                        get_word (fptr, word);
                        vsndoffset  = wtoi (word) / wordsize;
                        fprintf (verbose, "    * VSND Offset:        0x%.8X\n", vsndoffset);
                        get_word (fptr, word);
                        value       = wtoi (word) / wordsize;
                        fprintf (verbose, "    * VSND size:          0x%.8X\n", value);
                        get_word (fptr, word);
                        fprintf (verbose, "    * reserved\n");
                        get_word (fptr, word);
                        fprintf (verbose, "    * reserved\n\n");
                        break;

                    case 'B': // V32_VBIN
                        headertype  = V32_VBIN;
                        get_word (fptr, word);
                        value       = wtoi (word);
                        fprintf (verbose, "    * size: %18u (0x%X) words\n\n", value, value);
                        fseek (fptr, (value * 4), SEEK_CUR);
                        wordoffset  = wordoffset + value;
                        break;

                    case 'E': // V32_MEMC
                        headertype  = V32_MEMC;
                        break;

                    case 'I': // V32_BIOS
                        headertype  = V32_BIOS;
                        break;

                    case 'S': // V32_VSND
                        headertype  = V32_VSND;
                        get_word (fptr, word);
                        value       = wtoi (word);
                        fprintf (verbose, "    * # sound samples: %15u\n", value);
                        fseek (fptr, (value * 4), SEEK_CUR);
                        wordoffset  = wordoffset + value;
                        break;

                    case 'T': // V32_VTEX
                        headertype  = V32_VTEX;
                        get_word (fptr, word);
                        vtex_wide   = (uint16_t) wtoi (word);
                        fprintf (verbose, "    * texture resolution: %3u x ", vtex_wide);
                        get_word (fptr, word);
                        vtex_high   = (uint16_t) wtoi (word);
                        fprintf (verbose, "%-3u\n\n", vtex_high);
                        fseek (fptr, (vtex_wide * vtex_high) * 4, SEEK_CUR);
                        wordoffset  = wordoffset + (vtex_wide * vtex_high);
                        break;
                }
            }

            get_word (fptr, word);
        }

        fclose (fptr);
        fprintf (stdout, "\n");
        optind                      = optind + 1;
    }

    fclose (verbose);
    free (word);

    return (0);
}

void  display_usage (int8_t *argv)
{
    fprintf (stdout, "Usage: %s [OPTION]... FILE\n", argv);
    fprintf (stdout, "List recognized Vircon32 section headers read from FILE.\n\n");
    fprintf (stdout, "Mandatory arguments to long options are mandatory for ");
    fprintf (stdout, "short options too.\n\n");
    fprintf (stdout, "  -l                       for ls-style usage (same as -v)\n");
    fprintf (stdout, "  -v, --verbose            enable operational verbosity\n");
    fprintf (stdout, "  -h, --help               display this usage information\n\n");

    exit (0);
}

void  get_word (FILE *fptr, int8_t *word)
{
    int32_t  byte       = 0;
    int32_t  index      = 0;

    for (index = 0; index < 4; index++)
    {
        byte            = fgetc (fptr);
        if (feof (fptr))
        {
            incomplete  = index;
            break;
        }

        *(word+index)   = byte;
        byteoffset      = byteoffset + 1;
    }
    *(word+wordsize)    = '\0';
    wordoffset          = wordoffset + 1;
}

uint32_t wtoi (int8_t *word)
{
    int8_t    shift  = 0;
    int32_t   data   = 0;
    int32_t   index  = 0;
    uint32_t  value  = 0;

    for (index = wordsize - 1; index >= 0; index--)
    {
        data         = *(word+index);
        data         = data  & 0x000000FF;
        shift        = index * 8;
        data         = data << shift;
        value        = value | data;
    }

    return (value);
}
