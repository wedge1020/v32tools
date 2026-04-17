//
// bincode.c: C program to process offsets and string data for embedding into
// Vircon32 VBIN files as part of debuggerBIOS-ng's C code debugging features
//
// Contains base64 decoding logic originally sourced from:
//
// https://stackoverflow.com/a/6782480
//
// Posted by ryyst, modified by community. See post 'Timeline' for change history
//
// Retrieved 2026-04-06, License - CC BY-SA 3.0
//
// compile with: gcc -Wall -o bincode bincode.c -Wno-char-subscripts -Wno-pointer-sign
//
// run for header:  ./bincode
// run for offset:  ./bincode 0xOFFSET
// run for string:  ./bincode string BASE64DATA
//
// Will output results in "binary" (as char) to STDOUT, with the intent is to
// use I/O redirection to append the data to the desired VBIN file amidst the
// cartridge build process.
//
// This program is called by the `inject-debug.sh` script
//
////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define  MODE_NONE        0
#define  MODE_HEADER      1
#define  MODE_WORD        2
#define  MODE_OFFSET      4
#define  MODE_STRING      8
#define  MODE_DECODE     16
#define  MODE_ENDIAN     32
#define  MODE_FLOAT      64
#define  MODE_NEGATE    128
#define  MODE_NOOFFSET  256
#define  MODE_TEXT      512
#define  MODE_VERBOSE  1024
#define  MODE_HELP     2048

typedef  struct   option  gopt_t;
typedef  union    f2i     f2i_t;

uint8_t *base64_decode        (char *, size_t, size_t *);
void     build_decoding_table ();
void     base64_cleanup       ();
void     process_offset       (int,    int);

static char   encoding_table[]  = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                    'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                    '4', '5', '6', '7', '8', '9', '+', '/' };

static char *decoding_table  = NULL;

union f2i
{
    int   i32;
    float f32;
};

