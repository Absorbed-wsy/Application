bug：
缓存刷新需要将颜色取反才是原本颜色，原因未知？



demo：
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "gpio.h"
#include "lcd.h"
#include "spi.h"

int main() 
{
	char buffer[100];
    // Initialize LCD
    LCD_Init();
	
	while(1) {
	//画线
	LCD_DrawLine(10, 10, 100, 100);
	LCD_Clear(~WHITE);	 
	sleep(1);
	
	//显示字符
	sprintf(buffer, "I Love You!");
    LCD_ShowString(30,50,16,buffer,0);
	LCD_Clear(~BLUE);
	sleep(1);
	
	//画矩形
	LCD_DrawRectangle(20, 20, 200, 200);
	LCD_Clear(~GREEN);
	sleep(1);
	
	//填充
	LCD_Fill(50,50,300,300,~BRED);
	LCD_Clear(~YELLOW);
	sleep(1);
	
	//画圆
	LCD_circle(100, 100,~BRED,50, 1);
	LCD_Clear(~GRAY);
	sleep(1);

	LCD_Clear(~BLACK);
	sleep(1);
	}
	
    //Wait for a while
    //sleep(10);

    cleanup_gpio();

    return 0;
}
