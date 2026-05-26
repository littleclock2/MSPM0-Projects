# MSPM0 DDS-Signal-Gen

基于 TI MSPM0G3507 和 AD9833/AD9910 的 DDS 信号发生器，支持多种波形输出。

## 硬件需求

- 主控芯片：MSPM0G3507
- DDS 芯片：AD9833 / AD9910
- 显示：0.96 寸 OLED（SPI 接口）
- 按键：独立按键 / 旋转编码器

## 软件环境

- IDE：Keil MDK-ARM / VS Code + Keil Assistant
- SDK：TI MSPM0 SDK

## 目录结构

```
MSPM0-DDS-Signal-Gen/
├── DDS-Signal-Generator/
│   ├── basic_modules/     # 基础模块
│   ├── DDS/               # DDS 基础版
│   ├── DDSv(3.0)/         # DDS 3.0 版本
│   ├── DDS_MCU2.0/        # DDS MCU 2.0 版本
│   └── DDS_OLED_SPI_MCU/  # DDS OLED SPI 版本
├── LICENSE
└── .gitignore
```

## 功能特性

- 正弦波、方波、三角波输出
- 频率可调（1Hz - 1MHz）
- 幅度可调
- OLED 实时显示参数

## 许可证

[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/deed.zh-hans)
