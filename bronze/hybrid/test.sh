#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
cd "$work"
compiler=("${CC:-cc}")
if [ "${SANITIZE:-0}" = 1 ]; then
    # Native genetics has existing signed-shift UB; check memory and division.
    compiler+=(-fsanitize=address,integer-divide-by-zero -fno-omit-frame-pointer)
fi
"${compiler[@]}" -std=c99 -O1 -g -DBRONZE_SIDECAR_TEST=1 -DBRONZE_SIM=1 -DHIDE_THUNDER=1 -DCOMMAND_LINE_EXPLICIT \
    "$root"/toolkit/*.c "$root"/script/*.c "$root"/render/*.c \
    "$root"/sim/*.c "$root"/entity/*.c "$root"/universe/*.c \
    "$root"/gui/{draw,buffer,tutorial}.c \
    "$root"/bronze/dsl/*.c "$root"/bronze/scenario/bronze_scenario.c \
    "$root"/bronze/tasks/bronze_tasks.c "$root"/bronze/work/bronze_work.c "$root"/bronze/actors/bronze_actors.c \
    "$root"/bronze/hybrid/{bronze_hybrid,test_hybrid}.c \
    -lz -lm -lpthread -o hybrid-test
if [ "${1:-}" = "--qualify-full" ]; then
    ./hybrid-test "$root/bronze/content/bronze_age_full.bronze" 42
    ./hybrid-test "$root/bronze/content/bronze_age_full.bronze" 1337
else
    ./hybrid-test
fi
