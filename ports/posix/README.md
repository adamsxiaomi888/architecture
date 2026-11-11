# POSIX 端口

在 Linux 中由一个固定周期线程读取单调时钟（`clock_gettime(CLOCK_MONOTONIC, ...)`），复制传感器快照，调用 `er_runtime_step`，再把 `er_output_t` 写入驱动。建议使用 `SCHED_FIFO`、CPU affinity、`mlockall`，并把 AI/规划线程通过单写者命令邮箱隔离。
