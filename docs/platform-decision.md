# 第一目标平台冻结

## 决策

第一阶段目标平台固定为：

```text
MCU：STM32G474RE
开发板：NUCLEO-G474RE
RTOS：FreeRTOS 与 Zephyr 双端口
执行器：FEETECH HD-1910-C001 TTL 半双工总线舵机
主控链路：Linux 通过 UART/USB 转 TTL 总线发送版本化命令
```

STM32G474RE 适合第一阶段的原因是它面向电机控制，后续可扩展 ADC/PWM/编码器；Zephyr 官方有维护中的 `nucleo_g474re` 板级目标，FreeRTOS 也可使用同一 Cortex-M4 控制任务。第一阶段先控制 HD1910，不把 FOC 和功率级同时引入，降低验收范围。

## 为什么暂不选 H563

STM32H563/H573 更适合安全启动、TrustZone 和复杂 OTA，但会同时引入安全域、启动链、密钥生命周期和更复杂的板级验证。它可以作为第二产品线；第一阶段优先证明业务、OSAL、设备接口和 Linux/MCU 通信的跨平台结构。

## 端口策略

```text
同一份 control_task.c / runtime / protocol
       ├── FreeRTOS OSAL + STM32G4 BSP
       └── Zephyr OSAL + Zephyr device tree
```

FreeRTOS 端口使用静态任务、单调硬件计时器和 UART 驱动；Zephyr 端口使用 `k_thread`/`k_sleep` 或硬件定时器、设备树 UART 和同一设备接口。二者不能共享 RTOS 头文件；共享部分只能依赖公共 C 接口和协议。

## 验收顺序

1. 两个 RTOS 都在 `nucleo_g474re` 编译并启动，打印固件版本和板 ID。
2. UART 回环与 HD1910 帧 CRC/序列/超时测试。
3. 单舵机低速、小角度、无负载控制。
4. Linux 失联、传感器错误、复位和看门狗测试。
5. 再加入参数存储和 MCU OTA；FOC/PWM 功率级另立硬件安全评审。

Zephyr 板级支持参考：[Nucleo G474RE 官方 Zephyr 文档](https://docs.zephyrproject.org/latest/boards/st/nucleo_g474re/doc/index.html)。目标板和芯片采购后仍需核对板卡修订、引脚复用、供电和总线收发器；模拟器不能替代这些检查。
