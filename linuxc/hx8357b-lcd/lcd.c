#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "font.h" 
#include "lcd.h"
#include "spi.h"
	   
_lcd_dev lcddev;
uint16_t POINT_COLOR = 0xFFFF,BACK_COLOR = 0x0000;  	 

#define HARD_SPI
#define IN_LEN	1024
#define OUT_LEN	(IN_LEN*9)/16

#ifdef HARD_SPI
int spi_fd=0;

//default MSB
typedef union {
    struct {
		unsigned int reserved : 7;  // reserved 7
        unsigned int byte : 8;      // 1byte data
		unsigned int bit1 : 1;      // 1bit DC

    } fields;
    unsigned int rawData;
} LCDUnion;

void pack_9bit_to_16bit(const uint16_t inputData[], uint16_t outputData[], size_t inputSize) 
{
    // 输入数据长度应该是16
    if (inputSize != IN_LEN) {
        return;
    }

    int outputIndex = 0;
    int bitPos = 0;
    uint32_t buffer = 0;

    for (unsigned int i = 0; i < inputSize; i++) {
        // 将高9位数据取出并移动到缓冲区的正确位置
        buffer |= ((inputData[i] >> 7) & 0x01FF) << (23 - bitPos);

        bitPos += 9; // 更新bitPos

        // 如果缓冲区有足够的16位数据
        if (bitPos >= 16) {
            // 提取16位数据存入输出数组
            outputData[outputIndex++] = (buffer >> 16) & 0xFFFF;
            // 保留剩余位
            buffer <<= 16;
            bitPos -= 16;
        }
    }
}

int write_buffer_num=0;
uint16_t iData[IN_LEN]={0};
uint16_t oData[OUT_LEN]={0};

void TFT_SPI_Write_buffer(uint16_t u)
{
	iData[write_buffer_num++] = u;

	if(write_buffer_num == IN_LEN) {	
		pack_9bit_to_16bit(iData, oData, IN_LEN);
		spi_transfer(spi_fd, (uint8_t *)&oData, NULL, OUT_LEN*2);
		write_buffer_num = 0;
		memset(iData, 0, sizeof(uint16_t)*IN_LEN);
		memset(oData, 0, sizeof(uint16_t)*OUT_LEN);
	}
}	

void LCD_WR_REG(uint8_t CMD)	  
{ 
	LCDUnion tx;
	
	tx.fields.bit1 = 0;
	tx.fields.byte = CMD;
	tx.fields.reserved = 0;
	
	//spi_transfer(spi_fd, (uint8_t *)&tx.rawData, NULL, 2);
	TFT_SPI_Write_buffer(tx.rawData);
}

void LCD_WR_DATA(uint8_t DATA)	  
{ 	 
	LCDUnion tx;
	
	tx.fields.bit1 = 1;
	tx.fields.byte = DATA;
	tx.fields.reserved = 0;
	
	//spi_transfer(spi_fd, (uint8_t *)&tx.rawData, NULL, 2);
	TFT_SPI_Write_buffer(tx.rawData);
}
#else
	
void TFT_SPI_Write_Byte(uint8_t num)    
{  
	uint8_t count=0;  
	
	for(count=0;count<8;count++)  
	{ 
		CLR_CLK();	 
		if(num&0x80)SET_SDA(); 
		else CLR_SDA();   
		num<<=1;    
		SET_CLK();   		
	} 			    
}

void LCD_WR_REG(uint8_t CMD)	  
{ 
	CLR_CS(); 
	CLR_CLK();
	CLR_SDA();
	SET_CLK(); 
	TFT_SPI_Write_Byte(CMD);
	SET_CS();         
}

void LCD_WR_DATA(uint8_t DATA)	  
{ 	 
	CLR_CS();
	CLR_CLK();
	SET_SDA();
	SET_CLK(); 
	TFT_SPI_Write_Byte(DATA);
	SET_CS();   
}
#endif
  
