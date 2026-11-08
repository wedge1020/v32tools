#!/bin/sh
# v32t-doctor.sh -- report host programs and the state of every tool
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
. "$ROOT/scripts/lib.sh"

FLEXDIR=${V32T_DEPS_PREFIX:-$ROOT/build/deps}
[ -x "$FLEXDIR/bin/flex" ] && PATH=$FLEXDIR/bin:$PATH

chk() {  # chk PROGRAM [note]
    if have "$1"; then printf '  %-8s ok   %s\n' "$1" "$(command -v "$1")"
    else printf '  %-8s --   %s\n' "$1" "${2:-}"; fi
}

echo "Host programs:"
if have_cc; then printf '  %-8s ok   %s\n' cc "$(command -v cc || command -v gcc || command -v clang)"; else printf '  %-8s --   needed by every tool\n' cc; fi
chk make    "GNU make is required (BSD: install gmake)"
chk cmake   "needed only for the CMake front end / CMake backend"
chk git     "needed for bootstrap/update"
chk bison   "needed by v32as, v32sim"
chk flex    "needed by v32as, v32sim; 'make flex' builds the vendored one"
chk m4      "needed to build the vendored flex"
chk python3 "needed by some tools' test suites"
chk unzip   "needed by 'make import' for .zip files"
echo
echo "Tools:"
printf '  %-8s %-9s %-11s %s\n' TOOL STATE BUILDSYS MISSING
for t in $(manifest_tools); do
    if tool_present "$t"; then
        state=ok; bs=
        [ -f "$ROOT/tools/$t/Makefile" ] && bs=make
        [ -f "$ROOT/tools/$t/CMakeLists.txt" ] && bs=${bs:+$bs,}cmake
    else state=absent; bs=-; fi
    miss=
    for d in $(manifest_field "$t" 3 | tr ',' ' '); do
        case $d in -) ;; cc) have_cc || miss="$miss cc" ;; *) have "$d" || miss="$miss $d" ;; esac
    done
    printf '  %-8s %-9s %-11s %s\n' "$t" "$state" "$bs" "${miss# }"
done
