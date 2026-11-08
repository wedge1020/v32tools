#!/bin/sh
# add-tool.sh NAME URL [BACKENDS [HOSTDEPS [DESCRIPTION...]]]
#
# Register a new tool: append it to tools.list (at the END of the build order;
# move the line if it must build earlier) and add it as a git submodule.
#   BACKENDS  default make        e.g. make,cmake
#   HOSTDEPS  default cc          e.g. cc,flex,bison  ('-' = none)
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
. "$ROOT/scripts/lib.sh"
cd "$ROOT"

[ $# -ge 2 ] || die "usage: add-tool.sh NAME URL [BACKENDS [HOSTDEPS [DESCRIPTION...]]]"
name=$1 url=$2 backends=${3:-make} hostdeps=${4:-cc}
[ $# -gt 4 ] && { shift 4; desc=$*; } || desc=$name

case $name in *[!A-Za-z0-9_+-]*|'') die "bad tool name '$name'" ;; esac
[ -z "$(manifest_field "$name" 1)" ] || die "'$name' is already in tools.list"

printf '%-9s %-11s %-16s %-45s %s\n' "$name" "$backends" "$hostdeps" "$url" "$desc" >>"$MANIFEST"
info "added $name to tools.list"
"$ROOT/scripts/v32t-bootstrap.sh" "$name"
