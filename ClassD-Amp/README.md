# MSPM0 ClassD-Amp

基于 TI MSPM0G3507 的 D 类音频功率放大器，支持自动音量控制功能。

## 硬件需求

- 主控芯片：MSPM0G3507
- 功放芯片：TPA3110 / TPA3116
- 音频输入：3.5mm AUX / 蓝牙
- 显示：OLED / 串口屏

## 软件环境

- IDE：Keil MDK-ARM / VS Code + Keil Assistant
- SDK：TI MSPM0 SDK

## 目录结构

```
MSPM0-ClassD-Amp/
├── Type-D-AutoSetVolume/
│   ├── Core/          # 核心代码
│   ├── Driver/        # TI SDK 驱动库
│   └── keil/          # Keil 工程文件
├── LICENSE
└── .gitignore
```

## 功能特性

- D类功放驱动
- 自动音量控制算法
- 音频信号检测与增益调节

## 许可证

[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/deed.zh-hans)
