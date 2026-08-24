# printf.h — printf() / sprintf() for Vircon32 C

A single-header library that emulates the C standard library's `printf()` and
`sprintf()` for the [Vircon32](https://www.vircon32.com/) fantasy console's
C compiler.

## Why this exists

Vircon32 C doesn't support variadic functions — you can't write a function
that takes a variable number of arguments the way standard `printf(fmt, ...)`
does. So instead of one function, this library provides fixed-arity families:

```c
printf1( x, y, fmt, a0 )                                  // 1 substitution
printf2( x, y, fmt, a0, a1 )                               // 2 substitutions
...
printf8( x, y, fmt, a0, a1, a2, a3, a4, a5, a6, a7 )        // 8 substitutions

sprintf1( dest, fmt, a0 )
sprintf2( dest, fmt, a0, a1 )
...
sprintf8( dest, fmt, a0, a1, a2, a3, a4, a5, a6, a7 )
```

The number in the name is how many `%`-substitutions the call needs. Pick
the variant that matches your format string.

- **`printfN`** formats the string and draws it directly to the screen at
  `(x, y)`, using `print_at()` under the hood — think "printf that already
  knows where the console output goes."
- **`sprintfN`** formats the string into a buffer you provide (`dest`),
  exactly like standard `sprintf()`. Nothing is drawn to screen.

## Installation

Drop `printf.h` into your project and include it after the standard headers
it depends on:

```c
#include "video.h"     // needed if you use printfN (print_at)
#include "string.h"     // pulled in by printf.h, but fine to include yourself too
#include "printf.h"
```

`printf.h` itself includes `string.h`, `misc.h`, and `video.h`, so you don't
strictly need to include those yourself unless you use their functions
elsewhere in your file.

## Quick start

```c
#include "video.h"
#include "printf.h"

void main()
{
    int   [32] string_data;
    int        player_score = 175;
    float      health_pct   = 87.413;

    clear_screen( color_black );

    printf1( 10, 10, "%3s", printf_sarg( "hi" ) );
    printf1( 10, 30, "score: %05d", player_score );
    printf2( 10, 50, "%-8s: %3.1f%%", printf_sarg( "health" ), printf_farg( health_pct ) );

    sprintf3( string_data, "%.4d-%.2d-%.2d", 2026, 8, 24 );
    printf1( 10, 70, "%s", printf_sarg( string_data ) );

    printf1( 10, 90, "score (hex): 0x%.8X", player_score );

    end_frame();
}
```

## The one rule you need to remember: wrap non-int arguments

Every substitution slot (`a0`, `a1`, ... `a7`) is declared as a plain `int`.
Vircon32 C won't implicitly convert a string pointer or a `float` into an
`int` argument — you'll get a compiler error like:

```
error: cannot assign int* to const-qualified int
```

So, two helper functions exist to pack other types into an `int` slot:

| Your value is a...     | Wrap it with...          |
| ----------------------- | ------------------------- |
| `int`                   | nothing — pass it directly |
| string (`int*`, string literal, `int [N]` array) | `printf_sarg( ... )` |
| `float`                  | `printf_farg( ... )` |

```c
printf1( x, y, "%s",   printf_sarg( my_string ) );
printf1( x, y, "%f",   printf_farg( my_float ) );
printf1( x, y, "%d",   my_int );                    // no wrapper needed
```

Forget the wrapper and the compiler will tell you immediately (it's a
compile error, not a silent bug) — just add the matching `printf_sarg()` /
`printf_farg()` call around the offending argument.

The `fmt` argument and `sprintfN`'s `dest` argument are declared `int*`
directly, so string literals and `int [N]` arrays pass into *those* without
any wrapping — it's only the generic `argN` slots that need it.

## Format specifier reference

```
%[flags][width][.precision]conversion
```

**Conversions**

| Conversion | Meaning                                  |
| ---------- | ----------------------------------------- |
| `%d`, `%i` | signed decimal integer                    |
| `%u`       | decimal integer (see limitation below)    |
| `%x`       | hexadecimal, lowercase                    |
| `%X`       | hexadecimal, uppercase                    |
| `%o`       | octal                                     |
| `%c`       | single character                          |
| `%s`       | string                                    |
| `%f`       | floating point                            |
| `%%`       | literal `%`                               |

**Flags**

| Flag | Effect                                                        |
| ---- | -------------------------------------------------------------- |
| `-`  | left-align within the field width (default is right-align)    |
| `0`  | pad with `0` instead of spaces (numeric conversions only)     |
| `+`  | force a leading `+` on non-negative numbers                   |

**Width** — a plain number, e.g. `%8d`, sets the *minimum* field width. The
value is padded (with spaces, or `0` if the `0` flag is set) to reach it. It
never truncates a longer value.

**Precision** — `.` followed by a number, meaning depends on the conversion:

| Conversion | `.N` means                                            |
| ---------- | ------------------------------------------------------- |
| `%d %i %u %x %X %o` | minimum number of digits — zero-pads the *value itself*, independent of the `0` width flag |
| `%s`       | maximum number of characters printed (truncates longer strings) |
| `%f`       | number of digits after the decimal point               |

**Examples**

```c
printf1( x, y, "%05d",  42 );        // "00042"   (width + zero flag)
printf1( x, y, "%.5d",  42 );        // "00042"   (precision - same result, different mechanism)
printf1( x, y, "%.2d",   8 );        // "08"
printf1( x, y, "%.8X", 175 );        // "000000AF"
printf1( x, y, "%-8s|", printf_sarg("hi") );   // "hi      |"
printf1( x, y, "%.3s",  printf_sarg("hello") ); // "hel"
printf1( x, y, "%3.1f", printf_farg(87.413) );  // "87.4"
printf1( x, y, "100%%" );            // "100%"
```

## Known limitations

Being upfront about what this does *not* do, so nothing is a surprise later:

- **`%u` behaves like `%d`.** Vircon32 doesn't expose unsigned arithmetic
  operations, so a negative bit pattern isn't reinterpreted as a large
  positive number the way real C's `%u` would. If you need that, treat it
  as a documented gap.
- **`%f` precision truncates rather than rounds.** The last kept decimal
  digit is not rounded up — `%.1f` on `1.99` gives `1.9`, not `2.0`. This
  follows from `ftoa()`'s own (fixed) output being edited after the fact,
  not from a proper rounding pass.
- **No `%*d`** — width/precision can't be pulled from an argument, only
  written literally in the format string.
- **No positional arguments** (`%1$s` and friends).
- **Field buffer is fixed-size** (`PF_FIELD_BUFFER_SIZE`, default 64 ints).
  A single formatted value longer than that — e.g. `%s` with a very large
  precision, or an absurdly large width — will overrun it. Bump the
  `#define` at the top of `printf.h` if you need more headroom.
- **Malformed format strings aren't validated.** An unknown conversion
  character (e.g. `%q`) is echoed back literally (`%q`) rather than causing
  undefined behavior, but there's no general error-reporting mechanism.

## Configuration

Two constants near the top of `printf.h` control buffer sizes:

```c
#define PF_FIELD_BUFFER_SIZE 64    // scratch space for a single formatted value
#define PF_LINE_BUFFER_SIZE  256   // scratch space used internally by printfN
```

`PF_LINE_BUFFER_SIZE` only matters for `printfN` (it's the buffer that gets
built before being handed to `print_at()`). `sprintfN` writes directly into
your own `dest` buffer, so its size is entirely up to you at the call site —
just like standard `sprintf()`, there's no bounds checking, so make sure
`dest` is large enough for the result.

## How it works internally (for the curious)

- All 16 entry points (`printf1..8`, `sprintf1..8`) are thin wrappers: they
  pack their fixed number of arguments into a local `int` array and hand it,
  along with the format string, to a single shared engine function,
  `__pf_format()`. That's the only place the actual parsing/formatting logic
  lives.
- Since Vircon32 C won't implicitly (or even explicitly, via cast) convert
  between `int` and pointer/`float` types, `printf_sarg()` and
  `printf_farg()` — and their internal inverses, `__pf_bits_to_ptr()` and
  `__pf_bits_to_float()` — use `memcpy()` to reinterpret the raw bits of a
  pointer or float as an `int`, and back again. Vircon32 is word-addressed,
  so a string pointer, a float, and an int are all one word wide, making
  this a safe, non-lossy round trip. This is why every non-`int` argument
  needs an explicit wrapper call — there's no way around it given the
  compiler's type rules, so the library leans into it deliberately instead
  of fighting it.

## A note on Vircon32 C syntax quirks that affected this library's design

- Array **declarations** put the size in brackets *before* the variable
  name — `int [32] buffer;` — while array **accesses** look normal —
  `buffer[3]`. If you're used to standard C's `int buffer[32];`, this is
  the one to watch for.
- There is no `char` type. Characters are `int`, and strings are
  null-terminated `int*` (or `int [N]` arrays, which decay to `int*` when
  passed to a parameter declared that way).
- `int` and pointer/`float` types are strictly separate — no implicit
  conversion, and (as far as this library's development established)
  no direct cast either. `memcpy()`-based bit reinterpretation, as used
  by `printf_sarg()`/`printf_farg()` above, is the reliable way to move a
  value between these type "lanes" when you genuinely need to.
