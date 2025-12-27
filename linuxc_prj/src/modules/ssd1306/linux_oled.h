#ifndef __LINUX_OLED_H
#define __LINUX_OLED_H 

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Linux SPI设备路径
#define SPI_DEVICE "/dev/spidev1.0"

// GPIO芯片和引脚定义（使用libgpiod）
#define GPIO_CHIP "/dev/gpiochip1"
#define DC_PIN_OFFSET 5  // DC引脚在gpiochip1的第5个引脚

// OLED命令/数据定义
#define OLED_CMD  0	// 写命令
#define OLED_DATA 1	// 写数据

// 类型定义
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

// 显示缓冲区（128x64像素，按页组织）
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_PAGES  8      // 64/8 = 8页

extern u8 OLED_GRAM[OLED_WIDTH][OLED_PAGES];

// Linux特定函数
int linux_oled_init(void);
void linux_oled_deinit(void);

// libgpiod相关函数
int linux_gpiod_init(void);
void linux_gpiod_deinit(void);
int linux_gpiod_set_dc(int value);

// SPI通信函数
void OLED_WR_Byte(u8 dat, u8 cmd);
int linux_spi_transfer(u8 *tx_buf, u8 *rx_buf, int len);

// OLED基本功能函数
void OLED_ColorTurn(u8 i);
void OLED_DisplayTurn(u8 i);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_Refresh(void);
void OLED_Clear(void);
int OLED_Init(void);

// 图形绘制函数
void OLED_DrawPoint(u8 x, u8 y);
void OLED_ClearPoint(u8 x, u8 y);
void OLED_DrawLine(u8 x1, u8 y1, u8 x2, u8 y2);
void OLED_DrawCircle(u8 x, u8 y, u8 r);

// 文本显示函数
void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 size1);
void OLED_ShowString(u8 x, u8 y, u8 *chr, u8 size1);
void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size1);
void OLED_ShowChinese(u8 x, u8 y, u8 num, u8 size1);
void OLED_ScrollDisplay(u8 num, u8 space);
void OLED_ShowPicture(u8 x0, u8 y0, u8 x1, u8 y1, u8 BMP[]);

// 工具函数
u32 OLED_Pow(u8 m, u8 n);
void OLED_WR_BP(u8 x, u8 y);

#endif