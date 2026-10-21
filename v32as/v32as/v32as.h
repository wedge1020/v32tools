//
// v32as.h: shared types and prototypes for v32as, the C implementation of the
// Vircon32 assembler (flex for lexing, bison for parsing)
//
////////////////////////////////////////////////////////////////////////////////////////

#ifndef V32AS_H
#define V32AS_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define  VERSION            "20261015-dev"

#define  BIOS_BASE          0x10000000u
#define  CART_BASE          0x20000000u
#define  PORT_MASK          0x3FFFu          // the port field of an instruction is 14 bits

////////////////////////////////////////////////////////////////////////////////////////
//
// Source locations (compatible with bison's default location members)
//
typedef struct
{
    int  first_line;
    int  first_column;
    int  last_line;
    int  last_column;
}
loc_t;

////////////////////////////////////////////////////////////////////////////////////////
//
// Instruction operand classes: each one corresponds to a family of grammar rules in
// parser.y. The table in tables.c assigns one class to each of the 64 opcodes.
//
typedef enum
{
    OC_NONE,        // HLT WAIT RET MOVS SETS
    OC_R,           // one register
    OC_RI,          // register, register|integer
    OC_RF,          // register, register|float (integer literals are converted)
    OC_RR,          // register, register (ATAN2, POW)
    OC_JMP,         // JMP CALL: register|address
    OC_CJMP,        // JT JF:    register, register|address
    OC_MOV,         // the eight MOV addressing modes
    OC_LEA,         // LEA register, [register (+offset)]
    OC_IN,          // IN register, port
    OC_OUT          // OUT port, register|value
}
opclass_t;

typedef enum
{
    KW_OPCODE,
    KW_DIRECTIVE,
    KW_REGISTER,
    KW_PORT,
    KW_PORTVALUE,
    KW_CONSTANT,
    KW_SYSREG                       // IP IR IV: internal registers (MOV only)
}
kw_kind_t;

typedef struct
{
    const char *name;
    kw_kind_t   kind;
    int         cls;                // opclass_t for opcodes; directive id for directives
    uint32_t    value;              // opcode number, register index, port, constant
}
kw_t;

typedef enum
{
    DIR_INTEGER,
    DIR_FLOAT,
    DIR_STRING,                     // one character per word, zero-terminated
    DIR_ZSTRING,                    // byte-packed (4 per word, little-endian), zero-terminated
    DIR_POINTER,
    DIR_DATAFILE
}
directive_t;

////////////////////////////////////////////////////////////////////////////////////////
//
// Values produced by the lexer and used by the grammar
//
typedef struct
{
    uint32_t  bits;                 // the 32-bit pattern (IEEE-754 for floats)
    int       is_float;
}
num_t;

typedef struct sym
{
    char         *name;
    uint32_t      addr;             // absolute address (valid once defined)
    int           defined;
    struct sym   *next;             // hash chain
    struct sym   *order;            // definition/reference order list
}
sym_t;

typedef struct
{
    enum { IMM_NONE, IMM_BITS, IMM_SYM }  kind;
    uint32_t  bits;
    sym_t    *sym;
    int       is_float;
}
imm_t;

#define  NOIMM  ((imm_t) { IMM_NONE, 0, NULL, 0 })

typedef struct
{
    char    *p;
    size_t   n;
}
str_t;

////////////////////////////////////////////////////////////////////////////////////////
//
// Assembler context
//
typedef struct
{
    uint32_t     word;              // index into the output word array
    sym_t       *sym;
    const char  *file;
    loc_t        loc;
}
fixup_t;

// one record per assembled instruction, for the -g debug file
typedef struct
{
    uint32_t     word;              // index of the instruction's first word in the output
    const char  *file;
    int          line;
}
dbgrec_t;

typedef enum { DBG_PROGRAM, DBG_VBIN, DBG_V32 } dbgmode_t;

