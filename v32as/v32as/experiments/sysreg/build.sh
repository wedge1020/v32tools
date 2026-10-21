#!/bin/sh
# Builds experiment.vbin (a CART program ROM) from payload.asm + experiment.asm.
# usage: ./build.sh [path/to/v32asc]
set -e
AS=${1:-../../../bin/v32asc}
cd "$(dirname "$0")"
mkdir -p build
$AS --origin 0x1000 --binary -o build/payload.vbin payload.asm
tail -c +13 build/payload.vbin > build/payload.dat          # strip the 12-byte VBIN header
WORDS=$(( $(wc -c < build/payload.dat) / 4 ))
sed "s/@PAYLOAD_WORDS@/$WORDS/" experiment.asm > build/experiment.asm
$AS --cart --binary -o build/experiment.vbin build/experiment.asm
echo "payload: $WORDS words;  ROM: $(( ($(wc -c < build/experiment.vbin) - 12) / 4 )) words  ->  build/experiment.vbin"
