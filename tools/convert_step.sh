#!/usr/bin/env bash
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
INPUT="${1:-$ROOT/models/HD-1910-C001-20260902.stp}"
OUTPUT="${2:-$ROOT/web/assets/hd1910.step.stl}"
if ! command -v FreeCAD >/dev/null 2>&1 && ! command -v freecad >/dev/null 2>&1; then
  echo "需要先安装 FreeCAD/OpenCascade 转换器：sudo apt install freecad" >&2
  exit 2
fi
mkdir -p "$(dirname "$OUTPUT")"
FREECAD_BIN="$(command -v FreeCAD || command -v freecad)"
exec "$FREECAD_BIN" -c "import Part; s=Part.read(r'$INPUT'); s.exportStl(r'$OUTPUT')"
