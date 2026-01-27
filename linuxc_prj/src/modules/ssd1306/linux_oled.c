#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <errno.h>
#include <gpiod.h>  // libgpiod header file

#include "linux_oled.h"

// Include font data
#include "oledfont.h"

// Global variables
u8 OLED_GRAM[OLED_WIDTH][OLED_PAGES];

// SPI file descriptor
static int spi_fd = -1;

// libgpiod related variables
static struct gpiod_chip *gpio_chip = NULL;
static struct gpiod_line_request *dc_request = NULL;

// SPI mode settings
static uint8_t spi_mode = SPI_MODE_0;
static uint8_t spi_bits = 8;
static uint32_t spi_speed = 1000000; // 1MHz

// libgpiod initialization
int linux_gpiod_init(void) {
    // Open GPIO chip
    gpio_chip = gpiod_chip_open(GPIO_CHIP);
    if (!gpio_chip) {
        perror("Failed to open GPIO chip");
        return -1;
    }
    
    // Create line settings for DC pin
    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (!settings) {
        perror("Failed to create line settings");
        gpiod_chip_close(gpio_chip);
        gpio_chip = NULL;
        return -1;
    }
    
    // Configure DC pin as output with initial value 0
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);
    
    // Create line config
    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    if (!line_cfg) {
        perror("Failed to create line config");
        gpiod_line_settings_free(settings);
        gpiod_chip_close(gpio_chip);
        gpio_chip = NULL;
        return -1;
    }
    
    // Add line settings
    unsigned int offset = DC_PIN_OFFSET;
    if (gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings) < 0) {
        perror("Failed to add line settings");
       gpiod_line_config_free(line_cfg);
        gpiod_line_settings_free(settings);
        gpiod_chip_close(gpio_chip);
        gpio_chip = NULL;
        return -1;
    }
    
    // Create request config
    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    if (!req_cfg) {
        perror("Failed to create request config");
        gpiod_line_config_free(line_cfg);
        gpiod_line_settings_free(settings);
        gpiod_chip_close(gpio_chip);
        gpio_chip = NULL;
        return -1;
    }
    
    gpiod_request_config_set_consumer(req_cfg, "oled_dc");
    
    // Request DC pin
    dc_request = gpiod_chip_request_lines(gpio_chip, req_cfg, line_cfg);
    if (!dc_request) {
        perror("Failed to request DC line");
        gpiod_request_config_free(req_cfg);
        gpiod_line_config_free(line_cfg);
        gpiod_line_settings_free(settings);
        gpiod_chip_close(gpio_chip);
        gpio_chip = NULL;
        return -1;
    }
    
    // Clean up temporary objects
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    
    return 0;
}

// libgpiod cleanup
void linux_gpiod_deinit(void) {
    if (dc_request) {
        gpiod_line_request_release(dc_request);
        dc_request = NULL;
    }
    if (gpio_chip) {
        gpiod_chip_close(gpio_chip);
        gpio_chip = NULL;
    }
}

// Set DC pin value
int linux_gpiod_set_dc(int value) {
    if (!dc_request) {
        fprintf(stderr, "DC line not initialized\n");
        return -1;
    }
    
    enum gpiod_line_value gpio_value = value ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE;
    
    if (gpiod_line_request_set_value(dc_request, DC_PIN_OFFSET, gpio_value) < 0) {
        perror("Failed to set DC line value");
        return -1;
    }
    
    return 0;
}

// SPI communication function
int linux_spi_transfer(u8 *tx_buf, u8 *rx_buf, int len) {
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = len,
        .delay_usecs = 0,
        .speed_hz = spi_speed,
        .bits_per_word = spi_bits,
    };
    
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        perror("SPI transfer failed");
        return -1;
    }
    
    return 0;
}

// OLED write byte function (core communication function)
void OLED_WR_Byte(u8 dat, u8 cmd) {
    u8 tx_buf[2];
    
    // Set DC pin (command/data)
    if (cmd) {
        linux_gpiod_set_dc(1); // Data mode
    } else {
        linux_gpiod_set_dc(0); // Command mode
    }
    
    // Send data
    tx_buf[0] = dat;
    linux_spi_transfer(tx_buf, NULL, 1);
    
    // Short delay (optional)
    usleep(1);
}

