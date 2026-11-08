#!/bin/sh
# v32t-flex.sh -- build the vendored flex (deps/flex-*.tar.gz) into a private prefix.
#
# Used when the host has no flex: v32as and v32sim need it to generate their
# lexers. Nothing is installed system-wide; the result lives in
#   $V32T_DEPS_PREFIX   (default: <repo>/build/deps)
# and scripts/v32t-run.sh / the Makefile / the CMake driver put its bin/ first on
# the PATH when it exists.
#
# Needs: a C compiler, make, m4 (flex runs m4 at run time) and, on Windows, an
# MSYS2 shell (flex is an autotools package).
#
#   scripts/v32t-flex.sh           build and install if not already there
#   scripts/v32t-flex.sh --force   rebuild
set -eu

ROOT=$(cd "$(dirname "$0")/.." && pwd)
. "$ROOT/scripts/lib.sh"

PREFIX=${V32T_DEPS_PREFIX:-$ROOT/build/deps}
TARBALL=$(ls "$ROOT"/deps/flex-*.tar.gz 2>/dev/null | sort | tail -n 1 || true)
force=0
[ "${1:-}" = "--force" ] && force=1

[ -n "$TARBALL" ] || die "no deps/flex-*.tar.gz to build"

if [ $force -eq 0 ] && { [ -x "$PREFIX/bin/flex" ] || [ -x "$PREFIX/bin/flex.exe" ]; }; then
    info "flex: already built in $PREFIX"
    exit 0
fi

have_cc  || die "flex: no C compiler found"
have m4  || die "flex: m4 not found (flex needs it at run time); install it first"
MAKE=${MAKE:-make}
have "$MAKE" || MAKE=gmake
have "$MAKE" || die "flex: make not found"

# Do not leak the parent make's flags/variable overrides into flex's own build.
unset MAKEFLAGS MFLAGS MAKEOVERRIDES

work=$ROOT/build/deps-src
rm -rf "$work"
mkdir -p "$work" "$PREFIX"
info "flex: unpacking $(basename "$TARBALL")"
gzip -dc "$TARBALL" | tar xf - -C "$work"
srcdir=$(find "$work" -mindepth 1 -maxdepth 1 -type d | head -n 1)
[ -n "$srcdir" ] || die "flex: archive is empty"

cd "$srcdir"
info "flex: configure (prefix $PREFIX)"
./configure --prefix="$PREFIX" --disable-nls --disable-shared \
    ${V32T_FLEX_CONFIGURE_ARGS:-} >"$work/configure.log" 2>&1 \
    || { tail -n 30 "$work/configure.log" >&2; die "flex: configure failed (log: $work/configure.log)"; }
info "flex: build"
"$MAKE" ${V32T_JOBS:+-j"$V32T_JOBS"} >"$work/make.log" 2>&1 \
    || { tail -n 30 "$work/make.log" >&2; die "flex: build failed (log: $work/make.log)"; }
"$MAKE" install >"$work/install.log" 2>&1 \
    || { tail -n 30 "$work/install.log" >&2; die "flex: install failed (log: $work/install.log)"; }
rm -rf "$work"
info "flex: installed in $PREFIX/bin"
