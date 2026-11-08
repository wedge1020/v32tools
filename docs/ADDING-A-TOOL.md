# Adding or changing tools

## A new tool

```sh
scripts/add-tool.sh NAME GIT-URL [BACKENDS [HOSTDEPS [DESCRIPTION...]]]
# e.g.
scripts/add-tool.sh v32gfx https://github.com/you/v32gfx.git make cc "Graphics converter"
git add tools.list .gitmodules tools/v32gfx && git commit -m "Add v32gfx"
```

This appends a line to `tools.list` and runs `git submodule add`. By hand: add the line, then
`make bootstrap TOOLS=NAME`. **Order matters**: tools are processed top to bottom, so place a
tool after anything it needs built first (build order is also uninstall order, not reversed).

`tools.list` columns: `name backends hostdeps url description…`

* `backends`: `make`, `cmake` or `make,cmake`: informational/listing; the front ends look at which of
  `Makefile` / `CMakeLists.txt` actually exist in the tool.
* `hostdeps`: programs needed *to build* (`cc` = any C compiler; `-` = none). They are checked before a
  build/test/install; `flex` triggers the vendored build when missing. Test-only needs belong in docs.

## What a tool must provide

For the Makefile backend, any of these targets (missing ones are skipped with a note, except `all`):
`all`, `debug`, `clean`, `distclean`, `test` (or `check`), `install`, `uninstall`, `sysinstall`,
`sysuninstall`. Honor `PREFIX` and `DESTDIR` (with `?=`) in `sysinstall`: the front ends refuse a staged
install for a Makefile that never mentions them.

For the CMake backend: a normal `install()` and, for `uninstall`, an `uninstall` target (the existing
tools already ship `cmake/cmake_uninstall.cmake.in`; copy that pattern). `BUILD_TESTING`/`ctest` for `test`.

## Giving a Make-only tool a CMake build (v32as, v32cc, v32sim)

Nothing to change here. When `tools/v32as/CMakeLists.txt` appears (after you update the submodule),
`BACKEND=auto|cmake` (Make) or the default `V32T_BACKEND=auto` (CMake) picks it up, and
`make v32as BACKEND=make` still builds with the Makefile. Update the `backends` column to
`make,cmake` so `make list` shows it. A good template is v32opt (smallest) or v32lua (has the
flex/bison regeneration option you will want for v32as/v32sim): same options naming
(`V32X_STATIC_MINGW`, `V32X_REGENERATE_PARSER`), the same `GNUInstallDirs` layout, the Windows prefix
tweak, and `cmake/cmake_uninstall.cmake.in`.

## A tool you only have as an archive

`make import ARCHIVE=path/v32foo-project.zip` (tool must be listed). It is a plain directory afterwards;
later you can replace it with a submodule: `rm -rf tools/v32foo && make bootstrap TOOLS=v32foo`.

## Updating tools

`make update` moves submodules to their remote tips; commit the new pointers. To pin a release:
`git -C tools/v32opt checkout <tag>` then `git add tools/v32opt`.

## The vendored flex

`deps/flex-2.6.4.tar.gz` is the upstream *release* tarball (https://github.com/westes/flex/releases):
it carries a generated `configure`, which a git checkout of flex does not (that needs autoconf,
automake, gettext, libtool, help2man…). To upgrade, drop a newer `flex-X.Y.Z.tar.gz` into `deps/` and
remove the old one; `scripts/v32t-flex.sh` uses the newest. It is built only when no `flex` is on the
`PATH`, into `build/deps` (never system-wide).
