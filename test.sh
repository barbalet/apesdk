#!/bin/bash
# Canonical fast ApeSDK test runner.

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

run_in "${ROOT_DIR}/toolkit/test" bash test_toolkit_wo_exec.sh
run_in "${ROOT_DIR}/toolkit/test" ./test_math
run_in "${ROOT_DIR}/toolkit/test" ./test_prototype
run_in "${ROOT_DIR}/toolkit/test" ./test_object_string
run_in "${ROOT_DIR}/toolkit/test" ./test_object
run_in "${ROOT_DIR}/toolkit/test" bash test_object_file.sh
run_in "${ROOT_DIR}/toolkit/test" bash test_object_file_vect2.sh
run_in "${ROOT_DIR}/toolkit/test" bash test_memory_list.sh
run_in "${ROOT_DIR}/toolkit/test" ./test_memory_list_run

run_in "${ROOT_DIR}/test" bash test_script_wo_exec.sh
run_in "${ROOT_DIR}/test" ./test_apescript check_apescript.txt

run_in "${ROOT_DIR}/test" bash test_wo_exec.sh
run_in "${ROOT_DIR}/test" ./test_sim

run_in "${ROOT_DIR}/test" bash test_gui_wo_exec.sh
run_in "${ROOT_DIR}/test" ./test_gui

printf "\nAll fast tests passed.\n"
