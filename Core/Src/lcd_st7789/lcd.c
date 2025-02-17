#include "lcd.h"
#include "stdlib.h"
#include "lcdfont.h"
#include "spi.h"

//#define ROTATE
uint16_t BACK_COLOR=WHITE, POINT_COLOR=BLACK;   //背景色，画笔色

uint8_t LcdDataBuf[LCD_W*LCD_W];
//uint8_t *LcdDataBuf = (uint8_t *)0x10000000;

void LCD_Writ_Bus(char dat)   //串行数据写入
{	
	HAL_SPI_Transmit(&hspi1,(uint8_t *)&dat,1,10);
	
}

void LCD_WR_DATA8(char da) //发送数据-8位参数
{
    LCD_DC_Set();
	LCD_Writ_Bus(da);  
}  
 void LCD_WR_DATA(int da)
{
    LCD_DC_Set();
	LCD_Writ_Bus(da>>8);
    LCD_Writ_Bus(da);
}	  
void LCD_WR_REG(char da)	 
{
    LCD_DC_Clr();
	LCD_Writ_Bus(da);
}
 void LCD_WR_REG_DATA(int reg,int da)
{
    LCD_WR_REG(reg);
	LCD_WR_DATA(da);
}

void Address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
{ 
	LCD_WR_REG(0x2a);
    LCD_WR_DATA8(x1>>8);
    LCD_WR_DATA8(x1);
    LCD_WR_DATA8(x2>>8);
    LCD_WR_DATA8(x2);
  
    LCD_WR_REG(0x2b);
    LCD_WR_DATA8(y1>>8);
    LCD_WR_DATA8(y1);
    LCD_WR_DATA8(y2>>8);
    LCD_WR_DATA8(y2);

    LCD_WR_REG(0x2C);
}

void Lcd_Init(void)
{
	LCD_CS_Set();
	LCD_RST_Clr();
	HAL_Delay(100);
	LCD_RST_Set();
	HAL_Delay(20);
	LCD_BLK_Set();

//************* Start Initial Sequence **********// 
	LCD_WR_REG(0x36);
#ifndef ROTATE
	LCD_WR_DATA8(0x00);//正常显示

#else
	LCD_WR_DATA8(0x70);//旋转90度
#endif
	LCD_WR_REG(0x3A);
	LCD_WR_DATA8(0x05);

	LCD_WR_REG(0xB2);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x33);

	LCD_WR_REG(0xB7);
	LCD_WR_DATA8(0x35);

	LCD_WR_REG(0xBB);
	LCD_WR_DATA8(0x19);

	LCD_WR_REG(0xC0);
	LCD_WR_DATA8(0x2C);

	LCD_WR_REG(0xC2);
	LCD_WR_DATA8(0x01);

	LCD_WR_REG(0xC3);
	LCD_WR_DATA8(0x12);

	LCD_WR_REG(0xC4);
	LCD_WR_DATA8(0x20);

	LCD_WR_REG(0xC6);
	LCD_WR_DATA8(0x0F);

	LCD_WR_REG(0xD0);
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1);

	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2B);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x54);
	LCD_WR_DATA8(0x4C);
	LCD_WR_DATA8(0x18);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x23);

	LCD_WR_REG(0xE1);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2C);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x44);
	LCD_WR_DATA8(0x51);
	LCD_WR_DATA8(0x2F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x20);
	LCD_WR_DATA8(0x23);

	LCD_WR_REG(0x21);

	LCD_WR_REG(0x11);
	HAL_Delay(20);

	LCD_WR_REG(0x29);
 
} 

//清屏函数
//Color:要清屏的填充色
//void LCD_Clear(uint16_t Color)
//{
//	uint16_t i,j;
//	Address_set(0,0,LCD_W-1,LCD_H-1);
//    for(i=0;i<LCD_W;i++)
//	 {
//	  for (j=0;j<LCD_H;j++)
//	   	{
//        	LCD_WR_DATA(Color);	 			 
//	    }

//	  }
//}

void LCD_Clear(uint16_t Color)
{
	uint16_t i,j;
	
/*	Address_set(0,0,LCD_W-1,LCD_H-1);
	LCD_DC_Set();
	for (j=0;j<LCD_H;j++)
	{
		LcdDataBuf[j+j] = Color>>8;
		LcdDataBuf[j+j+1] = Color;
	}
	 for(i=0;i<LCD_W;i++)
	 {
		LCD_DC_Set();
		HAL_SPI_Transmit(&hspi1,LcdDataBuf,LCD_H*2,1000);
	 }*/


	Address_set(0,0,LCD_W-1,LCD_H-1);
	LCD_DC_Set();
	for (j=0;j<LCD_H * LCD_W;j+=2)
	{
		LcdDataBuf[j] = Color>>8;
		LcdDataBuf[j+1] = Color;
	}
		LCD_DC_Set();
		HAL_SPI_Transmit(&hspi1,LcdDataBuf,LCD_H * LCD_W,1000);
		HAL_SPI_Transmit(&hspi1,LcdDataBuf,LCD_H * LCD_W,1000);
}


