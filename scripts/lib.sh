# lib.sh -- shared helpers for the v32tools scripts (POSIX sh; source it, do not run it)
#
# Needs $ROOT (the repository root) to be set by the caller.

MANIFEST=${V32T_MANIFEST:-$ROOT/tools.list}

die()  { printf 'v32tools: error: %s\n' "$*" >&2; exit 1; }
warn() { printf 'v32tools: warning: %s\n' "$*" >&2; }
info() { printf '%s\n' "$*"; }
have() { command -v "$1" >/dev/null 2>&1; }

# manifest_tools: the tool names, in build order
manifest_tools() {
    awk '!/^[ \t]*(#|$)/ { print $1 }' "$MANIFEST"
}

# manifest_field NAME N: column N (1-based) of NAME's row; empty if there is no such tool
manifest_field() {
    awk -v t="$1" -v n="$2" '!/^[ \t]*#/ && $1 == t { print $n; exit }' "$MANIFEST"
}

# manifest_desc NAME: the description column (everything after the 4th)
manifest_desc() {
    awk -v t="$1" '!/^[ \t]*#/ && $1 == t { $1 = $2 = $3 = $4 = ""; sub(/^ +/, ""); print; exit }' "$MANIFEST"
}

# in_list ITEM COMMA,LIST
in_list() {
    case ",$2," in *",$1,"*) return 0 ;; esac
    return 1
}

# tool_present NAME: does tools/NAME hold a buildable tree?
tool_present() {
    [ -f "$ROOT/tools/$1/Makefile" ] || [ -f "$ROOT/tools/$1/CMakeLists.txt" ]
}

# have_cc: any C compiler
have_cc() { have cc || have gcc || have clang || have "${CC:-cc}"; }
