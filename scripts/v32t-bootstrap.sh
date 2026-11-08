#!/bin/sh
# v32t-bootstrap.sh [TOOL...] -- make tools/<name> exist for every tool in tools.list.
#
#  * already a git checkout / non-empty directory  -> left alone
#  * registered in .gitmodules but not checked out -> git submodule update --init
#  * otherwise                                     -> git submodule add <url> tools/<name>
#
# The URL comes from tools.list; V32T_URL_<name> in the environment overrides it
# (e.g. a fork, an ssh URL, or a local path while testing).  V32T_BRANCH_<name>
# picks a branch.  Commit the resulting .gitmodules and gitlinks yourself.
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
. "$ROOT/scripts/lib.sh"
cd "$ROOT"

have git || die "git not found"
git rev-parse --git-dir >/dev/null 2>&1 || die "$ROOT is not a git repository (git init first)"

tools=${*:-$(manifest_tools)}
rc=0
for t in $tools; do
    [ -n "$(manifest_field "$t" 1)" ] || { warn "'$t' is not in tools.list"; rc=1; continue; }
    dir=tools/$t
    if [ -e "$dir/.git" ] || { [ -d "$dir" ] && [ -n "$(ls -A "$dir" 2>/dev/null)" ]; }; then
        info "$t: present"
        continue
    fi
    if git config -f .gitmodules --get "submodule.$dir.url" >/dev/null 2>&1; then
        info "$t: initialising submodule"
        git submodule update --init -- "$dir" || rc=1
        continue
    fi
    url=$(eval "printf '%s' \"\${V32T_URL_$t:-}\"")
    [ -n "$url" ] || url=$(manifest_field "$t" 4)
    branch=$(eval "printf '%s' \"\${V32T_BRANCH_$t:-}\"")
    info "$t: adding submodule $url"
    # shellcheck disable=SC2086
    git submodule add ${branch:+-b "$branch"} "$url" "$dir" || { warn "$t: could not add $url"; rc=1; }
done
exit $rc
