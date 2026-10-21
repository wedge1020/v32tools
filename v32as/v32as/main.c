//
// main.c: command-line front end for v32as, the C implementation of the Vircon32
// assembler. Handles option parsing, file and STDIN input, and the interactive mode.
//
// run for help:          ./v32as --help
// assemble to a listing: ./v32as --cart program.asm
// assemble to a VBIN:    ./v32as --cart --binary -o program.vbin program.asm
// interactive mode:      ./v32as            (when STDIN is a terminal)
//
// The bash v32as options (bios, cart, none, binary, step, verbose, help) are still
// accepted as bare words, as long as no file with that name exists.
//
////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "v32as.h"

typedef struct
{
    int          bios;
    int          debug;
    dbgmode_t    debug_mode;
    int          origin_set;
    uint32_t     origin;
    int          cart;
    int          binary;
    int          step;
    int          verbose;
    int          help;
    int          version;
    const char  *output;
}
options_t;

////////////////////////////////////////////////////////////////////////////////////////
//
// Usage
//
static void  usage (FILE *fp)
{
    fprintf (fp,
        "usage: v32as [OPTION...] [ASMFILE...]\n"
        "Assemble Vircon32 assembly (as produced by v32pp) into machine code.\n"
        "\n"
        "  -b, --bios           assemble for BIOS (addresses start at 0x10000000)\n"
        "  -c, --cart           assemble for CART (addresses start at 0x20000000)\n"
        "  -n, --none           standalone instruction assembly (default)\n"
        "  -O, --origin ADDR    assemble for an arbitrary base address (e.g. code that will run from RAM)\n"
        "  -B, --binary         output a VBIN file (requires --bios or --cart)\n"
        "  -o, --output=FILE    write the output to FILE instead of STDOUT\n"
        "  -g, --debug[=MODE]   also write FILE.debug (CSV: address,file,line[,label]) next to the\n"
        "                       -o FILE binary; MODE is program (default), vbin or v32, as in the\n"
        "                       official assembler's \"-g MODE\". Needs --binary and -o.\n"
        "  -s, --step           pause after each statement (files only)\n"
        "  -v, --verbose        trace the assembly of every instruction to STDERR\n"
        "  -h, --help           display this usage information and exit\n"
        "  -V, --version        display the version and exit\n"
        "\n"
        "With no ASMFILE (or with '-'), input comes from STDIN. When STDIN is a terminal\n"
        "this starts the interactive mode: each line is assembled as soon as it is\n"
        "entered; type .help for the commands available there.\n"
        "\n"
        "The bash-style words bios, cart, none, binary, step, verbose and help are also\n"
        "accepted in place of the options above.\n"
        "\n"
        "NOTE: preprocessor directives (%%include, %%define, ...) are not handled here;\n"
        "run the source through v32pp first.\n");
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Reading whole files (or STDIN) into memory
//
static char  *read_stream (FILE *fp, size_t *len)
{
    size_t  cap = 1 << 16;
    size_t  n   = 0;
    char   *buf = malloc (cap);

    if (buf == NULL)
    {
        return (NULL);
    }

    for (;;)
    {
        size_t  got;

        if (n == cap)
        {
            char  *bigger = realloc (buf, cap * 2);

            if (bigger == NULL)
            {
                free (buf);
                return (NULL);
            }

            buf  = bigger;
            cap *= 2;
        }

        got = fread (buf + n, 1, cap - n, fp);

        if (got == 0)
        {
            break;
        }

        n += got;
    }

    if (ferror (fp))
    {
        free (buf);
        return (NULL);
    }

    *len = n;
    return (buf);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Interactive mode: assemble each line as it is typed
//
static char  *read_line (const char *prompt)
{
#ifdef HAVE_READLINE
    return (readline (prompt));
#else
    char    *line = NULL;
    size_t   cap  = 0;
    ssize_t  got;

    fputs  (prompt, stdout);
    fflush (stdout);

    got = getline (&line, &cap, stdin);

    if (got < 0)
    {
        free (line);
        return (NULL);
    }

    if (got > 0 && line[got - 1] == '\n')
    {
        line[got - 1] = 0;
    }

    return (line);
#endif
}

static void  repl_help (void)
{
    puts ("Type any instruction or data directive to assemble it. Labels (_name:) work\n"
          "too; references to labels defined later stay symbolic in the listing.\n"
          "\n"
          "  .help      show this text\n"
          "  .labels    list the labels defined so far\n"
          "  .dump      show every word assembled so far\n"
          "  .reset     forget everything assembled so far\n"
          "  .quit      leave (so does Ctrl-D)");
}

static void  repl_dump (const asm_t *a)
{
    size_t  i;

    for (i = 0; i < a -> nwords; i++)
    {
        printf ("0x%08X  0x%08X\n", a -> base + (uint32_t) i, a -> words[i]);
    }
}

// returns 0 to leave the interactive mode
static int  repl_command (asm_t *a, const char *cmd)
{
    if (strcmp (cmd, ".help") == 0)
    {
        repl_help ();
    }
    else if (strcmp (cmd, ".labels") == 0)
    {
        asm_print_labels (a, stdout);
    }
    else if (strcmp (cmd, ".dump") == 0)
    {
        repl_dump (a);
    }
    else if (strcmp (cmd, ".reset") == 0)
    {
        asm_reset (a);
        puts ("(cleared)");
    }
    else if (strcmp (cmd, ".quit") == 0 || strcmp (cmd, ".exit") == 0)
    {
        return (0);
    }
    else
    {
        fprintf (stderr, "unknown command '%s' (try .help)\n", cmd);
    }

    return (1);
}

static int  repl (asm_t *a)
{
    int  lineno = 0;

    a -> interactive = 1;
    a -> step        = 0;

    printf ("v32as %s interactive mode: .help for commands, Ctrl-D to quit\n", VERSION);

    for (;;)
    {
        char   prompt[32];
        char  *line;
        char  *p;
        char  *end;

        if (a -> has_base)
        {
            snprintf (prompt, sizeof (prompt), "[0x%08X] > ", a -> base + (uint32_t) a -> nwords);
        }
        else
        {
            snprintf (prompt, sizeof (prompt), "v32as> ");
        }

        line = read_line (prompt);

        if (line == NULL)
        {
            putchar ('\n');
            break;
        }

        p = line;

        while (isspace ((unsigned char) *p))
        {
            p++;
        }

        end = p + strlen (p);

        while (end > p && isspace ((unsigned char) end[-1]))
        {
            *--end = 0;
        }

        if (*p == 0)
        {
            free (line);
            continue;
        }

#ifdef HAVE_READLINE
        add_history (p);
#endif

        if (*p == '.')
        {
            int  keep = repl_command (a, p);

            free (line);

            if (!keep)
            {
                break;
            }

            continue;
        }

        lineno++;
        asm_parse (a, "<stdin>", p, strlen (p), lineno);
        free (line);
    }

    if (a -> nfix > 0)
    {
        fprintf (stderr, "note: %zu label reference(s) were never resolved\n", a -> nfix);
    }

    return (0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Command line
//
static int  legacy_word (const char *arg, options_t *opt)
{
    // the bash version took bare words; honor them unless a file has that name
    if (access (arg, F_OK) == 0)
    {
        return (0);
    }

    if (strcasecmp (arg, "bios")    == 0)  { opt -> bios    = 1;  return (1); }
    if (strcasecmp (arg, "cart")    == 0)  { opt -> cart    = 1;  return (1); }
    if (strcasecmp (arg, "none")    == 0)  {                      return (1); }
    if (strcasecmp (arg, "binary")  == 0)  { opt -> binary  = 1;  return (1); }
    if (strcasecmp (arg, "step")    == 0)  { opt -> step    = 1;  return (1); }
    if (strcasecmp (arg, "verbose") == 0)  { opt -> verbose = 1;  return (1); }
    if (strcasecmp (arg, "help")    == 0)  { opt -> help    = 1;  return (1); }

    return (0);
}

int  main (int  argc, char **argv)
{
    static const struct option  long_options[] =
    {
        { "bios",     no_argument,       0, 'b' },
        { "cart",     no_argument,       0, 'c' },
        { "none",     no_argument,       0, 'n' },
        { "binary",   no_argument,       0, 'B' },
        { "debug",    optional_argument, 0, 'g' },
        { "origin",   required_argument, 0, 'O' },
        { "output",   required_argument, 0, 'o' },
        { "step",     no_argument,       0, 's' },
        { "verbose",  no_argument,       0, 'v' },
        { "help",     no_argument,       0, 'h' },
        { "version",  no_argument,       0, 'V' },
        { 0,          0,                 0,  0  }
    };

    options_t    opt   = { 0 };
    const char **files = calloc ((size_t) argc + 1, sizeof (char *));
    int          nfiles = 0;
    int          c;
    int          i;
    asm_t        a;
    FILE        *out   = stdout;
    int          rc    = 0;

    if (files == NULL)
    {
        return (2);
    }

    while ((c = getopt_long (argc, argv, "bcnBgO:o:svhV", long_options, NULL)) != -1)
    {
        switch (c)
        {
            case 'b':  opt.bios    = 1;       break;
            case 'c':  opt.cart    = 1;       break;
            case 'n':                         break;
            case 'B':  opt.binary  = 1;       break;
            case 'g':
            {
                const char  *m = optarg;

                // "-g" takes its mode as the next word, like the official assembler's
                // "-g program"; the mode is optional here and defaults to "program"
                if (m == NULL && optind < argc
                    && (strcmp (argv[optind], "program") == 0 || strcmp (argv[optind], "vbin") == 0
                        || strcmp (argv[optind], "v32") == 0))
                {
                    m = argv[optind++];
                }

                opt.debug      = 1;
                opt.debug_mode = DBG_PROGRAM;

                if (m != NULL && strcmp (m, "vbin") == 0)         { opt.debug_mode = DBG_VBIN; }
                else if (m != NULL && strcmp (m, "v32") == 0)     { opt.debug_mode = DBG_V32; }
                else if (m != NULL && strcmp (m, "program") != 0)
                {
                    fprintf (stderr, "v32as: unrecognized reference mode '%s' for -g "
                                     "(use program, vbin or v32)\n", m);
                    return (2);
                }
                break;
            }
            case 'O':
            {
                char  *end;
                unsigned long  v = strtoul (optarg, &end, 0);
                if (*optarg == '\0' || *end != '\0' || v > 0xFFFFFFFFul)
                {
                    fprintf (stderr, "v32as: invalid --origin '%s'\n", optarg);
                    return (2);
                }
                opt.origin_set = 1;
                opt.origin     = (uint32_t) v;
                break;
            }
            case 'o':  opt.output  = optarg;  break;
            case 's':  opt.step    = 1;       break;
            case 'v':  opt.verbose = 1;       break;
            case 'h':  opt.help    = 1;       break;
            case 'V':  opt.version = 1;       break;
            default:
                usage (stderr);
                return (2);
        }
    }

    for (i = optind; i < argc; i++)
    {
        if (!legacy_word (argv[i], &opt))
        {
            files[nfiles++] = argv[i];
        }
    }

    if (opt.help)
    {
        usage (stdout);
        return (0);
    }

    if (opt.version)
    {
        printf ("v32as %s\n", VERSION);
        return (0);
    }

    if (opt.debug && (!opt.binary || opt.output == NULL))
    {
        fprintf (stderr, "v32as: -g needs --binary and -o FILE (the debug info goes to FILE.debug)\n");
        return (2);
    }

    if (opt.origin_set && (opt.bios || opt.cart))
    {
        fprintf (stderr, "v32as: --origin cannot be combined with --bios or --cart\n");
        return (2);
    }

    if (opt.bios && opt.cart)
    {
        fprintf (stderr, "v32as: --bios and --cart cannot be combined\n");
        return (2);
    }

    if (opt.binary && !opt.bios && !opt.cart && !opt.origin_set)
    {
        fprintf (stderr, "v32as: binary output requires --bios, --cart or --origin\n");
        return (2);
    }

    asm_init (&a);

    a.has_base = (opt.bios || opt.cart || opt.origin_set);
    a.base     = opt.origin_set ? opt.origin : (opt.bios ? BIOS_BASE : (opt.cart ? CART_BASE : 0));
    a.verbose  = opt.verbose;
    a.step     = opt.step;
    a.listing  = !opt.binary;

    // text output goes out as the assembly proceeds; binary output only after success
    if (opt.output != NULL && !opt.binary)
    {
        out = fopen (opt.output, "w");

        if (out == NULL)
        {
            fprintf (stderr, "v32as: cannot write '%s': %s\n", opt.output, strerror (errno));
            return (2);
        }
    }

    a.out = out;

    if (nfiles == 0 || (nfiles == 1 && strcmp (files[0], "-") == 0))
    {
        if (nfiles == 0 && isatty (STDIN_FILENO) && !opt.binary)
        {
            rc = repl (&a);
            asm_free (&a);
            return (rc);
        }

        // single shot from STDIN: stepping makes no sense here
        {
            size_t  len  = 0;
            char   *text = read_stream (stdin, &len);

            if (text == NULL)
            {
                fprintf (stderr, "v32as: unable to read STDIN\n");
                return (2);
            }

            a.step = 0;
            asm_parse (&a, "<stdin>", text, len, 1);
            free (text);
        }
    }
    else
    {
        if (opt.step && !isatty (STDIN_FILENO))
        {
            fprintf (stderr, "v32as: warning: STDIN is not a terminal; ignoring --step\n");
            a.step = 0;
        }

        for (i = 0; i < nfiles; i++)
        {
            FILE   *fp   = (strcmp (files[i], "-") == 0) ? stdin : fopen (files[i], "rb");
            size_t  len  = 0;
            char   *text;

            if (fp == NULL)
            {
                fprintf (stderr, "v32as: unable to read '%s': %s\n", files[i], strerror (errno));
                rc = 1;
                continue;
            }

            text = read_stream (fp, &len);

            if (fp != stdin)
            {
                fclose (fp);
            }

            if (text == NULL)
            {
                fprintf (stderr, "v32as: unable to read '%s'\n", files[i]);
                rc = 1;
                continue;
            }

            asm_parse (&a, files[i], text, len, 1);
            free (text);
        }
    }

    asm_finish (&a);

    if (a.errors > 0)
    {
        rc = 1;
    }

    if (rc == 0 && opt.binary)
    {
        if (opt.output != NULL)
        {
            out = fopen (opt.output, "wb");

            if (out == NULL)
            {
                fprintf (stderr, "v32as: cannot write '%s': %s\n", opt.output, strerror (errno));
                rc = 2;
            }
        }
        else if (isatty (STDOUT_FILENO))
        {
            fprintf (stderr, "v32as: refusing to write binary data to a terminal "
                             "(use -o FILE or redirect STDOUT)\n");
            rc = 2;
        }

        if (rc == 0 && asm_write_vbin (&a, out) != 0)
        {
            fprintf (stderr, "v32as: error writing the output\n");
            rc = 2;
        }
    }

    if (rc == 0 && opt.binary && opt.debug)
    {
        char   *path = malloc (strlen (opt.output) + 8);
        FILE   *dfp;

        sprintf (path, "%s.debug", opt.output);
        dfp = fopen (path, "w");

        if (dfp == NULL || asm_write_debug (&a, dfp, opt.debug_mode) != 0)
        {
            fprintf (stderr, "v32as: cannot write '%s': %s\n", path, strerror (errno));
            rc = 2;
        }

        if (dfp != NULL)
        {
            fclose (dfp);
        }

        free (path);
    }

    if (out != stdout)
    {
        fclose (out);
    }
    else
    {
        fflush (stdout);
    }

    asm_free (&a);
    free     (files);

    return (rc);
}
