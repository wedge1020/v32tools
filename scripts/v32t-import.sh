#!/bin/sh
# v32t-import.sh ARCHIVE [NAME] -- unpack a .zip/.tar.gz/.tgz snapshot into tools/NAME
#
# For tools you only have as an archive (e.g. the output of a tool's `make archive`).
# NAME defaults to the archive name minus ".zip"/".tar.gz" and any "-project..." tail
# (v32as-project.zip -> v32as). It must be listed in tools.list. A single top-level
# directory inside the archive is stripped. An existing non-empty tools/NAME is
# refused unless FORCE=1 (the old contents are then removed first).
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
. "$ROOT/scripts/lib.sh"

[ $# -ge 1 ] || die "usage: v32t-import.sh ARCHIVE [NAME]"
archive=$1
[ -f "$archive" ] || die "$archive: no such file"
archive=$(cd "$(dirname "$archive")" && pwd)/$(basename "$archive")

name=${2:-}
if [ -z "$name" ]; then
    name=$(basename "$archive")
    name=${name%.zip}; name=${name%.tar.gz}; name=${name%.tgz}
    name=${name%%-project*}
fi
[ -n "$(manifest_field "$name" 1)" ] || die "'$name' is not in tools.list (add it first: scripts/add-tool.sh, or pass NAME)"

dest=$ROOT/tools/$name
if [ -n "$(ls -A "$dest" 2>/dev/null || true)" ]; then
    [ "${FORCE:-0}" = 1 ] || die "tools/$name is not empty (FORCE=1 to replace it)"
    [ -e "$dest/.git" ] && die "tools/$name is a git checkout; refusing to replace it"
    rm -rf "$dest"
fi
mkdir -p "$ROOT/build"
tmp=$(mktemp -d "$ROOT/build/import.XXXXXX")
trap 'rm -rf "$tmp"' EXIT

case $archive in
    *.zip)
        if   have unzip; then unzip -q "$archive" -d "$tmp"
        elif have bsdtar; then bsdtar -xf "$archive" -C "$tmp"
        else die "need unzip or bsdtar to read a .zip"; fi ;;
    *.tar.gz|*.tgz) gzip -dc "$archive" | tar xf - -C "$tmp" ;;
    *) die "unsupported archive type: $archive" ;;
esac

# strip a lone top-level directory
n=$(ls -A "$tmp" | wc -l | tr -d ' ')
if [ "$n" = 1 ] && [ -d "$tmp/$(ls -A "$tmp")" ]; then root=$tmp/$(ls -A "$tmp"); else root=$tmp; fi

mkdir -p "$dest"
( cd "$root" && tar cf - . ) | ( cd "$dest" && tar xf - )
info "$name: imported into tools/$name"
