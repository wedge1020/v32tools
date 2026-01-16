#!/usr/bin/env -S tcc -run

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define  MATCH  0

uint32_t  byteoffset;
uint8_t   wordsize;
uint32_t  wordoffset;

int32_t  get_word (FILE *, int8_t *);

int32_t  main (int  argc, char **argv)
{
    FILE     *fptr         = NULL;
    int32_t   byte         = 0;
    int32_t   headercheck  = 0;
    int32_t   index        = 0;
    int32_t   incomplete   = 0;
    int8_t   *word         = NULL;

    byteoffset             = 0;
    wordoffset             = 0;
    wordsize               = 4;

    if (argc              >  1)
    {
        fptr               = fopen (argv[1], "r");
        if (fptr          == NULL)
        {
            fprintf (stderr, "[error] could not open '%s' for reading!\n", argv[1]);
            exit (1);
        }
    }

    word                   = (int8_t *) malloc (sizeof (int8_t) * (wordsize + 1));
    if (word              == NULL)
    {
        fprintf (stderr, "[error] could not allocate memory for 'word'!\n");
        exit (2);
    }

    incomplete             = get_word (fptr, word);
    while (!feof (fptr))
    {
        headercheck        = strncmp (word, "V32-", 5);
        if (headercheck   == MATCH)
        {
            incomplete     = get_word (fptr, word);

            switch (*(word+1))
            {
                case 'A':
                    fprintf (stdout, "V32-CART header at 0x%X\n", wordoffset);
                    break;

                case 'B':
                    fprintf (stdout, "V32-VBIN header at 0x%X\n", wordoffset);
                    break;

                case 'E':
                    fprintf (stdout, "V32-MEMC header at 0x%X\n", wordoffset);
                    break;

                case 'I':
                    fprintf (stdout, "V32-BIOS header at 0x%X\n", wordoffset);
                    break;

                case 'S':
                    fprintf (stdout, "V32-VSND header at 0x%X\n", wordoffset);
                    break;

                case 'T':
                    fprintf (stdout, "V32-VTEX header at 0x%X\n", wordoffset);
                    break;

                default:
                    fprintf (stdout, "unrecognized header %s at 0x%X!", word, wordoffset);
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
