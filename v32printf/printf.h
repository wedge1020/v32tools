/* ==========================================================================
   printf.h  -  A drop-in printf()/sprintf() emulation for Vircon32 C
   ==========================================================================

   Vircon32 C has no variadic functions, no char type (characters are just
   int, strings are int*), and no unions/generics. So instead of a single
   printf(fmt, ...), this provides fixed-arity families:

       printf1( x, y, fmt, a0 )                    - like print_at + printf
       printf2( x, y, fmt, a0, a1 )
       ...
       printf8( x, y, fmt, a0, a1, a2, a3, a4, a5, a6, a7 )

       sprintf1( dest, fmt, a0 )                    - like sprintf
       sprintf2( dest, fmt, a0, a1 )
       ...
       sprintf8( dest, fmt, a0, a1, a2, a3, a4, a5, a6, a7 )

   Every substitution argument is passed as a plain `int`:
     - For %d %i %u %x %X %o %c  : pass the integer value directly.
     - For %s                    : pass the string pointer (int*) directly.
     - For %f                    : you MUST wrap the float with printf_farg(),
                                    e.g.  printf1(x, y, "%.2f", printf_farg(3.14));
                                    (Vircon32 int and float are different bit
                                    layouts, so the raw float has to be
                                    reinterpreted as an int to fit in a
                                    generic argument slot. printf_farg() does
                                    that via memcpy(), and %f internally
                                    reverses it before calling ftoa().)

   Supported format spec:  %[flags][width][.precision]conv
     flags:      '-' left-align      '0' zero-pad      '+' force sign (numeric)
     width:      decimal digit sequence (minimum field width)
     precision:  '.' + decimal digit sequence
                     %s  -> max characters printed
                     %f  -> decimal places (see note below)
                     other convs -> currently ignored
     conv:       d i u x X o c s f %

   KNOWN LIMITATIONS (flagging these explicitly rather than pretending they
   don't exist):
     - %u is formatted the same as %d (base-10, signed). Vircon32 doesn't
       expose unsigned arithmetic ops, so a true unsigned rendering of a
       negative bit pattern isn't attempted.
     - %f precision is applied by padding/truncating the string ftoa()
       produces - it does NOT round the last kept digit, it just truncates.
       (ftoa's own precision behavior is undocumented/fixed, so this is the
       closest reasonable emulation with the tools available.)
     - No "%*d" (width/precision taken from an argument).
     - No positional args ("%1$s").
     - Field buffer is PF_FIELD_BUFFER_SIZE ints; a single formatted value
       (e.g. a %s with huge precision) longer than that will overrun it.
       Bump the constant below if you need more.

   VERIFICATION STATUS: This was written against the published Vircon32 C
   API reference (string.h / misc.h / video.h) but has NOT been compiled or
   run through v32sim - I don't have a way to do that for a standalone C
   header the way I can trace the assembly runtime. A few things I couldn't
   confirm from the docs and made a judgment call on:
     - Whether `(int*) some_int_variable` style casts are accepted by the
       compiler. If it complains, the fix is trivial: just drop the cast on
       the line `int* s = (int*) arg;` inside __pf_format (Vircon32 may treat
       int and int* as freely interchangeable without requiring one).
     - Whether local variables must all be declared at the very top of a
       function/block (I assumed yes, C89-style, and wrote it that way).
     - Whether char literals like '%%%%' or '-' are valid int-valued
       constants (I assumed yes, since isdigit/isalpha etc. imply this).
   Please compile this as-is first; if you hit errors, paste them back and
   I'll adjust - much faster than me guessing further.
   ========================================================================== */

#include "string.h"
#include "misc.h"
#include "video.h"

#define PF_FIELD_BUFFER_SIZE 64
#define PF_LINE_BUFFER_SIZE  256

/* ---- float <-> int, and pointer <-> int, raw bit-pattern reinterpretation
   Uses memcpy() (word copy) rather than a cast or a union, since neither
   is guaranteed to be accepted by the compiler for int <-> pointer/float
   conversions, whereas memcpy() operates on raw words regardless of type. */

int printf_farg( float value )
{
    int bits;
    memcpy( &bits, &value, 1 );
    return bits;
}

float __pf_bits_to_float( int bits )
{
    float value;
    memcpy( &value, &bits, 1 );
    return value;
}

int printf_sarg( int* text )
{
    int bits;
    memcpy( &bits, &text, 1 );
    return bits;
}

int* __pf_bits_to_ptr( int bits )
{
    int* text;
    memcpy( &text, &bits, 1 );
    return text;
}

