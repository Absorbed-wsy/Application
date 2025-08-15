#ifndef __LCD_H
#define __LCD_H

#include "gpio.h"
#include <stdint.h>

typedef struct
{
	uint16_t width;
	uint16_t height;
	uint16_t id;
	uint8_t  dir;
	uint16_t wramcmd;
	uint16_t setxcmd;
	uint16_t setycmd;
}_lcd_dev;

extern _lcd_dev lcddev;

//=====================User Config=======================//
#define USE_HORIZONTAL  		1

extern uint16_t  POINT_COLOR;	//default red
extern uint16_t  BACK_COLOR; 	//default white

#define	LCD_LED gpio_write(GPIO_BL, 1)
#define	LCD_RST(s) gpio_write(GPIO_RST, s)

#define CLR_SDA()	gpio_write(GPIO_MOSI, 0)
#define SET_SDA()	gpio_write(GPIO_MOSI, 1)
#define CLR_CLK()	gpio_write(GPIO_CLK, 0)
#define SET_CLK()	gpio_write(GPIO_CLK, 1)
#define CLR_CS()    gpio_write(GPIO_CS, 0)
#define SET_CS()    gpio_write(GPIO_CS, 1)

#define WHITE       ~0xFFFF
#define BLACK      	~0x0000
#define BLUE       	~0x001F
#define BRED        ~0XF81F
#define GRED 		~0XFFE0
#define GBLUE		~0X07FF
#define RED         ~0xF800
#define MAGENTA     ~0xF81F
#define GREEN       ~0x07E0
#define CYAN        ~0x7FFF
#define YELLOW      ~0xFFE0
#define BROWN 		~0XBC40
#define BRRED 		~0XFC07
#define GRAY  		~0X8430
					
#define DARKBLUE	~0X01CF
#define LIGHTBLUE	~0X7D7C
#define GRAYBLUE	~0X5458
					
#define LIGHTGREEN	~0X841F
#define LIGHTGRAY	~0XEF5B
#define LGRAY		~0XC618
					
#define LGRAYBLUE	~0XA651
#define LBBLUE		~0X2B12



void LCD_Init(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Clear(uint16_t Color);
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd);

void LCD_WR_DATA(uint8_t data);
void LCD_WR_DATA16(uint16_t DATA);
void LCD_WriteRAM_Prepare(void);
void LCD_SetParam(void);

void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color);
void LCD_ShowChar(uint16_t x,uint16_t y,uint16_t fc, uint16_t bc, uint8_t num,uint8_t size,uint8_t mode);
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size);
void LCD_ShowString(uint16_t x,uint16_t y,uint8_t size,char *p,uint8_t mode);
void LCD_DrawFont16(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s,uint8_t mode);
void LCD_DrawFont24(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s,uint8_t mode);
void LCD_DrawFont32(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s,uint8_t mode);
void Show_Str(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *str,uint8_t size,uint8_t mode);
void LCD_Drawbmp16(uint16_t x,uint16_t y,const unsigned char *p);
void LCD_circle(int xc, int yc,uint16_t c,int r, int fill);
void LCD_StrCenter(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *str,uint8_t size,uint8_t mode);
void LCD_DrawFillRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

#endif





