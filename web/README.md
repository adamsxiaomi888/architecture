# 浏览器仿真控制台

这是无依赖的静态可视化：直接打开 `index.html` 即可运行。它模拟两个有反馈执行器，提供目标位置、运动闭环、故障锁存和 JSON 状态展示。

当前页面是教学和交互演示，不会调用真实硬件，也不等同于 C11 核心的实时保证。下一步可以用 WebAssembly 编译同一份 C 核心，并通过 WebSocket 接入桌面仿真器；页面本身保持为观察和故障注入工具。

## 从 Windows 浏览器访问

在 VMware Ubuntu 终端执行：

```sh
cd /home/alientek/work/embodied-runtime
./web/start-server.sh
```

脚本会打印访问地址，例如 `http://192.168.120.128:8000/`，把它复制到 Windows 浏览器即可。服务运行期间不要关闭 Ubuntu 终端；按 `Ctrl+C` 停止。若无法访问，优先将 VMware 网络改为桥接模式，并检查 Ubuntu 防火墙是否允许 TCP 8000。
