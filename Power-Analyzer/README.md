# MSPM0 Power-Analyzer

基于 TI MSPM0G3507 的单相用电器功耗分析仪，可测量电压、电流、功率、功率因数等参数。

## 硬件需求

- 主控芯片：MSPM0G3507
- ADC：ADS8688 / 内置ADC
- 显示：串口屏（USART HMI）
- 电压采样：电阻分压网络
- 电流采样：ACS712 霍尔电流传感器

## 软件环境

- IDE：Keil MDK-ARM / VS Code + Keil Assistant
- SDK：TI MSPM0 SDK
- 调试器：Nu-Link

## 目录结构

```
MSPM0-Power-Analyzer/
├── PA-Ultra/                    # 增强版（最终版本）
├── Provincial-Competition/      # 省赛版本（含多个迭代）
│   ├── PA_re - 副本/
│   ├── Rxtest/
│   ├── TSB/
│   ├── TSB (3)/
│   ├── TSB (3) - core/
│   ├── TSB - core/
│   ├── TSB -test/
│   └── Template - DOTest/
├── LICENSE
└── .gitignore
```

## 功能特性

- 实时电压/电流采样
- 有功功率、无功功率、视在功率计算
- 功率因数测量
- 谐波分析（可选）
- 串口屏实时显示

## 使用方法

1. 使用 Keil MDK 打开 `PA-Ultra/keil/` 下的工程文件
2. 编译工程
3. 通过 Nu-Link 下载到 MSPM0G3507 开发板
4. 连接电压/电流采样电路和串口屏

## 许可证

[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/deed.zh-hans)
