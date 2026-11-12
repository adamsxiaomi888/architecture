# FreeRTOS OSAL 端口

`osal_freertos.c` 提供与 Linux POSIX 端口相同的 `er_osal_clock_t`：时间来自 `xTaskGetTickCount()`，等待通过 `vTaskDelay()`。它不创建任务；产品启动代码应在 FreeRTOS 任务中调用同一份 `er_control_task_run` 所需的周期逻辑。

## 可选构建

默认 CMake 不编译此端口，因为仓库没有替产品冻结的 `FreeRTOSConfig.h`、内核移植和芯片时钟。使用目标 kernel 和配置时：

```sh
cmake -S . -B build-freertos \
  -DER_BUILD_FREERTOS_PORT=ON \
  -DFREERTOS_KERNEL_DIR=/path/to/FreeRTOS-Kernel \
  -DFREERTOS_CONFIG_DIR=/path/to/product/config
cmake --build build-freertos --target er_osal_freertos
```

这个端口的微秒时间是由 tick 换算的，存在量化和 tick 回绕问题；量产产品必须使用 64 位扩展硬件计数器或验证回绕策略，不能把该示例直接当作高精度电机时钟。`vTaskDelay()` 只能在任务上下文调用。端口尚未在目标 MCU 上验证。