// Linux OLED initialization
int linux_oled_init(void) {
    // Initialize libgpiod
    if (linux_gpiod_init() < 0) {
        fprintf(stderr, "Failed to initialize GPIO\n");
        return -1;
    }
    
    // Initialize SPI
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        perror("Failed to open SPI device");
        linux_gpiod_deinit();
        return -1;
    }
    
    // Set SPI mode
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &spi_mode) < 0) {
        perror("Failed to set SPI mode");
        close(spi_fd);
        linux_gpiod_deinit();
        return -1;
    }
    
    // Set SPI bits per word
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits) < 0) {
        perror("Failed to set SPI bits per word");
        close(spi_fd);
        linux_gpiod_deinit();
        return -1;
    }
    
    // Set SPI speed
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0) {
        perror("Failed to set SPI speed");
        close(spi_fd);
        linux_gpiod_deinit();
        return -1;
    }
    
    return 0;
}

// Linux OLED cleanup
void linux_oled_deinit(void) {
    if (spi_fd >= 0) {
        close(spi_fd);
        spi_fd = -1;
    }
    
    linux_gpiod_deinit();
}

// The following functions maintain the same implementation as STM32 version, only hardware-related parts are modified

void OLED_ColorTurn(u8 i) {
    if(i==0) {
        OLED_WR_Byte(0xA6, OLED_CMD); // Normal display
    }
    if(i==1) {
        OLED_WR_Byte(0xA7, OLED_CMD); // Inverted display
    }
}

void OLED_DisplayTurn(u8 i) {
    if(i==0) {
        OLED_WR_Byte(0xC8, OLED_CMD); // Normal display
        OLED_WR_Byte(0xA1, OLED_CMD);
    }
    if(i==1) {
        OLED_WR_Byte(0xC0, OLED_CMD); // Rotated display
        OLED_WR_Byte(0xA0, OLED_CMD);
    }
}

void OLED_DisPlay_On(void) {
    OLED_WR_Byte(0x8D, OLED_CMD); // Charge pump enable
    OLED_WR_Byte(0x14, OLED_CMD); // Enable charge pump
    OLED_WR_Byte(0xAF, OLED_CMD); // Turn on screen
    OLED_Refresh(); // Ensure display refresh
}

void OLED_DisPlay_Off(void) {
    OLED_WR_Byte(0x8D, OLED_CMD); // Charge pump enable
    OLED_WR_Byte(0x10, OLED_CMD); // Disable charge pump
    OLED_WR_Byte(0xAF, OLED_CMD); // Turn off screen
}

void OLED_Refresh(void) {
    u8 i,n;
    for(i=0;i<OLED_PAGES;i++) {
        OLED_WR_Byte(0xb0+i, OLED_CMD); // Set page start address
        OLED_WR_Byte(0x00, OLED_CMD);   // Set column start address low 4 bits
        OLED_WR_Byte(0x10, OLED_CMD);   // Set column start address high 4 bits
        for(n=0;n<OLED_WIDTH;n++)
            OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
    }
}

void OLED_Clear(void) {
    u8 i,n;
    for(i=0;i<OLED_PAGES;i++) {
        for(n=0;n<OLED_WIDTH;n++) {
            OLED_GRAM[n][i]=0; // Clear display buffer
        }
    }
    OLED_Refresh(); // Refresh display
}

void OLED_DrawPoint(u8 x,u8 y) {
    u8 i,m,n;
    i=y/8;
    m=y%8;
    n=1<<m;
    OLED_GRAM[x][i]|=n;
}

void OLED_ClearPoint(u8 x,u8 y) {
    u8 i,m,n;
    i=y/8;
    m=y%8;
    n=1<<m;
    OLED_GRAM[x][i]=~OLED_GRAM[x][i];
    OLED_GRAM[x][i]|=n;
    OLED_GRAM[x][i]=~OLED_GRAM[x][i];
}

void OLED_DrawLine(u8 x1,u8 y1,u8 x2,u8 y2) {
    u8 i,k,k1,k2,y0;
    if((x1<0)||(x2>OLED_WIDTH)||(y1<0)||(y2>OLED_HEIGHT)||(x1>x2)||(y1>y2))return;
    if(x1==x2) { // Vertical line
        for(i=0;i<(y2-y1);i++) {
            OLED_DrawPoint(x1,y1+i);
        }
    }
    else if(y1==y2) { // Horizontal line
        for(i=0;i<(x2-x1);i++) {
            OLED_DrawPoint(x1+i,y1);
        }
    }
    else { // Diagonal line
        k1=y2-y1;
        k2=x2-x1;
        k=k1*10/k2;
        for(i=0;i<(x2-x1);i++) {
            OLED_DrawPoint(x1+i,y1+i*k/10);
        }
    }
}

