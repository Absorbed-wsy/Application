#ifndef __LINUX_OLED_H
#define __LINUX_OLED_H 

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Linux SPI device path
#define SPI_DEVICE "/dev/spidev1.0"

// GPIO chip and pin definitions (using libgpiod)
#define GPIO_CHIP "/dev/gpiochip1"
#define DC_PIN_OFFSET 5  // DC pin is the 5th pin on gpiochip1

// OLED command/data definitions
#define OLED_CMD  0	// Write command
#define OLED_DATA 1	// Write data

// Type definitions
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

// Display buffer (128x64 pixels, organized by pages)
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_PAGES  8      // 64/8 = 8 pages

extern u8 OLED_GRAM[OLED_WIDTH][OLED_PAGES];

// Linux specific functions
int linux_oled_init(void);
void linux_oled_deinit(void);

// libgpiod related functions
int linux_gpiod_init(void);
void linux_gpiod_deinit(void);
int linux_gpiod_set_dc(int value);

// SPI communication functions
void OLED_WR_Byte(u8 dat, u8 cmd);
int linux_spi_transfer(u8 *tx_buf, u8 *rx_buf, int len);

// OLED basic functions
void OLED_ColorTurn(u8 i);
void OLED_DisplayTurn(u8 i);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_Refresh(void);
void OLED_Clear(void);
int OLED_Init(void);

// Graphics drawing functions
void OLED_DrawPoint(u8 x, u8 y);
void OLED_ClearPoint(u8 x, u8 y);
void OLED_DrawLine(u8 x1, u8 y1, u8 x2, u8 y2);
void OLED_DrawCircle(u8 x, u8 y, u8 r);

// Text display functions
void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 size1);
void OLED_ShowString(u8 x, u8 y, u8 *chr, u8 size1);
void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size1);
void OLED_ShowChinese(u8 x, u8 y, u8 num, u8 size1);
void OLED_ScrollDisplay(u8 num, u8 space);
void OLED_ShowPicture(u8 x0, u8 y0, u8 x1, u8 y1, u8 BMP[]);

// Utility functions
u32 OLED_Pow(u8 m, u8 n);
void OLED_WR_BP(u8 x, u8 y);

#endif