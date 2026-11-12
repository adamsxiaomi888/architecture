# 第一目标平台冻结

## 决策

第一阶段采用**一个 MCU**，不是 G4 与 H5 同时使用：

```text
主控：Linux
MCU：STM32H563ZI
开发板：NUCLEO-H563ZI
RTOS：FreeRTOS 与 Zephyr 双端口
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

选择 H563 是因为第一版目标同时包含实时控制、OTA、安全启动、参数、日志和多 RTOS 端口。H563 的 Cortex-M33、TrustZone、较大的 Flash/RAM 和安全启动生态更适合作为通用量产控制器。Zephyr 官方有维护中的 `nucleo_h563zi` 板级目标。[Zephyr Nucleo H563ZI 文档](https://docs.zephyrproject.org/latest/boards/st/nucleo_h563zi/doc/index.html)

## 为什么不同时使用 G474

G474 作为未来直接 FOC/功率级电机控制的备选平台，不进入第一版硬件。HD1910 已经内置电机、编码器和闭环控制器，第一版 MCU 不需要自己做 FOC；同时使用两颗 MCU 会增加 BOM、通信、升级、看门狗、同步和故障传播复杂度。

## 双 RTOS 端口

```text
同一份 control_task.c / runtime / protocol
       ├── FreeRTOS OSAL + STM32H5 HAL/BSP
       └── Zephyr OSAL + Zephyr device tree
```

两套端口不能共享 RTOS 头文件；共享部分只能依赖公共 C 接口和协议。第一阶段先让两个 RTOS 在同一开发板启动并完成 UART/HD1910 控制，再冻结量产 RTOS 选择；量产产品最终只选择一个 RTOS，双端口用于验证和产品线复用。

## 验收顺序

1. FreeRTOS 和 Zephyr 都在 `nucleo_h563zi` 编译、启动，打印固件版本和板 ID。
2. UART 回环与 HD1910 帧 CRC、序列、超时测试。
3. 单舵机低速、小角度、无负载控制。
4. Linux 失联、传感器错误、复位和看门狗测试。
5. 安全启动、参数持久化、MCU OTA 和回滚测试。
6. 如果后续需要直接 FOC，再单独评审 G474/H7，不把它并入本版硬件。

目标板采购后仍需核对板卡修订、引脚复用、供电、TTL 收发器和 HD1910 线序；模拟器不能替代这些检查。
