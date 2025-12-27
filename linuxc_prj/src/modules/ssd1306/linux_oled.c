#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <errno.h>
#include <gpiod.h>  // libgpiod头文件

#include "linux_oled.h"

// 包含字体数据
#include "oledfont.h"

// 全局变量
u8 OLED_GRAM[OLED_WIDTH][OLED_PAGES];

// SPI文件描述符
static int spi_fd = -1;

// libgpiod相关变量
static struct gpiod_chip *gpio_chip = NULL;
static struct gpiod_line *dc_line = NULL;

// SPI模式设置
static uint8_t spi_mode = SPI_MODE_0;
static uint8_t spi_bits = 8;
static uint32_t spi_speed = 1000000; // 1MHz

// libgpiod初始化
int linux_gpiod_init(void) {
    // 打开GPIO芯片
    gpio_chip = gpiod_chip_open(GPIO_CHIP);
    if (!gpio_chip) {
        perror("Failed to open GPIO chip");
        return -1;
    }
    
    // 获取DC引脚
    dc_line = gpiod_chip_get_line(gpio_chip, DC_PIN_OFFSET);
    if (!dc_line) {
        perror("Failed to get DC line");
        gpiod_chip_close(gpio_chip);
        gpio_chip = NULL;
        return -1;
    }
    
    // 设置DC引脚为输出
    if (gpiod_line_request_output(dc_line, "oled_dc", 0) < 0) {
        perror("Failed to set DC line as output");
        gpiod_line_release(dc_line);
        gpiod_chip_close(gpio_chip);
        dc_line = NULL;
        gpio_chip = NULL;
        return -1;
    }
    
    return 0;
}

// libgpiod清理
void linux_gpiod_deinit(void) {
    if (dc_line) {
        gpiod_line_release(dc_line);
        dc_line = NULL;
    }
    if (gpio_chip) {
        gpiod_chip_close(gpio_chip);
        gpio_chip = NULL;
    }
}

// 设置DC引脚值
int linux_gpiod_set_dc(int value) {
    if (!dc_line) {
        fprintf(stderr, "DC line not initialized\n");
        return -1;
    }
    
    if (gpiod_line_set_value(dc_line, value) < 0) {
        perror("Failed to set DC line value");
        return -1;
    }
    
    return 0;
}

// SPI通信函数
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

// OLED写字节函数（核心通信函数）
void OLED_WR_Byte(u8 dat, u8 cmd) {
    u8 tx_buf[2];
    
    // 设置DC引脚（命令/数据）
    if (cmd) {
        linux_gpiod_set_dc(1); // 数据模式
    } else {
        linux_gpiod_set_dc(0); // 命令模式
    }
    
    // 发送数据
    tx_buf[0] = dat;
    linux_spi_transfer(tx_buf, NULL, 1);
    
    // 短暂延时（可选）
    usleep(1);
}

// Linux OLED初始化
int linux_oled_init(void) {
    // 初始化libgpiod
    if (linux_gpiod_init() < 0) {
        fprintf(stderr, "Failed to initialize GPIO\n");
        return -1;
    }
    
    // 初始化SPI
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        perror("Failed to open SPI device");
        linux_gpiod_deinit();
        return -1;
    }
    
    // 设置SPI模式
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &spi_mode) < 0) {
        perror("Failed to set SPI mode");
        close(spi_fd);
        linux_gpiod_deinit();
        return -1;
    }
    
    // 设置SPI位宽
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits) < 0) {
        perror("Failed to set SPI bits per word");
        close(spi_fd);
        linux_gpiod_deinit();
        return -1;
    }
    
    // 设置SPI速度
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0) {
        perror("Failed to set SPI speed");
        close(spi_fd);
        linux_gpiod_deinit();
        return -1;
    }
    
    return 0;
}

// Linux OLED清理
void linux_oled_deinit(void) {
    if (spi_fd >= 0) {
        close(spi_fd);
        spi_fd = -1;
    }
    
    linux_gpiod_deinit();
}

// 以下函数保持与STM32版本相同的实现，只修改硬件相关部分

void OLED_ColorTurn(u8 i) {
    if(i==0) {
        OLED_WR_Byte(0xA6, OLED_CMD); // 正常显示
    }
    if(i==1) {
        OLED_WR_Byte(0xA7, OLED_CMD); // 反色显示
    }
}

void OLED_DisplayTurn(u8 i) {
    if(i==0) {
        OLED_WR_Byte(0xC8, OLED_CMD); // 正常显示
        OLED_WR_Byte(0xA1, OLED_CMD);
    }
    if(i==1) {
        OLED_WR_Byte(0xC0, OLED_CMD); // 旋转显示
        OLED_WR_Byte(0xA0, OLED_CMD);
    }
}

