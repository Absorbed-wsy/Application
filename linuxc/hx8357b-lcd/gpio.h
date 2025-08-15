#ifndef GPIO_H
#define GPIO_H

int gpio_export(int pin);
int gpio_unexport(int pin);
int gpio_direction(int pin, int dir);
int gpio_write(int pin, int value);
int gpio_read(int pin);

void setup_gpio(void);
void blacklight_gpio(void);
void cs_gpio(void);
void res_gpio(void);
void cleanup_gpio(void);

#define GPIO_CLK 	0   
#define GPIO_MOSI 	0
#define GPIO_CS 	0	
	
#define GPIO_RST 	7		
#define GPIO_BL 	6

#endif // GPIO_H
