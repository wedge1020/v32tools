# IP / IR / IV experiment

Checks the `MOV ..., IP|IR|IV` expansions of v32asc on a real Vircon32 (emulator).

    ./build.sh            # -> build/experiment.vbin  (CART program ROM)

Pack `experiment.vbin` into a cartridge with the official RomPacker (it needs a ROM
description XML; no textures or sounds are required), then run it.

## What you will see (full-screen colors)

| order | color | meaning |
|-------|-------|---------|
| 1 | blue | start |
| 2-7 | black, then **green** (pass) or **red** (fail), six times | tests 0-5 below |
| 8 | black, then **cyan** or **magenta** | ROM probe |
| 9 | white | done |

Results are also left in RAM words `0x800`-`0x806` for anyone with a memory viewer.

## Tests (payload.asm, run from RAM at 0x1000)

| n | test | pass means |
|---|------|------------|
| 0 | `MOV Rn, IP` vs the return address `CALL` really pushes | assembler's IP value is right, and code runs from RAM |
| 1 | `MOV Rn, IR` vs the instruction word actually in memory | IR value is right |
| 2 | `MOV IP, _label` | jumped over the poison instruction |
| 3 | `MOV IP, Rn` | same, via register |
| 4 | `MOV IR, Rn` | the *next* instruction was replaced at run time (`MOV R7,111` became `MOV R8,111`) |
| 5 | `MOV IV, Rn` | the *next* instruction's immediate was replaced (111 became 222) |

The payload is assembled with `v32asc --origin 0x1000`, so every address the assembler
bakes in is the RAM address the code actually runs at. The cartridge copies it with `MOVS`.

## ROM probe (last color)

The cartridge itself runs `MOV IV, R9` against its own ROM. **Cyan**: the write was ignored,
ROM is read-only, so IR/IV patching needs RAM (what the assembler warns about).
**Magenta**: ROM took the write. If the emulator stops or errors at this point, that is
also an answer: it means writes to ROM are rejected loudly.

## Caveats

- I could not run this on a real emulator here. The program was checked in a small CPU model
  I wrote from the spec, which predicts: six greens, then cyan (and magenta if ROM is writable).
  Disagreement between that and your emulator is exactly what this experiment is for.
- Assumes execution from RAM is allowed and the BIOS leaves a usable stack for `CALL`.