void LCD_WR_DATA16(uint16_t DATA)	  
{ 	 
	LCD_WR_DATA(DATA>>8);
	LCD_WR_DATA(DATA);
}	 

void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.wramcmd);
}	 

void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_SetWindows(x,y,x+1,y+1);
	LCD_WriteRAM_Prepare();
	LCD_WR_DATA16(color); 
}

void LCD_Clear(uint16_t Color)
{
	uint32_t index=0;      
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1); 
	for(index=0;index<lcddev.width*lcddev.height;index++)
	{
		LCD_WR_DATA(Color>>8);    
		LCD_WR_DATA(Color); 
	}
} 

void LCD_GPIOInit(void)
{
	printf("LCD_GPIOInit...\n");
	
	#ifdef HARD_SPI
		spi_fd = spi_init(SPI_DEVICE, SPI_MODE, SPI_BITS_PER_WORD, SPI_SPEED);
	#else
		setup_gpio();
		cs_gpio();
	#endif
	
	res_gpio();
	blacklight_gpio();
}

void LCD_Init(void)
{
		printf("LCD_Init...\n");
		
		LCD_GPIOInit();
		LCD_RST(0);

		//************* Start Initial Sequence **********//		
		usleep(10000);
		LCD_RST(1);
		LCD_WR_REG(0x11);//Sleep Out 
		usleep(120000); 

		LCD_WR_REG(0xEE);//Set EQ 
		LCD_WR_DATA(0x02); 
		LCD_WR_DATA(0x01); 
		LCD_WR_DATA(0x02); 
		LCD_WR_DATA(0x01); 

		LCD_WR_REG(0xED);//Set DIR TIM 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x9A); 
		LCD_WR_DATA(0x9A); 
		LCD_WR_DATA(0x9B); 
		LCD_WR_DATA(0x9B); 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0xAE); 
		LCD_WR_DATA(0xAE); 
		LCD_WR_DATA(0x01); 
		LCD_WR_DATA(0xA2); 
		LCD_WR_DATA(0x00); 

		LCD_WR_REG(0xB4);//Set RM, DM 
		LCD_WR_DATA(0x00);// 

		LCD_WR_REG(0xC0);  //Set Panel Driving
		LCD_WR_DATA(0x10); //REV SM GS
		LCD_WR_DATA(0x3B); // NL[5:0] 
		LCD_WR_DATA(0x00); //SCN[6:0] 
		LCD_WR_DATA(0x02); //NDL 0 PTS[2:0]
		LCD_WR_DATA(0x11); //PTG ISC[3:0] 

		LCD_WR_REG(0xC1);//
		LCD_WR_DATA(0x10);//ne inversion

		LCD_WR_REG(0xC8);//Set Gamma 
		LCD_WR_DATA(0x00);  //KP1,KP0
		LCD_WR_DATA(0x46);  //KP3,KP2
		LCD_WR_DATA(0x12);  //KP5,KP4
		LCD_WR_DATA(0x20);  //RP1,RP0
		LCD_WR_DATA(0x0c);  //VRP0  01
		LCD_WR_DATA(0x00);  //VRP1
		LCD_WR_DATA(0x56);  //KN1,KN0
		LCD_WR_DATA(0x12);  //KN3,KN2
		LCD_WR_DATA(0x67);  //KN5,KN4
		LCD_WR_DATA(0x02);  //RN1,RN0
		LCD_WR_DATA(0x00);  //VRN0
		LCD_WR_DATA(0x0c);  //VRN1  01

		LCD_WR_REG(0xD0);//Set Power 
		LCD_WR_DATA(0x44);//DDVDH :5.28
		LCD_WR_DATA(0x42); // BT VGH:15.84    VGL:-7.92
		LCD_WR_DATA(0x06);//VREG1  4.625V

		LCD_WR_REG(0xD1);//Set VCOM 
		LCD_WR_DATA(0x43); //VCOMH
		LCD_WR_DATA(0x16);

		LCD_WR_REG(0xD2); 
		LCD_WR_DATA(0x04); 
		LCD_WR_DATA(0x22); //12

		LCD_WR_REG(0xD3); 
		LCD_WR_DATA(0x04); 
		LCD_WR_DATA(0x12); 

		LCD_WR_REG(0xD4); 
		LCD_WR_DATA(0x07); 
		LCD_WR_DATA(0x12); 

		LCD_WR_REG(0xE9); //Set Panel
		LCD_WR_DATA(0x00);

		LCD_WR_REG(0xC5); //Set Frame rate
		LCD_WR_DATA(0x08);//61.51Hz
		
		LCD_WR_REG(0x36);
		LCD_WR_DATA(0x0a);
		
		LCD_WR_REG(0X3A);
		LCD_WR_DATA(0X55);

		usleep(120000);

		LCD_WR_REG(0x21);

		LCD_WR_REG(0x29); //Display On
		usleep(50000);

	LCD_SetParam();	 
	LCD_LED; 
	LCD_Clear(WHITE);
	
	printf("LCD_Init OK\n");
}
  		  
