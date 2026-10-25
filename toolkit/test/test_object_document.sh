#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
build=$(mktemp -d)
trap 'rm -rf "$build"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -Wpedantic -DAPESDK_OBJECT_JSON_ONLY=1 \
    ${CFLAGS:-} "$root/object.c" "$root/test/test_object_document.c" -lm -o "$build/test"
"$build/test"
