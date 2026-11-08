# v32tools

A meta repository  for the Vircon32 community tools. It  does not contain
the tools' code; it **links to  them** (as git submodules under `tools/`)
and gives  you one place  to **build,  test, install and  uninstall** any
selection  of  them, on  Linux,  BSD,  macOS and  Windows  (MSYS2/MinGW),
through either a `Makefile` or `cmake`.

| tool   | what                      | native build    | needs to build            |
|--------|---------------------------|-----------------|---------------------------|
| v32as  | assembler + linker        | Makefile        | cc, flex, bison           |
| v32opt | assembly optimizer        | Makefile, CMake | cc                        |
| v32sim | text debugger / simulator | Makefile        | cc, flex, bison, readline |
| v32cc  | C compiler + C library    | Makefile        | cc                        |
| v32lua | Lua-subset translator     | Makefile, CMake | cc                        |
| v32cxx | C++-subset transpiler     | Makefile, CMake | cc                        |

The  list (and  the order  tools  are processed  in) lives  in one  file,
[`tools.list`](tools.list), read by both front ends.

## Quick start

```sh
$ git clone git@github.com:wedge1020/v32tools.git v32tools
$ cd v32tools
$ make bootstrap            # git submodule add/init for every tool in tools.list
$ make doctor               # what is missing on this host?
$ make                      # = make help
$ make build                # build everything
$ make test                 # run the tools' own test targets
$ make install              # per-user install  (~/bin, ...)
$ sudo make sysinstall      # system-wide install (/usr/local)
```

The same with CMake:

```sh
$ cmake -S . -B build
$ cmake --build build                         # build everything
$ cmake --build build --target v32t-test
$ cmake --build build --target v32t-install   # per-user
$ cmake --install build                       # system-wide  (add --prefix DIR)
```

Select  tools:   `make  build   TOOLS="v32as  v32opt"`,   `make  v32opt`,
`make  v32opt-clean`, `SKIP=NAME`;  CMake: `-DV32T_TOOLS="v32as;v32opt"`,
`--target v32opt`, `--target v32opt-clean`.

No compiler is  needed to *configure* the meta project;  each tool brings
its own requirements (see [docs/PLATFORMS.md](docs/PLATFORMS.md)). If the
host has no  `flex`, the vendored `deps/flex-2.6.4.tar.gz`  is built into
`build/deps` automatically (needs `m4`), without touching the system.

## Documentation

* [docs/USAGE.md](docs/USAGE.md): every target,  variable and option, for
  both front ends; where each tool installs; per-tool knobs (`DEST_LANG`,
  `COMPONENTS`, ...).

* [docs/PLATFORMS.md](docs/PLATFORMS.md):  Linux,   BSD,  macOS,  Windows
  (MSYS2/MinGW) notes and prerequisites.

* [docs/ADDING-A-TOOL.md](docs/ADDING-A-TOOL.md):    adding    a    tool,
  importing an archive, giving a Make-only tool a CMake build later.

* [docs/DESIGN.md](docs/DESIGN.md): why  submodules, how the  pieces fit,
  known issues found in the tools.

## Layout

```
tools.list      the manifest: tools, build order, host dependencies, git URLs
Makefile        make front end (GNU make)
CMakeLists.txt  CMake front end
cmake/          manifest reader + the CMake driver script (v32t-run.cmake)
scripts/        POSIX sh helpers: v32t-run.sh (the make front end's driver), bootstrap,
                add-tool, import, flex, doctor
deps/           vendored third-party sources (flex 2.6.4 release tarball)
tools/<name>/   the tools (git submodules)
build/          everything this repository builds itself (ignored by git)
```