/* Left-pads the digit portion of field (after any leading '-'/'+' sign)
   with '0' until it has at least `precision` digits. This is the C
   standard meaning of precision for %d/%i/%u/%x/%X/%o - it is unrelated
   to the '0' width flag. Returns the new length of field. */

int __pf_zero_extend( int* field, int flen, int precision )
{
    int sign_offset;
    int digits;
    int shift;
    int k;
    int z;

    if( precision < 0 ) { return flen; }

    sign_offset = 0;
    if( flen > 0 && ( field[0] == '-' || field[0] == '+' ) ) { sign_offset = 1; }

    digits = flen - sign_offset;
    if( digits >= precision ) { return flen; }

    shift = precision - digits;

    k = flen;
    while( k >= sign_offset )
    {
        field[ k + shift ] = field[ k ];
        k = k - 1;
    }

    z = 0;
    while( z < shift )
    {
        field[ sign_offset + z ] = '0';
        z = z + 1;
    }

    return flen + shift;
}

/* ---- shared formatting engine ----------------------------------------
   Writes the formatted result (null-terminated) into dest.
   dest must be large enough to hold the result - just like real sprintf,
   no bounds checking is performed on dest. */

void __pf_format( int* dest, int* fmt, int* args, int argc )
{
    int  pos;
    int  ai;
    int  i;
    int  [PF_FIELD_BUFFER_SIZE] field;

    int  left_align;
    int  zero_pad;
    int  force_sign;
    int  width;
    int  precision;
    int  conv;
    int  arg;
    int  flen;
    int  is_numeric;
    int  padcount;
    int  padchar;
    int  dot;
    int  have;
    int  p;
    int  k;
    int* s;
    int  slen;
    float fval;

    pos = 0;
    ai  = 0;
    i   = 0;

    while( fmt[i] != 0 )
    {
        if( fmt[i] != '%' )
        {
            dest[pos] = fmt[i];
            pos = pos + 1;
            i = i + 1;
        }
        else
        {
            i = i + 1;

            if( fmt[i] == '%' )
            {
                dest[pos] = '%';
                pos = pos + 1;
                i = i + 1;
            }
            else
            {
                /* --- flags --- */
                left_align = 0;
                zero_pad   = 0;
                force_sign = 0;

                while( fmt[i] != 0 && ( fmt[i] == '-' || fmt[i] == '0' || fmt[i] == '+' ) )
                {
                    if( fmt[i] == '-' ) { left_align = 1; }
                    if( fmt[i] == '0' ) { zero_pad   = 1; }
                    if( fmt[i] == '+' ) { force_sign = 1; }
                    i = i + 1;
                }

                /* --- width --- */
                width = 0;
                while( fmt[i] != 0 && isdigit( fmt[i] ) )
                {
                    width = ( width * 10 ) + ( fmt[i] - '0' );
                    i = i + 1;
                }

                /* --- precision --- */
                precision = -1;
                if( fmt[i] == '.' )
                {
                    i = i + 1;
                    precision = 0;
                    while( fmt[i] != 0 && isdigit( fmt[i] ) )
                    {
                        precision = ( precision * 10 ) + ( fmt[i] - '0' );
                        i = i + 1;
                    }
                }

                /* --- conversion character --- */
                conv = fmt[i];
                if( conv != 0 ) { i = i + 1; }

                /* --- fetch next argument (0 if we ran out) --- */
                arg = 0;
                if( ai < argc )
                {
                    arg = args[ai];
                    ai = ai + 1;
                }

                flen       = 0;
                is_numeric = 0;

                if( conv == 'd' || conv == 'i' || conv == 'u' )
                {
                    itoa( arg, field, 10 );
                    flen = strlen( field );
                    flen = __pf_zero_extend( field, flen, precision );
                    is_numeric = 1;

                    if( force_sign && arg >= 0 )
                    {
                        k = flen;
                        while( k >= 0 )
                        {
                            field[ k + 1 ] = field[ k ];
                            k = k - 1;
                        }
                        field[0] = '+';
                        flen = flen + 1;
                    }
                }
                else if( conv == 'x' || conv == 'X' )
                {
                    itoa( arg, field, 16 );
                    flen = strlen( field );
                    flen = __pf_zero_extend( field, flen, precision );
                    is_numeric = 1;

                    if( conv == 'X' )
                    {
                        k = 0;
                        while( k < flen )
                        {
                            field[k] = toupper( field[k] );
                            k = k + 1;
                        }
                    }
                }
                else if( conv == 'o' )
                {
                    itoa( arg, field, 8 );
                    flen = strlen( field );
                    flen = __pf_zero_extend( field, flen, precision );
                    is_numeric = 1;
                }
                else if( conv == 'c' )
                {
                    field[0] = arg;
                    field[1] = 0;
                    flen = 1;
                }
                else if( conv == 's' )
                {
                    s = __pf_bits_to_ptr( arg );
                    slen = strlen( s );
                    if( precision >= 0 && precision < slen ) { slen = precision; }

                    k = 0;
                    while( k < slen )
                    {
                        field[k] = s[k];
                        k = k + 1;
                    }
                    field[slen] = 0;
                    flen = slen;
                }
                else if( conv == 'f' )
                {
                    fval = __pf_bits_to_float( arg );
                    ftoa( fval, field );
                    flen = strlen( field );
                    is_numeric = 1;

                    if( precision >= 0 )
                    {
                        dot = -1;
                        k = 0;
                        while( k < flen )
                        {
                            if( field[k] == '.' ) { dot = k; }
                            k = k + 1;
                        }

                        if( dot == -1 )
                        {
                            if( precision > 0 )
                            {
                                field[flen] = '.';
                                flen = flen + 1;
                                p = 0;
                                while( p < precision )
                                {
                                    field[flen] = '0';
                                    flen = flen + 1;
                                    p = p + 1;
                                }
                            }
                        }
                        else
                        {
                            have = flen - dot - 1;
                            if( have > precision )
                            {
                                if( precision == 0 ) { flen = dot; }
                                else { flen = dot + 1 + precision; }
                                field[flen] = 0;
                            }
                            else
                            {
                                p = have;
                                while( p < precision )
                                {
                                    field[flen] = '0';
                                    flen = flen + 1;
                                    p = p + 1;
                                }
                                field[flen] = 0;
                            }
                        }
                    }
                }
                else
                {
                    /* Unknown conversion - emit it literally so mistakes
                       are visible instead of silently swallowed. */
                    field[0] = '%';
                    field[1] = conv;
                    field[2] = 0;
                    flen = 2;
                    ai = ai - 1; /* we didn't actually consume an argument */
                    if( ai < 0 ) { ai = 0; }
                }

                field[flen] = 0;

                /* --- padding to width --- */
                padcount = width - flen;
                if( padcount < 0 ) { padcount = 0; }

                if( left_align )
                {
                    k = 0;
                    while( k < flen )
                    {
                        dest[pos] = field[k];
                        pos = pos + 1;
                        k = k + 1;
                    }
                    p = 0;
                    while( p < padcount )
                    {
                        dest[pos] = ' ';
                        pos = pos + 1;
                        p = p + 1;
                    }
                }
                else
                {
                    padchar = ' ';
                    if( zero_pad && is_numeric ) { padchar = '0'; }

                    if( padchar == '0' && flen > 0 && ( field[0] == '-' || field[0] == '+' ) )
                    {
                        dest[pos] = field[0];
                        pos = pos + 1;
                        p = 0;
                        while( p < padcount )
                        {
                            dest[pos] = '0';
                            pos = pos + 1;
                            p = p + 1;
                        }
                        k = 1;
                        while( k < flen )
                        {
                            dest[pos] = field[k];
                            pos = pos + 1;
                            k = k + 1;
                        }
                    }
                    else
                    {
                        p = 0;
                        while( p < padcount )
                        {
                            dest[pos] = padchar;
                            pos = pos + 1;
                            p = p + 1;
                        }
                        k = 0;
                        while( k < flen )
                        {
                            dest[pos] = field[k];
                            pos = pos + 1;
                            k = k + 1;
                        }
                    }
                }
            }
        }
    }

    dest[pos] = 0;
}

