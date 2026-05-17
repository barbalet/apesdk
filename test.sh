#!/bin/bash
# Canonical deterministic ApeSDK test runner.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"

cleanup() {
    find "${ROOT_DIR}" -name "*.o" -delete
    find "${ROOT_DIR}" -name "*.gcda" -delete
    find "${ROOT_DIR}" -name "*.gcno" -delete
    find "${ROOT_DIR}" -name "*.gcov" -delete
    rm -f "${ROOT_DIR}/toolkit/json"/2*.json
}

run_in() {
    local directory="$1"
    shift
    printf "\n==> %s: %s\n" "${directory#${ROOT_DIR}/}" "$*"
    ( cd "${directory}" && "$@" )
}

trap cleanup EXIT

run_in "${ROOT_DIR}/toolkit/unit" bash toolkit_tests.sh
run_in "${ROOT_DIR}/toolkit/unit" ./toolkit_tests

run_in "${ROOT_DIR}/script/unit" bash script_tests.sh
run_in "${ROOT_DIR}/script/unit" ./script_tests

run_in "${ROOT_DIR}/sim/unit" bash sim_tests.sh
run_in "${ROOT_DIR}/sim/unit" ./sim_tests

run_in "${ROOT_DIR}/render/unit" bash render_tests.sh
run_in "${ROOT_DIR}/render/unit" ./render_tests

run_in "${ROOT_DIR}/entity/test" bash test_immune.sh
run_in "${ROOT_DIR}/entity/test" ./test_immune

run_in "${ROOT_DIR}/entity/unit" bash entity_tests.sh
run_in "${ROOT_DIR}/entity/unit" ./entity_tests

run_in "${ROOT_DIR}/universe/unit" bash universe_tests.sh
run_in "${ROOT_DIR}/universe/unit" ./universe_tests

printf "\nAll deterministic tests passed.\n"
