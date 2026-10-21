%{
//
// lexer.l: flex scanner for the Vircon32 assembly language accepted by v32as
//
// Number formats (all inherited from the bash v32as):
//
//     0xFF  FFh        hexadecimal (1-8 digits)
//     0b1010           binary (1-32 digits)
//     0777             octal
//     42    #42        decimal (the sign is handled by the grammar: -42, #-42)
//     3.14             float
//     'A'  '\x41'      character literal (an integer; same escapes as strings)
//
// Identifiers are classified through the hash table in tables.c: mnemonics, registers,
// directives and I/O port names are case-insensitive; labels start with an underscore.
//
////////////////////////////////////////////////////////////////////////////////////////

#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "v32as.h"
#include "parser.tab.h"

#define  YY_USER_ACTION                                                          \
    yylloc->first_line    = yylloc->last_line = yyextra->lex_line;               \
    yylloc->first_column  = yyextra->lex_col;                                    \
    yyextra->lex_col     += (int) yyleng;                                        \
    yylloc->last_column   = yyextra->lex_col - 1;

static int  lex_fail (asm_t *a, const loc_t *loc, const char *fmt, ...)
            __attribute__ ((format (printf, 3, 4)));

static int  lex_fail (asm_t *a, const loc_t *loc, const char *fmt, ...)
{
    char     buf[256];
    va_list  ap;

    va_start (ap, fmt);
    vsnprintf (buf, sizeof (buf), fmt, ap);
    va_end (ap);

    asm_error (a, loc, "%s", buf);
    a -> lex_error = 1;

    return (T_BAD);
}

static int  lex_number (asm_t *a, const loc_t *loc, YYSTYPE *lv, const char *digits,
                        int base)
{
    unsigned long long  value = 0;

    errno = 0;
    value = strtoull (digits, NULL, base);

    if (errno != 0 || value > 0xFFFFFFFFull)
    {
        return (lex_fail (a, loc, "number out of range for a 32-bit value"));
    }

    lv -> num.bits     = (uint32_t) value;
    lv -> num.is_float = 0;

    return (T_NUMBER);
}

// Decode one escape sequence at s[*i] (just after the backslash); returns the byte
static int  lex_escape (asm_t *a, const loc_t *loc, const char *s, size_t *i, int *bad)
{
    char  c = s[(*i)++];

    switch (c)
    {
        case 'n':   return ('\n');
        case 'r':   return ('\r');
        case 't':   return ('\t');
        case '\\':  return ('\\');
        case '\'':  return ('\'');
        case '"':   return ('"');

        case 'x':
            if (isxdigit ((unsigned char) s[*i]) && isxdigit ((unsigned char) s[*i + 1]))
            {
                char  hex[3] = { s[*i], s[*i + 1], 0 };
                *i += 2;
                return ((int) strtol (hex, NULL, 16));
            }
            lex_fail (a, loc, "bad hexadecimal escape (expected 2 hex digits)");
            *bad = 1;
            return (0);

        default:
            asm_warn (a, loc, "unknown escape character '\\%c'", c);
            return ((unsigned char) c);
    }
}

%}

%option reentrant bison-bridge bison-locations
%option noyywrap nounput noinput nodefault
%option extra-type="asm_t *"

WS          [ \t\r\f\v]+
IDENT       [A-Za-z_][A-Za-z0-9_]*

%%

{WS}                    { /* skip blanks */ }
;[^\n]*                 { /* comment */ }
\n                      {
                            yyextra -> lex_line++;
                            yyextra -> lex_col = 1;
                            return (T_NL);
                        }

","                     { return (T_COMMA);  }
"["                     { return (T_LBRACK); }
"]"                     { return (T_RBRACK); }
"+"                     { return (T_PLUS);   }
"-"                     { return (T_MINUS);  }

 /* ------------------------------ numbers ------------------------------ */

0[xX][0-9A-Fa-f]+       { return (lex_number (yyextra, yylloc, yylval, yytext + 2, 16)); }
0[bB][01]+              { return (lex_number (yyextra, yylloc, yylval, yytext + 2,  2)); }
[0-9A-Fa-f]+[hH]        { return (lex_number (yyextra, yylloc, yylval, yytext,     16)); }
0[0-7]+                 { return (lex_number (yyextra, yylloc, yylval, yytext + 1,  8)); }
0[0-9]+                 { return (lex_fail (yyextra, yylloc, "invalid octal number '%s'", yytext)); }
[0-9]+                  { return (lex_number (yyextra, yylloc, yylval, yytext, 10)); }
#[+-]?[0-9]+            {
                            int      neg = (yytext[1] == '-');
                            const char *d = yytext + 1 + ((yytext[1] == '+' || neg) ? 1 : 0);
                            int      tok = lex_number (yyextra, yylloc, yylval, d, 10);

                            if (tok == T_NUMBER && neg)
                            {
                                yylval -> num = num_negate (yyextra, yylloc, yylval -> num);
                            }
                            return (tok);
                        }
[0-9]+"."[0-9]+         {
                            float  f = strtof (yytext, NULL);

                            if (isinf (f))
                            {
                                return (lex_fail (yyextra, yylloc, "number out of range for a float"));
                            }
                            memcpy (&yylval -> num.bits, &f, sizeof (f));
                            yylval -> num.is_float = 1;
                            return (T_NUMBER);
                        }

 /* ------------------------ character and string literals -------------------- */