/* ==========================================================================
   sprintf1 .. sprintf8
   ========================================================================== */

void sprintf1( int* dest, int* fmt, int a0 )
{
    int [1] args;
    args[0] = a0;
    __pf_format( dest, fmt, args, 1 );
}

void sprintf2( int* dest, int* fmt, int a0, int a1 )
{
    int [2] args;
    args[0] = a0; args[1] = a1;
    __pf_format( dest, fmt, args, 2 );
}

void sprintf3( int* dest, int* fmt, int a0, int a1, int a2 )
{
    int [3] args;
    args[0] = a0; args[1] = a1; args[2] = a2;
    __pf_format( dest, fmt, args, 3 );
}

void sprintf4( int* dest, int* fmt, int a0, int a1, int a2, int a3 )
{
    int [4] args;
    args[0] = a0; args[1] = a1; args[2] = a2; args[3] = a3;
    __pf_format( dest, fmt, args, 4 );
}

void sprintf5( int* dest, int* fmt, int a0, int a1, int a2, int a3, int a4 )
{
    int [5] args;
    args[0] = a0; args[1] = a1; args[2] = a2; args[3] = a3; args[4] = a4;
    __pf_format( dest, fmt, args, 5 );
}

void sprintf6( int* dest, int* fmt, int a0, int a1, int a2, int a3, int a4, int a5 )
{
    int [6] args;
    args[0] = a0; args[1] = a1; args[2] = a2; args[3] = a3; args[4] = a4; args[5] = a5;
    __pf_format( dest, fmt, args, 6 );
}

