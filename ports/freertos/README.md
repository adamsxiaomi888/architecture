# FreeRTOS 端口

用一个高优先级周期任务（`vTaskDelayUntil`）调用 `er_runtime_step`；ISR 只投递已校验的传感器快照或急停事件。命令邮箱可用长度为 1 的 queue，更新时覆盖旧命令。系统 tick 转换为微秒时需使用 64 位扩展计数器，防止 tick 回绕。
