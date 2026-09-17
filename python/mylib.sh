#!/usr/bin/env bash
set -eu

python_dir="$(cd "$(dirname "$0")" && pwd)"
root_dir="$(cd "$python_dir/.." && pwd)"
output_file="$python_dir/apesim.so"

if [ "$(uname -s)" = "Darwin" ]; then
    shared_flag=-dynamiclib
else
    shared_flag=-shared
fi

cc -std=c99 -O2 -fPIC "$shared_flag" -DPYTHON_BUILD -DCOMMAND_LINE_EXPLICIT \
    "$root_dir"/toolkit/*.c \
    "$root_dir"/script/*.c \
    "$root_dir"/render/graph.c \
    "$root_dir"/sim/*.c \
    "$root_dir"/entity/*.c \
    "$root_dir"/universe/*.c \
    "$root_dir"/longterm.c \
    -o "$output_file" -lz -lm -lpthread -w

echo "Built $output_file"
