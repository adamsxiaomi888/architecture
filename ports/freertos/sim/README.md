# FreeRTOS Linux 模拟器

此目录使用 FreeRTOS Kernel 的 GCC/Posix port 在 Linux 上创建真实 FreeRTOS task。它验证任务入口、tick 驱动和 FreeRTOS OSAL 的连接；不会证明 MCU 中断延迟、PWM/ADC 时序或目标板实时性。

构建并运行（需要先下载 kernel，例如 `git clone --depth 1 --branch V11.1.0 https://github.com/FreeRTOS/FreeRTOS-Kernel.git`）：

```sh
tools/build_freertos_sim.sh /path/to/FreeRTOS-Kernel
```

脚本输出构建命令和任务结果，失败时返回非零。所有业务逻辑来自 `examples/control_task.c`。
