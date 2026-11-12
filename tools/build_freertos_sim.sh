#!/usr/bin/env bash
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
KERNEL="${1:-${FREERTOS_KERNEL_DIR:-}}"
if [ -z "$KERNEL" ] || [ ! -f "$KERNEL/include/FreeRTOS.h" ]; then
  echo "usage: $0 /path/to/FreeRTOS-Kernel" >&2; exit 2
fi
OUT="${TMPDIR:-/tmp}/embodied-freertos-sim"
mkdir -p "$OUT"
COMMON="-std=c11 -O2 -Wall -Wextra -pthread -D_GNU_SOURCE -DprojCOVERAGE_TEST=0 -include stdlib.h"
INC="-I$ROOT/include -I$ROOT/examples -I$ROOT/sim -I$ROOT/ports/freertos -I$ROOT/ports/freertos/sim -I$KERNEL/include -I$KERNEL/portable/ThirdParty/GCC/Posix -I$KERNEL/portable/ThirdParty/GCC/Posix/utils"
SRC="$KERNEL/tasks.c $KERNEL/queue.c $KERNEL/list.c $KERNEL/timers.c $KERNEL/event_groups.c $KERNEL/stream_buffer.c $KERNEL/croutine.c $KERNEL/portable/MemMang/heap_3.c $KERNEL/portable/ThirdParty/GCC/Posix/port.c $KERNEL/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c"
CMD="cc $COMMON $INC $SRC $ROOT/src/osal.c $ROOT/ports/freertos/osal_freertos.c $ROOT/examples/control_task.c $ROOT/src/runtime.c $ROOT/src/devices.c $ROOT/sim/sim.c $ROOT/sim/actuator.c $ROOT/ports/freertos/sim/main.c -lm -o $OUT/er_freertos_sim"
echo "+ $CMD"
eval "$CMD"
echo "+ $OUT/er_freertos_sim"
timeout 15 "$OUT/er_freertos_sim"