void OLED_DisPlay_On(void) {
    OLED_WR_Byte(0x8D, OLED_CMD); // 电荷泵使能
    OLED_WR_Byte(0x14, OLED_CMD); // 开启电荷泵
    OLED_WR_Byte(0xAF, OLED_CMD); // 开启屏幕
    OLED_Refresh(); // 确保显示刷新
}

void OLED_DisPlay_Off(void) {
    OLED_WR_Byte(0x8D, OLED_CMD); // 电荷泵使能
    OLED_WR_Byte(0x10, OLED_CMD); // 关闭电荷泵
    OLED_WR_Byte(0xAF, OLED_CMD); // 关闭屏幕
}

void OLED_Refresh(void) {
    u8 i,n;
    for(i=0;i<OLED_PAGES;i++) {
        OLED_WR_Byte(0xb0+i, OLED_CMD); // 设置页起始地址
        OLED_WR_Byte(0x00, OLED_CMD);   // 设置列起始地址低4位
        OLED_WR_Byte(0x10, OLED_CMD);   // 设置列起始地址高4位
        for(n=0;n<OLED_WIDTH;n++)
            OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
    }
}

void OLED_Clear(void) {
    u8 i,n;
    for(i=0;i<OLED_PAGES;i++) {
        for(n=0;n<OLED_WIDTH;n++) {
            OLED_GRAM[n][i]=0; // 清空显示缓存
        }
    }
    OLED_Refresh(); // 刷新显示
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
    if(x1==x2) { // 垂直线
        for(i=0;i<(y2-y1);i++) {
            OLED_DrawPoint(x1,y1+i);
        }
    }
    else if(y1==y2) { // 水平线
        for(i=0;i<(x2-x1);i++) {
            OLED_DrawPoint(x1+i,y1);
        }
    }
    else { // 斜线
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
        num = (a * a + b * b) - r*r; // 计算画的点到圆心的距离
        if(num > 0) {
            b--;
            a--;
        }
    }
}

void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1) {
    u8 i,m,temp,size2,chr1;
    u8 y0=y;
    size2=(size1/8+((size1%8)?1:0))*(size1/2);  // 得到字体一个字符对应点阵占字节数
    chr1=chr-' ';  // 计算偏移后值
    for(i=0;i<size2;i++) {
        if(size1==12)
            {temp=asc2_1206[chr1][i];} // 调用1206字体
        else if(size1==16)
            {temp=asc2_1608[chr1][i];} // 调用1608字体
        else if(size1==24)
            {temp=asc2_2412[chr1][i];} // 调用2412字体
        else return;
        for(m=0;m<8;m++) {           // 写入数据
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
    while((*chr>=' ')&&(*chr<='~')) { // 判断是不是非法字符!
        OLED_ShowChar(x,y,*chr,size1);
        x+=size1/2;
        if(x>OLED_WIDTH-size1) { // 换行
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
                {temp=Hzk1[chr1][i];} // 调用16*16字体
            else if(size_one==24)
                {temp=Hzk2[chr1][i];} // 调用24*24字体
            else if(size_one==32)       
                {temp=Hzk3[chr1][i];} // 调用32*32字体
            else if(size_one==64)
                {temp=Hzk4[chr1][i];} // 调用64*64字体
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
            OLED_ShowChinese(OLED_WIDTH,24,t,16); // 写入一个汉字到OLED_GRAM[][]缓存区
            t++;
        }
        if(t==num) {
            for(r=0;r<16*space;r++) {      // 显示延迟
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
        for(i=0;i<OLED_WIDTH;i++) {   // 实现滚动
            for(n=0;n<OLED_PAGES;n++) {
                OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
            }
        }
        OLED_Refresh();
    }
}

void OLED_WR_BP(u8 x,u8 y) {
    OLED_WR_Byte(0xb0+y,OLED_CMD); // 设置页起始地址
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

// OLED初始化函数（保持与STM32版本兼容）
int OLED_Init(void) {
    // 初始化Linux硬件
    if (linux_oled_init() < 0) {
        fprintf(stderr, "OLED initialization failed\n");
        return -1;
    }
    
    // 注意：RES引脚固定接高，不需要软件复位
    
    // OLED初始化序列
    OLED_WR_Byte(0xAE,OLED_CMD); //--turn off oled panel
    OLED_WR_Byte(0x00,OLED_CMD); //---set low column address
    OLED_WR_Byte(0x10,OLED_CMD); //---set high column address
    OLED_WR_Byte(0x40,OLED_CMD); //--set start line address Set Mapping RAM Display Start Line (0x00~0x3F)
    OLED_WR_Byte(0x81,OLED_CMD); //--set contrast control register
    OLED_WR_Byte(0xCF,OLED_CMD); // Set SEG Output Current Brightness
    OLED_WR_Byte(0xA1,OLED_CMD); //--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
    OLED_WR_Byte(0xC8,OLED_CMD); //Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
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
    OLED_DisPlay_On(); // 确保屏幕开启
    return 0;
}