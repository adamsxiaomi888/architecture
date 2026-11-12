#!/usr/bin/env bash
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
INPUT="${1:-$ROOT/models/HD-1910-C001-20260902.stp}"
OUTPUT="${2:-$ROOT/web/assets/hd1910.step.stl}"
if ! command -v freecadcmd >/dev/null 2>&1 && ! command -v FreeCADCmd >/dev/null 2>&1; then
  echo "需要先安装 FreeCAD：sudo apt install freecad" >&2
  exit 2
fi
mkdir -p "$(dirname "$OUTPUT")"
FREECAD_BIN="$(command -v freecadcmd || command -v FreeCADCmd)"
# Coarser tessellation is sufficient for browser visualization and avoids
# pathological runtimes on detailed vendor assemblies. STEP remains the source.
exec "$FREECAD_BIN" -c "import Part; s=Part.read(r'$INPUT'); s.exportStl(r'$OUTPUT', 0.20)"
