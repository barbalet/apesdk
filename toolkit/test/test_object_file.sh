#!/bin/bash
#	test.sh
#
#	=============================================================
#
#   Copyright 1996-2024 Tom Barbalet. All rights reserved.
#
#   Permission is hereby granted, free of charge, to any person
#   obtaining a copy of this software and associated documentation
#   files (the "Software"), to deal in the Software without
#   restriction, including without limitation the rights to use,
#   copy, modify, merge, publish, distribute, sublicense, and/or
#   sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following
#   conditions:
#
#   The above copyright notice and this permission notice shall be
#	included in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#   OTHER DEALINGS IN THE SOFTWARE.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TOOLKIT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
JSON_DIR="${TOOLKIT_DIR}/json"

cleanup() {
    rm -f "${JSON_DIR}"/2*.json "${TOOLKIT_DIR}/apesdk-json"
}

trap cleanup EXIT

round_trip() {
    local input_name="$1"
    local input_file="${JSON_DIR}/${input_name}"
    local output_file="${JSON_DIR}/2${input_name#?}"

    "${TOOLKIT_DIR}/apesdk-json" "${input_file}"
    diff -u "${input_file}" "${output_file}"
}

cd "${TOOLKIT_DIR}"
./apesdk-json.sh

round_trip example_four_two_vect2.json
round_trip example_urban.json
round_trip example_road.json
round_trip example0c.json
round_trip example1.json
round_trip example1b.json
round_trip example1c.json
round_trip example2.json
round_trip example2c.json
round_trip example3.json
round_trip example3c.json
round_trip example3d.json
round_trip example3e.json
round_trip example4.json
round_trip example4b.json
round_trip example5.json
round_trip battle.json
round_trip sim.json
round_trip example6.json
round_trip example7.json
round_trip random.json
round_trip randomb.json
round_trip randomc.json
round_trip random2.json
round_trip neighborhood.json
round_trip new_ashford.json
round_trip new_folkestone.json
