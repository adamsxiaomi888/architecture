# 仿真、网页和命令闭环

更新日期：2026-11-12。本页描述当前实现；Linux + MCU 是后续部署目标，目前还没有两端通信进程、真实 RTOS 或硬件驱动。

## 两个运行入口，同一份 C 源码

```text
桌面：er_sim_runner ──────────────┐
                                 ├─ runtime.c → 控制输出 → sim.c → 传感器反馈
浏览器：app.js → WASM bridge ─────┘
```

这里表示代码复用，并非浏览器与桌面共享一个进程。浏览器 C 核心在浏览器本地执行；Ubuntu 的 HTTP 服务只提供静态文件。

`runtime.c` 检查命令与传感器后计算受限速度；`sim.c` 按速度积分得到下一步位置。虚拟时间由调用者递增，不读取墙上时钟。模型没有惯量、真实电流、温度或碰撞，不应称为经过硬件标定的数字孪生。

## 桌面场景与回放

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/er_sim_runner normal
./build/er_sim_runner sensor > /tmp/servo-sensor.jsonl
python3 tools/replay.py ./build/er_sim_runner /tmp/servo-sensor.jsonl
```

每个场景推进 300 步，每步 10 ms，输出 JSONL。normal 检查位置收敛；stale 检查命令超时；sensor 注入一次传感器失效并检查故障锁存；stall 只检查虚拟轴不动，**尚无堵转保护**。

`tools/replay.py` 在相同构建下重跑固定场景并逐条比较记录，拒绝不支持的场景和不匹配数据。它不是任意 AI 命令回放器，也没有随机噪声模型。详情见 [场景测试](scenarios.md)。

## 浏览器 C/WASM 闭环

`web/app.js` 自动加载 `er_core.js` / `er_core.wasm`，通过 `web/wasm/bridge.c` 导出的 reset、command、step、fault、stop、state 接口运行同一份 C 核心和仿真模型。页面步长为 20 ms，与桌面 10 ms 不同，因此不能要求两者逐条轨迹相同。

构建产物已提供；加载失败时页面明确显示 JavaScript 演示模式。两种后端的日志禁止混用。重新构建方法见 [WASM 说明](../web/wasm/README.md)。

网页支持目标调整、正常/传感器断线/命令超时场景，以及 JSONL 事件导出和导入回放。`window.erConsole` 提供可供自动化调用的命令/观测接口，尚未接入真实 AI 模型或训练系统。网页自动播放会续发命令，脚本单独调用 step 不续发命令，便于复现超时。协议见 [浏览器接口](../web/protocol.md)。

## 3D 画面是什么

`web/servo3d.js` 用 Canvas 投影程序化几何体，展示黑色 HD1910M 示意机身、输出盘、实际/目标摇臂和贴面标签。它读取与曲线相同的 `position / target / faults`，不另行计算控制算法。

官方 STEP 与转换后的 STL 已存入 `models/` 和 `web/assets/`。当前主视图为了可读性使用程序化外壳，并非完整 CAD 装配；加载状态不代表机械尺寸、孔位或运动轴已完成工程校验。原始 CAD 是参考，制造仍需型号图纸和实物确认。视觉接近不等于真实动力学一致。

## 自动测试及边界

```sh
(cd build && ctest --output-on-failure)
node tests/test_web_wasm.cjs
```

有 Node 时 CTest 注册 8 项：runtime、ota、四个桌面场景、web_bridge、web_wasm；无 Node 时注册 7 项。WASM 测试执行实际编译产物和网页适配层，使用模拟 DOM 检查运动、故障和事件回放，不是截图测试。

OTA 测试当前只覆盖原型正常路径，不代表已验证真实升级安全。浏览器与桌面通过也不能证明 PWM/ADC 同步、中断抖动、真实 Flash 掉电恢复、功率保护或机械安全。

后续工作包括真实硬件/RTOS 端口、参数与日志服务、产测、完整 OTA 故障注入；实现前保持“计划”状态。