void LCD_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd)
{	
	if(lcddev.dir==1)
	{	
		LCD_WR_REG(lcddev.setxcmd);	
		LCD_WR_DATA(yStar>>8);	
		LCD_WR_DATA(yStar);
		LCD_WR_DATA(yEnd>>8);	
		LCD_WR_DATA(yEnd);
		LCD_WR_REG(lcddev.setycmd);	
		LCD_WR_DATA(xStar>>8);	
		LCD_WR_DATA(xStar);
		LCD_WR_DATA(xEnd>>8);	
		LCD_WR_DATA(xEnd);
	}
	else 
	{
		LCD_WR_REG(lcddev.setxcmd);
		LCD_WR_DATA(xStar>>8);
		LCD_WR_DATA(xStar);
		LCD_WR_DATA(xEnd>>8);
		LCD_WR_DATA(xEnd);

		LCD_WR_REG(lcddev.setycmd);
		LCD_WR_DATA(yStar>>8);
		LCD_WR_DATA(yStar);
		LCD_WR_DATA(yEnd>>8);
		LCD_WR_DATA(yEnd);
	} 
	  
	LCD_WriteRAM_Prepare();			
}   

void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{	  	    			
	LCD_SetWindows(Xpos,Ypos,Xpos+1,Ypos+1);         

} 

void LCD_SetParam(void)
{ 	
	lcddev.wramcmd=0x2C;
#if USE_HORIZONTAL==1  
	lcddev.dir=1;
	lcddev.width=480;
	lcddev.height=320;
	lcddev.setxcmd=0x2B;
	lcddev.setycmd=0x2A;
	LCD_WR_REG(0X036);
	LCD_WR_DATA(0X3b);
#else
	lcddev.dir=0;				 	 		
	lcddev.width=320;
	lcddev.height=480;
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;
	LCD_WR_REG(0X036);
	LCD_WR_DATA(0X0a);
#endif
}	  

void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
{  	
	uint16_t i,j;			
	uint16_t width=ex-sx+1;
	uint16_t height=ey-sy+1;
	LCD_SetWindows(sx,sy,ex-1,ey-1);
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		LCD_WR_DATA16(color);
	}
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);
}

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1;
	else if(delta_x==0)incx=0; 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x;
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )
	{  
		LCD_DrawPoint(uRow,uCol,POINT_COLOR); 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
} 

void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}  
 
void LCD_DrawFillRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_Fill(x1,y1,x2,y2,POINT_COLOR);

}
  
