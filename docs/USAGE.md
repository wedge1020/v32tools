# Using v32tools

Two front ends, same operations.  Pick whichever you like per invocation;
they can share the checked-out tools (but see "Build trees" below).

## Steps

| step           | meaning                                                                                  |
|----------------|------------------------------------------------------------------------------------------|
| `build`        | build the tool (the tool's `all`)                                                        |
| `debug`        | debug build (tool's `debug` target; CMake backend: `Debug` build type). Skipped by tools without one |
| `test`         | tool's `test` target (or `check`); CMake backend: `ctest`. Some suites need other tools or the Vircon32 DevTools, see each tool's README |
| `clean`        | remove build products                                                                    |
| `distclean`    | `clean` + everything else generated (tool's `distclean` if it has one; CMake backend: remove its build tree) |
| `install`      | **per-user** install                                                                     |
| `uninstall`    | undo `install`                                                                           |
| `sysinstall`   | **system-wide** install (`/usr/local` by default; may need `sudo`)                       |
| `sysuninstall` | undo `sysinstall`                                                                        |

Tools  are processed  one at  a time,  in `tools.list`  order. The  first
failure stops the run (`make -k` keeps going).

## Makefile front end

GNU make required (`gmake` on BSD).

```
make [STEP]                    every selected tool        (default goal: help)
make <tool>                    build one tool
make <tool>-<step>             one step of one tool       make v32as-clean  make v32cc-sysinstall
make list | doctor | status | bootstrap | update | flex | import
```

| variable        | default  | effect |
|-----------------|----------|--------|
| `TOOLS`         | `all`    | which tools; comma or space separated: `TOOLS="v32as v32opt"` |
| `SKIP`          | empty    | tools to leave out |
| `BACKEND`       | `make`   | `make`: each tool's Makefile. `cmake`/`auto`: a tool's `CMakeLists.txt` where it has one (build trees in `build/cmake/<tool>`), its Makefile otherwise |
| `PREFIX`        | tool's own | system install prefix, for `sysinstall`/`sysuninstall` (and CMake builds' `CMAKE_INSTALL_PREFIX`) |
| `DESTDIR`       | empty    | staging root for packaging |
| `USER_PREFIX`   | `$HOME`  | prefix of the per-user `install` for **CMake-backend** tools (so `~/bin`, `~/share/man`, like the Makefiles) |
| `BUILD_TYPE`    | `Release`| CMake backend |
| `JOBS`          | empty    | `-jN` inside each tool. See the note on v32as below |
| `FLEX`          | `auto`   | `off`: never build the vendored flex |
| `V32T_MAKE_ARGS`, `V32T_CMAKE_ARGS`, `V32T_CMAKE_GENERATOR` | empty | extra arguments for the tools' make / configure; generator for CMake-built tools |

Any  other   variable  (`DEST_LANG=es`,   `COMPONENTS=both`,  `CC=clang`,
`CFLAGS=...`) reaches  the tools  through the  **environment**, not  as a
command-line override, so the tools' own `?=` defaults honor it and their
internal assignments  are not  clobbered. (Variables whose  tool Makefile
uses a plain `=`  therefore ignore it: that is by  design here; set those
in the tool's own invocation.)

Other commands:

* `make  bootstrap  [TOOLS=...]`:  `git  submodule add`  for  every  tool
  whose  directory  is  empty  (uses  the  URL  column  of  `tools.list`;
  `V32T_URL_<name>=...`  overrides  one,   `V32T_BRANCH_<name>`  picks  a
  branch);  initialises submodules  that are  registered but  not checked
  out; leaves existing directories alone.
 
* `make  update  [TOOLS=...]`:  `git  submodule  update  --init  --remote
  --merge`; commit the moved pointers.

* `make status`: submodule state plus `describe`/branch of each tool.

* `make import ARCHIVE=v32as-project.zip  [TOOL=v32as] [FORCE=1]`: unpack
  `a snapshot (zip, tar.gz) into  tools/<name>/`, for tools you only have
  `as an archive.

* `make  flex`  /  `make  flex-rebuild`: build  the  vendored  flex  into
  `build/deps`.

* `make doctor`: which host programs exist; which tool lacks what.

## CMake front end

```
cmake -S . -B build [options]
cmake --build build [--target T]
cmake --install build [--prefix DIR]       = system install of the selected tools
```

Targets:  `v32t-<step>`  for  all  selected tools  (`v32t-build`  is  the
default  target), `<tool>`  (build),  `<tool>-<step>`, plus  `v32t-flex`,
`v32t-doctor`,  `v32t-list`.  (`install`  itself is  reserved  by  CMake,
hence the  `v32t-` prefix; `v32t-install`  is the *per-user*  one, `cmake
--install` and `v32t-sysinstall` the system one.)

| cache option           | default | effect |
|------------------------|---------|--------|
| `V32T_TOOLS`           | `all`   | `-DV32T_TOOLS="v32as;v32opt"` |
| `V32T_SKIP`            | empty   | tools to leave out |
| `V32T_BACKEND`         | `auto`  | `auto`/`cmake`: CMake where a tool has `CMakeLists.txt`; `make`: always the Makefiles |
| `CMAKE_INSTALL_PREFIX` | tools' defaults | system prefix; if you do not set it, each tool keeps its own platform default (they differ on Windows) |
| `V32T_USER_PREFIX`     | `$HOME` | per-user install prefix for CMake-built tools |
| `V32T_BUILD_TYPE`      | `Release` | |
| `V32T_JOBS`            | empty   | parallelism inside each tool |
| `V32T_FLEX`            | `AUTO`  | `OFF`: never build the vendored flex |
| `V32T_MAKE_ARGS`, `V32T_CMAKE_ARGS` | empty | extra arguments for tool make / tool configure |
| `V32T_CMAKE_GENERATOR` | empty   | generator for CMake-built tools, e.g. `"MinGW Makefiles"`, `Ninja` |

`DESTDIR`  is  honored  from   the  environment:  `DESTDIR=/stage  cmake.
`--install build  --prefix /usr` tools.list` is  a configure dependency,.
`so adding a tool re-configures automatically                           .

## Where things go

| step         | Makefile backend (tool's own targets)                                   | CMake backend |
|--------------|--------------------------------------------------------------------------|---------------|
| `install`    | v32as `~/bin` + `~/man`; v32opt/v32sim/v32lua/v32cxx `~/bin` (v32sim also `~/share/v32sim`); v32cc `~/.local/lib/v32cc` linked from `~/bin`  | `<USER_PREFIX>/bin`, `/share/man`, ... |
| `sysinstall` | `/usr/local/bin`, `/usr/local/share/man`; v32cc, v32lua, v32cxx also put headers/libraries/docs under `<prefix>/Vircon32/v32tools/...` | same layout under `PREFIX` |

Note that the per-user `install` of v32opt, v32lua and v32sim **skips** (and says so) when neither
`~/bin/bin.<arch>` nor `~/bin` exists: `mkdir ~/bin` first.

### Tool-specific knobs

* v32opt, v32lua,  v32cxx: `DEST_LANG=en|es|fr`  (and `all` for  cxx) for
  v32the documentation language.

* v32cc, v32sim: `V32SIM_HOME`, `V32AS`, ...  for their test suites; with
  this layout their  defaults (`../v32sim`, `../v32as/bin/v32as`) resolve
  correctly because the tools are siblings under `tools/`.

### Staged installs and tools that ignore `PREFIX`/`DESTDIR`

Before running a Makefile's `sysinstall`  with `PREFIX` or `DESTDIR` set,
the front ends  check that the tool's Makefile mentions  that variable at
all.  If not  (v32cxx's does  not),  they **refuse**  instead of  writing
outside your staging  root; use `BACKEND=cmake` for that  tool (its CMake
build supports both),  or set `V32T_FORCE=1` to override.  A subtler case
the check cannot see: v32as's  `sysinstall` is hard-wired to `/usr/local`
(it  only honors  `DESTDIR`); use  `BACKEND=cmake`  once it  has a  CMake
build, or `make -C tools/v32as install PREFIX=/your/prefix`.

## Build trees

Makefile-backend tools  build **in their source  tree** (`tools/<n>/bin`,
`build`, `obj`,  ...); each  tool's `.gitignore` keeps  that out  of git.
CMake-backend  tools build  in `build/cmake/<tool>`  (Make front  end) or
`<your build dir>/cmake/<tool>` (CMake front end). A tool built both ways
keeps two independent trees; `distclean`  removes the one for the backend
selected.

## Parallel builds

`JOBS=n` / `V32T_JOBS` is passed to each tool, and `make -jN` on the meta
Makefile reaches the tools too (the  meta Makefile itself still runs them
one  at a  time). **v32as's  Makefile currently  races under  `-j`**: its
parser rule produces `parser.tab.c` and `parser.tab.h` as two independent
targets, so  bison runs twice at  once and corrupts the  output. Build it
serially  (`make  v32as`,  no  `JOBS`)  until that  rule  is  fixed  (see
docs/DESIGN.md).
