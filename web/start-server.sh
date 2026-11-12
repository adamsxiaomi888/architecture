#!/usr/bin/env bash
set -eu
WEB_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
PORT="${1:-8000}"
IP="$(hostname -I | awk '{print $1}')"
echo "浏览器打开: http://${IP}:${PORT}/"
echo "按 Ctrl+C 停止网页服务"
exec python3 -m http.server "$PORT" --bind 0.0.0.0 --directory "$WEB_DIR"
