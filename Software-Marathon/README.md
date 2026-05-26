# MSPM0 Software-Marathon

基于 TI MSPM0G3507 的软件马拉松参赛作品，实现简易信号测量装置。

## 硬件需求

- 主控芯片：MSPM0G3507
- 显示：0.96 寸 OLED（SPI 接口）
- 按键：矩阵键盘
- 传感器：根据赛题需求配置

## 软件环境

- IDE：Keil MDK-ARM / VS Code + Keil Assistant
- SDK：TI MSPM0 SDK

## 目录结构

```
MSPM0-Software-Marathon/
├── SoftwareMarathon/
│   ├── Core/          # 核心代码
│   ├── Driver/        # TI SDK 驱动库
│   ├── KArrayV151/    # 矩阵键盘驱动
│   └── keil/          # Keil 工程文件
├── LICENSE
└── .gitignore
```

## 许可证

[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/deed.zh-hans)
