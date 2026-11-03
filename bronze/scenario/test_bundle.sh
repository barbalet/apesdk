#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
app_path="$(cd "$1" && pwd)"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
cd "$work"
cc -std=c99 -mmacosx-version-min=14.0 -I"$root/bronze/dsl" -c "$root"/bronze/dsl/*.c "$root/bronze/scenario/bronze_scenario.c"
swiftc -target "$(uname -m)-apple-macosx14.0" -parse-as-library -import-objc-header "$root/bronze/scenario/bronze_scenario.h" \
    "$root/bronze/ui/BronzeScenarioUI.swift" "$root/bronze/scenario/BundleTests.swift" \
    ./*.o -o bundle-tests
./bundle-tests "$app_path"