void sprintf7( int* dest, int* fmt, int a0, int a1, int a2, int a3, int a4, int a5, int a6 )
{
    int [7] args;
    args[0] = a0; args[1] = a1; args[2] = a2; args[3] = a3; args[4] = a4; args[5] = a5; args[6] = a6;
    __pf_format( dest, fmt, args, 7 );
}

void sprintf8( int* dest, int* fmt, int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7 )
{
    int [8] args;
    args[0] = a0; args[1] = a1; args[2] = a2; args[3] = a3;
    args[4] = a4; args[5] = a5; args[6] = a6; args[7] = a7;
    __pf_format( dest, fmt, args, 8 );
}

/* ==========================================================================
   printf1 .. printf8
   Formats into an internal line buffer, then draws it with print_at().
   ========================================================================== */

void printf1( int x, int y, int* fmt, int a0 )
{
    int  [ PF_LINE_BUFFER_SIZE ] buf;
    int [1] args;
    args[0] = a0;
    __pf_format( buf, fmt, args, 1 );
    print_at( x, y, buf );
}

void printf2( int x, int y, int* fmt, int a0, int a1 )
{
    int  [ PF_LINE_BUFFER_SIZE ] buf;
    int  [2] args;
    args[0] = a0; args[1] = a1;
    __pf_format( buf, fmt, args, 2 );
    print_at( x, y, buf );
}

void printf3( int x, int y, int* fmt, int a0, int a1, int a2 )
{
    int  [ PF_LINE_BUFFER_SIZE ] buf;
    int  [3] args;
    args[0] = a0; args[1] = a1; args[2] = a2;
    __pf_format( buf, fmt, args, 3 );
    print_at( x, y, buf );
}

void printf4( int x, int y, int* fmt, int a0, int a1, int a2, int a3 )
{
    int  [ PF_LINE_BUFFER_SIZE ] buf;
    int  [4] args;
    args[0] = a0; args[1] = a1; args[2] = a2; args[3] = a3;
    __pf_format( buf, fmt, args, 4 );
    print_at( x, y, buf );
}

void printf5( int x, int y, int* fmt, int a0, int a1, int a2, int a3, int a4 )
{
    int  [ PF_LINE_BUFFER_SIZE ] buf;
    int  [5] args;
    args[0] = a0; args[1] = a1; args[2] = a2; args[3] = a3; args[4] = a4;
    __pf_format( buf, fmt, args, 5 );
    print_at( x, y, buf );
}

void printf6( int x, int y, int* fmt, int a0, int a1, int a2, int a3, int a4, int a5 )
{
    int  [ PF_LINE_BUFFER_SIZE ] buf;
    int  [6] args;
    args[0] = a0; args[1] = a1; args[2] = a2; args[3] = a3; args[4] = a4; args[5] = a5;
    __pf_format( buf, fmt, args, 6 );
    print_at( x, y, buf );
}

void printf7( int x, int y, int* fmt, int a0, int a1, int a2, int a3, int a4, int a5, int a6 )
{
    int  [ PF_LINE_BUFFER_SIZE ] buf;
    int  [7] args;
    args[0] = a0; args[1] = a1; args[2] = a2; args[3] = a3; args[4] = a4; args[5] = a5; args[6] = a6;
    __pf_format( buf, fmt, args, 7 );
    print_at( x, y, buf );
}

void printf8( int x, int y, int* fmt, int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7 )
{
    int  [ PF_LINE_BUFFER_SIZE ] buf;
    int  [8] args;
    args[0] = a0; args[1] = a1; args[2] = a2; args[3] = a3;
    args[4] = a4; args[5] = a5; args[6] = a6; args[7] = a7;
    __pf_format( buf, fmt, args, 8 );
    print_at( x, y, buf );
}

/* ==========================================================================
   Example usage:

       printf1( 10, 10, "%3s", "hi" );
       printf1( 10, 30, "score: %05d", player_score );
       printf2( 10, 50, "%-8s: %3.1f%%", "health", printf_farg( health_pct ) );
       sprintf3( string_data, "%s-%s-%s", str1, str2, str3 );
   ========================================================================== */
