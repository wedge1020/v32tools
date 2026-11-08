#!/bin/sh
# v32t-run.sh TOOL ACTION -- run one action for one tool, with the tool's own build system.
#
# ACTION: build debug clean distclean test install uninstall sysinstall sysuninstall
#
# Called by the top-level Makefile; usable by hand. Environment:
#   V32T_BACKEND      make (default) | cmake | auto   (cmake/auto: CMake where the tool has CMakeLists.txt)
#   V32T_BUILD_TYPE   CMake build type (default Release)
#   V32T_BUILD_DIR    where CMake-backend build trees go (default <repo>/build)
#   V32T_USER_PREFIX  prefix for the CMake backend's user-level `install` (default $HOME)
#   V32T_JOBS         parallel jobs for the tool's build
#   V32T_FLEX         auto (default) | off : build the vendored flex when the host has none
#   V32T_MAKE_ARGS / V32T_CMAKE_ARGS   extra arguments for the tool's make / cmake configure
#   PREFIX, DESTDIR, DEST_LANG, COMPONENTS, ...  pass straight through to the tool
set -eu

ROOT=$(cd "$(dirname "$0")/.." && pwd)
. "$ROOT/scripts/lib.sh"

[ $# -eq 2 ] || die "usage: v32t-run.sh TOOL ACTION"
tool=$1
action=$2

hostdeps=$(manifest_field "$tool" 3)
[ -n "$hostdeps" ] || die "'$tool' is not in tools.list"
src=$ROOT/tools/$tool
tool_present "$tool" || die "tools/$tool is empty -- run 'make bootstrap' (or 'make import ARCHIVE=...')"

case $action in
    build|debug|clean|distclean|test|install|uninstall|sysinstall|sysuninstall) ;;
    *) die "unknown action '$action'" ;;
esac

# ---- backend -------------------------------------------------------------------------
backend=make
case ${V32T_BACKEND:-make} in
    make) ;;
    cmake|auto)
        if [ -f "$src/CMakeLists.txt" ]; then backend=cmake
        elif [ "${V32T_BACKEND:-}" = cmake ]; then
            warn "$tool has no CMakeLists.txt; using its Makefile"
        fi ;;
    *) die "V32T_BACKEND must be make, cmake or auto" ;;
esac
if [ "$backend" = make ] && [ ! -f "$src/Makefile" ]; then
    [ -f "$src/CMakeLists.txt" ] || die "$tool: no build system"
    backend=cmake
fi

# ---- host dependencies ---------------------------------------------------------------
# flex first: prefer the private build from v32t-flex.sh when it exists
FLEXDIR=${V32T_DEPS_PREFIX:-$ROOT/build/deps}
if [ -x "$FLEXDIR/bin/flex" ] || [ -x "$FLEXDIR/bin/flex.exe" ]; then
    PATH=$FLEXDIR/bin:$PATH
    export PATH
fi

case $action in
    build|debug|test|install|sysinstall)
        missing=
        for d in $(printf '%s' "$hostdeps" | tr ',' ' '); do
            case $d in
                -) ;;
                cc) have_cc || missing="$missing cc" ;;
                flex)
                    if ! have flex; then
                        if [ "${V32T_FLEX:-auto}" = off ]; then missing="$missing flex"
                        else
                            info "==> $tool: no flex on this host; building the vendored one"
                            "$ROOT/scripts/v32t-flex.sh" || die "could not build flex"
                            PATH=$FLEXDIR/bin:$PATH; export PATH
                        fi
                    fi ;;
                *) have "$d" || missing="$missing $d" ;;
            esac
        done
        [ -z "$missing" ] || die "$tool needs:$missing (install it, then retry; see docs/PLATFORMS.md)" ;;
esac

info "==> $tool: $action [$backend]"

jobs_flag=
[ -n "${V32T_JOBS:-}" ] && jobs_flag=-j$V32T_JOBS

# ======================================================================================
# Makefile backend
# ======================================================================================
make_target_exists() {
    grep -Eq "^$1[[:space:]]*:([^=]|\$)" "$src/Makefile"
}

