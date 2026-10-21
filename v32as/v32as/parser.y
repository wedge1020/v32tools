%code requires {
//
// parser.y: bison grammar for the Vircon32 assembly language accepted by v32as
//
// The grammar is line oriented:
//
//     line    :  { label: }  [ instruction | directive ]  NEWLINE
//
// Every instruction is assembled by a semantic action as soon as its line has been
// parsed; forward references to labels are recorded as fixups and patched by
// asm_finish() once the whole input has been read.
//
// Operand rules follow the Vircon32 CPU specification (and the official assembler):
//
//     OC_R      PUSH POP CMPS CIF CFI CIB CFB NOT BNOT ISGN IABS FSGN FABS FLR CEIL
//               ROUND SIN ACOS LOG                       register
//     OC_RI     IEQ..ILE AND OR XOR SHL IADD..IMOD IMIN IMAX    register, register|integer
//     OC_RF     FEQ..FLE FADD..FMOD FMIN FMAX            register, register|float
//     OC_RR     ATAN2 POW                                register, register
//
////////////////////////////////////////////////////////////////////////////////////////

#include "v32as.h"

typedef void  *yyscan_t;
}

%code {
    #include <stdlib.h>

    int  yylex (YYSTYPE *lvalp, YYLTYPE *llocp, yyscan_t scanner);

    static void  yyerror (YYLTYPE *loc, asm_t *ctx, yyscan_t scanner, const char *msg);
}

%define api.pure full
%define api.location.type {loc_t}
%define parse.error verbose
%locations
%expect 0

%parse-param { asm_t *ctx }
%parse-param { yyscan_t scanner }
%lex-param   { yyscan_t scanner }

%union
{
    num_t     num;
    unsigned  reg;
    int       op;
    uint32_t  port;
    sym_t    *sym;
    str_t     str;
    imm_t     imm;
}

%token T_NL                  "end of line"
%token T_COMMA               "','"
%token T_LBRACK              "'['"
%token T_RBRACK              "']'"
%token T_PLUS                "'+'"
%token T_MINUS               "'-'"
%token T_BAD                 "invalid token"

%token <num>   T_NUMBER      "number"
%token <reg>   T_REG         "register"
%token <reg>   T_SYSREG       "internal register"
%token <port>  T_PORT        "I/O port"
%token <sym>   T_LABEL       "label"
%token <sym>   T_LABELDEF    "label definition"
%token <str>   T_STRING      "string"

%token T_DIR_INTEGER         "'integer'"
%token T_DIR_FLOAT           "'float'"
%token T_DIR_STRING          "'string'"
%token T_DIR_ZSTRING         "'zstring'"
%token T_DIR_POINTER         "'pointer'"
%token T_DIR_DATAFILE        "'datafile'"

%token <op> T_OP_NONE        "instruction without operands"
%token <op> T_OP_R           "single-register instruction"
%token <op> T_OP_RI          "integer instruction"
%token <op> T_OP_RF          "float instruction"
%token <op> T_OP_RR          "two-register instruction"
%token <op> T_OP_JMP         "jump instruction"
%token <op> T_OP_CJMP        "conditional jump"
%token <op> T_OP_MOV         "MOV"
%token <op> T_OP_LEA         "LEA"
%token <op> T_OP_IN          "IN"
%token <op> T_OP_OUT         "OUT"

%type <reg>   reg
%type <num>   sint offs
%type <imm>   imm_val
%type <port>  port

%destructor { free ($$.p); } <str>

%%

program
    : %empty
    | program line
    ;

line
    : labels stmt_opt T_NL           { asm_end_line (ctx); }
    | error T_NL                     { yyerrok; asm_end_line (ctx); }
    ;

labels
    : %empty
    | labels T_LABELDEF              { asm_label_define (ctx, $2, &@2); }
    ;

stmt_opt
    : %empty
    | instruction
    | directive
    ;

