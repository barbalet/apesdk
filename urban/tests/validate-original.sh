#!/bin/sh
# Usage: ./tests/validate-original.sh /path/to/apesdk /path/to/urbanscribble/urban
set -eu
cd "$(dirname "$0")/.."
SDK=${1:?ApeSDK checkout required}
URBAN=${2:?Urban source directory required}
SDK=$(cd "$SDK" && pwd)
URBAN=$(cd "$URBAN" && pwd)
ROOT=$(pwd)
BUILD=${PRA_BUILD:-build}
make all BUILD="$BUILD"
TEMP=$(mktemp -d)
trap 'rm -rf "$TEMP"' EXIT HUP INT TERM
for name in house tree fence; do cp "$URBAN/game/$name.c" "$TEMP/$name.c"; done
printf '#include "%s/src/pra_internal.h"\n' "$ROOT" > "$TEMP/mushroom.h"
for name in house tree fence; do
    "${CC:-cc}" -std=c11 -g -O1 ${PRA_SANITIZERS:-} -Iinclude -Isrc -I"$SDK/toolkit" -DPRA_REFERENCE \
        -c "$TEMP/$name.c" -o "$TEMP/$name.o"
done
"${CC:-cc}" -std=c11 -g -O1 ${PRA_SANITIZERS:-} -Iinclude -Isrc -I"$SDK/toolkit" -DPRA_REFERENCE \
    -c tests/reference.c -o "$TEMP/reference.o"
for name in file io math memory object vect; do
    "${CC:-cc}" -std=c11 -g -O1 ${PRA_SANITIZERS:-} -c "$SDK/toolkit/$name.c" -o "$TEMP/sdk-$name.o"
done
"${CC:-cc}" -std=c11 -g -O1 ${PRA_SANITIZERS:-} -Iinclude -Isrc -I"$SDK/toolkit" tests/original.c "$TEMP/house.o" "$TEMP/tree.o" "$TEMP/fence.o" "$TEMP/reference.o" "$TEMP"/sdk-*.o "$BUILD/libpra.a" -lm -o "$TEMP/compare"
"$TEMP/compare"
