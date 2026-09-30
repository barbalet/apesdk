#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
cd "$work"
: "${CC:=cc}"
flags="-std=c99 -O2 -DCOMMAND_LINE_EXPLICIT -Wall -Wextra -I$root/toolkit -I$root/script -I$root/render -I$root/sim -I$root/entity -I$root/universe -I$root/bronze -I$root/bronze/scenario-runtime/include"
$CC $flags -c $root/toolkit/*.c $root/script/*.c $root/render/*.c $root/sim/*.c $root/entity/*.c $root/universe/*.c $root/bronze/scenario-runtime/src/scenario_runtime.c $root/bronze/bronze_apesdk_adapter.c $root/bronze/bronze_apesdk_land_harness.c
$CC -o bronze_apesdk_land_harness ./*.o -lz -lm -lpthread
./bronze_apesdk_land_harness
