#!/bin/bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$(mktemp -d)"
trap 'rm -rf "$BUILD_DIR"' EXIT
"${CC:-cc}" -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
    "$SCRIPT_DIR"/../*.c "$SCRIPT_DIR/fuzz_object_json.c" "$SCRIPT_DIR/test_object_json_fuzz.c" \
    -o "$BUILD_DIR/fuzz_json" -lz -lm -lpthread
export FUZZ_FAILURE_PATH="${FUZZ_FAILURE_PATH:-/tmp/apesdk-json-fuzz-failure.bin}"
"$BUILD_DIR/fuzz_json" "$SCRIPT_DIR"/../json/json_*.json "$SCRIPT_DIR"/../json/invalid/*.json
