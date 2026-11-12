#!/usr/bin/env bash
set -eu
ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
cd "$ROOT_DIR"
if ! command -v emcc >/dev/null 2>&1; then
  echo 'Missing emcc: install Emscripten and activate its environment.' >&2
  exit 1
fi
emcc -Iinclude -Isim web/wasm/bridge.c src/runtime.c sim/sim.c \
  -O2 -s WASM=1 \
  -s EXPORTED_FUNCTIONS='["_er_web_reset","_er_web_command","_er_web_step","_er_web_set_fault","_er_web_stop","_er_web_state"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' -o web/er_core.js
