#!/bin/bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$(mktemp -d)"
trap 'rm -rf "$BUILD_DIR"' EXIT
cd "$BUILD_DIR"
FLAGS=(-g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer)
for source in "$SCRIPT_DIR"/../*.c; do
    if [[ "$source" == */memory.c ]]; then
        "${CC:-cc}" "${FLAGS[@]}" -Dmemory_new=allocation_backend_new -Dmemory_free=allocation_backend_free -c "$source"
    else
        "${CC:-cc}" "${FLAGS[@]}" -c "$source"
    fi
done
"${CC:-cc}" "${FLAGS[@]}" "$SCRIPT_DIR/test_object_json_alloc.c" ./*.o -o test_alloc -lz -lm -lpthread
./test_alloc
