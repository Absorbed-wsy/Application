#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "gpio.h"
#include "lcd.h"
#include "spi.h"

int main() 
{
    LCD_Init();
	LCD_Clear(BLACK);
	
	while(1) {
	
		display_system_info();
		usleep(1000);
	}
	
    cleanup_gpio();

    return 0;
}