typedef struct asm_ctx
{
    // configuration
    int          has_base;          // BIOS or CART mode: addresses are meaningful
    uint32_t     base;
    int          verbose;
    int          step;
    int          listing;           // print the human-readable listing
    int          interactive;       // true inside the REPL
    FILE        *out;               // listing stream
    FILE        *dbg;               // verbose (debug) stream

    // output
    uint32_t    *words;
    size_t       nwords;
    size_t       cap;

    // symbols and forward references
    sym_t      **tab;
    size_t       tabcap;
    size_t       nsyms;
    sym_t       *first;
    sym_t       *last;
    fixup_t     *fix;
    size_t       nfix;
    size_t       fixcap;

    // instruction -> source line records (for -g)
    dbgrec_t    *rec;
    size_t       nrec;
    size_t       reccap;

    // state
    const char  *cur_file;
    unsigned     errors;
    unsigned     warnings;
    int          lex_error;         // lexer already reported a problem on this line
    int          line_active;       // the current line produced a label or statement
    int          stmt_bad;          // an error was reported inside the current statement
    size_t       stmt_start;        // first word of the statement being assembled
    size_t       stmt_fix;          // fixup count at statement start
    int          lex_line;          // lexer bookkeeping
    int          lex_col;
}
asm_t;

////////////////////////////////////////////////////////////////////////////////////////
//
// tables.c: keyword lookup (opcodes, directives, registers, ports, constants)
//
const kw_t  *kw_lookup      (const char *s, size_t n);      // case-insensitive
const char  *opcode_name    (unsigned opcode);
int          port_is_known  (unsigned port);

////////////////////////////////////////////////////////////////////////////////////////
//
// asm.c: the assembler core
//
void    asm_init            (asm_t *a);
void    asm_free            (asm_t *a);
void    asm_reset           (asm_t *a);
int     asm_parse           (asm_t *a, const char *name, const char *text, size_t len,
                             int first_line);
int     asm_finish          (asm_t *a);
int     asm_write_vbin      (asm_t *a, FILE *fp);
int     asm_write_debug     (asm_t *a, FILE *fp, dbgmode_t mode);
void    asm_print_labels    (asm_t *a, FILE *fp);

void    asm_error           (asm_t *a, const loc_t *loc, const char *fmt, ...)
                            __attribute__ ((format (printf, 3, 4)));
void    asm_warn            (asm_t *a, const loc_t *loc, const char *fmt, ...)
                            __attribute__ ((format (printf, 3, 4)));

sym_t  *asm_sym_intern      (asm_t *a, const char *name, size_t n);
void    asm_label_define    (asm_t *a, sym_t *s, const loc_t *loc);

// numeric helpers used by the grammar
num_t   num_negate          (asm_t *a, const loc_t *loc, num_t n);
imm_t   imm_from_sym        (sym_t *s);
imm_t   imm_any             (num_t n);                      // bits exactly as written
imm_t   imm_integer         (asm_t *a, const loc_t *loc, num_t n);
imm_t   imm_float           (num_t n);                      // integers get converted
imm_t   imm_address         (asm_t *a, const loc_t *loc, imm_t v);   // int or label
uint32_t port_from_num      (asm_t *a, const loc_t *loc, num_t n);

// emitters used by the grammar
void    asm_inst            (asm_t *a, const loc_t *loc, unsigned opcode,
                             unsigned r1, unsigned r2, unsigned mode, unsigned port,
                             imm_t imm);
// internal registers (IP, IR, IV), reached through MOV expansions; see asm.c
enum { SYS_IP = 0, SYS_IR = 1, SYS_IV = 2 };
void    asm_mov_sysreg_read  (asm_t *a, const loc_t *loc, unsigned rn, unsigned sys);
void    asm_mov_sysreg_write (asm_t *a, const loc_t *loc, unsigned sys, unsigned rn);
void    asm_mov_ip_value     (asm_t *a, const loc_t *loc, imm_t target);
void    asm_data_begin      (asm_t *a);
void    asm_data_word       (asm_t *a, const loc_t *loc, imm_t v);
void    asm_data_string     (asm_t *a, str_t s);
void    asm_data_zstring    (asm_t *a, str_t s);
void    asm_data_file       (asm_t *a, const loc_t *loc, str_t path);
void    asm_data_end        (asm_t *a);
void    asm_end_line        (asm_t *a);

////////////////////////////////////////////////////////////////////////////////////////
//
// lexer.l: scanner management (the generated scanner header is kept private)
//
void   *v32_lex_new         (asm_t *a);
void    v32_lex_free        (void *scanner);
void    v32_lex_buffer      (void *scanner, char *buf, size_t size);   // buf: text\n\0\0

#endif
