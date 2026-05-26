# OLED_SPI 说明
1. 使用硬件SPI实现驱动SSD1306 OLED，原有的OLED示例使用GPIO软件模拟SPI。
2. 函数名与原有OLED驱动保持一致，传参类型有些许改变，不过不重要，可以通用，只需要把oled_spi.c与oled_spi.h放入对应位置，再把主函数include的oled.h改成oled_spi.h即可。当然sysconfig也要改成和这个工程一致。
# 关于sysconfig
1. SPI直接添加，SPI Profiles选择Custom大部分都不用改。Advanced Configuration 里面 Communication Direction改成PICO only,然后PinMux引脚跟示例一样就行。另：默认SPI速率为8M，最高16M好像也能用，这个具体看个人。
2. GPIO里面添加好DC引脚。这个引脚控制了写入oled的数据是command还是data，这个功能其实在SPI Controller Advanced Configuration里面有，不过使用这个自带功能时，DC脚不能选PA4,但底板上又是连到PA4的改不了，所以以后在不使用底板的时候可以自行探索。