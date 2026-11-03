#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
cd "$work"
"${CC:-cc}" -std=c99 -O1 -g -DBRONZE_SIM=1 -DHIDE_THUNDER=1 -DCOMMAND_LINE_EXPLICIT \
    "$root"/toolkit/*.c "$root"/script/*.c "$root"/render/*.c \
    "$root"/sim/*.c "$root"/entity/*.c "$root"/universe/*.c \
    "$root"/gui/{draw,buffer,tutorial}.c \
    "$root"/bronze/actors/bronze_actors.c \
    "$root"/bronze/hybrid/{bronze_hybrid,test_hybrid}.c \
    -lz -lm -lpthread -o hybrid-test
./hybrid-test
