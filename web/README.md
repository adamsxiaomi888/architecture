# 浏览器仿真控制台

通过 `./web/start-server.sh` 提供 HTTP 页面。页面优先加载随工程提供的 WebAssembly C 核心。它模拟两个有反馈执行器，提供目标位置、运动闭环、故障锁存和 JSON 状态展示。

页面使用同一份 C11 控制核心和虚拟舵机，支持正常运动、传感器失效、命令超时及 JSONL 导出/回放。加载失败时显示 JavaScript 演示模式，两种模式的日志不混用。仿真不会调用真实硬件，也不能验证 MCU 实时时序。

## 从 Windows 浏览器访问

在 VMware Ubuntu 终端执行：

```sh
cd /home/alientek/work/embodied-runtime
./web/start-server.sh
```

脚本会打印访问地址，例如 `http://192.168.120.128:8000/`，把它复制到 Windows 浏览器即可。服务运行期间不要关闭 Ubuntu 终端；按 `Ctrl+C` 停止。若无法访问，优先将 VMware 网络改为桥接模式，并检查 Ubuntu 防火墙是否允许 TCP 8000。
