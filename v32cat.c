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
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#define  FANCY_NEVER    0
#define  FANCY_DEFAULT  1
#define  FANCY_COLORS   2
#define  FANCY_ALWAYS   3

#define  V32_NONE       0
#define  V32_CART       1
#define  V32_VBIN       2
#define  V32_VSND       4
#define  V32_VTEX       8
#define  V32_MEMC       16
#define  V32_BIOS       32

#define  WORD_CLEAR     0
#define  WORD_HOLD      1
#define  NEW_WORD       0
#define  WORD_LOCK      3

#define  WORD_LITTLE    0
#define  WORD_BIG       1

#define  MATCH          1

typedef struct word Word;
struct word
{
    int8_t  *data;
    uint8_t  flag;
};

typedef struct node Node;
struct node
{
    uint32_t  addr;
    Node     *next;
};

struct string_data
{
    int8_t  name[40];
    int8_t  lower;
    int8_t  upper;
};
typedef struct string_data String_data;

Node     *list;
int32_t   incomplete;
uint8_t   wordsize;
uint8_t   linewidth;
uint32_t  byteoffset;
uint32_t  wordoffset;
 
////////////////////////////////////////////////////////////////////////////////////////
//
// Function prototypes
//
void      add_node       (uint32_t);
void      decode         (uint32_t, uint32_t);
void      display_offset (uint8_t,  uint8_t);
void      display_usage  (int8_t *);
uint32_t  wtoi           (Word *,   int32_t, uint8_t);
uint8_t  *get_str_word   (Word *,   int32_t);
void      get_word       (FILE *, Word *);
Word     *new_word       (uint8_t);
void      rev_word       (Word *,   Word *,  uint8_t);