void OLED_DrawCircle(u8 x,u8 y,u8 r) {
    int a, b,num;
    a = 0;
    b = r;
    while(2 * b * b >= r * r) { 
        OLED_DrawPoint(x + a, y - b);
        OLED_DrawPoint(x - a, y - b);
        OLED_DrawPoint(x - a, y + b);
        OLED_DrawPoint(x + a, y + b);
 
        OLED_DrawPoint(x + b, y + a);
        OLED_DrawPoint(x + b, y - a);
        OLED_DrawPoint(x - b, y - a);
        OLED_DrawPoint(x - b, y + a);
        
        a++;
        num = (a * a + b * b) - r*r; // Calculate distance from drawn point to circle center
        if(num > 0) {
            b--;
            a--;
        }
    }
}

void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1) {
    u8 i,m,temp,size2,chr1;
    u8 y0=y;
    size2=(size1/8+((size1%8)?1:0))*(size1/2);  // Get number of bytes occupied by font character bitmap
    chr1=chr-' ';  // Calculate offset value
    for(i=0;i<size2;i++) {
        if(size1==12)
            {temp=asc2_1206[chr1][i];} // Use 1206 font
        else if(size1==16)
            {temp=asc2_1608[chr1][i];} // Use 1608 font
        else if(size1==24)
            {temp=asc2_2412[chr1][i];} // Use 2412 font
        else return;
        for(m=0;m<8;m++) {           // Write data
            if(temp&0x80)OLED_DrawPoint(x,y);
            else OLED_ClearPoint(x,y);
            temp<<=1;
            y++;
            if((y-y0)==size1) {
                y=y0;
                x++;
                break;
            }
        }
    }
}

void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 size1) {
    while((*chr>=' ')&&(*chr<='~')) { // Check if it's an invalid character!
        OLED_ShowChar(x,y,*chr,size1);
        x+=size1/2;
        if(x>OLED_WIDTH-size1) { // Line break
            x=0;
            y+=2;
        }
        chr++;
    }
}

u32 OLED_Pow(u8 m,u8 n) {
    u32 result=1;
    while(n--) {
        result*=m;
    }
    return result;
}

void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size1) {
    u8 t,temp;
    for(t=0;t<len;t++) {
        temp=(num/OLED_Pow(10,len-t-1))%10;
        if(temp==0) {
            OLED_ShowChar(x+(size1/2)*t,y,'0',size1);
        }
        else {
            OLED_ShowChar(x+(size1/2)*t,y,temp+'0',size1);
        }
    }
}

void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size_one) {
    u8 i,m,n=0,temp,chr1;
    u8 x0=x,y0=y;
    u8 size3=size_one/8;
    while(size3--) {
        chr1=num*size_one/8+n;
        n++;
        for(i=0;i<size_one;i++) {
            if(size_one==16)
                {temp=Hzk1[chr1][i];} // Use 16*16 font
            else if(size_one==24)
                {temp=Hzk2[chr1][i];} // Use 24*24 font
            else if(size_one==32)       
                {temp=Hzk3[chr1][i];} // Use 32*32 font
            else if(size_one==64)
                {temp=Hzk4[chr1][i];} // Use 64*64 font
            else return;
            for(m=0;m<8;m++) {
                if(temp&0x01)OLED_DrawPoint(x,y);
                else OLED_ClearPoint(x,y);
                temp>>=1;
                y++;
            }
            x++;
            if((x-x0)==size_one) {
                x=x0;
                y0=y0+8;
            }
            y=y0;
        }
    }
}

