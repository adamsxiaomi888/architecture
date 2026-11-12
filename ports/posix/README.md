# Linux POSIX 时间端口

`osal_posix.c` 已实现 `CLOCK_MONOTONIC` 和绝对截止时间等待 `clock_nanosleep(TIMER_ABSTIME)`；信号中断 EINTR 后继续等待原截止点。

构建与契约见 [OSAL 说明](../../docs/osal.md)。该端口不创建线程，不修改进程调度策略，不提供硬实时保证。

实际产品仍需按平台验证调度优先级、内存锁定、CPU 亲和性和控制周期预算；尚未实现任务/队列/锁或 MCU 端口。
