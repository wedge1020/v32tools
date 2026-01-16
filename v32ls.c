#!/usr/bin/env -S tcc -run

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define  MATCH     0

#define  V32_NONE  0
#define  V32_BIOS  1
#define  V32_CART  2
#define  V32_MEMC  4
#define  V32_VBIN  8
#define  V32_VSND  16
#define  V32_VTEX  32

uint32_t byteoffset;
uint8_t  headertype;
uint8_t  wordsize;
uint32_t wordoffset;

int32_t  get_word (FILE *, int8_t *);
uint32_t wtoi     (int8_t *);

int32_t  main (int  argc, char **argv)
{
    FILE     *fptr                  = NULL;
    int32_t   byte                  = 0;
    int32_t   count                 = 0;
    int32_t   headercheck           = 0;
    int32_t   index                 = 0;
    int32_t   incomplete            = 0;
    uint32_t  value                 = 0;
    int8_t   *word                  = NULL;

    byteoffset                      = 0;
    headertype                      = V32_NONE;
    wordoffset                      = 0;
    wordsize                        = 4;

    if (argc                       >  1)
    {
        fptr                        = fopen (argv[1], "r");
        if (fptr                   == NULL)
        {
            fprintf (stderr, "[error] could not open '%s' for reading!\n", argv[1]);
            exit (1);
        }
    }

    value                           = sizeof (int8_t) * (wordsize + 1);
    word                            = (int8_t *) malloc (value);
    if (word                       == NULL)
    {
        fprintf (stderr, "[error] could not allocate memory for 'word'!\n");
        exit (2);
    }

    fprintf (stdout, "%s:\n", argv[1]);

    incomplete                      = get_word (fptr, word);
    wordoffset                      = wordoffset - 1;
    while (!feof (fptr))
    {
        headercheck                 = strncmp (word, "V32-", 5);
        if (headercheck            == MATCH)
        {
            incomplete              = get_word (fptr, word);

            fprintf (stdout, "  > V32-%s header at offset: ", word);
            switch (*(word+1))
            {
                case 'A':
                    fprintf (stdout, "0x%.8X\n", (wordoffset - 1));
                    headertype      = V32_CART;
                    incomplete      = get_word (fptr, word);
                    value           = wtoi (word);
                    fprintf (stdout, "    * Vircon32 version:   %u\n", value);
                    incomplete      = get_word (fptr, word);
                    value           = wtoi (word);
                    fprintf (stdout, "    * Vircon32 revision:  %u\n", value);
                    fprintf (stdout, "    * Cartridge Title:    \"");
                    for (index = 0; index < 64; index+=4)
                    {
                        incomplete  = get_word (fptr, word);
                        for (count = 0; count < wordsize; count++)
                        {
                            value   = *(word+count);
                            if (value  == 0)
                            {
                                break;
                            }
                            fprintf (stdout, "%c", value);
                        }

                        if (value  == 0)
                        {
                            break;
                        }
                    }
                    fprintf (stdout, "\"\n");

                    if (index      <  60)
                    {
                        for (; index < 60; index += 4)
                        {
                            incomplete      = get_word (fptr, word);
                        }
                    }

                    incomplete      = get_word (fptr, word);
                    value           = wtoi (word);
                    fprintf (stdout, "    * ROM version:        %u\n", value);
                    incomplete      = get_word (fptr, word);
                    value           = wtoi (word);
                    fprintf (stdout, "    * ROM revision:       %u\n", value);
                    incomplete      = get_word (fptr, word);
                    value           = wtoi (word);
                    fprintf (stdout, "    * # of VTEX:          %u\n", value);
                    incomplete      = get_word (fptr, word);
                    value           = wtoi (word);
                    fprintf (stdout, "    * # of VSND:          %u\n", value);
                    incomplete      = get_word (fptr, word);
                    value           = wtoi (word) / wordsize;
                    fprintf (stdout, "    * Program ROM offset: 0x%.8X\n", value);
                    incomplete      = get_word (fptr, word);
                    value           = wtoi (word);
                    fprintf (stdout, "    * Program ROM size:   0x%.8X\n", value);
                    incomplete      = get_word (fptr, word);
                    value           = wtoi (word) / wordsize;
                    fprintf (stdout, "    * VTEX Offset:        0x%.8X\n", value);
                    incomplete      = get_word (fptr, word);
                    value           = wtoi (word) / wordsize;
                    fprintf (stdout, "    * VTEX size:          0x%.8X\n", value);
                    incomplete      = get_word (fptr, word);
                    value           = wtoi (word) / wordsize;
                    fprintf (stdout, "    * VSND Offset:        0x%.8X\n", value);
                    incomplete      = get_word (fptr, word);
                    value           = wtoi (word) / wordsize;
                    fprintf (stdout, "    * VSND size:          0x%.8X\n", value);
                    incomplete      = get_word (fptr, word);
                    fprintf (stdout, "    * reserved\n");
                    incomplete      = get_word (fptr, word);
                    fprintf (stdout, "    * reserved\n\n");
                    break;

                case 'B':
                    fprintf (stdout, "0x%.8X\n", (wordoffset - 1));
                    headertype     = V32_VBIN;
                    incomplete     = get_word (fptr, word);
                    value          = wtoi (word);
                    fprintf (stdout, "    * size: %19u (0x%X) words\n\n", value, value);
                    break;

                case 'E':
                    fprintf (stdout, "0x%.8X\n", (wordoffset - 1));
                    headertype     = V32_MEMC;
                    break;

                case 'I':
                    fprintf (stdout, "0x%.8X\n", (wordoffset - 1));
                    headertype     = V32_BIOS;
                    break;

                case 'S':
                    fprintf (stdout, "0x%.8X\n", (wordoffset - 1));
                    headertype     = V32_VSND;
                    break;

                case 'T':
                    fprintf (stdout, "0x%.8X\n", (wordoffset - 1));
                    headertype     = V32_VTEX;
                    incomplete     = get_word (fptr, word);
                    value          = wtoi (word);
                    fprintf (stdout, "    * texture resolution: %3u x ", value);
                    incomplete     = get_word (fptr, word);
                    value          = wtoi (word);
                    fprintf (stdout, "%-3u\n\n", value);
                    break;
            }
        }

        incomplete         = get_word (fptr, word);
    }

    fclose (fptr);

    free (word);

    return (0);
}

int32_t  get_word (FILE *fptr, int8_t *word)
{
    int32_t  byte      = 0;
    int32_t  index     = 0;

    for (index = 0; index < 4; index++)
    {
        byte           = fgetc (fptr);
        if (feof (fptr))
        {
            break;
        }

        *(word+index)  = byte;
        byteoffset     = byteoffset + 1;
    }
    *(word+wordsize)   = '\0';
    wordoffset         = wordoffset + 1;

    return (index);
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
