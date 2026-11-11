# embodied-runtime

`embodied-runtime` 是一个面向具身智能执行器的最小可量产运行时：同一份无动态内存的 C11 核心可编译到 Linux/POSIX，也可被 FreeRTOS、RT-Thread 等任务直接调用。当前示例是双关节位置/速度受限控制器，故意把“策略”和“实时安全执行”分开。

## 快速运行（Linux）

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
ctest --test-dir build --output-on-failure
./build/er_demo
```

核心调用顺序是：

```c
er_runtime_init(&rt, &limits);             /* 上电初始化 */
er_runtime_submit_command(&rt, &cmd);       /* 非实时策略线程提交 */
er_runtime_arm(&rt);                         /* 经过上电检查后使能 */
er_runtime_step(&rt, now_us, &sensor, &out); /* 固定频率实时循环 */
```

`step` 失败、命令超时、时间回拨、传感器无效、越界或 NaN 都会锁存 fault，并清零输出。必须先 `disarm` 再 `reset_faults`，最后重新检查并 `arm`，避免故障自动恢复造成危险动作。

## 目录

- `include/er/runtime.h`：稳定的跨平台 ABI 和数据结构。
- `src/runtime.c`：无锁、无 malloc 的确定性核心。
- `ports/posix`：Linux 时钟/线程/设备接入骨架。
- `ports/freertos`：FreeRTOS 任务、队列和 tick 适配说明。
- `tests`：边界和故障锁存测试。
- `docs/design.md`：架构和实时性设计。
- `docs/production.md`：量产、验证、升级和故障闭环。

目前核心只负责“安全命令执行器”；神经网络推理、规划、CAN/串口协议属于上层适配器，应通过拷贝到单写入命令邮箱接入，不能在 ISR 中运行推理或修改运行时状态。
