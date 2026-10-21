//
// asm.c: the assembler core for v32as
//
// Holds everything that happens after parsing: the symbol table, the output word
// buffer, label fixups, instruction encoding, data directives, listings and the
// VBIN writer. The bison grammar (parser.y) calls into this file from its actions.
//
// Instruction word layout (MSB to LSB), as defined by the Vircon32 CPU specification:
//
//     opcode:6  immediate:1  register1:4  register2:4  addressing-mode:3  port:14
//
////////////////////////////////////////////////////////////////////////////////////////

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "v32as.h"
#include "parser.tab.h"

////////////////////////////////////////////////////////////////////////////////////////
//
// Small helpers
//
static void  *xrealloc (void *ptr, size_t size)
{
    void  *p = realloc (ptr, size ? size : 1);

    if (p == NULL)
    {
        fprintf (stderr, "v32as: out of memory\n");
        exit    (2);
    }

    return (p);
}

static char  *xstrndup (const char *s, size_t n)
{
    char  *p = xrealloc (NULL, n + 1);

    memcpy (p, s, n);
    p[n] = 0;

    return (p);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Diagnostics
//
static void  report (asm_t *a, const loc_t *loc, const char *kind, const char *fmt,
                     va_list ap)
{
    fflush (a -> out);

    if (loc != NULL)
    {
        fprintf (stderr, "%s:%d:%d: %s: ", a -> cur_file ? a -> cur_file : "<input>",
                 loc -> first_line, loc -> first_column, kind);
    }
    else
    {
        fprintf (stderr, "v32as: %s: ", kind);
    }

    vfprintf (stderr, fmt, ap);
    fputc    ('\n', stderr);
}

void  asm_error (asm_t *a, const loc_t *loc, const char *fmt, ...)
{
    va_list  ap;

    va_start (ap, fmt);
    report   (a, loc, "error", fmt, ap);
    va_end   (ap);

    a -> errors   = a -> errors + 1;
    a -> stmt_bad = 1;
}

void  asm_warn (asm_t *a, const loc_t *loc, const char *fmt, ...)
{
    va_list  ap;

    va_start (ap, fmt);
    report   (a, loc, "warning", fmt, ap);
    va_end   (ap);

    a -> warnings = a -> warnings + 1;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Context life cycle
//
void  asm_init (asm_t *a)
{
    memset (a, 0, sizeof (*a));

    a -> tabcap  = 256;
    a -> tab     = xrealloc (NULL, a -> tabcap * sizeof (sym_t *));
    memset (a -> tab, 0, a -> tabcap * sizeof (sym_t *));

    a -> out     = stdout;
    a -> dbg     = stderr;
    a -> listing = 1;
}

void  asm_free (asm_t *a)
{
    sym_t  *s = a -> first;

    while (s != NULL)
    {
        sym_t  *next = s -> order;

        free (s -> name);
        free (s);
        s = next;
    }

    free (a -> tab);
    free (a -> words);
    free (a -> fix);
    free (a -> rec);

    a -> tab   = NULL;
    a -> words = NULL;
    a -> fix   = NULL;
    a -> rec   = NULL;
    a -> first = a -> last = NULL;
}

void  asm_reset (asm_t *a)
{
    asm_t  saved = *a;

    asm_free (a);
    asm_init (a);

    a -> has_base    = saved.has_base;
    a -> base        = saved.base;
    a -> verbose     = saved.verbose;
    a -> step        = saved.step;
    a -> listing     = saved.listing;
    a -> interactive = saved.interactive;
    a -> out         = saved.out;
    a -> dbg         = saved.dbg;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Symbol table: chained hash (FNV-1a), growing at a load factor of 1
//
static uint32_t  sym_hash (const char *s, size_t n)
{
    uint32_t  h = 2166136261u;
    size_t    i;

    for (i = 0; i < n; i++)
    {
        h ^= (unsigned char) s[i];
        h *= 16777619u;
    }

    return (h);
}

static void  sym_grow (asm_t *a)
{
    size_t   newcap = a -> tabcap * 2;
    sym_t  **newtab = xrealloc (NULL, newcap * sizeof (sym_t *));
    sym_t   *s;

    memset (newtab, 0, newcap * sizeof (sym_t *));

    for (s = a -> first; s != NULL; s = s -> order)
    {
        size_t  slot = sym_hash (s -> name, strlen (s -> name)) & (newcap - 1);

        s -> next      = newtab[slot];
        newtab[slot]   = s;
    }

    free (a -> tab);
    a -> tab    = newtab;
    a -> tabcap = newcap;
}

sym_t  *asm_sym_intern (asm_t *a, const char *name, size_t n)
{
    size_t  slot = sym_hash (name, n) & (a -> tabcap - 1);
    sym_t  *s;

    for (s = a -> tab[slot]; s != NULL; s = s -> next)
    {
        if (strlen (s -> name) == n && memcmp (s -> name, name, n) == 0)
        {
            return (s);
        }
    }

    if (a -> nsyms >= a -> tabcap)
    {
        sym_grow (a);
        slot = sym_hash (name, n) & (a -> tabcap - 1);
    }

    s              = xrealloc (NULL, sizeof (*s));
    s -> name      = xstrndup (name, n);
    s -> addr      = 0;
    s -> defined   = 0;
    s -> order     = NULL;
    s -> next      = a -> tab[slot];
    a -> tab[slot] = s;

    if (a -> last != NULL)
    {
        a -> last -> order = s;
    }
    else
    {
        a -> first = s;
    }

    a -> last   = s;
    a -> nsyms  = a -> nsyms + 1;

    return (s);
}

static uint32_t  here (asm_t *a)
{
    return (a -> base + (uint32_t) a -> nwords);
}

void  asm_label_define (asm_t *a, sym_t *s, const loc_t *loc)
{
    // the lexer hands over "_name" (without the colon), already interned
    a -> line_active = 1;

    if (s -> defined)
    {
        asm_error (a, loc, "duplicate label '%s'", s -> name);
        return;
    }

    s -> addr    = here (a);
    s -> defined = 1;

    if (a -> verbose)
    {
        fprintf (a -> dbg, "[label] %s at 0x%08X\n", s -> name, s -> addr);
    }

    if (a -> interactive && a -> listing)
    {
        fprintf (a -> out, "label:       %s = 0x%08X\n", s -> name, s -> addr);
    }
}

void  asm_print_labels (asm_t *a, FILE *fp)
{
    const sym_t  *s;

    for (s = a -> first; s != NULL; s = s -> order)
    {
        if (s -> defined)
        {
            fprintf (fp, "0x%08X  %s\n", s -> addr, s -> name);
        }
        else
        {
            fprintf (fp, "<undefined>   %s\n", s -> name);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Output words and fixups
//
static void  ensure_words (asm_t *a, size_t extra)
{
    if (a -> nwords + extra > a -> cap)
    {
        size_t  newcap = a -> cap ? a -> cap : 1024;

        while (newcap < a -> nwords + extra)
        {
            newcap *= 2;
        }

        a -> words = xrealloc (a -> words, newcap * sizeof (uint32_t));
        a -> cap   = newcap;
    }
}

static size_t  emit_word (asm_t *a, uint32_t w)
{
    ensure_words (a, 1);
    a -> words[a -> nwords] = w;

    return (a -> nwords++);
}

// emit a word holding an immediate: a plain value, or a label (now or via a fixup)
static size_t  emit_imm (asm_t *a, const loc_t *loc, imm_t imm)
{
    size_t  index;

    if (imm.kind == IMM_SYM)
    {
        index = emit_word (a, imm.sym -> defined ? imm.sym -> addr : 0);

        if (!imm.sym -> defined)
        {
            if (a -> nfix == a -> fixcap)
            {
                a -> fixcap = a -> fixcap ? a -> fixcap * 2 : 256;
                a -> fix    = xrealloc (a -> fix, a -> fixcap * sizeof (fixup_t));
            }

            a -> fix[a -> nfix].word = (uint32_t) index;
            a -> fix[a -> nfix].sym  = imm.sym;
            a -> fix[a -> nfix].file = a -> cur_file;
            a -> fix[a -> nfix].loc  = *loc;
            a -> nfix                = a -> nfix + 1;
        }
    }
    else
    {
        index = emit_word (a, imm.bits);
    }

    return (index);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Numeric helpers used by the grammar
//
num_t  num_negate (asm_t *a, const loc_t *loc, num_t n)
{
    if (n.is_float)
    {
        n.bits = n.bits ^ 0x80000000u;
    }
    else if (n.bits > 0x80000000u)
    {
        asm_error (a, loc, "number out of range for a negative 32-bit value");
    }
    else
    {
        n.bits = 0u - n.bits;
    }

    return (n);
}

imm_t  imm_from_sym (sym_t *s)
{
    imm_t  v = NOIMM;

    v.kind = IMM_SYM;
    v.sym  = s;

    return (v);
}

imm_t  imm_any (num_t n)
{
    imm_t  v = NOIMM;

    v.kind     = IMM_BITS;
    v.bits     = n.bits;
    v.is_float = n.is_float;

    return (v);
}

imm_t  imm_integer (asm_t *a, const loc_t *loc, num_t n)
{
    if (n.is_float)
    {
        asm_error (a, loc, "an integer is required here, not a float");
        n.bits     = 0;
        n.is_float = 0;
    }

    return (imm_any (n));
}

imm_t  imm_float (num_t n)
{
    if (!n.is_float)
    {
        // integer literals are converted to float values, as in the official assembler
        float  f = (float) (int32_t) n.bits;

        memcpy (&n.bits, &f, sizeof (f));
        n.is_float = 1;
    }

    return (imm_any (n));
}

imm_t  imm_address (asm_t *a, const loc_t *loc, imm_t v)
{
    if (v.kind == IMM_BITS && v.is_float)
    {
        asm_error (a, loc, "an address must be an integer or a label, not a float");
        v.bits     = 0;
        v.is_float = 0;
    }

    return (v);
}

uint32_t  port_from_num (asm_t *a, const loc_t *loc, num_t n)
{
    if (n.is_float)
    {
        asm_error (a, loc, "an I/O port must be a port name or an integer, not a float");
        return (0);
    }

    if (n.bits > PORT_MASK)
    {
        asm_warn (a, loc, "port number 0x%X does not fit in 14 bits; truncated", n.bits);
    }
    else if (!port_is_known (n.bits))
    {
        asm_warn (a, loc, "0x%03X is not a known I/O port", n.bits);
    }

    return (n.bits & PORT_MASK);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Listings (the human-readable output) and the verbose trace
//
static const char  *pending_label (const asm_t *a, size_t word)
{
    size_t  i;

    for (i = a -> stmt_fix; i < a -> nfix; i++)
    {
        if (a -> fix[i].word == word)
        {
            return (a -> fix[i].sym -> name);
        }
    }

    return (NULL);
}

static void  list_statement (asm_t *a, int is_data)
{
    size_t       i;
    const char  *name;

    if (!a -> listing)
    {
        return;
    }

    if (a -> has_base)
    {
        fprintf (a -> out, "OFFSET:      0x%08X\n", a -> base + (uint32_t) a -> stmt_start);
    }

    for (i = a -> stmt_start; i < a -> nwords; i++)
    {
        const char  *tag = is_data ? "data:       " :
                           (i == a -> stmt_start) ? "instruction:" : "immediate:  ";

        name = pending_label (a, i);

        if (name != NULL)
        {
            fprintf (a -> out, "%s %s\n", tag, name);
        }
        else
        {
            fprintf (a -> out, "%s 0x%08X\n", tag, a -> words[i]);
        }
    }

    fputs ("-----------------------\n", a -> out);
}

static void  trace_instruction (asm_t *a, unsigned opcode, unsigned r1, unsigned r2,
                                unsigned mode, unsigned port, int has_imm)
{
    uint32_t     w     = a -> words[a -> stmt_start];
    const char  *name  = pending_label (a, a -> stmt_start + 1);
    int          color = isatty (fileno (a -> dbg));
    int          bit;

    fprintf (a -> dbg, "======================================================\n");

    if (a -> has_base)
    {
        fprintf (a -> dbg, "OFFSET:        0x%08X\n", a -> base + (uint32_t) a -> stmt_start);
    }

    fprintf (a -> dbg, "mnemonic:      %s (opcode %u)\n", opcode_name (opcode), opcode);
    fprintf (a -> dbg, "imm_flag:      %d\n", has_imm);
    fprintf (a -> dbg, "register1:     %u\n", r1);
    fprintf (a -> dbg, "register2:     %u\n", r2);
    fprintf (a -> dbg, "addr mode:     %u\n", mode);
    fprintf (a -> dbg, "port:          0x%03X\n", port & PORT_MASK);
    fprintf (a -> dbg, "instruction:   ");

    // opcode | imm | reg1 | reg2 | mode | port, each field in its own color
    {
        static const int   width[6] = { 6, 1, 4, 4, 3, 14 };
        static const char *paint[6] = { "31", "33", "32", "36", "34", "35" };
        int                field    = 0;
        int                left     = width[0];

        for (bit = 31; bit >= 0; bit--)
        {
            if (color && left == width[field])
            {
                fprintf (a -> dbg, "\033[%sm", paint[field]);
            }

            fputc (((w >> bit) & 1) ? '1' : '0', a -> dbg);
            left--;

            if (left == 0)
            {
                if (color)
                {
                    fputs ("\033[0m", a -> dbg);
                }

                if (bit > 0)
                {
                    fputc (' ', a -> dbg);
                    field++;
                    left = width[field];
                }
            }
        }

        fputc ('\n', a -> dbg);
    }

    fprintf (a -> dbg, "INSTRUCTION:   0x%08X\n", w);

    if (has_imm)
    {
        if (name != NULL)
        {
            fprintf (a -> dbg, "IMMEDIATE:     %s (forward reference)\n", name);
        }
        else
        {
            fprintf (a -> dbg, "IMMEDIATE:     0x%08X\n", a -> words[a -> stmt_start + 1]);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Instruction encoding
//
void  asm_inst (asm_t *a, const loc_t *loc, unsigned opcode, unsigned r1, unsigned r2,
                unsigned mode, unsigned port, imm_t imm)
{
    int       has_imm = (imm.kind != IMM_NONE);
    uint32_t  word;

    a -> line_active = 1;

    if (a -> stmt_bad)
    {
        return;
    }

    word = ((uint32_t) (opcode & 0x3F) << 26)
         | ((uint32_t) (has_imm ? 1 : 0) << 25)
         | ((uint32_t) (r1 & 0xF)   << 21)
         | ((uint32_t) (r2 & 0xF)   << 17)
         | ((uint32_t) (mode & 0x7) << 14)
         | ((uint32_t) (port & PORT_MASK));

    a -> stmt_start = a -> nwords;
    a -> stmt_fix   = a -> nfix;

    // remember where this instruction came from (debug file)
    if (a -> nrec == a -> reccap)
    {
        a -> reccap = a -> reccap ? a -> reccap * 2 : 256;
        a -> rec    = xrealloc (a -> rec, a -> reccap * sizeof (dbgrec_t));
    }
    a -> rec[a -> nrec].word = (uint32_t) a -> nwords;
    a -> rec[a -> nrec].file = a -> cur_file ? a -> cur_file : "<input>";
    a -> rec[a -> nrec].line = loc -> first_line;
    a -> nrec++;

    emit_word (a, word);

    if (has_imm)
    {
        emit_imm (a, loc, imm);
    }

    if (a -> verbose)
    {
        trace_instruction (a, opcode, r1, r2, mode, port, has_imm);
    }

    list_statement (a, 0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Internal registers. IP, IR and IV cannot be named by the CPU, but their effects can be
// had indirectly. Every form below assembles to exactly one real instruction and uses no
// hidden scratch register.  A is the address of the expansion's first word.
//
//     MOV Rn, IP        MOV Rn, A+2          IP = address of the next instruction
//     MOV Rn, IR        MOV Rn, <that word>  IR = the instruction being executed
//     MOV Rn, IV        (rejected: IV would be the immediate that is being defined)
//     MOV IP, Rn|addr   JMP Rn|addr
//     MOV IR, Rn        MOV [A+2], Rn        overwrite the NEXT instruction word
//     MOV IV, Rn        MOV [A+3], Rn        overwrite the NEXT instruction's immediate
//
#define  OP_MOV  19u
#define  OP_JMP  ((unsigned) kw_lookup ("JMP", 3) -> value)

static imm_t  imm_bits (uint32_t v)
{
    imm_t  i = { IMM_BITS, v, NULL, 0 };
    return i;
}

void  asm_mov_sysreg_read (asm_t *a, const loc_t *loc, unsigned rn, unsigned sys)
{
    if (sys == SYS_IV)
    {
        asm_error (a, loc, "cannot read IV: it would be the immediate of this very instruction");
        return;
    }

    if (sys == SYS_IP)
    {
        if (!a -> has_base)
        {
            asm_warn (a, loc, "IP is relative to address 0 here; use --bios or --cart for real addresses");
        }
        asm_inst (a, loc, OP_MOV, rn, 0, 0, 0, imm_bits (here (a) + 2));
        return;
    }

    // IR: the encoding of this MOV Rn, imm itself
    asm_inst (a, loc, OP_MOV, rn, 0, 0, 0,
              imm_bits ((OP_MOV << 26) | (1u << 25) | ((rn & 0xF) << 21)));
}

void  asm_mov_sysreg_write (asm_t *a, const loc_t *loc, unsigned sys, unsigned rn)
{
    if (sys == SYS_IP)
    {
        asm_inst (a, loc, OP_JMP, rn, 0, 0, 0, NOIMM);
        return;
    }

    if (a -> has_base && a -> base >= BIOS_BASE)
    {
        asm_warn (a, loc, "%s is patched at run time: this code must execute from RAM, "
                          "cartridge ROM is read-only", sys == SYS_IR ? "IR" : "IV");
    }

    asm_inst (a, loc, OP_MOV, 0, rn, 5, 0, imm_bits (here (a) + (sys == SYS_IR ? 2 : 3)));
}

void  asm_mov_ip_value (asm_t *a, const loc_t *loc, imm_t target)
{
    asm_inst (a, loc, OP_JMP, 0, 0, 0, 0, imm_address (a, loc, target));
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Data directives: integer, float, pointer, string, zstring, datafile
//
void  asm_data_begin (asm_t *a)
{
    a -> line_active = 1;
    a -> stmt_start  = a -> nwords;
    a -> stmt_fix    = a -> nfix;
}

void  asm_data_word (asm_t *a, const loc_t *loc, imm_t v)
{
    if (!a -> stmt_bad)
    {
        emit_imm (a, loc, v);
    }
}

void  asm_data_string (asm_t *a, str_t s)
{
    size_t  i;

    // each character takes a whole word; the string is zero-terminated
    ensure_words (a, s.n + 1);

    for (i = 0; i < s.n; i++)
    {
        emit_word (a, (uint32_t) (unsigned char) s.p[i]);
    }

    emit_word (a, 0);
    free      (s.p);
}

// byte-packed string: four characters per word (first character in the low byte),
// followed by a zero byte; the final word is zero-padded
void  asm_data_zstring (asm_t *a, str_t s)
{
    size_t    i, nwords = (s.n + 1 + 3) / 4;
    uint32_t  w = 0;

    ensure_words (a, nwords);

    for (i = 0; i < nwords * 4; i++)
    {
        uint32_t  c = (i < s.n) ? (unsigned char) s.p[i] : 0;
        w |= c << (8 * (i % 4));
        if (i % 4 == 3)
        {
            emit_word (a, w);
            w = 0;
        }
    }

    free (s.p);
}

// open a data file, trying the directory of the current source file first
static FILE  *open_data_file (asm_t *a, const char *path)
{
    const char  *slash = a -> cur_file ? strrchr (a -> cur_file, '/') : NULL;
    FILE        *fp    = NULL;

    if (path[0] != '/' && slash != NULL)
    {
        size_t  dirlen = (size_t) (slash - a -> cur_file) + 1;
        char   *full   = xrealloc (NULL, dirlen + strlen (path) + 1);

        memcpy (full, a -> cur_file, dirlen);
        strcpy (full + dirlen, path);
        fp = fopen (full, "rb");
        free (full);
    }

    return (fp != NULL ? fp : fopen (path, "rb"));
}

void  asm_data_file (asm_t *a, const loc_t *loc, str_t path)
{
    FILE          *fp;
    unsigned char  chunk[4096];
    unsigned char  carry[4];
    size_t         carried = 0;
    size_t         got;

    if (path.n == 0 || strlen (path.p) != path.n)
    {
        asm_error (a, loc, "invalid data file path");
        free (path.p);
        return;
    }

    fp = open_data_file (a, path.p);

    if (fp == NULL)
    {
        asm_error (a, loc, "cannot open data file \"%s\"", path.p);
        free (path.p);
        return;
    }

    // the file content becomes consecutive little-endian words
    while ((got = fread (chunk, 1, sizeof (chunk), fp)) > 0)
    {
        size_t  i;

        ensure_words (a, got / 4 + 1);

        for (i = 0; i < got; i++)
        {
            carry[carried++] = chunk[i];

            if (carried == 4)
            {
                emit_word (a, (uint32_t) carry[0]
                            | ((uint32_t) carry[1] <<  8)
                            | ((uint32_t) carry[2] << 16)
                            | ((uint32_t) carry[3] << 24));
                carried = 0;
            }
        }
    }

    fclose (fp);

    if (carried != 0)
    {
        asm_error (a, loc, "data file \"%s\" size must be a multiple of 4 bytes", path.p);
    }

    free (path.p);
}

void  asm_data_end (asm_t *a)
{
    if (!a -> stmt_bad)
    {
        list_statement (a, 1);
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// End of line: roll back statements that failed, pause in step mode, reset flags
//
static void  step_pause (asm_t *a)
{
    int  c;

    if (a -> verbose)
    {
        fputs ("\033[H\033[2J", a -> dbg);
    }

    fputs ("[step] press Enter to continue ", stderr);
    fflush (stderr);

    do
    {
        c = getchar ();
    }
    while (c != '\n' && c != EOF);

    if (c == EOF)
    {
        a -> step = 0;
    }
}

void  asm_end_line (asm_t *a)
{
    if (a -> stmt_bad)
    {
        // forget whatever a failed statement had already put in the buffers
        a -> nwords = a -> stmt_start;
        a -> nfix   = a -> stmt_fix;

        while (a -> nrec > 0 && a -> rec[a -> nrec - 1].word >= a -> stmt_start)
        {
            a -> nrec--;
        }
    }
    else if (a -> step && a -> line_active)
    {
        step_pause (a);
    }

    a -> lex_error   = 0;
    a -> stmt_bad    = 0;
    a -> line_active = 0;
    a -> stmt_start  = a -> nwords;
    a -> stmt_fix    = a -> nfix;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Parsing entry point: assembles a block of text (a whole file, or one REPL line)
//
int  asm_parse (asm_t *a, const char *name, const char *text, size_t len, int first_line)
{
    unsigned  before = a -> errors;
    char     *buf;
    size_t    n = len;
    void     *scanner;

    // skip a UTF-8 byte order mark
    if (n >= 3 && (unsigned char) text[0] == 0xEF && (unsigned char) text[1] == 0xBB
               && (unsigned char) text[2] == 0xBF)
    {
        text += 3;
        n    -= 3;
    }

    // the scanner wants the text, a final newline and two terminating NUL bytes
    buf = xrealloc (NULL, n + 4);
    memcpy (buf, text, n);

    if (n == 0 || buf[n - 1] != '\n')
    {
        buf[n++] = '\n';
    }

    buf[n]     = 0;
    buf[n + 1] = 0;

    a -> cur_file    = name;
    a -> lex_line    = first_line;
    a -> lex_col     = 1;
    a -> lex_error   = 0;
    a -> stmt_bad    = 0;
    a -> line_active = 0;
    a -> stmt_start  = a -> nwords;
    a -> stmt_fix    = a -> nfix;

    scanner = v32_lex_new (a);

    if (scanner == NULL)
    {
        free (buf);
        asm_error (a, NULL, "unable to create the scanner");
        return (1);
    }

    v32_lex_buffer (scanner, buf, n + 2);

    if (yyparse (a, scanner) == 2)
    {
        asm_error (a, NULL, "parser ran out of memory");
    }

    v32_lex_free (scanner);
    free         (buf);

    return ((int) (a -> errors - before));
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Second pass: patch forward references, complain about labels never defined
//
int  asm_finish (asm_t *a)
{
    size_t  i;

    for (i = 0; i < a -> nfix; i++)
    {
        fixup_t  *f = &a -> fix[i];

        if (f -> sym -> defined)
        {
            a -> words[f -> word] = f -> sym -> addr;
        }
        else
        {
            a -> cur_file = f -> file;
            asm_error (a, &f -> loc, "undefined label '%s'", f -> sym -> name);
        }
    }

    a -> nfix = 0;

    return ((int) a -> errors);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// VBIN output: "V32-VBIN", the program size in words, then the words (little endian)
//
int  asm_write_vbin (asm_t *a, FILE *fp)
{
    size_t         total = 12 + a -> nwords * 4;
    unsigned char *buf   = xrealloc (NULL, total);
    size_t         i;
    int            ok;

    memcpy (buf, "V32-VBIN", 8);

    for (i = 0; i < 4; i++)
    {
        buf[8 + i] = (unsigned char) (((uint32_t) a -> nwords >> (8 * i)) & 0xFF);
    }

    for (i = 0; i < a -> nwords; i++)
    {
        uint32_t  w = a -> words[i];

        buf[12 + i * 4 + 0] = (unsigned char) (w         & 0xFF);
        buf[12 + i * 4 + 1] = (unsigned char) ((w >>  8) & 0xFF);
        buf[12 + i * 4 + 2] = (unsigned char) ((w >> 16) & 0xFF);
        buf[12 + i * 4 + 3] = (unsigned char) ((w >> 24) & 0xFF);
    }

    ok = (fwrite (buf, 1, total, fp) == total);
    free (buf);

    return (ok ? 0 : -1);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Debug file (-g): the same CSV the official assembler writes with "-g <mode>"
//
//     ADDRESS,file,line[,label]
//
// one line per instruction (data is not listed). ADDRESS is 0x%08X.
//   program: the instruction's memory address in words (0x20000000+n for a cartridge),
//            byte-for-byte what the official "-g program" prints
//   vbin:    byte offset in the .vbin file (0x0C header + 4 per word)
//   v32:     byte offset in the final .v32 cartridge file (0x8C header + 4 per word)
// The official assembler's vbin/v32 modes add that header to the *absolute* address times
// 4, which wraps around in 32 bits (e.g. 0x8000000C); here they give the real file offsets.
// The label column holds the alphabetically first label defined at that instruction.
//
typedef struct
{
    uint32_t     addr;
    const sym_t *sym;
}
labaddr_t;

static int  labaddr_cmp (const void *x, const void *y)
{
    const labaddr_t  *p = x, *q = y;

    if (p -> addr != q -> addr)
    {
        return (p -> addr < q -> addr ? -1 : 1);
    }

    return (strcmp (p -> sym -> name, q -> sym -> name));
}

static const sym_t  *label_at (const labaddr_t *v, size_t n, uint32_t addr)
{
    size_t  lo = 0, hi = n;

    while (lo < hi)
    {
        size_t  mid = (lo + hi) / 2;

        if (v[mid].addr < addr)
        {
            lo = mid + 1;
        }
        else
        {
            hi = mid;
        }
    }

    return ((lo < n && v[lo].addr == addr) ? v[lo].sym : NULL);
}

static void  put_path (FILE *fp, const char *path)
{
    // like the official tool: backslashes become '/', "//" collapses, a leading "./" goes
    const char  *p = path;
    int          prev_slash = 0;

    if (p[0] == '.' && p[1] == '/')
    {
        p += 2;
    }

    for (; *p; p++)
    {
        char  c = (*p == '\\') ? '/' : *p;

        if (c == '/' && prev_slash)
        {
            continue;
        }

        prev_slash = (c == '/');
        fputc (c, fp);
    }
}

int  asm_write_debug (asm_t *a, FILE *fp, dbgmode_t mode)
{
    uint32_t    base_off = (mode == DBG_V32) ? 0x8C : (mode == DBG_VBIN ? 0x0C : 0);
    uint32_t    unit     = (mode == DBG_PROGRAM) ? 1 : 4;
    labaddr_t  *v        = NULL;
    size_t      n = 0, i;
    sym_t      *s;

    for (s = a -> first; s != NULL; s = s -> order)
    {
        n += s -> defined ? 1 : 0;
    }

    v = xrealloc (NULL, (n + 1) * sizeof (labaddr_t));

    for (n = 0, s = a -> first; s != NULL; s = s -> order)
    {
        if (s -> defined)
        {
            v[n].addr = s -> addr;
            v[n].sym  = s;
            n++;
        }
    }

    qsort (v, n, sizeof (labaddr_t), labaddr_cmp);

    for (i = 0; i < a -> nrec; i++)
    {
        const dbgrec_t  *r   = &a -> rec[i];
        const sym_t     *lab = label_at (v, n, a -> base + r -> word);

        // program mode prints the address the CPU will see (this is what the official
        // assembler really prints); the file modes print the byte offset in the file
        fprintf (fp, "0x%08X,", mode == DBG_PROGRAM ? a -> base + r -> word
                                                    : base_off + r -> word * unit);
        put_path (fp, r -> file);
        fprintf (fp, ",%d", r -> line);

        if (lab != NULL)
        {
            fprintf (fp, ",%s", lab -> name);
        }

        fputc ('\n', fp);
    }

    free (v);

    return (ferror (fp) ? -1 : 0);
}
