# 确定性场景与回放

仿真器复用 `src/runtime.c`，使用虚拟时钟，每步 10 ms，运行 300 步。
正常场景每步更新命令，避免旧示例仅发送一次命令后在 110 ms 超时。
输出每行是独立 JSON 对象，第一行描述场景，其后为控制计算前的观测及当次输出。

```sh
cmake -S . -B /tmp/er-build -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/er-build
(cd /tmp/er-build && ctest --output-on-failure)
/tmp/er-build/er_sim_runner normal > /tmp/normal.jsonl
python3 tools/replay.py /tmp/er-build/er_sim_runner /tmp/normal.jsonl
```

| 场景参数 | 注入行为 | 验收 |
|---|---|---|
| normal | 持续刷新有效命令 | 3 秒内两轴位置误差 < 0.001 rad，无故障 |
| stale | 不刷新初始命令 | 100 ms 边界仍有效，110 ms 起命令过期，持续禁能 |
| sensor | 200 ms 传感器无效，210 ms 恢复 | 故障锁存，恢复反馈不自动恢复运动 |
| stall | 第一轴固定不动 | 第一轴位置保持 0，第二轴正常收敛 |

**stall 仅验证卡死模型，不代表堵转保护已经实现。** 当前核心仍保持使能，无电流、温度或跟踪误差保护；不能把此测试通过理解为可以安全连接真实电机。

回放当前使用相同版本的程序重新运行一个固定场景，并逐条对比 JSONL；它不是任意 AI 动作流回放，也不承诺跨编译器、CPU 的浮点逐位一致。修改日志中任意观测或截断日志会导致失败。AI 任意命令流的输入协议和记录重放后续另行实现。
