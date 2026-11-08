# Design notes

## Why submodules

The tools  are independent repositories  with their own  history, release
rhythm and  (in several  cases) their  own `make  archive`/`cpack` flows.
Submodules keep  each one a first-class  repo, pin the exact  commit that
`v32tools` was  tested with  (`git submodule status`),  and let  you work
inside `tools/<name>` as an ordinary  clone (commit, push, branch). `make
update` moves the pins.

The  alternatives were  rejected: *subtree/vendoring*  duplicates history
and makes  pushing back awkward;  *`ExternalProject`/`FetchContent` only*
would  tie  fetching  to  CMake  and  give  the  Makefile  side  nothing.
Directories populated  some other  way (`make import`)  work identically:
the front ends only look at `tools/<name>/Makefile` and `CMakeLists.txt`.

## One manifest, two front ends

`tools.list`    is    read    by   `Makefile`    (awk),    `scripts/*.sh`
and   `cmake/V32ToolsManifest.cmake`.   It   is   deliberately   a   flat
whitespace-separated file so no front end needs a parser dependency.

## Each tool keeps its own build system

The front ends  never merge the tools into  one build (`add_subdirectory`
would  collide   on  target   names  such   as  `uninstall`/`man`/`test`,
`project()` variables and options). They *drive* each tool's native build
with the same step vocabulary:

* `scripts/v32t-run.sh TOOL  STEP` is  the logic  for the  Makefile front
  end.

* `cmake/v32t-run.cmake` is the  same logic in CMake script  mode, so the
  CMake front end  needs no POSIX shell (except to  build flex) and works
  on Windows with only MinGW + CMake.

* Backend choice  per tool  is by  file presence, so  a tool  gains CMake
  support the moment it ships a `CMakeLists.txt`.

Variables passed on  the meta `make` command line are  *not* forwarded as
command-line overrides (`MAKEOVERRIDES` is cleared): they would overrides
assignments inside the tools' Makefiles.

## Findings in the current tool snapshots

These are in the tools, not in  this repo; none blocks v32tools, but they
shape its behavior:

1. **v32as:      `make       -j`      race.**      `$(BUILD)/parser.tab.c
   $(BUILD)/parser.tab.h:  parser.y` is  a  two-target  rule, which  make
   treats  as  two  independent  rules;   under  `-j`  bison  runs  twice
   concurrently and the output is corrupted (12/12 failures at `-j8` with
   a  clean  tree). Fix:  make  the  header  depend  on the  source  file
   (`$(BUILD)/parser.tab.h: $(BUILD)/parser.tab.c`) with  the recipe only
   on the `.c`,  or use a grouped  target (`&:`, GNU make  ≥ 4.3). Same
   shape in v32sim (`parser.c`/`parser.h`) is worth checking.

2. **v32cxx:  `sysinstall` ignores  `PREFIX` and  `DESTDIR`** (hard-coded
   `/usr/local`; the  Makefile never  mentions them). A  "staged" install
   through its Makefile  lands on the live system. The  front ends refuse
   that case; its CMake build does it right.

3. **v32as:  `sysinstall`   is  `$(MAKE)   install  PREFIX=/usr/local`**:
   `PREFIX=/opt/x make sysinstall` is silently ignored (the inner command
   line wins). `DESTDIR` works.

4. v32opt/v32lua/v32sim  per-user `install`  silently skips  when `~/bin`
   does not exist.

5. v32cc/v32sim/v32as  have no  CMake  build yet  (planned); once  added,
   `BACKEND=cmake`  gives them  consistent `PREFIX`/`DESTDIR`/`uninstall`
   behavior.

## Not done / ideas

* Dependency edges between tools (e.g. v32cc's tests wanting v32as/v32sim
  built first) are expressed only by order in `tools.list`.

* No packaging (`cpack`)  at the meta level; the tools  that have `cpack`
  keep it (`cd build/cmake/<tool> && cpack`).

* `bison` is not  vendored (flex is, because it was  supplied); on a host
  without it, install it.
