# 第一目标平台冻结

## 决策

第一阶段采用**一个 MCU**，不是 G4 与 H5 同时使用：

```text
主控：Linux
MCU：STM32H563ZI
开发板：NUCLEO-H563ZI
RTOS：Zephyr（量产主 RTOS）；FreeRTOS/POSIX 仅作为兼容性验证后端
执行器：FEETECH HD-1910-C001 TTL 半双工总线舵机
主控链路：Linux 通过 UART/USB 转 TTL 总线发送版本化命令
```

```text
Linux 主控
    ↓ UART / CAN-FD / Ethernet（按产品选择）
STM32H563ZI
    ├── 实时控制
    ├── HD1910 总线舵机
    ├── 参数、日志、产测
    ├── 看门狗和故障处理
    └── MCU OTA / 安全启动
```

选择 H563 是因为第一版目标同时包含实时控制、OTA、安全启动、参数和日志。H563 的 Cortex-M33、TrustZone、较大的 Flash/RAM 和安全启动生态更适合作为通用量产控制器。Zephyr 官方有维护中的 `nucleo_h563zi` 板级目标。[Zephyr Nucleo H563ZI 文档](https://docs.zephyrproject.org/latest/boards/st/nucleo_h563zi/doc/index.html)

## 为什么不同时使用 G474

G474 作为未来直接 FOC/功率级电机控制的备选平台，不进入第一版硬件。HD1910 已经内置电机、编码器和闭环控制器，第一版 MCU 不需要自己做 FOC；同时使用两颗 MCU 会增加 BOM、通信、升级、看门狗、同步和故障传播复杂度。

## RTOS 选择与兼容层

```text
同一份 control_task.c / runtime / protocol
       ├── Zephyr OSAL + Zephyr device tree（量产）
       ├── FreeRTOS OSAL（兼容性与迁移验证）
       └── POSIX OSAL（Linux/CI/桌面仿真）
```

业务层、设备能力接口、协议、故障模型和测试夹具不直接包含任何 RTOS 头文件，只依赖公共 OSAL/HAL 接口。Zephyr 负责量产固件的线程、同步、设备树、驱动和系统服务；FreeRTOS 端口用于验证公共接口没有隐藏的 Zephyr 依赖，POSIX 端口用于 Linux 和 CI 仿真。单个量产产品只冻结 Zephyr，避免双 RTOS 带来的配置、测试矩阵和升级风险。

## 验收顺序

1. Zephyr 在 `nucleo_h563zi` 编译、启动，打印固件版本和板 ID；同时运行 POSIX/FreeRTOS 兼容性构建。
2. UART 回环与 HD1910 帧 CRC、序列、超时测试。
3. 单舵机低速、小角度、无负载控制。
4. Linux 失联、传感器错误、复位和看门狗测试。
5. 安全启动、参数持久化、MCU OTA 和回滚测试。
6. 量产冻结 Zephyr 版本、SDK、模块清单和 Kconfig；建立可重复构建与升级包签名流程。
7. 如果后续需要直接 FOC，再单独评审 G474/H7，不把它并入本版硬件。

目标板采购后仍需核对板卡修订、引脚复用、供电、TTL 收发器和 HD1910 线序；模拟器不能替代这些检查。
