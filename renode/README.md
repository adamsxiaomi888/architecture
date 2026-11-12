# MCU 固件仿真入口（Renode）

这里是第三层仿真的入口：加载真实 Cortex-M 固件 ELF，在虚拟平台上运行 FreeRTOS、启动代码和串口协议。当前文件是**平台适配骨架**，不是已经匹配某个 STM32 型号的完整模型；在选定 MCU 后必须用该型号的内存映射、启动文件、UART、定时器和 Flash 外设替换示例地址。

## 安装与运行

在 Ubuntu 安装 Renode 后：

```sh
tools/run_renode.sh /path/to/control.elf
```

脚本会检查 `renode`、固件和平台文件，并执行 `test.resc`。没有 Renode 或没有 ELF 时明确失败，不会把 Linux/FreeRTOS 模拟结果冒充 MCU 仿真。

## 当前验证边界

- 可加载：目标固件 ELF（取决于平台描述和地址匹配）。
- 计划接入：UART/TTL 总线、虚拟 HD1910、虚拟 Flash、看门狗和故障注入。
- 尚未声称：STM32G431/H5 的寄存器等价、PWM/ADC 时序、真实中断延迟或电气安全。

Renode 适合验证固件启动、任务、协议和可重复故障；真实 MCU 仍需要编程器、示波器和硬件在环验收。
