#ifndef __OLED_H
#define __OLED_H			  	 
#include "sys.h"
 /**************************************************************************
作者：平衡小车之家 
淘宝店铺：http://shop114407458.taobao.com/
**************************************************************************/
//-----------------OLED端口定义---------------- 
#define OLED_RST_Clr() PBout(3)=0   //RST
#define OLED_RST_Set() PBout(3)=1   //RST

#define OLED_RS_Clr() PAout(15)=0    //DC
#define OLED_RS_Set() PAout(15)=1    //DC

#define OLED_SCLK_Clr()  PBout(5)=0  //SCL
#define OLED_SCLK_Set()  PBout(5)=1   //SCL

#define OLED_SDIN_Clr()  PBout(4)=0   //SDA
#define OLED_SDIN_Set()  PBout(4)=1   //SDA

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据
//OLED控制用函数
void OLED_WR_Byte(u8 dat,u8 cmd);	    
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Refresh_Gram(void);		   				   		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode);
//void OLED_ShowNumber(int x,int y,int num,int len,int size);
void OLED_ShowFloat(u8 x, u8 y, float num, u8 len, u8 decimal_digits, u8 size);
void OLED_ShowNumber(u8 x, u8 y, long long num, u8 len, u8 size);
void OLED_ShowString(u8 x,u8 y,const u8 *p);	 
void OLED_ShowChinese(u8 x,u8 y,u16 chr,u8 mode);
void OLED_ShowChinese_12(u8 x,u8 y,u16 chr,u8 mode);

/* 支持反白显示 */
//void OLED_ShowNumber(u8 x,u8 y,u32 num,u8 len,u8 mode);
//void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 mode);
#endif  
	 