void _draw_circle_8(int xc, int yc, int x, int y, uint16_t c)
{
	LCD_DrawPoint(xc + x, yc + y, c);

	LCD_DrawPoint(xc - x, yc + y, c);

	LCD_DrawPoint(xc + x, yc - y, c);

	LCD_DrawPoint(xc - x, yc - y, c);

	LCD_DrawPoint(xc + y, yc + x, c);

	LCD_DrawPoint(xc - y, yc + x, c);

	LCD_DrawPoint(xc + y, yc - x, c);

	LCD_DrawPoint(xc - y, yc - x, c);
}

/* c : colar
*  fill : 0/1
*/
void LCD_circle(int xc, int yc,uint16_t c,int r, int fill)
{
	int x = 0, y = r, yi, d;

	d = 3 - 2 * r;


	if (fill) 
	{
		while (x <= y) {
			for (yi = x; yi <= y; yi++)
				_draw_circle_8(xc, yc, x, yi, c);

			if (d < 0) {
				d = d + 4 * x + 6;
			} else {
				d = d + 4 * (x - y) + 10;
				y--;
			}
			x++;
		}
	} else 
	{
		while (x <= y) {
			_draw_circle_8(xc, yc, x, y, c);
			if (d < 0) {
				d = d + 4 * x + 6;
			} else {
				d = d + 4 * (x - y) + 10;
				y--;
			}
			x++;
		}
	}
}
 
void LCD_ShowChar(uint16_t x,uint16_t y,uint16_t fc, uint16_t bc, uint8_t num,uint8_t size,uint8_t mode)
{  
    uint8_t temp;
    uint8_t pos,t;
	uint16_t colortemp=POINT_COLOR;      
		   
	num=num-' ';
	LCD_SetWindows(x,y,x+size/2-1,y+size-1);
	if(!mode)
	{
		
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=asc2_1206[num][pos];
			else temp=asc2_1608[num][pos];
			for(t=0;t<size/2;t++)
		    {                 
		      if(temp&0x01) LCD_WR_DATA16(fc); 
					else LCD_WR_DATA16(bc);
					temp>>=1; 
				
		    }
			
		}	
	}else
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=asc2_1206[num][pos];
			else temp=asc2_1608[num][pos];
			for(t=0;t<size/2;t++)
		    {   
				POINT_COLOR=fc;              
		        if(temp&0x01)LCD_DrawPoint(x+t,y+pos,POINT_COLOR);   
		        temp>>=1; 
		    }
		}
	}
	POINT_COLOR=colortemp;	
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);   	   	 	  
} 
	  
void LCD_ShowString(uint16_t x,uint16_t y,uint8_t size,char *p,uint8_t mode)
{         
    while((*p<='~')&&(*p>=' '))
    {   
		if(x>(lcddev.width-1)||y>(lcddev.height-1)) 
		return;     
        LCD_ShowChar(x,y,POINT_COLOR,BACK_COLOR,*p,size,mode);
        x+=size/2;
        p++;
    }  
} 

uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}
			 
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,POINT_COLOR,BACK_COLOR,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,POINT_COLOR,BACK_COLOR,temp+'0',size,0); 
	}
} 

void LCD_DrawFont16(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s,uint8_t mode)
{
	uint8_t i,j;
	uint16_t k;
	uint16_t HZnum;
	uint16_t x0=x;
	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);
	
			
	for (k=0;k<HZnum;k++) 
	{
	  if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
	  { 	LCD_SetWindows(x,y,x+16-1,y+16-1);
		    for(i=0;i<16*2;i++)
		    {
				for(j=0;j<8;j++)
		    	{	
					if(!mode)
					{
						if(tfont16[k].Msk[i]&(0x80>>j))	LCD_WR_DATA16(fc);
						else LCD_WR_DATA16(bc);
					}
					else
					{
						POINT_COLOR=fc;
						if(tfont16[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y,POINT_COLOR);
						x++;
						if((x-x0)==16)
						{
							x=x0;
							y++;
							break;
						}
					}

				}
				
			}
			
			
		}				  	
		continue;
	}

	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);  
} 

