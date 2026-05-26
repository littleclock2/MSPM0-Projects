# MSPM0 Line-Length-Meter

基于 TI MSPM0G3507 的线长测量仪，通过超声波或光电方式测量线缆长度。

## 硬件需求

- 主控芯片：MSPM0G3507
- 激励源：AD9910 DDS 信号发生器
- 显示：0.96 寸 OLED（SPI 接口）
- 测量传感器：TDC-GP22 超声波测距模块

## 软件环境

- IDE：Keil MDK-ARM / VS Code + Keil Assistant
- SDK：TI MSPM0 SDK
- 调试器：Nu-Link

## 目录结构

```
MSPM0-Line-Length-Meter/
├── v0.1/              # 初版（基础测量功能）
├── v1.0/              # 正式版（改进精度和UI）
├── Final/             # 最终版（优化算法和稳定性）
├── LICENSE
└── .gitignore
```

## 版本说明

- **v0.1**：初版，实现基本线长测量功能
- **v1.0**：正式版，改进测量精度，增加OLED显示
- **Final**：最终版，优化算法，提高稳定性

## 使用方法

1. 使用 Keil MDK 打开对应版本的工程文件
2. 编译工程
3. 通过 Nu-Link 下载到 MSPM0G3507 开发板
4. 连接 AD9910 模块和 TDC-GP22 传感器

## 许可证

[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/deed.zh-hans)
