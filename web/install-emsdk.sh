#!/usr/bin/env bash
# One-time local Emscripten setup for the ApeSDK WebAssembly build.
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
SDK_DIR="${1:-$HOME/emsdk}"
SDK_VERSION="${EMSDK_VERSION:-latest}"

if [ ! -x "$SDK_DIR/emsdk" ]; then
    git clone https://github.com/emscripten-core/emsdk.git "$SDK_DIR"
fi

"$SDK_DIR/emsdk" install "$SDK_VERSION"
"$SDK_DIR/emsdk" activate "$SDK_VERSION"

# shellcheck disable=SC1090
source "$SDK_DIR/emsdk_env.sh"
emcc --version

cd "$ROOT_DIR/web"
make build

printf '\nWeb build complete: %s/web/web\n' "$ROOT_DIR"
printf 'Preview it with: cd "%s/web" && make serve\n' "$ROOT_DIR"