int32_t  main (int argc, char **argv)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Declare and initialize variables
    //
    FILE     *fptr                 = NULL; // input FILE pointer
    FILE     *iptr                 = NULL; // input FILE pointer
	FILE     *verbose              = NULL; // verbosity FILE pointer
	FILE     *color                = NULL; // color FILE pointer
	FILE     *fancy                = NULL; // fancy (content rendering) FILE pointer
    int8_t   *byte                 = NULL;
    uint8_t   headerflag           = 0;
    uint8_t   headertype           = 0;
    uint8_t   dataflag             = 0;
    uint8_t   fancyflag            = FANCY_DEFAULT;
    uint8_t   lineflag             = 0;
    uint8_t   skipflag             = 0;
    uint8_t   wordflag             = WORD_CLEAR;
    uint8_t   progressflag         = 0;
    uint8_t   renderflag           = 1;
    uint8_t   flag                 = 0;
    uint8_t   count                = 0;
    uint8_t   immflag              = 0;
    uint8_t   size                 = 0;
    uint8_t   pos                  = 0;
    int32_t   data                 = 0;        // variable holding input
    int32_t   headerchk            = 0;        // variable holding input
    uint32_t  word                 = 0;        // variable holding condensed word
    uint32_t  immval               = 0;        // variable holding condensed word
    uint32_t  lineaddr             = 0;        // start of line address
    uint32_t  offset               = 0;        // count of input bytes
    uint32_t  offsetmask           = 0;        // mask to apply to current offset
    uint32_t  start                = 0;        // starting address
    uint32_t  stop                 = 0;        // ending address
    uint32_t  bound                = 0;
    int32_t   index                = 0;
    int32_t   wordpos              = 0;
    int32_t   opt                  = 0;
    int32_t   this_option_optind   = optind ? optind : 1;
    int32_t   option_index         = 0;
    Word     *line                 = NULL;     // array for line input
    Word     *line2                = NULL;     // array for line reversal
    Word     *immediate            = NULL;     // array for immediate data
    Word     *imm                  = NULL;              
    Node     *tmp                  = NULL;
    int8_t    HEADER[]             = { 'V', '3', '2', '-' };

    list                           = NULL;
    incomplete                     = -1;
    byteoffset                     = 0;
    wordoffset                     = 0;
    wordsize                       = 4;        // Vircon32 CPU word size (in bytes)
    linewidth                      = wordsize; // how many words per line

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // getopt(3) long options and mapping to short options
    //
    struct option long_options[]   = {
       { "address",  required_argument, 0, 'a' },
       { "colors",   no_argument,       0, 'c' },
       { "column",   no_argument,       0, '1' },
       { "decode",   no_argument,       0, 'd' },
       { "fancy",    no_argument,       0, 'f' },
       { "file",     no_argument,       0, 'F' },
       { "memory",   no_argument,       0, 'm' },
       { "range",    required_argument, 0, 'r' },
       { "raw",      no_argument,       0, 'R' },
       { "start",    required_argument, 0, 's' },
       { "stop",     required_argument, 0, 'S' },
       { "verbose",  no_argument,       0, 'v' },
       { "width",    required_argument, 0, 'W' },
       { "wordsize", required_argument, 0, 'w' },
       { "help",     no_argument,       0, 'h' },
       { 0,          0,                 0,  0  }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Process command-line arguments, via getopt(3)
    //
    opt                            = getopt_long (argc, argv,
                                                  "a:1cdfFmr:Rs:S:vW:w:h",
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
                add_node (offset);
                break;

            case 'c':
                fancyflag          = FANCY_COLORS;
				if (color         == NULL)
				{
					color          = stdout;
				}
                break;

            case 'd':
                linewidth          = 1;
                renderflag         = 0;
                break;

            case 'f':
                fancyflag          = FANCY_ALWAYS;
				if (color         == NULL)
				{
					color          = stdout;
				}

				if (fancy         == NULL)
				{
					fancy          = stdout;
				}
                break;

            case 'F':
                offsetmask         = 0x00000000;
                break;

            case 'm':
                offsetmask         = 0x20000000;
                break;

            case 'r':
                byte               = strtok (optarg, "-");
                offset             = strtol (byte,   NULL, 16);

                byte               = strtok (NULL,   "-");
                bound              = strtol (byte,   NULL, 16);

                for (index = offset; index <= bound; index++)
                {
                    add_node (index);
                }
                break;

            case 'R':
                fancyflag          = FANCY_NEVER;
				if (color         == NULL)
				{
					color          = fopen ("/dev/null", "w");
				}

				if (fancy         == NULL)
				{
					fancy          = fopen ("/dev/null", "w");
				}
                break;

            case 's':
                start              = strtol (optarg, NULL, 16);
                if ((stop         >  0) &&
                    (stop         <  start))
                {
                    fprintf (stderr, "[error] start address larger than end\n");
                    start          = 0;
                }
                break;

            case 'S':
                stop               = strtol (optarg, NULL, 16);
                if (stop          <  start)
                {
                    fprintf (stderr, "[error] end address smaller than start\n");
                    stop           = 0;
                }
                break;

            case 'v':
                break;

            case 'w':
                wordsize           = (uint8_t) atoi (optarg);
                if (wordsize      <  1)
                {
                    fprintf (stderr, "[error] invalid wordsize (%u)!\n", wordsize);
                    exit (4);
                }
                break;

            case 'W':
                linewidth          = (uint8_t) atoi (optarg);
                if (linewidth     <  1)
                {
                    fprintf (stderr, "[error] invalid width (%u)!\n", linewidth);
                    exit (5);
                }
                break;

            case 'h':
                display_usage (argv[0]);
                break;
        }
        opt                        = getopt_long (argc, argv,
                                                  "a:1cdfr:Rs:S:vW:w:h",
                                                  long_options,
                                                  &option_index);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Check that STDOUT (fd 1) is a genuine terminal
    //
    if (isatty (STDOUT_FILENO))
    {
        fprintf (stdout, "[verbose] standard terminal detected\n");
    }
    else
    {
        fprintf (stdout, "[verbose] pipe or file redirection detected\n");
        if (fancyflag                        == FANCY_DEFAULT)
        {
            fancyflag                         = FANCY_NEVER;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Allocate space for the transaction word variables
    //
    line                                      = new_word (linewidth);
    line2                                     = new_word (linewidth);
    immediate                                 = new_word (wordsize);

/*
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Allocate space for the line reversal array
    //
    size                                      = sizeof (Word) * (wordsize * linewidth);
    line2                                     = (Word *) malloc (size);
    if (line2                                == NULL)
    {
        fprintf (stderr, "[error] Could not allocate space for 'line2'\n");
        exit (4);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Allocate space for the immediate value
    //
    size                                      = sizeof (Word) * (wordsize * linewidth);
    immediate                                 = (Word *) malloc (size);
    if (immediate                            == NULL)
    {
        fprintf (stderr, "[error] Could not allocate space for 'immediate'\n");
        exit (5);
    }
*/   
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
    // Open specified file (for immediate value access)
    //
    iptr                                      = fopen (argv[optind], "r");
    if (iptr                                 == NULL)
    {
        fprintf (stderr, "[error] could not open '%s' for reading!\n", argv[optind]);
        exit (2);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Seek to starting offset (if specified)
    //
    if (start                                >  0)
    {
        fseek (fptr, (start * wordsize), SEEK_SET);
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
    wordoffset                                = start;
    pos                                       = 0;
    do
    {
        ////////////////////////////////////////////////////////////////////////////////
        //
        // Read from the file and build the line array
        //
        lineaddr                              = wordoffset;
        for (index = 0; index < linewidth; index++)
        {
            for (wordpos = 0; wordpos < wordsize; wordpos++)
            {
                ////////////////////////////////////////////////////////////////////////
                //
                // If we have  an upper bound address that is  not EOF (stop argument
                // set), check to see if we have hit it, bail out if so
                //
                if (stop                     >  0)
                {
                    if (wordoffset           >  stop)
                    {
                        fseek (fptr, 0, SEEK_END);
                    }
                }

                ////////////////////////////////////////////////////////////////////////
                //
                // Read a byte of data from the file
                //
                data                          = fgetc (fptr);

                ////////////////////////////////////////////////////////////////////////
                //
                // Check for EOF condition
                //
                if (feof (fptr))
                {
                    incomplete                = index * wordpos;
                    break;
                }

                ////////////////////////////////////////////////////////////////////////
                //
                // Store that byte in our current word of the line array
                //
                byte                          = (line+index) -> data;
                *(byte+wordpos)               = data;
                byteoffset                    = byteoffset + 1;
            }
            wordoffset                        = wordoffset + 1;

            ////////////////////////////////////////////////////////////////////////////
            //
            // If we aren't already actively servicing a header, check for V32 HEADER
            //
            if (headerflag               == 0)
            {
                ////////////////////////////////////////////////////////////////////////
                //
                // Check to see if we have encountered a V32 header
                //
                headerchk                 = strncmp ((line+index) -> data, "V32-", 4);
                if (headerchk            == MATCH)
                {
                    count                 = count + 1;
                    pos                   = pos   + 1;
                
                    ////////////////////////////////////////////////////////////////////
                    //
                    // If we have enough to match a V32 HEADER, set lineflag and bail
                    //
                    if (count                == 3)
                    {
                        headerflag            = 4; // our next target: header type
                        lineflag              = 9; // highligh all 8 values
                        count                 = 0;
                        pos                   = 0;
                    }
                }

                ////////////////////////////////////////////////////////////////////////
                //
                // Matching has failed, reset
                //
                else
                {
                    count                     = 0;
                    pos                       = 0;
                }
            }

            if (headerflag                   >  0)
            {
                headerflag                    = headerflag - 1;
                if (headerflag               == 0)
                {
                    switch (data)
                    {
                        case 'A': // C-A-RT
                            headertype        = V32_CART;
                            dataflag          = 30;
                            break;

                        case 'B': // V-B-IN
                            headertype        = V32_VBIN;
                            dataflag          = 1;
                            break;

                        case 'S': // V-S-ND
                            headertype        = V32_VSND;
                            dataflag          = 1;
                            break;

                        case 'T': // V-T-EX
                            headertype        = V32_VTEX;
                            dataflag          = 2;
                            break;

                        case 'E': // M-E-MC
                            headertype        = V32_MEMC;
                            dataflag          = 1;
                            break;

                        case 'I': // B-I-OS
                            headertype        = V32_BIOS;
                            dataflag          = 14;
                            break;

                        default:
                            fprintf (stdout, "HEADERTYPE hit default\n");
                            headertype        = 0;
                            dataflag          = 0;
                            break;
                    }
                }
            }

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
            //(line+index) -> data              = data;

            ////////////////////////////////////////////////////////////////////////////
            //
            // Check for address highlight match (cycle through linked list)
            //
            tmp                               = list;
            while (tmp                       != NULL)
            {
                if (tmp -> addr              == wordoffset)
                {
                    (line+index) -> flag      = 1;
                    lineflag                  = 1;
                    break;
                }
                tmp                           = tmp -> next;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // Word valid and processing complete, increment offset
            //
            wordoffset                        = wordoffset + 1;
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
        // Check for line address highlight: if enabled, do the escape sequence
        //
        if (lineflag                         >  0)
        {
            if (fancyflag                    != FANCY_NEVER)
            {
                fprintf (stdout, "\e[1;33m");
            }
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Display left side offset
        //
        fprintf (stdout, "[0x%.8X]  ", (lineaddr | offsetmask));

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Check for line address highlight: if enabled, wrap the escape sequence
        //
        if (lineflag                         >  0)
        {
			fprintf (color, "\e[m");
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Process the line array, displaying (and formatting) line contents
        //
        for (index = 0; index < linewidth; index++)
        {
			for (count = 0; count < wordsize; count++)
			{
				////////////////////////////////////////////////////////////////////////
				//
				// On incomplete lines, pad spaces to maintain alignment
				//
				if ((incomplete              >  -1) &&
					(index                   >= incomplete))
				{
					fprintf (stdout, "   ", incomplete);
					if (count                == 0)
					{
						fprintf (stdout, " ");
					}
					wordoffset                = wordoffset + 1;
				}

				////////////////////////////////////////////////////////////////////////
				//
				// Normal line content processing
				//
				else
				{
					////////////////////////////////////////////////////////////////////
					//
					// Check for address highlight match: enable if confirmed
					//
					if ((line+index) -> flag == 1)
					{
						if (flag             == 0)
						{
							fprintf (color, "\e[1;32m");
							flag              = 1;
						}
					}

					////////////////////////////////////////////////////////////////////
					//
					// Check for V32 HEADER highlighting
					//
					else if (lineflag        >  1)
					{
						if (flag             == 0)
						{
							fprintf (color, "\e[1;31m");
							flag              = 1;
						}
					}

					////////////////////////////////////////////////////////////////////
					//
					// Check for V32 HEADER data highlighting
					//
					else if (dataflag        >  0)
					{
						switch (headertype)
						{
							case V32_CART:
							case V32_BIOS:
								if (flag     == 0)
								{
									fprintf (color, "\e[1;35m");
									flag      = 1;
								}
								break;

							case V32_VBIN:
								if (flag     == 0)
								{
									fprintf (color, "\e[1;34m");
									flag      = 1;
								}
								break;
							
							case V32_VSND:
								if (flag     == 0)
								{
									fprintf (color, "\e[1;33m");
									flag      = 1;
								}
								break;

							case V32_VTEX:
								if (flag     == 0)
								{
									fprintf (color, "\e[1;36m");
									flag      = 1;
								}
								break;

							case V32_MEMC:
								break;    
						}
					}

					////////////////////////////////////////////////////////////////////
					//
					// Otherwise, no special formatting
					//
					else
					{
						flag                  = 0;
					}

					////////////////////////////////////////////////////////////////////
					//
					// Collect the current word bytes into one integer
					//
					if (count                == NEW_WORD)
					{
						word                  = wtoi (line, index, WORD_LITTLE);
						size                  = wordsize * linewidth;
						rev_word (line, line2, size);

						if (headertype       == V32_VBIN)
						{
							immflag           = (word & 0x02000000) >> 25;
							if (immflag      == 1)
							{
								fseek (iptr, ftell (fptr), SEEK_SET);
								get_word (fptr, immediate);
								immval        = wtoi (immediate, 0, WORD_LITTLE);
							}
							else
							{
								immflag       = 0;
								immval        = 0;
							}
						}
					}

					////////////////////////////////////////////////////////////////////
					//
					// Display the byte of data
					//
					if (lineflag             >  1)
					{
						byte                  = (line+index) -> data;
						if (fancyflag        != FANCY_NEVER)
						{
							fprintf (fancy,  "%2c ",   *(byte+count));
						}
						else
						{
							fprintf (stdout, "%2hhX ", *(byte+count));
						}
					}

					////////////////////////////////////////////////////////////////////
					//
					// Display the header data contents
					//
					else if ((dataflag       >  0) &&
							 (wordflag       != WORD_HOLD))
					{
						word                  = wtoi (line, index, WORD_LITTLE);
						switch (headertype)
						{
							case V32_CART:
							case V32_BIOS:
								switch (dataflag)
								{
									case 30: // Vircon32 version
										fprintf (fancy, "v32 ver:%3u ", word);
										break;

									case 29: // Vircon32 revision
										fprintf (fancy, "v32 rev:%3u ", word);
										break;

									case 12: // ROM version
										fprintf (fancy, "ROM ver:%3u ", word);
										break;

									case 11: // ROM revision
										fprintf (fancy, "ROM rev:%3u ", word);
										break;

									case 10: // number of textures
										fprintf (fancy, "# VTEX: %3u ", word);
										break;

									case 9: // number of sounds
										fprintf (fancy, "# VSND: %3u ", word);
										break;

									case 8: // Program ROM offset
										fprintf (fancy, "ROM: 0x%.4X ", (word / wordsize));
										break;

									case 7: // Program ROM size
										fprintf (fancy, "size:%.6X ",    word);
										break;

									case 6: // Video ROM offset
										fprintf (fancy, "VO:%.8X ",     (word / wordsize));
										break;

									case 5: // Video ROM size
										fprintf (fancy, "VS:%.8X ",     (word / wordsize));
										break;

									case 4: // Audio ROM offset
										fprintf (fancy, "AO:%.8X ",     (word / wordsize));
										break;

									case 3: // Audio ROM size
										fprintf (fancy, "AS:%.8X ",     (word / wordsize));
										break;

									case 2: // Reserved
									case 1: // Reserved
										fprintf (fancy, "%-11s ",       "[reserved]");
										break;

									default: // CART name
										byte  = (line+index) -> data;
										fprintf (fancy, "%-11s ",  byte);
										break;
								}
								break;

							case V32_VBIN:
								fprintf (stdout, "size: %.5X ", word);
								break;
                        
							case V32_VSND:
								switch (dataflag)
								{
									case 1: // height
										fprintf (stdout, "# %-9u ",    word);
										break;
								}
								break;

							case V32_VTEX:
								switch (dataflag)
								{
									case 2: // width
										fprintf (stdout, "W: %6u x ",  word);
										break;

									case 1: // height
										fprintf (stdout, "H: %-8u ",   word);
										break;
								}
								break;

							case V32_MEMC:
								break;    
						}

                    ////////////////////////////////////////////////////////////////////
                    //
                    // When dealing with V32 header DATA, dataflag serves
                    // as a  counter to highlight bytes;  decrement it by
                    // one as a byte has just been displayed
                    //
                    dataflag                  = dataflag - 1;
                }

                ////////////////////////////////////////////////////////////////////////
                //
                // otherwise: a standard byte to display
                //
                else
                {
					switch (headertype)
					{
						case V32_VBIN:
							byte           = (line2+index) -> data;
							for (count = 0; count < wordsize; count++)
							{
								fprintf (stdout, "%.2hhX ", *(byte+count));
							}
							break;

						case V32_VSND:
							imm            = line;
							if (fancyflag != FANCY_NEVER)
							{
								imm        = line2;
							}

							byte           = (imm+index) -> data;
							for (count = 0; count < wordsize; count++)
							{
								switch (count)
								{
									case 0:    // RED (right channel)
									case 1:
										fprintf (color, "\e[1;31m");
										break;

									case 2:    // CYAN (left channel)
									case 3:
										fprintf (color, "\e[1;36m");
										break;
								}
								fprintf (stdout, "%.2hhX ", *(byte+count));
							}
							fprintf (color, "\e[m");
							break;

						case V32_VTEX:
							imm            = line;
							if (fancyflag != FANCY_NEVER)
							{
								imm        = line2;
							}

							byte           = (imm+index) -> data;
							for (count = 0; count < wordsize; count++)
							{
								switch (count)
								{
									case 0:    // RED
										fprintf (color, "\e[1;31m");
										break;

									case 1:    // GREEN
										fprintf (color, "\e[1;32m");
										break;

									case 2:    // BLUE
										fprintf (color, "\e[1;34m");
										break;

									case 3:    // ALPHA
										fprintf (color, "\e[1;37m");
										break;
								}
								fprintf (stdout, "%.2hhX ", *(byte+count));
							}
							fprintf (color, "\e[m");
							break;

						default:
							byte       = (line+index) -> data;
							for (count = 0; count < wordsize; count++)
							{
								fprintf (stdout, "%.2hhX ", *(byte+count));
							}
							break;
					}
				}

                ////////////////////////////////////////////////////////////////////////
                //
                // Clear the current array entry flag
                //
                (line+index) -> flag          = 0;

                ////////////////////////////////////////////////////////////////////////
                //
                // If we are on a wordsize boundary, display an extra whitespace
                //
				if (count                    == 0)
                {
                    fprintf (stdout, " ");
                    if (flag                 == 1)
                    {
						fprintf (fancy, "\e[m");
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
						fprintf (color, "\e[m");
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
			fprintf (color, "\e[m");
            flag                              = 0;
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // right side offset rendering
        //
        if (renderflag                       == 1)
        {
            ////////////////////////////////////////////////////////////////////////////
            //
            // Check for line address highlight: if enabled, do the escape sequence
            //
            if (lineflag                     >  0)
            {
				fprintf (color, "\e[1;33m");
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // Display right side offset
            //
            fprintf (stdout, "[0x%.8X]\n", (wordoffset | offsetmask));
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Handle decoding mode
        //
        else
        {
            if ((headertype                      == V32_VBIN) &&
                (wordoffset                      >  0x23))
            {
                if (skipflag                     == 0)
                {
                    decode (word, immval);
                    if (immflag                  == 1)
                    {
                        skipflag                  = 1;
                    }
                }
                else
                {
                    fprintf (stdout, "\n");
                    immflag                       = 0;
                    skipflag                      = 0;
                }
            }
            else
            {
                fprintf (stdout, "\n");
            }
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Check for line address highlight: if enabled, wrap the escape sequence
        //
        if (lineflag                         >  0)
        {
			fprintf (color, "\e[m");

            if (lineflag                     == 1)
            {
                lineflag                      = 0;
            }
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Clear current line array entry
        //
		byte                                  = (line+index) -> data;
		for (count = 0; count < wordsize; count++)
		{
			*(byte+count)                     = 0;
		}

    } while (!feof (fptr));

    display_offset (wordsize, linewidth);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // De-allocate the list nodes
    //
    tmp                                       = list;
    while (list                              != NULL)
    {
        list                                  = list -> next;
        free (tmp);
        tmp                                   = list;
    }

    free (line);
    free (line2);
    free (immediate);

    fclose (fptr);

    return (0);
}

void display_offset (uint8_t  wordsize, uint8_t  linewidth)
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
    fprintf (stdout, "  -d, --decode               decode instructions in-line\n");
    fprintf (stdout, "  -f, --fancy                enable fancy content rendering\n");
    fprintf (stdout, "  -F, --file                 reference offset from file\n");
    fprintf (stdout, "  -m, --memory               reference offset from memory\n");
    fprintf (stdout, "  -r, --range=ADDR1-ADDR2    highlight WORDs in ADDR range\n");
    fprintf (stdout, "  -R, --raw                  no fancy content or colors\n");
    fprintf (stdout, "  -s, --start=ADDR           start processing at ADDR\n");
    fprintf (stdout, "  -S, --stop=ADDR            stop processing at ADDR\n");
    fprintf (stdout, "  -W, --width=WIDTH          set line WIDTH (in bytes)\n");
    fprintf (stdout, "  -w, --wordsize=SIZE        set WORD size to SIZE (in bytes)\n");
    fprintf (stdout, "  -v, --verbose              enable operational verbosity\n");
    fprintf (stdout, "  -h, --help                 display this usage information\n\n");

    exit (0);
}

void  add_node (uint32_t address)
{
    Node *tmp                = NULL;
    Node *tmp2               = NULL;

    if (list                == NULL)
    {
        list                 = (Node *) malloc (sizeof (Node) * 1);
        if (list            == NULL)
        {
            fprintf (stderr, "[add_node] ERROR: Could not allocate for list\n");
            exit (1);
        }

        list -> addr         = address;
        list -> next         = NULL;
    }
    else
    {
        tmp                  = list;

        tmp2                 = (Node *) malloc (sizeof (Node) * 1);
        if (tmp2            == NULL)
        {
            fprintf (stderr, "[add_node] ERROR: Could not allocate for list node\n");
            exit (2);
        }

        tmp2 -> addr         = address;
        tmp2 -> next         = NULL;

        ////////////////////////////////////////////////////////////////////////////////
        //
        // If the new address belongs at the start of the list (being the lowest)
        //
        if (address         <  tmp -> addr)
        {
            tmp2 -> next     = list;
            list             = tmp2;
        }
        else
        {
            ////////////////////////////////////////////////////////////////////////////
            //
            // Check if the new address has already been placed in the list
            //
            while (tmp -> next  != NULL)
            {
                ////////////////////////////////////////////////////////////////////////
                //
                // If we have a match, set tmp to NULL and break out of the loop
                //
                if (address     == tmp -> addr)
                {
                    free (tmp2);
                    tmp2         = NULL;
                    break;
                }

                ////////////////////////////////////////////////////////////////////////
                //
                // If address is less than the address of the current node, break
                // out of the loop here
                //
                if (address     <  tmp -> next -> addr)
                {
                    tmp2 -> next = tmp -> next;
                    tmp  -> next = tmp2;
                    tmp2         = NULL;
                    break;
                }

                tmp              = tmp -> next;
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // If no assignment has taken place, slap it on the end
            //
            if (tmp2            != NULL)
            {
                tmp -> next      = tmp2;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// wtoi(): assemble individual bytes in a word into an integer
//
uint32_t  wtoi (Word *line, int32_t  offset, uint8_t flag)
{
    int32_t   index  = 0;
    uint32_t  data   = 0;
    uint32_t  shift  = 0;
    uint32_t  word   = 0;

    for (index = 0; index < 4; index++)
    {
        data         = (uint32_t) (line+offset+index) -> value;
        data         = data & 0x000000FF;
        if (flag    == WORD_LITTLE)
        {
            shift    = index * 8;
        }
        else
        {
            shift    = 24 - (index * 8);
        }
        data         = data << shift;
        word         = word | data;
    }

    return (word);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// get_str_word(): assemble individual bytes in a word into a string
//
uint8_t  *get_str_word (Word *line, int32_t  offset)
{
    int32_t   index        = 0;
    uint32_t  data         = 0;
    uint8_t   pos          = 0;
    uint8_t   word[12];

    for (index = 0; index < 4; index++)
    {
        word[(3*index)+0]  = ' ';
        word[(3*index)+1]  = (line+offset+index) -> value;
        word[(3*index)+2]  = ' ';
    }
    word[11]               = '\0';

    return (word);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// decode(): decode and display instruction and associated data at provided address
//
void  decode (uint32_t word, uint32_t immediate_value)
{
    uint8_t   opcode     = 0;
    uint8_t   immflag    = 0;
    uint8_t   dstreg     = 0;
    uint8_t   srcreg     = 0;
    uint8_t   addrmode   = 0;
    uint8_t   size       = 0;
    uint8_t   flag       = 0;
    uint8_t   category   = 0;
    uint8_t   attribute  = 0;
    uint16_t  portnum    = 0;
    uint32_t  data[11];

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // initialize our opcodes array with the available instructions
    //
    String_data  opcodes[64]  =
    {
        { "HLT"  }, { "WAIT"  }, { "JMP"   }, { "CALL" },
        { "RET"  }, { "JT"    }, { "JF"    }, { "IEQ"  },
        { "INE"  }, { "IGT"   }, { "IGE"   }, { "ILT"  },
        { "ILE"  }, { "FEQ"   }, { "FNE"   }, { "FGT"  },
        { "FGE"  }, { "FLT"   }, { "FLE"   }, { "MOV"  },
        { "LEA"  }, { "PUSH"  }, { "POP"   }, { "IN"   },
        { "OUT"  }, { "MOVS"  }, { "SETS"  }, { "CMPS" },
        { "CIF"  }, { "CFI"   }, { "CIB"   }, { "CFB"  },
        { "NOT"  }, { "AND"   }, { "OR"    }, { "XOR"  },
        { "BNOT" }, { "SHL"   }, { "IADD"  }, { "ISUB" },
        { "IMUL" }, { "IDIV"  }, { "IMOD"  }, { "ISGN" },
        { "IMIN" }, { "IMAX"  }, { "IABS"  }, { "FADD" },
        { "FSUB" }, { "FMUL"  }, { "FDIV"  }, { "FMOD" },
        { "FSGN" }, { "FMIN"  }, { "FMAX"  }, { "FABS" },
        { "FLR"  }, { "CEIL"  }, { "ROUND" }, { "SIN"  },
        { "ACOS" }, { "ATAN2" }, { "LOG"   }, { "POW"  }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // initialize our port category arrays with the available port names
    //
    ////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // time port names
    //
    String_data  tim_ports[4]   =
    {
        { "TIM_CurrentDate"          },
        { "TIM_CurrentTime"          },
        { "TIM_FrameCounter"         },
        { "TIM_CycleCounter"         }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // RNG port names
    //
    String_data  rng_ports[1]   =
    {
        { "RNG_CurrentValue"         }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // GPU port names
    //
    String_data  gpu_ports[18]  =
    {
        { "GPU_Command"              },
        { "GPU_RemainingPixels"      },
        { "GPU_ClearColor"           },
        { "GPU_MultiplyColor"        },
        { "GPU_ActiveBlending"       },
        { "GPU_SelectedTexture"      },
        { "GPU_SelectedRegion"       },
        { "GPU_DrawingPointX"        },
        { "GPU_DrawingPointY"        },
        { "GPU_DrawingScaleX"        },
        { "GPU_DrawingScaleY"        },
        { "GPU_DrawingAngle"         },
        { "GPU_RegionMinX"           },
        { "GPU_RegionMinY"           },
        { "GPU_RegionMaxX"           },
        { "GPU_RegionMaxY"           },
        { "GPU_RegionHotspotX"       },
        { "GPU_RegionHotspotY"       }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // SPU port names
    //
    String_data  spu_ports[14]  =
    {
        { "SPU_Command"              },
        { "SPU_GlobalVolume"         },
        { "SPU_SelectedSound"        },
        { "SPU_SelectedChannel"      },
        { "SPU_SoundLength"          },
        { "SPU_SoundPlayWithLoop"    },
        { "SPU_SoundLoopStart"       },
        { "SPU_SoundLoopEnd"         },
        { "SPU_ChannelState"         },
        { "SPU_ChannelAssignedSound" },
        { "SPU_ChannelVolume"        },
        { "SPU_ChannelSpeed"         },
        { "SPU_ChannelLoopEnabled"   },
        { "SPU_ChannelPosition"      }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // INP port names
    //
    String_data  inp_ports[13]  =
    {
        { "INP_SelectedGamepad"      },
        { "INP_GamepadConnected"     },
        { "INP_GamepadLeft"          },
        { "INP_GamepadRight"         },
        { "INP_GamepadUp"            },
        { "INP_GamepadDown"          },
        { "INP_GamepadButtonStart"   },
        { "INP_GamepadButtonA"       },
        { "INP_GamepadButtonB"       },
        { "INP_GamepadButtonX"       },
        { "INP_GamepadButtonY"       },
        { "INP_GamepadButtonL"       },
        { "INP_GamepadButtonR"       }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // CAR port names
    //
    String_data  car_ports[4]   =
    {
        { "CAR_Connected"            },
        { "CAR_ProgramROMSize"       },
        { "CAR_NumberOfTextures"     },
        { "CAR_NumberOfSounds"       }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // MEM port names
    //
    String_data  mem_ports[1]   =
    {
        { "MEM_Connected"            }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // connect all the categories together
    //
    size                        = sizeof (String_data *) * 7;
    String_data **port          = (String_data **) malloc (size);
    *(port+0)                   = tim_ports;
    *(port+1)                   = rng_ports;
    *(port+2)                   = gpu_ports;
    *(port+3)                   = spu_ports;
    *(port+4)                   = inp_ports;
    *(port+5)                   = car_ports;
    *(port+6)                   = mem_ports;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // mask out and obtain individual instruction elements
    //
    opcode                      = (word    & 0xFC000000) >> 26;
    immflag                     = (word    & 0x02000000) >> 25;
    dstreg                      = (word    & 0x01E00000) >> 21;
    srcreg                      = (word    & 0x001E0000) >> 17;
    addrmode                    = (word    & 0x0001C000) >> 14;
    portnum                     = (word    & 0x00003FFF);

    category                    = (portnum & 0x00000700) >> 8;
    attribute                   = (portnum & 0x000000FF);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // display the obtained instruction
    //
    fprintf (stdout, "%-5s ", opcodes[opcode].name);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // special case processing:
    //   * zero parameter instructions (we're done)
    //   * one parameter instruction (display the first parameter and return)
    //   * everything else is a two parameter instruction
    //
    switch (opcode)
    {
        // zero parameter instructions
        case 0x00: // HLT
        case 0x01: // WAIT
        case 0x04: // RET
        case 0x19: // MOVS
        case 0x1A: // SETS
            fprintf (stdout, "\n");
            return;
            break;

        // one parameter instructions
        case 0x02: // JMP
        case 0x03: // CALL
        case 0x15: // PUSH
        case 0x16: // POP
        case 0x1B: // CMPS
        case 0x1C: // CIF
        case 0x1D: // CFI
        case 0x1E: // CIB
        case 0x1F: // CFB
        case 0x20: // NOT
        case 0x24: // BNOT
        case 0x2B: // ISGN
        case 0x2E: // IABS
        case 0x34: // FSGN
        case 0x37: // FABS
        case 0x38: // FLR
        case 0x39: // CEIL
        case 0x3A: // ROUND
        case 0x3B: // SIN
        case 0x3C: // ACOS
        case 0x3E: // LOG
            if (immflag        == 1)
            {
                fprintf (stdout, "0x%X\n", immediate_value);
            }
            else // register variant
            {
                fprintf (stdout, "R%hhu\n", dstreg);
            }
            return;
            break;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // ALL the first parameter processing for two parameter
    // instructions (with special cases for MOV and OUT)
    //
    if (opcode                 == 0x13) // MOV
    {
        if (addrmode           >  4) // MOV addressing modes 5, 6, 7 we dereference
        {
            fprintf (stdout, "[");
        }

        if ((addrmode          == 5)  ||
            (addrmode          == 7))
        {
            if (addrmode       == 7)
            {
                fprintf (stdout, "R%hhu+", srcreg);
            }

            // print out `immediate_value`
            fprintf (stdout, "0x%X", immediate_value);
        }
        else // otherwise, a register reference is made
        {
            fprintf (stdout, "R%hhu", dstreg);
        }

        if (addrmode           >  4)
        {
            fprintf (stdout, "]");
        }
    }
    else if (opcode            == 0x18) // OUT: first parameter is always a port number
    {
        fprintf (stdout, "%s", (*(port+category)+attribute) -> name);
    }
    else // AVERAGE CASE: any other two parameter instruction; show dstreg
    {
        fprintf (stdout, "R%hhu", dstreg);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Display the parameter separating comma and space
    //
    fprintf (stdout, ", ");

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Two parameter, second parameter display. Special cases for LEA, OUT
    //
    flag                        = 0;
    if (opcode                 == 0x14) // LEA
    {
        fprintf (stdout, "[");
        flag                    = 1;

        if (immflag            == 1)
        {
            fprintf (stdout, "R%hhu+", srcreg);
        }
    }
    else if (opcode            == 0x13) // MOV
    {
        if ((addrmode          >= 2)  &&
            (addrmode          <= 4))
        {
            fprintf (stdout, "[");
            flag                = 1;

            if (addrmode       == 4)
            {
                fprintf (stdout, "R%hhu+", srcreg);
            }
        }
        else if (addrmode      >= 5)
        {
            flag                = 2;
        }
    }
    else if (opcode            == 0x18) // OUT special case (only if immflag is set)
    {
        if (immflag            == 1)
        {
            fprintf (stdout, "[");
            flag                = 1;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // AVERAGE CASE (not MOV)
    //
    if ((immflag               == 1) &&
        (flag                  != 2))   // immediate value variant
    {
        fprintf (stdout, "0x%X", immediate_value);
    }
    else if (opcode            == 0x17) // IN special case (port number)
    {
        fprintf (stdout, "%s", (*(port+category)+attribute) -> name);
    }
    else // register variant
    {
        fprintf (stdout, "R%hhu", srcreg);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Wrapping up special case two parameter, second parameter edge cases
    //
    if (flag                   == 1)
    {
        fprintf (stdout, "]");
    }

    fprintf (stdout, "\n");
}

void  rev_word (Word *line, Word *line2, uint8_t  size)
{
    int32_t  index               = 0;
    int32_t  offset              = size - 1;

    for (index = 0; index < size; index++)
    {
        (line2+offset) -> value  = (line+index) -> value;
        (line2+offset) -> flag   = (line+index) -> flag;
        offset                   = offset - 1;
    }
}

void  get_word (FILE *fptr, Word *word)
{
	int8_t  *byte       = NULL;
    int32_t  data       = 0;
    int32_t  index      = 0;

	byte                = word -> data;

    for (index = 0; index < 4; index++)
    {
        data            = fgetc (fptr);
        if (feof (fptr))
        {
            incomplete  = index;
            break;
        }

        *(byte+index)   = data;
        byteoffset      = byteoffset + 1;
    }
    *(byte+wordsize)    = '\0';
    wordoffset          = wordoffset + 1;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// new_word(): allocate and initialize a new word
//
Word  *new_word (uint8_t  length)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Declare and initialize local variables
    //
    int8_t  *byte   = NULL;
    int32_t  index  = 0;
    size_t   size   = 0;
    Word    *word   = NULL;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Allocate space for the line of input
    //
    size                                      = sizeof (Word) * length;
    word                                      = (Word *) malloc (size);
    if (word                                 == NULL)
    {
        fprintf (stderr, "[error] Could not allocate space for word\n");
        exit (3);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Allocate space for each word in the line
    //
    for (index = 0; index < linewidth; index++)
    {
        size                                  = sizeof (int8_t) * (wordsize + 1);
        (line+index) -> data                  = (int8_t *) malloc (size);
        (line+index) -> flag                  = 0;
        if ((line+index) -> data             == NULL)
        {
            fprintf (stderr, "[error] Could not allocate space for data in word\n");
            exit (4);
        }
        byte                                 = (line+index) -> data;
        *(byte+wordsize)                     = '\0';
    }

    return (word);
}
