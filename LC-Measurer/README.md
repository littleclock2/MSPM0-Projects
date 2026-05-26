# MSPM0 LC-Measurer

基于 TI MSPM0G3507 的 LC 电感电容测量仪，支持电感和电容的高精度测量。

## 硬件需求

- 主控芯片：MSPM0G3507
- 激励源：AD9910 DDS 信号发生器
- 显示：0.96 寸 OLED（SPI 接口）
- 交互：串口屏（USART HMI）
- 按键：独立按键

## 软件环境

- IDE：Keil MDK-ARM / VS Code + Keil Assistant
- SDK：TI MSPM0 SDK
- 调试器：Nu-Link

## 目录结构

```
MSPM0-LC-Measurer/
├── LC-Measurer/           # 基础版
│   ├── Core/              # 核心代码
│   │   ├── src/           # main.c, ti_msp_dl_config.c
│   │   ├── inc/           # 头文件
│   │   ├── ad9910/        # AD9910 DDS 驱动
│   │   ├── OLED/          # OLED 显示驱动
│   │   └── UartScreen/    # 串口屏通信
│   ├── Driver/            # TI SDK 驱动库
│   ├── keil/              # Keil 工程文件
│   └── toolchains/        # CMake 工具链配置
├── LC-Measurer-Ultra/     # 增强版（改进算法/精度）
│   └── ...
├── LICENSE
└── .gitignore
```

## 使用方法

1. 使用 Keil MDK 打开 `keil/ADC_TEST.uvprojx`
2. 编译工程
3. 通过 Nu-Link 下载到 MSPM0G3507 开发板
4. 连接 AD9910 模块、OLED 显示屏、串口屏

## 版本说明

- **LC-Measurer**：基础版，实现基本 LC 测量功能
- **LC-Measurer-Ultra**：增强版，优化测量精度和算法

## 许可证

[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/deed.zh-hans)
