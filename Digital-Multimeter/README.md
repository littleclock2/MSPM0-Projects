# MSPM0 Digital-Multimeter

基于 TI MSPM0G3507 的数字万用表，支持电压、电流、电阻、电容测量。

## 硬件需求

- 主控芯片：MSPM0G3507
- ADC：内置 12-bit ADC / 外置 ADS1118
- 显示：0.96 寸 OLED（SPI 接口）
- 测量前端：运放电路 + 模拟开关

## 软件环境

- IDE：Keil MDK-ARM / VS Code + Keil Assistant
- SDK：TI MSPM0 SDK

## 目录结构

```
MSPM0-Digital-Multimeter/
├── Digital-Multimeter/
│   ├── Multimeter_v1.1/   # v1.1 版本
│   └── Multimeter_v1.7/   # v1.7 版本（改进精度）
├── LICENSE
└── .gitignore
```

## 功能特性

- 直流电压测量（0-30V）
- 直流电流测量（0-10A）
- 电阻测量（0-10MΩ）
- 电容测量（0-1000μF）

## 许可证

[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/deed.zh-hans)