'(\\x[0-9A-Fa-f]{2}|\\.|[^'\\\n])'   {
                            size_t  i   = 1;
                            int     bad = 0;
                            int     c;

                            if (yytext[1] == '\\')
                            {
                                i++;
                                c = lex_escape (yyextra, yylloc, yytext, &i, &bad);
                            }
                            else
                            {
                                c = (unsigned char) yytext[1];
                            }

                            if (bad)
                            {
                                return (T_BAD);
                            }
                            yylval -> num.bits     = (uint32_t) c;
                            yylval -> num.is_float = 0;
                            return (T_NUMBER);
                        }
'                       { return (lex_fail (yyextra, yylloc, "malformed character literal")); }

\"(\\.|[^"\\\n])*\"     {
                            size_t  n   = (size_t) yyleng - 2;
                            char   *out = malloc (n + 1);
                            size_t  i   = 1;
                            size_t  len = 0;
                            int     bad = 0;

                            if (out == NULL)
                            {
                                return (lex_fail (yyextra, yylloc, "out of memory"));
                            }

                            while (i < (size_t) yyleng - 1)
                            {
                                if (yytext[i] == '\\')
                                {
                                    i++;
                                    out[len++] = (char) lex_escape (yyextra, yylloc, yytext, &i, &bad);
                                }
                                else
                                {
                                    out[len++] = yytext[i++];
                                }
                            }

                            if (bad)
                            {
                                free (out);
                                return (T_BAD);
                            }

                            out[len]          = 0;
                            yylval -> str.p   = out;
                            yylval -> str.n   = len;
                            return (T_STRING);
                        }
\"(\\.|[^"\\\n])*       { return (lex_fail (yyextra, yylloc, "string not terminated")); }

 /* ------------------------ labels, keywords, identifiers -------------------- */

_[A-Za-z0-9_]+":"       {
                            yylval -> sym = asm_sym_intern (yyextra, yytext, (size_t) yyleng - 1);
                            return (T_LABELDEF);
                        }
[A-Za-z][A-Za-z0-9_]*":" {
                            return (lex_fail (yyextra, yylloc,
                                              "label '%.*s' must start with an underscore",
                                              (int) yyleng - 1, yytext));
                        }

{IDENT}                 {
                            const kw_t  *k = kw_lookup (yytext, (size_t) yyleng);

                            if (k != NULL)
                            {
                                switch (k -> kind)
                                {
                                    case KW_REGISTER:
                                        yylval -> reg = k -> value;
                                        return (T_REG);

                                    case KW_SYSREG:
                                        yylval -> reg = k -> value;
                                        return (T_SYSREG);

                                    case KW_PORT:
                                        yylval -> port = k -> value;
                                        return (T_PORT);

                                    case KW_PORTVALUE:
                                    case KW_CONSTANT:
                                        yylval -> num.bits     = k -> value;
                                        yylval -> num.is_float = 0;
                                        return (T_NUMBER);

                                    case KW_DIRECTIVE:
                                        switch (k -> cls)
                                        {
                                            case DIR_INTEGER:   return (T_DIR_INTEGER);
                                            case DIR_FLOAT:     return (T_DIR_FLOAT);
                                            case DIR_STRING:    return (T_DIR_STRING);
                                            case DIR_ZSTRING:   return (T_DIR_ZSTRING);
                                            case DIR_POINTER:   return (T_DIR_POINTER);
                                            default:            return (T_DIR_DATAFILE);
                                        }

                                    case KW_OPCODE:
                                        yylval -> op = (int) k -> value;
                                        switch (k -> cls)
                                        {
                                            case OC_NONE:   return (T_OP_NONE);
                                            case OC_R:      return (T_OP_R);
                                            case OC_RI:     return (T_OP_RI);
                                            case OC_RF:     return (T_OP_RF);
                                            case OC_RR:     return (T_OP_RR);
                                            case OC_JMP:    return (T_OP_JMP);
                                            case OC_CJMP:   return (T_OP_CJMP);
                                            case OC_MOV:    return (T_OP_MOV);
                                            case OC_LEA:    return (T_OP_LEA);
                                            case OC_IN:     return (T_OP_IN);
                                            default:        return (T_OP_OUT);
                                        }
                                }
                            }

                            if (yytext[0] == '_' && yyleng >= 2)
                            {
                                yylval -> sym = asm_sym_intern (yyextra, yytext, (size_t) yyleng);
                                return (T_LABEL);
                            }

                            if (yytext[0] == '_')
                            {
                                return (lex_fail (yyextra, yylloc,
                                                  "a label needs at least one character after '_'"));
                            }

                            return (lex_fail (yyextra, yylloc, "unknown identifier '%s'", yytext));
                        }

.                       {
                            return (lex_fail (yyextra, yylloc, "unexpected character '%c'",
                                              (yytext[0] >= 32 && yytext[0] < 127) ? yytext[0] : '?'));
                        }

%%

////////////////////////////////////////////////////////////////////////////////////////
//
// Scanner management for the rest of the program (keeps the generated header private)
//
void  *v32_lex_new (asm_t *a)
{
    yyscan_t  scanner = NULL;

    if (yylex_init_extra (a, &scanner) != 0)
    {
        return (NULL);
    }

    return (scanner);
}

void  v32_lex_free (void *scanner)
{
    if (scanner != NULL)
    {
        yylex_destroy ((yyscan_t) scanner);
    }
}

void  v32_lex_buffer (void *scanner, char *buf, size_t size)
{
    yy_scan_buffer (buf, size, (yyscan_t) scanner);
}
