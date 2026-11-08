# Platform notes

Prerequisites per tool are in the table in the README; `make doctor` (or `cmake --build build
--target v32t-doctor`) reports what your host lacks.

## Linux

Install a C compiler, GNU make, `git`; `bison`; `flex` (or let v32tools build the vendored one, which
needs `m4`); GNU readline development files for v32sim; `cmake` ≥ 3.16 for the CMake front end
(the tools' own CMake builds ask for ≥ 3.13).

* Debian/Ubuntu: `sudo apt install build-essential git cmake bison flex m4 libreadline-dev`
* Fedora: `sudo dnf install gcc make git cmake bison flex m4 readline-devel`
* Arch: `sudo pacman -S base-devel git cmake bison flex m4 readline`

## FreeBSD / OpenBSD / NetBSD

The Makefiles are GNU make: `pkg install gmake` and run `gmake ...` (the front end uses `$(MAKE)`, so
nothing else changes). Install `bison`, `flex` (BSD's lex is not flex), `cmake`, `git`, `readline`.
The tools' `install -d`/`install -m` usage is portable; man page locations differ by system, so
prefer `PREFIX=` (e.g. `/usr/local`) and check `MANPATH`.

## macOS

Xcode command line tools provide `cc`, `make` (GNU make 3.81, **old**: install `brew install make` and
use `gmake`), `git`. `brew install cmake bison flex readline`; Apple's bison is 2.3, too old for
some grammars; put Homebrew's first on the `PATH`
(`export PATH="$(brew --prefix bison)/bin:$PATH"`). If v32sim cannot find readline, pass
`CFLAGS`/`LDFLAGS`... in the environment as that tool's README describes.

## Windows

Use **MSYS2** (https://www.msys2.org/) with the **MinGW-w64** toolchain, the same environment the
tools themselves document; MSVC is not supported by the tools (they need `getopt_long` etc.).

1. In an MSYS2 *MinGW64* shell:
   `pacman -S git make mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake bison flex m4 mingw-w64-x86_64-readline`
2. Then everything works as on Linux: `make bootstrap && make build` (the Makefile front end needs the
   MSYS2 `sh`, `awk`, `sed` it provides).
3. CMake front end from that shell: `cmake -G "MSYS Makefiles" -S . -B build` (or
   `-DV32T_CMAKE_GENERATOR="MinGW Makefiles"` / `Ninja` for the generator used by the tools' own CMake
   builds). From a plain `cmd`/PowerShell with MinGW and CMake on the `PATH` the CMake front end also
   works for CMake-built tools; Makefile-built tools need `make` (`mingw32-make` is found
   automatically). The vendored flex needs an MSYS2 `sh`; or install flex and put it on the `PATH`.
4. Install locations: the tools' own CMake builds default to `<Program Files>\Vircon32\v32tools\...`
   unless you give `-DCMAKE_INSTALL_PREFIX`; the front ends leave the prefix alone unless you set it.
   `make install`'s `~/bin` is MSYS2's home (`/home/<you>`), not `%USERPROFILE%`.

## Cross-platform caveats

* The per-user `install` differs by backend (see USAGE.md "Where things go").
* Symlinks (v32cc's `~/bin` link, v32cxx's man page aliases) need developer mode or MSYS2 settings on
  Windows (`export MSYS=winsymlinks:nativestrict`).
