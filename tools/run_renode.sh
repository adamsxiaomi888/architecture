#!/usr/bin/env bash
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
ELF="${1:-}"
if ! command -v renode >/dev/null 2>&1; then
  echo "missing renode: install Renode and put it on PATH" >&2
  exit 2
fi
if [ -z "$ELF" ] || [ ! -f "$ELF" ]; then
  echo "usage: $0 /path/to/firmware.elf" >&2
  exit 2
fi
ELF="$(CDPATH= cd -- "$(dirname -- "$ELF")" && pwd)/$(basename -- "$ELF")"
cd "$ROOT/renode"
echo "Renode platform: $ROOT/renode/platform.repl"
echo "Firmware ELF: $ELF"
renode --execute "set elf '$ELF'; include @test.resc"
