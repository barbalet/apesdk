#!/bin/bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$(mktemp -d)"
trap 'rm -rf "$BUILD_DIR"' EXIT
"${CC:-cc}" -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
    "${SCRIPT_DIR}"/../*.c "${SCRIPT_DIR}/test_object_json.c" \
    -o "${BUILD_DIR}/test_object_json" -lz -lm -lpthread
"${BUILD_DIR}/test_object_json"