reg
    : T_REG                          { $$ = $1; }
    ;

sint
    : T_NUMBER                       { $$ = $1; }
    | T_PLUS T_NUMBER                { $$ = $2; }
    | T_MINUS T_NUMBER               { $$ = num_negate (ctx, &@$, $2); }
    ;

offs
    : T_PLUS T_NUMBER                { $$ = $2; }
    | T_MINUS T_NUMBER               { $$ = num_negate (ctx, &@$, $2); }
    ;

imm_val
    : sint                           { $$ = imm_any ($1); }
    | T_LABEL                        { $$ = imm_from_sym ($1); }
    ;

port
    : T_PORT                         { $$ = $1; }
    | sint                           { $$ = port_from_num (ctx, &@1, $1); }
    ;

instruction
    : T_OP_NONE
        { asm_inst (ctx, &@$, $1, 0, 0, 0, 0, NOIMM); }

    | T_OP_R reg
        { asm_inst (ctx, &@$, $1, $2, 0, 0, 0, NOIMM); }

    | T_OP_RI reg T_COMMA reg
        { asm_inst (ctx, &@$, $1, $2, $4, 0, 0, NOIMM); }
    | T_OP_RI reg T_COMMA sint
        { asm_inst (ctx, &@$, $1, $2, 0, 0, 0, imm_integer (ctx, &@4, $4)); }

    | T_OP_RF reg T_COMMA reg
        { asm_inst (ctx, &@$, $1, $2, $4, 0, 0, NOIMM); }
    | T_OP_RF reg T_COMMA sint
        { asm_inst (ctx, &@$, $1, $2, 0, 0, 0, imm_float ($4)); }

    | T_OP_RR reg T_COMMA reg
        { asm_inst (ctx, &@$, $1, $2, $4, 0, 0, NOIMM); }

    | T_OP_JMP reg
        { asm_inst (ctx, &@$, $1, $2, 0, 0, 0, NOIMM); }
    | T_OP_JMP imm_val
        { asm_inst (ctx, &@$, $1, 0, 0, 0, 0, imm_address (ctx, &@2, $2)); }

    | T_OP_CJMP reg T_COMMA reg
        { asm_inst (ctx, &@$, $1, $2, $4, 0, 0, NOIMM); }
    | T_OP_CJMP reg T_COMMA imm_val
        { asm_inst (ctx, &@$, $1, $2, 0, 0, 0, imm_address (ctx, &@4, $4)); }

    | T_OP_LEA reg T_COMMA T_LBRACK reg T_RBRACK
        { asm_inst (ctx, &@$, $1, $2, $5, 0, 0, NOIMM); }
    | T_OP_LEA reg T_COMMA T_LBRACK reg offs T_RBRACK
        { if ($6.bits == 0) asm_inst (ctx, &@$, $1, $2, $5, 0, 0, NOIMM);
          else asm_inst (ctx, &@$, $1, $2, $5, 0, 0, imm_integer (ctx, &@6, $6)); }

    | T_OP_IN reg T_COMMA port
        { asm_inst (ctx, &@$, $1, $2, 0, 0, $4, NOIMM); }

    | T_OP_OUT port T_COMMA reg
        { asm_inst (ctx, &@$, $1, 0, $4, 0, $2, NOIMM); }
    | T_OP_OUT port T_COMMA sint
        { asm_inst (ctx, &@$, $1, 0, 0, 0, $2, imm_any ($4)); }

    /* internal registers IP, IR, IV: expanded by the assembler (see asm.c) */
    | T_OP_MOV reg T_COMMA T_SYSREG
        { asm_mov_sysreg_read (ctx, &@$, $2, $4); }
    | T_OP_MOV T_SYSREG T_COMMA reg
        { asm_mov_sysreg_write (ctx, &@$, $2, $4); }
    | T_OP_MOV T_SYSREG T_COMMA imm_val
        { if ($2 != SYS_IP) asm_error (ctx, &@$, "IR and IV can only be written from a register");
          else asm_mov_ip_value (ctx, &@$, $4); }

    /* MOV addressing modes 0 to 7 */
    | T_OP_MOV reg T_COMMA imm_val
        { asm_inst (ctx, &@$, $1, $2, 0, 0, 0, $4); }
    | T_OP_MOV reg T_COMMA reg
        { asm_inst (ctx, &@$, $1, $2, $4, 1, 0, NOIMM); }
    | T_OP_MOV reg T_COMMA T_LBRACK imm_val T_RBRACK
        { asm_inst (ctx, &@$, $1, $2, 0, 2, 0, imm_address (ctx, &@5, $5)); }
    | T_OP_MOV reg T_COMMA T_LBRACK reg T_RBRACK
        { asm_inst (ctx, &@$, $1, $2, $5, 3, 0, NOIMM); }
    | T_OP_MOV reg T_COMMA T_LBRACK reg offs T_RBRACK
        { if ($6.bits == 0) asm_inst (ctx, &@$, $1, $2, $5, 3, 0, NOIMM);
          else asm_inst (ctx, &@$, $1, $2, $5, 4, 0, imm_integer (ctx, &@6, $6)); }
    | T_OP_MOV T_LBRACK imm_val T_RBRACK T_COMMA reg
        { asm_inst (ctx, &@$, $1, 0, $6, 5, 0, imm_address (ctx, &@3, $3)); }
    | T_OP_MOV T_LBRACK reg T_RBRACK T_COMMA reg
        { asm_inst (ctx, &@$, $1, $3, $6, 6, 0, NOIMM); }
    | T_OP_MOV T_LBRACK reg offs T_RBRACK T_COMMA reg
        { if ($4.bits == 0) asm_inst (ctx, &@$, $1, $3, $7, 6, 0, NOIMM);
          else asm_inst (ctx, &@$, $1, $3, $7, 7, 0, imm_integer (ctx, &@4, $4)); }
    ;