void LCD_DrawFont24(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s,uint8_t mode)
{
	uint8_t i,j;
	uint16_t k;
	uint16_t HZnum;
	uint16_t x0=x;
	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);
		
			for (k=0;k<HZnum;k++) 
			{
			  if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
			  { 	LCD_SetWindows(x,y,x+24-1,y+24-1);
				    for(i=0;i<24*3;i++)
				    {
							for(j=0;j<8;j++)
							{
								if(!mode)
								{
									if(tfont24[k].Msk[i]&(0x80>>j))	LCD_WR_DATA16(fc);
									else LCD_WR_DATA16(bc>>8);
								}
							else
							{
								POINT_COLOR=fc;
								if(tfont24[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y,POINT_COLOR);
								x++;
								if((x-x0)==24)
								{
									x=x0;
									y++;
									break;
								}
							}
						}
					}
					
					
				}				  	
				continue;
			}

	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1); 
}

void LCD_DrawFont32(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s,uint8_t mode)
{
	uint8_t i,j;
	uint16_t k;
	uint16_t HZnum;
	uint16_t x0=x;
	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);
	for (k=0;k<HZnum;k++) 
			{
			  if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
			  { 	LCD_SetWindows(x,y,x+32-1,y+32-1);
				    for(i=0;i<32*4;i++)
				    {
						for(j=0;j<8;j++)
				    	{
							if(!mode)
							{
								if(tfont32[k].Msk[i]&(0x80>>j))	LCD_WR_DATA16(fc);
								else LCD_WR_DATA16(bc);
							}
							else
							{
								POINT_COLOR=fc;
								if(tfont32[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y,POINT_COLOR);
								x++;
								if((x-x0)==32)
								{
									x=x0;
									y++;
									break;
								}
							}
						}
					}
					
					
				}				  	
				continue;
			}
	
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);  
} 
   	   		   
void Show_Str(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *str,uint8_t size,uint8_t mode)
{					
	uint16_t x0=x;							  	  
  	uint8_t bHz=0;
    while(*str!=0)
    { 
        if(!bHz)
        {
			if(x>(lcddev.width-size/2)||y>(lcddev.height-size)) 
			return; 
	        if(*str>0x80)bHz=1;
	        else
	        {          
		        if(*str==0x0D)
		        {         
		            y+=size;
					x=x0;
		            str++; 
		        }  
		        else
				{
					if(size>16)
					{  
					LCD_ShowChar(x,y,fc,bc,*str,16,mode);
					x+=8;
					}
					else
					{
					LCD_ShowChar(x,y,fc,bc,*str,size,mode);
					x+=size/2;
					}
				} 
				str++; 
		        
	        }
        }else
        {   
			if(x>(lcddev.width-size)||y>(lcddev.height-size)) 
			return;  
            bHz=0;    
			if(size==32)
			LCD_DrawFont32(x,y,fc,bc,str,mode);	 	
			else if(size==24)
			LCD_DrawFont24(x,y,fc,bc,str,mode);	
			else
			LCD_DrawFont16(x,y,fc,bc,str,mode);
				
	        str+=2; 
	        x+=size;	    
        }						 
    }   
}
  
void LCD_StrCenter(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *str,uint8_t size,uint8_t mode)
{
	uint16_t len=strlen((const char *)str);
	uint16_t x1=(lcddev.width-len*8)/2;
	Show_Str(x+x1,y,fc,bc,str,size,mode);
} 
   
void LCD_Drawbmp16(uint16_t x,uint16_t y,const unsigned char *p)
{
  	int i; 
	unsigned char picH,picL; 
	LCD_SetWindows(x,y,x+40-1,y+40-1);
    for(i=0;i<40*40;i++)
	{	
	 	picL=*(p+i*2);
		picH=*(p+i*2+1);				
		LCD_WR_DATA16(picH<<8|picL);  	
	}	
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);

}