run_make() {
    MK=${MAKE:-make}
    have "$MK" || MK=gmake
    # shellcheck disable=SC2086
    "$MK" -C "$src" $jobs_flag ${V32T_MAKE_ARGS:-} "$@"
}

# A tool whose Makefile never mentions DESTDIR / PREFIX would ignore them and write to its
# fixed location -- on the real system, in the middle of what looked like a staged install.
guard_make_install() {
    case $action in sysinstall|sysuninstall) ;; *) return 0 ;; esac
    [ "${V32T_FORCE:-0}" = 1 ] && return 0
    for v in DESTDIR PREFIX; do
        eval "val=\${$v:-}"
        if [ -n "$val" ] && ! grep -q "$v" "$src/Makefile"; then
            die "$tool: its Makefile ignores $v and would install to its fixed location. Use BACKEND=cmake for this tool, or set V32T_FORCE=1 to go ahead anyway."
        fi
    done
}

do_make() {
    guard_make_install
    case $action in
        build)       run_make all ;;
        distclean)   if make_target_exists distclean; then run_make distclean; else run_make clean; fi ;;
        test)
            if   make_target_exists test;  then run_make test
            elif make_target_exists check; then run_make check
            else info "    (no test target)"; fi ;;
        debug|clean|install|uninstall|sysinstall|sysuninstall)
            if make_target_exists "$action"; then run_make "$action"
            else info "    (no '$action' target)"; fi ;;
    esac
}

# ======================================================================================
# CMake backend
# ======================================================================================
do_cmake() {
    CM=${CMAKE_COMMAND:-cmake}
    CT=${CTEST_COMMAND:-ctest}
    bld=${V32T_BUILD_DIR:-$ROOT/build}/cmake/$tool
    btype=${V32T_BUILD_TYPE:-Release}
    gen=
    [ -n "${V32T_CMAKE_GENERATOR:-}" ] && gen="-G $V32T_CMAKE_GENERATOR"

    configure() {
        # PREFIX (if the user gave one) also becomes CMAKE_INSTALL_PREFIX so the
        # configure-time summary is accurate; otherwise the tool's own default stands.
        # shellcheck disable=SC2086
        "$CM" -S "$src" -B "$bld" $gen -DCMAKE_BUILD_TYPE="$btype" \
            ${PREFIX:+-DCMAKE_INSTALL_PREFIX="$PREFIX"} ${V32T_CMAKE_ARGS:-} || die "$tool: configure failed"
    }
    build() {
        [ -f "$bld/CMakeCache.txt" ] || configure
        # shellcheck disable=SC2086
        "$CM" --build "$bld" --config "$btype" $jobs_flag || die "$tool: build failed"
    }
    # --build's -j needs CMake >= 3.12 and takes "-j N" or "-jN"; fine on anything current.

    case $action in
        build|debug)
            if [ "$action" = debug ]; then btype=Debug; fi
            build ;;
        clean)
            if [ -f "$bld/CMakeCache.txt" ]; then "$CM" --build "$bld" --target clean
            else info "    (nothing built)"; fi ;;
        distclean)  rm -rf "$bld" ;;
        test)
            build
            ( cd "$bld" && "$CT" --output-on-failure -C "$btype" ) ;;
        install)
            build
            "$CM" --install "$bld" --config "$btype" --prefix "${V32T_USER_PREFIX:-${HOME:?HOME is not set}}" ;;
        sysinstall)
            build
            if [ -n "${PREFIX:-}" ]; then
                "$CM" --install "$bld" --config "$btype" --prefix "$PREFIX"
            else
                "$CM" --install "$bld" --config "$btype"
            fi ;;
        uninstall|sysuninstall)
            [ -f "$bld/CMakeCache.txt" ] || die "$tool: no build tree at $bld to uninstall from (an uninstall replays install_manifest.txt of the last install)"
            "$CM" --build "$bld" --config "$btype" --target uninstall ;;
    esac
}

case $backend in
    make)  do_make ;;
    cmake) do_cmake ;;
esac