int  main (int  argc, char **argv)
{
    size_t  ilen                          = 0;
    size_t  olen                          = 0;
    char   *string                        = NULL;
    char   *string_arg                    = NULL;
    char   *string_data                   = NULL;
    int     index                         = 0;
    int     count                         = 0;
    int     mode                          = MODE_NONE;
    int     opt                           = 0;
    int     option_index                  = 0;
    f2i_t   value;

    gopt_t  long_options[]                = {
        { "header",        no_argument,       0, 'H' },
        { "offset",        required_argument, 0, 'o' },
        { "word",          required_argument, 0, 'w' },
        { "float",         required_argument, 0, 'f' },
        { "string",        required_argument, 0, 's' },
        { "decode-base64", no_argument,       0, 'd' },
        { "little-endian", no_argument,       0, 'e' },
        { "big-endian",    no_argument,       0, 'E' },
        { "negate",        no_argument,       0, 'n' },
        { "no-offset",     no_argument,       0, 'N' },
        { "text",          no_argument,       0, 't' },
        { "verbose",       no_argument,       0, 'v' },
        { "help",          no_argument,       0, 'h' },
        { 0,               0,                 0,  0  }
    };

    opt                                   = getopt_long (argc, argv,
                                                         "Ho:w:f:s:deEnNtvh",
                                                         long_options,
                                                         &option_index);

    while (opt                           != -1)
    {
        switch (opt)
        {
            case 'H':
                mode                      = mode | MODE_HEADER;
                break;

            case 'w':
                mode                      = mode | MODE_WORD;
            case 'o':
                mode                      = mode | MODE_OFFSET;
                value.i32                 = strtol (optarg, NULL, 16);
                break;

            case 'f':
                mode                      = mode | MODE_FLOAT;
                value.f32                 = strtof (optarg, NULL);
                break;

            case 's':
                mode                      = mode | MODE_STRING;
                ilen                      = sizeof (char) * (strlen (optarg) + 1);
                string_arg                = (char *) malloc (ilen);
                strcpy (string_arg, optarg);
                break;

            case 'd':
                mode                      = mode | MODE_DECODE;
                break;

            case 'e':
                mode                      = mode | MODE_ENDIAN;
                break;

            case 'n':
                mode                      = mode | MODE_NEGATE;
                break;

            case 'N':
                mode                      = mode | MODE_NOOFFSET;
                break;

            case 't':
                mode                      = mode | MODE_TEXT;
                break;

            case 'v':
                mode                      = mode | MODE_VERBOSE;
                break;

            case 'h':
                fprintf (stdout, "usage: %s [OPTION...]\n", argv[0]);
                fprintf (stdout, "Perform various binary manipulations.\n\n");
                fprintf (stdout, "  -H, --header         show \"V32-TEXT\" header\n");
                fprintf (stdout, "  -o, --offset VALUE   process VALUE as offset\n");
                fprintf (stdout, "  -w, --word   VALUE   process VALUE as dataword\n");
                fprintf (stdout, "  -f, --float  VALUE   process VALUE as float\n");
                fprintf (stdout, "  -s, --string \"VALUE\" process VALUE as string\n");
                fprintf (stdout, "  -d, --decode-base64  perform base64 decoding\n");
                fprintf (stdout, "  -e, --little-endian  encode as little endian\n");
                fprintf (stdout, "  -E, --big-endian     encode as big endian\n");
                fprintf (stdout, "  -n, --negate         negate the offset/word\n");
                fprintf (stdout, "  -N, --no-offset      no offset with string\n");
                fprintf (stdout, "  -t, --text           render as text\n");
                fprintf (stdout, "  -v, --verbose        enable verbosity\n");
                fprintf (stdout, "  -h, --help           display this help\n\n");
                fprintf (stdout, "Verbosity will disable any binary output\n");
                fprintf (stdout, "Text will be like verbosity without noise\n");
                fprintf (stdout, "Strings by default with display an offset\n\n");
                exit (0);
                break;
        }
        opt                               = getopt_long (argc, argv,
                                                         "Ho:w:f:s:deEnNtvh",
                                                         long_options,
                                                         &option_index);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // header mode: display the "V32-TEXT" header
    //
    if (MODE_HEADER                      == (mode & MODE_HEADER))
    {
        if (MODE_VERBOSE                 == (mode & MODE_VERBOSE))
        {
            fprintf (stdout, "[verbose] HEADER:  \"");
        }

        if ((mode & MODE_ENDIAN)         == MODE_ENDIAN)
        {
            fprintf (stdout, "-23VTXET");
        }
        else
        {
            fprintf (stdout, "V32-TEXT");
        }

        if (MODE_VERBOSE                 == (mode & MODE_VERBOSE))
        {
            fprintf (stdout, "\"\n");
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // For offsets (or words), do the deed
    //
    if (MODE_OFFSET                      == (mode & MODE_OFFSET))
    {
        if (MODE_NEGATE                  == (mode & MODE_NEGATE))
        {
            value.i32                     = (4294967296 - value.i32);
        }
        process_offset (mode, value.i32);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // For floating point values
    //
    if (MODE_FLOAT                       == (mode & MODE_FLOAT))
    {
        process_offset (mode, value.i32);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // data mode: ignore the first argument, take the base64 value and output its
    // size then the string in "binary"
    //
    if (MODE_STRING                      == (mode & MODE_STRING))
    {
        if (MODE_DECODE                  == (mode & MODE_DECODE))
        {
            ilen                          = strlen (string_arg);
            string                        = base64_decode (string_arg, ilen, &olen);
            string_data                   = (char *) calloc (sizeof (char),
                                                             (olen+(4-(olen%4))));
            strcpy (string_data, string);
        }
        else
        {
            olen                          = sizeof (char) * (strlen (string_arg) + 1);
            string_data                   = (char *) malloc (olen);
            strcpy (string_data, string_arg);
        }

        ilen                              = olen;
        olen                              = olen + (4 - (olen % 4));
        olen                              = olen / 4;

        if (MODE_NOOFFSET                != (mode & MODE_NOOFFSET))
        {
            process_offset (mode, olen);
        }

        if (MODE_VERBOSE                 == (mode & MODE_VERBOSE))
        {
            fprintf (stdout, "[verbose] string:  ");
        }

        for (count                        = 0;
             count                       <  olen;
             count                        = count + 1)
        {
            if (MODE_ENDIAN              == (mode & MODE_ENDIAN))
            {
                for (index                = 3;
                     index               >= 0;
                     index                = index - 1)
                {
                    if ((MODE_VERBOSE    == (mode & MODE_VERBOSE)) ||
                        (MODE_TEXT       == (mode & MODE_TEXT)))
                    {
                        fprintf (stdout, "%.2hhX ", *(string_data+((count * 4) + index)));
                    }
                    else
                    {
                        fprintf (stdout, "%c",      *(string_data+((count * 4) + index)));
                    }
                }
            }
            else
            {
                for (index                = 0;
                     index               <  4;
                     index                = index + 1)
                {
                    if ((MODE_VERBOSE    == (mode & MODE_VERBOSE)) ||
                        (MODE_TEXT       == (mode & MODE_TEXT)))
                    {
                        fprintf (stdout, "%.2hhX ", *(string_data+((count * 4) + index)));
                    }
                    else
                    {
                        fprintf (stdout, "%c",      *(string_data+((count * 4) + index)));
                    }
                }
            }
        }

        if (MODE_VERBOSE                 == (mode & MODE_VERBOSE))
        {
            fprintf (stdout, "\n");
        }
    }

    return (0);
}    

unsigned char *base64_decode (char   *data,
                              size_t  input_length,
                              size_t *output_length)
{
    uint8_t  *decoded_data       = NULL;
    int       index              = 0;
    int       count              = 0;
    uint32_t  sextet_a           = 0;
    uint32_t  sextet_b           = 0;
    uint32_t  sextet_c           = 0;
    uint32_t  sextet_d           = 0;
    uint32_t  triple             = 0;

    if (decoding_table          == NULL)
    {
        build_decoding_table ();
    }

    if ((input_length % 4)      != 0)
    {
        return (NULL);
    }

    *output_length               = input_length / 4 * 3;
    if (data[input_length-1]    == '=')
    {
        (*output_length)--;
    }

    if (data[input_length-2]    == '=')
    {
        (*output_length)--;
    }

    decoded_data                 = malloc (*output_length);
    if (decoded_data            == NULL)
    {
        return (NULL);
    }

    for (index                   = 0,
         count                   = 0;
         index                  <  input_length;)
    {
        sextet_a                 = (data[index] == '=') ? 0 & index++ : decoding_table[data[index++]];
        sextet_b                 = (data[index] == '=') ? 0 & index++ : decoding_table[data[index++]];
        sextet_c                 = (data[index] == '=') ? 0 & index++ : decoding_table[data[index++]];
        sextet_d                 = (data[index] == '=') ? 0 & index++ : decoding_table[data[index++]];

        triple                   = (sextet_a << 3 * 6) +
                                   (sextet_b << 2 * 6) +
                                   (sextet_c << 1 * 6) +
                                   (sextet_d << 0 * 6);

        if (count               < *output_length)
        {
            decoded_data[count]  = (triple >> 2 * 8) & 0xFF;
            count                = count + 1;
        }

        if (count               < *output_length)
        {
            decoded_data[count]  = (triple >> 1 * 8) & 0xFF;
            count                = count + 1;
        }

        if (count               < *output_length)
        {
            decoded_data[count]  = (triple >> 0 * 8) & 0xFF;
            count                = count + 1;
        }
    }

    return (decoded_data);
}

void  build_decoding_table ()
{
    int  index      = 0;
    decoding_table  = malloc (256);

    for (index      = 0;
         index     <  64;
         index      = index + 1)
    {
        decoding_table[(unsigned char) encoding_table[index]] = index;
    }
}

void  base64_cleanup ()
{
    free (decoding_table);
}

void  process_offset (int  mode, int  offset)
{
    int  index                  = 0;
    int  addr[4];

    if (MODE_VERBOSE           == (mode & MODE_VERBOSE))
    {
        for (index              = 0;
             index             <  MODE_HELP;
             index              = index + 1)
        {
            switch (mode & index)
            {
                case MODE_WORD:
                    fprintf (stdout, "[verbose] WORD:    0x%.8X\n", offset);
                    index       = MODE_HELP;
                    break;

                case MODE_OFFSET:
                    fprintf (stdout, "[verbose] OFFSET:  0x%.8X\n", offset);
                    index       = MODE_HELP;
                    break;

                case MODE_FLOAT:
                    fprintf (stdout, "[verbose] FLOAT:   0x%.8X\n", offset);
                    index       = MODE_HELP;
                    break;
            }
        }
    }

    for (index                  = 3;
         index                 >= 0;
         index                  = index - 1)
    {
        addr[index]             = offset & 0x000000FF;
        offset                  = offset >> 8;
        if (MODE_VERBOSE       == (mode & MODE_VERBOSE))
        {
            fprintf (stdout, "[verbose] addr[%d]: %.2hhX\n", index, addr[index]);
        }
    }

    if (MODE_VERBOSE           == (mode & MODE_VERBOSE))
    {
        fprintf (stdout, "[verbose] data:    ");
    }
    else if (MODE_TEXT         == (mode & MODE_TEXT))
    {
        fprintf (stdout, "0x");
    }

    if ((mode & MODE_ENDIAN)   == MODE_ENDIAN)
    {
        for (index              = 3;
             index             >= 0;
             index              = index - 1)
        {
            if (MODE_VERBOSE   == (mode & MODE_VERBOSE))
            {
                fprintf (stdout, "%.2hhX ", addr[index]);
            }
            else if (MODE_TEXT == (mode & MODE_TEXT))
            {
                fprintf (stdout, "%.2hhX",  addr[index]);
            }
            else
            {
                fprintf (stdout, "%c",      addr[index]);
            }
        }
    }
    else
    {
        for (index              = 0;
             index             <  4;
             index              = index + 1)
        {
            if (MODE_VERBOSE   == (mode & MODE_VERBOSE))
            {
                fprintf (stdout, "%.2hhX ", addr[index]);
            }
            else if (MODE_TEXT == (mode & MODE_TEXT))
            {
                fprintf (stdout, "%.2hhX",  addr[index]);
            }
            else
            {
                fprintf (stdout, "%c",      addr[index]);
            }
        }
    }

    if ((MODE_VERBOSE          == (mode & MODE_VERBOSE)) ||
        (MODE_TEXT             == (mode & MODE_TEXT)))
    {
        fprintf (stdout, "\n");
    }
}
