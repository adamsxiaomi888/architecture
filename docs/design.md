# 架构与接口设计

## 1. 分层

```
传感器驱动/总线 ISR -> sensor snapshot -> 实时控制任务(固定周期)
                                             |
策略/规划/推理线程 -> command mailbox ------------> er_runtime_step -> actuator driver
                                             |
                                  fault/event recorder -> 日志/诊断
```

公共核心只认识时间戳、序列号、传感器快照、限制和输出；硬件驱动通过端口层转换单位和总线报文。这样 Linux 上可以由 PREEMPT_RT 线程调用，在 MCU 上可以由最高优先级 RTOS task 调用，核心不依赖 pthread、FreeRTOS 或 libc 扩展。

## 2. 实时执行契约

实时任务每周期调用一次 `er_runtime_step(now_us, sensor, output)`。调用者保证 `now_us` 为单调微秒时间；任务不阻塞、不分配内存，并在输出后立即写入驱动。策略线程只能调用 `er_runtime_submit_command`，生产实现应在外围用单写入 mailbox 或临界区保护该调用（本核心本身是 single-owner、同步 API）。周期预算应通过 trace 测量，控制周期例如 1 kHz 时，最坏执行时间必须小于 500 us，并把通信和驱动预算单独记账。

## 3. 安全规则

- 命令序列必须递增，时间戳不能早于运行时当前时间。
- 位置、速度限制和传感器值必须是有限浮点数，且在标定限制内。
- `ER_MAX_CMD_AGE_US` 到期立即故障；时间回拨故障。
- 传感器 invalid、NaN/Inf、越界由核心锁存。
- 任意 fault 都会撤销 `armed` 并输出 disabled/零速度。
- 故障清除只能在 disarm 后显式完成，重新 arm 前需做外部安全检查（急停、驱动器 ready、编码器校准）。

真实产品还应在端口层增加独立硬件看门狗、驱动器 STO、过流/过温输入、双通道急停和输出限幅；软件 fault 不能替代硬件安全回路。

## 4. Linux 端口

使用 `CLOCK_MONOTONIC_RAW` 提供微秒时间；实时任务设置 `SCHED_FIFO`、锁定内存 `mlockall`，预分配消息池，绑定 CPU，并把策略线程与控制线程隔离。设备读写采用非阻塞 fd 或专用 I/O 线程，控制线程只消费时间戳快照。启动时先 disarm，完成设备探测、版本/标定校验和急停检查后再 arm。

## 5. RTOS 端口

FreeRTOS/RT-Thread 中用硬件定时器或高优先级周期任务触发控制；tick 不足时直接读取 DWT/通用定时器形成 64 位微秒时间。ISR 只写双缓冲快照并通知任务，不能调用核心的复杂路径。命令 mailbox 可用长度为 1 的队列或 seq-lock；任务拿到快照后调用相同的核心 API。将核心编译为 freestanding 子集时提供 `memset`、`isfinite` 等经过认证的实现，避免不可控 libc。

## 6. 可扩展性

双关节只是示例。量产版本应把轴数、单位标度、控制律和协议放在生成的配置中；保持 `er_runtime_*` ABI 稳定。新增状态机（BOOT/SAFE/ARMED/FAULT）、模式（位置、速度、阻抗）和故障码时，使用版本化结构体和编译期静态断言，禁止改变已有字段语义。