void OLED_ScrollDisplay(u8 num,u8 space) {
    u8 i,n,t=0,m=0,r;
    while(1) {
        if(m==0) {
            OLED_ShowChinese(OLED_WIDTH,24,t,16); // Write a Chinese character to OLED_GRAM[][] buffer
            t++;
        }
        if(t==num) {
            for(r=0;r<16*space;r++) {      // Display delay
                for(i=0;i<OLED_WIDTH;i++) {
                    for(n=0;n<OLED_PAGES;n++) {
                        OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
                    }
                }
                OLED_Refresh();
            }
            t=0;
        }
        m++;
        if(m==16){m=0;}
        for(i=0;i<OLED_WIDTH;i++) {   // Implement scrolling
            for(n=0;n<OLED_PAGES;n++) {
                OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
            }
        }
        OLED_Refresh();
    }
}

void OLED_WR_BP(u8 x,u8 y) {
    OLED_WR_Byte(0xb0+y,OLED_CMD); // Set page start address
    OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
    OLED_WR_Byte((x&0x0f),OLED_CMD);
}

void OLED_ShowPicture(u8 x0,u8 y0,u8 x1,u8 y1,u8 BMP[]) {
    u32 j=0;
    u8 x=0,y=0;
    if(y%8==0)y=0;
    else y+=1;
    for(y=y0;y<y1;y++) {
        OLED_WR_BP(x0,y);
        for(x=x0;x<x1;x++) {
            OLED_WR_Byte(BMP[j],OLED_DATA);
            j++;
        }
    }
}

// OLED initialization function (maintain compatibility with STM32 version)
int OLED_Init(void) {
    // Initialize Linux hardware
    if (linux_oled_init() < 0) {
        fprintf(stderr, "OLED initialization failed\n");
        return -1;
    }
    
    // Note: RES pin is fixed high, no software reset needed
    
    // OLED initialization sequence
    OLED_WR_Byte(0xAE,OLED_CMD); //--turn off oled panel
    OLED_WR_Byte(0x00,OLED_CMD); //---set low column address
    OLED_WR_Byte(0x10,OLED_CMD); //---set high column address
    OLED_WR_Byte(0x40,OLED_CMD); //--set start line address Set Mapping RAM Display Start Line (0x00~0x3F)
    OLED_WR_Byte(0x81,OLED_CMD); //--set contrast control register
    OLED_WR_Byte(0xCF,OLED_CMD); // Set SEG Output Current Brightness
    OLED_WR_Byte(0xA1,OLED_CMD); //--Set SEG/Column Mapping     0xa0 flip horizontally, 0xa1 normal
    OLED_WR_Byte(0xC8,OLED_CMD); //Set COM/Row Scan Direction   0xc0 flip vertically, 0xc8 normal
    OLED_WR_Byte(0xA6,OLED_CMD); //--set normal display
    OLED_WR_Byte(0xA8,OLED_CMD); //--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3f,OLED_CMD); //--1/64 duty
    OLED_WR_Byte(0xD3,OLED_CMD); //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    OLED_WR_Byte(0x00,OLED_CMD); //-not offset
    OLED_WR_Byte(0xd5,OLED_CMD); //--set display clock divide ratio/oscillator frequency
    OLED_WR_Byte(0x80,OLED_CMD); //--set divide ratio, Set Clock as 100 Frames/Sec
    OLED_WR_Byte(0xD9,OLED_CMD); //--set pre-charge period
    OLED_WR_Byte(0xF1,OLED_CMD); //Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    OLED_WR_Byte(0xDA,OLED_CMD); //--set com pins hardware configuration
    OLED_WR_Byte(0x12,OLED_CMD);
    OLED_WR_Byte(0xDB,OLED_CMD); //--set vcomh
    OLED_WR_Byte(0x40,OLED_CMD); //Set VCOM Deselect Level
    OLED_WR_Byte(0x20,OLED_CMD); //-Set Page Addressing Mode (0x00/0x01/0x02)
    OLED_WR_Byte(0x02,OLED_CMD); //
    OLED_WR_Byte(0x8D,OLED_CMD); //--set Charge Pump enable/disable
    OLED_WR_Byte(0x14,OLED_CMD); //--set(0x10) disable
    OLED_WR_Byte(0xA4,OLED_CMD); // Disable Entire Display On (0xa4/0xa5)
    OLED_WR_Byte(0xA6,OLED_CMD); // Disable Inverse Display On (0xa6/a7) 
    OLED_WR_Byte(0xAF,OLED_CMD);
    OLED_Clear();
    OLED_DisPlay_On(); // Ensure screen is turned on
    return 0;
}