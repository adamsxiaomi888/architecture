# 实现状态与验证边界

核对日期：2026-11-12。以代码、测试和本表为准；设计文档的目标结构不代表已经交付。

| 模块 | 当前实现 | 尚未完成 |
|---|---|---|
| C 控制核心 | 双轴比例速度计算、命令与传感器检查、故障锁存 | 可配置轴数、无反馈应用控制器、堵转保护、真实控制环 |
| 桌面仿真 | 虚拟时间、normal/stale/sensor/stall、300 步 JSONL | Linux/MCU 双进程、网络故障模型 |
| 浏览器闭环 | C runtime + sim 编译 WASM、命令/观测 API | AI 模型接入、训练流程 |
| 回放 | 网页事件导入导出；桌面固定场景重跑比较 | 跨后端/跨版本一致性保证 |
| 3D | Canvas 投影的程序化外壳、舵盘、实体摇臂、表面标签 | CAD 精确分件动画、碰撞/惯量/力矩物理 |
| CAD | 官方 STEP 与转换的 STL 已保存 | 制造公差、轴位置和实物比对验收 |
| HAL | 执行器绑定组件及有/无反馈仿真驱动；原生和 WASM 已通过接口访问设备 | 真实驱动、其他 HAL 实现 |
| RTOS | POSIX/FreeRTOS 接入说明 | 可编译 RTOS 端口和板级验证 |
| OTA | RAM 状态机原型与正常路径测试 | 状态缺陷修复、真实验签、Flash、启动恢复 |
| 参数/日志/产测 | 需求和设计说明 | 持久化服务、事件服务、产测执行与报告 |

## 验证方式

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
(cd build && ctest --output-on-failure)
```

Linux 上有 Node 时注册 12 项测试，否则 11 项（跳过 `web_wasm`）。WASM 集成测试使用真实 WASM 和模拟 DOM，不等于真实浏览器画面验收。不得把“找到 0 个测试”当成通过。

原生与 WASM 使用相同 C 源码，但未建立逐步跨后端数值对照测试。测试数量也不等于全部故障覆盖。

## 图与源码一起维护

功能变更时核对：本表、README 导航、代码导读、接口/场景说明及架构图；只变显示样式时更新 3D 状态和导读即可。新增发布记录需写明实际运行的测试，以及未验证边界。该规则需要后续开发实际执行，不意味着有后台自动更新服务。

本轮 Release 构建已验证，CTest 实际执行 12/12 通过，包含周期 OSAL、POSIX 端口、设备绑定与已有 WASM 测试。未进行真实硬件验收。

最小 OSAL 已实现单调时钟、绝对截止等待与周期调度，提供 Linux POSIX 端口；无 POSIX/RTOS 依赖的共用控制任务已通过 Linux 入口运行。任务/队列/锁及目标 MCU RTOS 端口尚未实现；FreeRTOS OSAL 源码与可选 CMake 目标已提供但未在目标板验证。见 [OSAL 说明](osal.md)。

FreeRTOS Kernel GCC/Posix 模拟已通过 `tools/build_freertos_sim.sh` 运行同一 `control_task.c`（40 个 10 ms 周期，normal 场景通过）。这不是目标 MCU RTOS 验收；FreeRTOS 端口仍需产品配置、芯片 port 和板级测量。

Renode 第三层 MCU 仿真入口已建立于 `renode/`，当前是待选定 MCU 的 Cortex-M 平台骨架；没有 Renode、目标 ELF 和冻结的寄存器地址前，不宣称固件已在 MCU 仿真中运行。
