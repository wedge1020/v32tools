**v32tools** is one place to build, test, install and uninstall the Vircon32 community tools: the assembler and linker (v32as), the C compiler and C library (v32cc), the C++-subset transpiler (v32cxx), the Lua-subset compiler (v32lua), the assembly optimizer (v32opt), the text debugger/simulator (v32sim) and the v32io/v32dev libraries (v32io). It links to each tool as a git submodule and drives each tool's own build, through either a `Makefile` or CMake, on Linux, BSD, macOS and Windows (MSYS2/MinGW).

## What's in this release

- **`v32tools-@STAMP@-src.zip`**: v32tools together with the sources of every tool at the commits listed below, for building everything without git.
- **Prebuilt binaries** are published by each tool: follow the links in the table below, and unpack the zips for your system into one `v32tools` folder (`/usr/local/Vircon32/v32tools`, or `C:\Program Files\Vircon32\v32tools` on Windows).
- Every tool's documentation is available in English, Spanish and French (README, user manual and manual pages; `DEST_LANG=en|es|fr` picks the installed language).

## Quick start

```sh
unzip v32tools-@STAMP@-src.zip && cd v32tools
make doctor            # what does this host lack?
make build             # every tool (TOOLS="v32as v32opt" for some)
make test
sudo make sysinstall   # or: make install (per user)
```

With CMake: `cmake -S . -B build && cmake --build build && cmake --install build`. If the host has no `flex`, the vendored flex 2.6.4 is built automatically. See [docs/USAGE.md](https://github.com/wedge1020/v32tools/blob/main/docs/USAGE.md) and [docs/PLATFORMS.md](https://github.com/wedge1020/v32tools/blob/main/docs/PLATFORMS.md). Check downloads against `SHA256SUMS.txt`.
