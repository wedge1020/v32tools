# v32as — Vircon32 assembler in C (flex + bison)

A C port of  the bash `v32as`. Same job (assemble  Vircon32 assembly to a
VBIN, with a listing/verbose/step mode  and an interactive REPL), but the
lexer  is flex,  the parser  is bison,  and symbols/fixups  live in  hash
tables instead of bash arrays and `bc`.

```
    make          # builds ../bin/v32as  (needs flex, bison, cc; readline optional)
    make test     # differential/golden tests (including .debug files) (sh + od + cmp only)
    make install  # copies to ~/bin
```

## Usage

```
    v32as [-b|-c|-n] [-B] [-o FILE] [-s] [-v] [FILE...|-]
```

- `-b/--bios`, `-c/--cart` set the base address (0x10000000 / 0x20000000); `-n/--none` = base 0
- `-B/--binary` writes a VBIN (requires --bios/--cart); `-o` names the output
- `-g/--debug[=MODE]` also writes `FILE.debug` (CSV `address,file,line[,label]`, one line per instruction) next to the `-o FILE` binary. Plain `-g` is byte-identical to the official assembler's `-g program` (absolute word addresses, e.g. `0x20000000`). `-g vbin` / `-g v32` give real byte offsets in the .vbin (0x0C header) or .v32 (0x8C header) file; the official tool's versions of those two print wrapped-around values. A source file literally named `program`, `vbin` or `v32` must be written `./program`.
- `-s/--step` pauses per line, `-v/--verbose` shows the encoded bit fields
- no files and a terminal on stdin starts the REPL: `.help .labels .dump .reset .quit`
- `-` reads stdin; the legacy bare words (`bios`, `cart`, ...) still work

## Verification

Output is  byte-identical to the  official Vircon32 assembler  for: every
opcode in every  legal operand form, all I/O ports  and port values (BIOS
and CART), and all data directives. `tests/golden/*.hex` were produced by
the official assembler.

## Differences from the bash version

- The VBIN header carries the real word count (bash hard-coded 155).
- Numeric ports in `IN`/`OUT` work (bash bug); unknown/truncated ports warn.
- Integer literals given to float-class ops are converted to float; ATAN2/POW take registers only (as official).
- `[R+0]` is encoded as `[R]` (as official).
- Data directives: `integer`, `float`, `pointer`, `string`, `zstring`, `datafile`.
  One word per element; `string` is one word per character plus a terminating 0.
  `zstring` is byte-packed: four characters per word (first character in the low byte),
  zero-terminated, last word zero-padded. (`zstring` is not in the official assembler;
  its golden file is hand-verified, not official output.)
  `datafile` reads little-endian words, relative to the source file's directory first.
- Added: `true/false`, character literals, `0b`, octal, `#dec`, `h` suffix numbers;
  labels and a statement may share a line; mnemonics/registers are case-insensitive.

## Internal registers (extension)

`IP`,  `IR`, `IV`  are not  CPU-addressable, but  `MOV` accepts  them and
expands to exactly one real instruction, using no hidden scratch register
(A = address of the expansion):

| source               | expands to              | meaning                                   |
|----------------------|-------------------------|-------------------------------------------|
| `MOV Rn, IP`         | `MOV Rn, A+2`           | address of the next instruction           |
| `MOV Rn, IR`         | `MOV Rn, <this word>`   | encoding of the instruction being executed|
| `MOV IP, Rn / addr`  | `JMP Rn / addr`         | jump                                      |
| `MOV IR, Rn`         | `MOV [A+2], Rn`         | overwrite the next instruction word       |
| `MOV IV, Rn`         | `MOV [A+3], Rn`         | overwrite the next instruction's immediate|

`MOV Rn, IV` is rejected (self-referential); IR/IV cannot be written from
an immediate. The IR/IV writes patch code  at run time, so that code must
run from RAM (the assembler warns in --bios/--cart mode). `MOV Rn, IP` in
--none mode is relative to address 0 (warns).
