#!/bin/sh
set -eu
cd "$(dirname "$0")"
: "${EMCC:=emcc}"
mkdir -p build/web
"$EMCC" -std=c11 -O2 -Wall -Wextra -Werror -DAPESDK_URBAN_ONLY=1 -Iinclude -Isrc -I../toolkit \
    src/pra.c src/mesh.c src/house.c src/tree.c src/fence.c ../toolkit/math.c ../toolkit/vect.c ../toolkit/memory.c src/wasm.c \
    -sMODULARIZE=1 -sEXPORT_NAME=PRA -sALLOW_MEMORY_GROWTH=1 -sFILESYSTEM=0 \
    '-sEXPORTED_FUNCTIONS=["_pra_demo_init","_pra_demo_destroy","_pra_demo_mesh","_pra_demo_vertex_count","_pra_demo_json","_pra_demo_open_all","_pra_demo_toggle","_pra_demo_set_door","_pra_demo_blocked","_pra_demo_sheltered","_pra_demo_probe"]' \
    '-sEXPORTED_RUNTIME_METHODS=["UTF8ToString","HEAPF32"]' -o build/web/pra.js
cp web/index.html build/web/index.html
