#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
cd "$work"
common=(-DBRONZE_EXTENDED_CONTENT=\"$root/bronze/content/example_large.bronze\" -std=c99 -O1 -g -DBRONZE_SIDECAR_TEST=1 -DBRONZE_SIM=1 -DHIDE_THUNDER=1 -DCOMMAND_LINE_EXPLICIT -I"$root/bronze/scenario-runtime/include")
native_sources=("$root"/bronze/dsl/*.c "$root/bronze/scenario/bronze_scenario.c" "$root"/toolkit/*.c "$root"/script/*.c "$root"/render/*.c
    "$root"/sim/*.c "$root"/entity/*.c "$root"/universe/*.c "$root"/gui/*.c
    "$root/bronze/hybrid/bronze_hybrid.c" "$root/bronze/adapter/bronze_apesdk_adapter.c"
    "$root/bronze/scenario-runtime/src/scenario_runtime.c")
if [ "$(uname -s)" = Darwin ]; then
    common+=(-DBRONZE_DISK_SAVE=1)
    native_sources+=("$root/bronze/save/bronze_save.c")
fi
if [ "${SANITIZE:-0}" = 1 ]; then
    # Original native genetics has known signed-shift UB. Keep ASan across all
    # sources, division checks on native code, and UBSan on the new layers.
    "${CC:-cc}" "${common[@]}" ${ACTOR_TEST_FLAGS:-} -fsanitize=address,integer-divide-by-zero -fno-omit-frame-pointer -c "${native_sources[@]}"
    "${CC:-cc}" "${common[@]}" ${ACTOR_TEST_FLAGS:-} -fsanitize=address,undefined -fno-omit-frame-pointer -c \
        "$root/bronze/tasks/bronze_tasks.c" "$root/bronze/work/bronze_work.c" "$root/bronze/actors/bronze_actors.c" "$root/bronze/actors/test_actors.c" "$root/bronze/work/test_work.c"
    "${CC:-cc}" -fsanitize=address,undefined ./*.o -lz -lm -lpthread -o actor-tests
else
    "${CC:-cc}" "${common[@]}" ${ACTOR_TEST_FLAGS:-} "${native_sources[@]}" \
        "$root/bronze/tasks/bronze_tasks.c" "$root/bronze/work/bronze_work.c" "$root/bronze/actors/bronze_actors.c" "$root/bronze/actors/test_actors.c" "$root/bronze/work/test_work.c" \
        -lz -lm -lpthread -o actor-tests
fi
./actor-tests
# Exercise restores with nonzero allocation tails on both CI Linux and macOS.
# Snapshots must not depend on malloc returning zero-filled storage.
MALLOC_PERTURB_=165 MallocScribble=1 ./actor-tests sessions

MALLOC_PERTURB_=165 MallocScribble=1 ./actor-tests work

./actor-tests extended

./actor-tests townships