directive
    : T_DIR_INTEGER { asm_data_begin (ctx); } integer_list { asm_data_end (ctx); }
    | T_DIR_FLOAT   { asm_data_begin (ctx); } float_list   { asm_data_end (ctx); }
    | T_DIR_POINTER { asm_data_begin (ctx); } pointer_list { asm_data_end (ctx); }
    | T_DIR_STRING T_STRING
        { asm_data_begin (ctx); asm_data_string (ctx, $2); asm_data_end (ctx); }
    | T_DIR_ZSTRING T_STRING
        { asm_data_begin (ctx); asm_data_zstring (ctx, $2); asm_data_end (ctx); }
    | T_DIR_DATAFILE T_STRING
        { asm_data_begin (ctx); asm_data_file (ctx, &@2, $2); asm_data_end (ctx); }
    ;

integer_list
    : sint                           { asm_data_word (ctx, &@1, imm_integer (ctx, &@1, $1)); }
    | integer_list T_COMMA sint      { asm_data_word (ctx, &@3, imm_integer (ctx, &@3, $3)); }
    ;

float_list
    : sint                           { asm_data_word (ctx, &@1, imm_float ($1)); }
    | float_list T_COMMA sint        { asm_data_word (ctx, &@3, imm_float ($3)); }
    ;

pointer_list
    : T_LABEL                        { asm_data_word (ctx, &@1, imm_from_sym ($1)); }
    | pointer_list T_COMMA T_LABEL   { asm_data_word (ctx, &@3, imm_from_sym ($3)); }
    ;

%%

static void  yyerror (YYLTYPE *loc, asm_t *ctx, yyscan_t scanner, const char *msg)
{
    (void) scanner;

    // the lexer already explained invalid tokens; don't pile a generic message on top
    if (ctx -> lex_error)
    {
        return;
    }

    asm_error (ctx, loc, "%s", msg);
}
