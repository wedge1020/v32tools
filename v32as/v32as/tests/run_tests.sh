#!/bin/sh
# usage: run_tests.sh path/to/v32asc   (needs only sh, od, cmp, grep)
BIN=${1:-../bin/v32asc}
cd "$(dirname "$0")" || exit 1
case $BIN in /*) ;; *) BIN=$OLDPWD/$BIN ;; esac
T=${TMPDIR:-/tmp}/v32asc_test.$$; mkdir -p "$T"; trap 'rm -rf "$T"' EXIT
pass=0; fail=0
ok()  { pass=$((pass+1)); }
bad() { fail=$((fail+1)); echo "FAIL: $1"; }
for g in golden/*.hex; do
  b=$(basename "$g" .hex); name=${b%.*}; mode=${b##*.}
  "$BIN" --$mode --binary -o "$T/o.vbin" cases/$name.asm 2>"$T/err" || { bad "$b (assemble)"; continue; }
  od -An -tx4 -v -w4 "$T/o.vbin" > "$T/o.hex"
  cmp -s "$T/o.hex" "$g" && ok || bad "$b (output differs)"
done
for g in golden/*.debug; do
  b=$(basename "$g" .debug); name=${b%.*}; mode=${b##*.}
  ( cd cases && "$BIN" --$mode -B -g -o "$T/d.vbin" $name.asm ) 2>/dev/null || { bad "$b (assemble -g)"; continue; }
  cmp -s "$T/d.vbin.debug" "$g" && ok || bad "$b (debug output differs)"
done
for e in errors/*.asm; do
  n=$(basename "$e" .asm)
  if "$BIN" --none "$e" >/dev/null 2>"$T/err"; then bad "$n (expected failure)"
  elif grep -q "$(cat errors/$n.expect)" "$T/err"; then ok
  else bad "$n (message mismatch)"; fi
done
# stdin batch mode and legacy bare word
printf 'HLT\n' | "$BIN" - >/dev/null 2>&1 && ok || bad "stdin"
echo "passed $pass, failed $fail"
[ $fail -eq 0 ]
