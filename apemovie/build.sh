#!/bin/sh
set -eu

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/apemovie/build"
OUTPUT="$ROOT_DIR/apemovie/apemovie"
OUTPUT_TEMP="$BUILD_DIR/apemovie.new"

mkdir -p "$BUILD_DIR"

CFLAGS="${CFLAGS:--O2 -w}"
DEFINES="-DCOMMAND_LINE_EXPLICIT"
INCLUDES="-I$ROOT_DIR -I$ROOT_DIR/toolkit -I$ROOT_DIR/script -I$ROOT_DIR/render -I$ROOT_DIR/sim -I$ROOT_DIR/entity -I$ROOT_DIR/universe -I$ROOT_DIR/gui"

OBJECTS=""

compile_c() {
    source_file="$1"
    object_name="$(echo "$source_file" | tr '/.' '__').o"
    object_file="$BUILD_DIR/$object_name"
    gcc $CFLAGS $DEFINES $INCLUDES -c "$ROOT_DIR/$source_file" -o "$object_file"
    OBJECTS="$OBJECTS $object_file"
}

for source_file in "$ROOT_DIR"/toolkit/*.c; do compile_c "${source_file#$ROOT_DIR/}"; done
for source_file in "$ROOT_DIR"/script/*.c; do compile_c "${source_file#$ROOT_DIR/}"; done
for source_file in "$ROOT_DIR"/render/*.c; do compile_c "${source_file#$ROOT_DIR/}"; done
for source_file in "$ROOT_DIR"/sim/*.c; do compile_c "${source_file#$ROOT_DIR/}"; done
for source_file in "$ROOT_DIR"/entity/*.c; do compile_c "${source_file#$ROOT_DIR/}"; done
for source_file in "$ROOT_DIR"/universe/*.c; do compile_c "${source_file#$ROOT_DIR/}"; done

compile_c "gui/buffer.c"
compile_c "gui/draw.c"
compile_c "gui/shared.c"
compile_c "gui/tutorial.c"
compile_c "gui/message.c"
compile_c "apemovie/apemovie_core.c"
compile_c "apemovie/main.c"

gcc $CFLAGS $OBJECTS -o "$OUTPUT_TEMP" -lz -lm -lpthread
mv -f "$OUTPUT_TEMP" "$OUTPUT"

echo "$OUTPUT"
