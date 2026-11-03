#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
cd "$work"
common=(-std=c99 -O1 -g -DBRONZE_SIM=1 -DHIDE_THUNDER=1 -DCOMMAND_LINE_EXPLICIT -I"$root/bronze/scenario-runtime/include")
native_sources=("$root"/toolkit/*.c "$root"/script/*.c "$root"/render/*.c
    "$root"/sim/*.c "$root"/entity/*.c "$root"/universe/*.c "$root"/gui/*.c
    "$root/bronze/hybrid/bronze_hybrid.c" "$root/bronze/adapter/bronze_apesdk_adapter.c"
    "$root/bronze/scenario-runtime/src/scenario_runtime.c")
if [ "${SANITIZE:-0}" = 1 ]; then
    # Original native genetics has known signed-shift UB. Keep ASan across all
    # sources and UBSan on the new identity layer and its integration harness.
    "${CC:-cc}" "${common[@]}" ${ACTOR_TEST_FLAGS:-} -fsanitize=address -fno-omit-frame-pointer -c "${native_sources[@]}"
    "${CC:-cc}" "${common[@]}" ${ACTOR_TEST_FLAGS:-} -fsanitize=address,undefined -fno-omit-frame-pointer -c \
        "$root/bronze/actors/bronze_actors.c" "$root/bronze/actors/test_actors.c"
    "${CC:-cc}" -fsanitize=address,undefined ./*.o -lz -lm -lpthread -o actor-tests
else
    "${CC:-cc}" "${common[@]}" ${ACTOR_TEST_FLAGS:-} "${native_sources[@]}" \
        "$root/bronze/actors/bronze_actors.c" "$root/bronze/actors/test_actors.c" \
        -lz -lm -lpthread -o actor-tests
fi
./actor-tests
./actor-tests sessions
