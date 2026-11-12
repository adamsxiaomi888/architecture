# 最小 OSAL：相同周期逻辑，替换时钟端口

本轮实现时间和周期等待这一小块 OSAL；任务创建、队列、锁以及 FreeRTOS/RT-Thread 端口尚未实现。

```text
相同周期任务逻辑
       ↓ er_periodic_init / er_periodic_wait
周期调度器（纯 C，无操作系统头文件）
       ↓ now / wait_until 函数表
Linux CLOCK_MONOTONIC / 测试虚拟时钟 / 未来 RTOS 端口
```

`now` 返回本地单调微秒；`wait_until` 等待同一时间域的绝对截止时间。时钟回拨、早于截止时间返回和数值溢出必须报告错误。该接口只能用于任务上下文，不能在 ISR 中等待。

周期按绝对截止时间推进，避免把业务执行耗时持续累加进周期。超过截止时间时报告迟到量；错过多个周期则跳到下一未来截止点，不连续补跑旧动作。实际控制任务应根据产品时间预算决定是否锁存故障，调度器本身不决定执行器安全动作。

## 原生运行

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
(cd build && ctest --output-on-failure)
./build/er_periodic_demo
```

例程通过 POSIX 端口运行 10 个 10ms 周期，打印迟到与错过周期数。它不是电机控制任务；打印也不能放进量产高频控制路径。Linux 调度可能迟到，测试不使用无法保证的抖动上限冒充硬实时验收。

现有网页和离线场景仍使用虚拟时间：不应让确定性测试被真实睡眠拖慢。周期调度器使用另一套可控测试时钟验证边界；目前尚未把整个应用包装成跨 OS 的任务框架。

## RTOS 迁移

RTOS 端口需要实现相同两个操作：扩展为 64 位的单调时钟，以及等待该时间域截止点。必须处理 tick 回绕、精度不足、唤醒取整和中断上下文限制；不能简单把微秒除以 tick 后向下取整。端口必须通过相同的契约测试并在目标板测量调度时序。

业务代码通过 OSAL 使用服务才能复用；直接调用 xTaskCreate 或芯片中断代码不会因此自动在 Linux 上运行。RTOS 官方模拟端口是另一种测试路径，不等于硬件时序验证。

## 当前共用任务验证

`examples/control_task.c` 是不包含 POSIX 或 RTOS 头文件的周期控制任务；`control_posix.c` 只负责注入 Linux 时钟端口。它通过 `er_devices` 和仿真 HAL 读取反馈、调用 `er_runtime_step`、提交输出。`normal`、`stale`、`sensor` 三个入口使用同一份任务代码：

```sh
./build/control_posix normal
./build/control_posix stale
./build/control_posix sensor
```

这证明业务任务与 Linux 启动入口已分开；FreeRTOS 端口源码现在位于 `ports/freertos/`，通过 `ER_BUILD_FREERTOS_PORT=ON` 可选编译；它需要产品提供 `FreeRTOSConfig.h`、kernel 和 MCU port。当前尚未在目标板编译或运行，因此仍不能声称 RTOS 已验证。下一步是为选定 MCU 固定配置、任务入口、tick 回绕策略，并复用同一 `er_control_task_run`。

## FreeRTOS Linux 模拟

仓库提供 `ports/freertos/sim/` 和 `tools/build_freertos_sim.sh`。它使用 FreeRTOS Kernel 的 GCC/Posix port 创建真实 FreeRTOS task，再调用同一份 `examples/control_task.c`；kernel 放在仓库外，不进入产品源码提交：

```sh
git clone --depth 1 --branch V11.1.0 https://github.com/FreeRTOS/FreeRTOS-Kernel.git /tmp/FreeRTOS-Kernel
tools/build_freertos_sim.sh /tmp/FreeRTOS-Kernel
```

本轮实际结果：任务启动、40 个 10 ms 周期、控制场景通过，输出 `freertos_sim control_task rc=0`。该结果证明接口连接和任务逻辑可运行；Posix port 使用 pthread 模拟调度，不能证明 MCU tick、中断延迟、PWM/ADC 或硬实时性能。