//在指定位置显示一个汉字(32*33大小)
//dcolor为内容颜色，gbcolor为背静颜色
void showhanzi(unsigned int x,unsigned int y,unsigned char index)	
{  
	unsigned char i,j;
	unsigned char *temp=hanzi;    
    Address_set(x,y,x+31,y+31); //设置区域      
	temp+=index*128;	
	for(j=0;j<128;j++)
	{
		for(i=0;i<8;i++)
		{ 		     
		 	if((*temp&(1<<i))!=0)
			{
				LCD_WR_DATA(POINT_COLOR);
			} 
			else
			{
				LCD_WR_DATA(BACK_COLOR);
			}   
		}
		temp++;
	 }
}
//画点
//POINT_COLOR:此点的颜色
void LCD_DrawPoint(uint16_t x,uint16_t y)
{
	Address_set(x,y,x,y);//设置光标位置 
	LCD_WR_DATA(POINT_COLOR); 	    
} 	 
//画一个大点
//POINT_COLOR:此点的颜色
void LCD_DrawPoint_big(uint16_t x,uint16_t y)
{
	LCD_Fill(x-1,y-1,x+1,y+1,POINT_COLOR);
} 
//在指定区域内填充指定颜色
//区域大小:
//  (xend-xsta)*(yend-ysta)
void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{          
	uint16_t i,j;
	Address_set(xsta,ysta,xend,yend);      //设置光标位置 
	for(i=ysta;i<=yend;i++)
	{													   	 	
		for(j=xsta;j<=xend;j++)LCD_WR_DATA(color);//设置光标位置 	    
	} 					  	    
}  
//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t t;
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		LCD_DrawPoint(uRow,uCol);//画点 
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
//画矩形
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}
//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判断下个点位置的标志
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a);             //3           
		LCD_DrawPoint(x0+b,y0-a);             //0           
		LCD_DrawPoint(x0-a,y0+b);             //1       
		LCD_DrawPoint(x0-b,y0-a);             //7           
		LCD_DrawPoint(x0-a,y0-b);             //2             
		LCD_DrawPoint(x0+b,y0+a);             //4               
		LCD_DrawPoint(x0+a,y0-b);             //5
		LCD_DrawPoint(x0+a,y0+b);             //6 
		LCD_DrawPoint(x0-b,y0+a);             
		a++;
		//使用Bresenham算法画圆     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 
		LCD_DrawPoint(x0+a,y0+b);
	}
} 
//在指定位置显示一个字符

//num:要显示的字符:" "--->"~"
//mode:叠加方式(1)还是非叠加方式(0)
//在指定位置显示一个字符

//num:要显示的字符:" "--->"~"

//mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t mode)
{
	uint16_t index =0;
    uint8_t temp;
    uint8_t pos,t;
	uint16_t x0=x;
	uint16_t colortemp=POINT_COLOR;
    if(x>LCD_W-16||y>LCD_H-16)return;	    
	//设置窗口		   
	num=num-' ';//得到偏移后的值
	Address_set(x,y,x+8-1,y+16-1);      //设置光标位置 
	if(!mode) //非叠加方式
	{

		for(pos=0;pos<16;pos++)
		{ 
			temp=asc2_1608[(uint16_t)num*16+pos];		 //调用1608字体
			for(t=0;t<8;t++)
		    {                 
		        if(temp&0x01)POINT_COLOR=colortemp;
				else POINT_COLOR=BACK_COLOR;
				//LCD_WR_DATA(POINT_COLOR);
				temp>>=1; 
				x++;
				LcdDataBuf[index+index] = POINT_COLOR>>8;
				LcdDataBuf[index+index+1] = POINT_COLOR;
				index++;
		    }
			x=x0;
			y++;
		}
		LCD_DC_Set();
		HAL_SPI_Transmit(&hspi1,(uint8_t *)LcdDataBuf,256,100);
	}else//叠加方式
	{
		for(pos=0;pos<16;pos++)
		{
		    temp=asc2_1608[(uint16_t)num*16+pos];		 //调用1608字体
			for(t=0;t<8;t++)
		    {                 
		        if(temp&0x01)LCD_DrawPoint(x+t,y+pos);//画一个点     
		        temp>>=1; 
		    }
		}
	}
	POINT_COLOR=colortemp;	    	   	 	  
}   
//m^n函数
u32 mypow(uint8_t m,uint8_t n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}			 
//显示2个数字
//x,y :起点坐标	 
//len :数字的位数
//color:颜色
//num:数值(0~4294967295);	
void LCD_ShowNum(uint16_t x,uint16_t y,u32 num,uint8_t len)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;
	num=(uint16_t)num;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+8*t,y,' ',0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+8*t,y,temp+48,0); 
	}
} 
//显示2个数字
//x,y:起点坐标
//num:数值(0~99);	 
void LCD_Show2Num(uint16_t x,uint16_t y,uint16_t num,uint8_t len)
{         	
	uint8_t t,temp;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
	 	LCD_ShowChar(x+8*t,y,temp+'0',1); 
	}
} 
//显示字符串
//x,y:起点坐标  
//*p:字符串起始地址
//用16字体
void LCD_ShowString(uint16_t x,uint16_t y,const uint8_t *p)
{         
    while(*p!='\0')
    {
    	if(*p == '\n')
    	{
    		x=0;y+=16;
    		p++;
    		continue;

    	}else if(*p == '\r' && *(p+1) == '\n')
    	{
    		x=0;y+=16;
    		p+=2;
    		continue;
    	}
    	else
    	{
			if(x>LCD_W-16){x=0;y+=16;}
			if(y>LCD_H-16){y=x=0;LCD_Clear(RED);}
    	}
        LCD_ShowChar(x,y,*p,0);
        x+=8;
        p++;
    }  
}

void LCD_ShowImg(uint8_t  *img)
{
	Address_set(0,0,LCD_W-1,LCD_H-1);
	LCD_DC_Set();
  HAL_SPI_Transmit(&hspi1,img,LCD_W*LCD_W,1000);
  HAL_SPI_Transmit(&hspi1,img+LCD_W*LCD_W,LCD_W*LCD_W,1000);

}
