/********************************************************************
*  
*  力矩有关的功能函数
*  功能：
*
********************************************************************/

#ifndef __APP_HMI_H_
#define __APP_HMI_H_


#include "config.h"



/////////////////////////////    LCD    /////////////////////////////////////
////   DW 迪文   说明： C--CMD, S--SET
#define LCD_WIDTH			320	//480
#define LCD_HIGH			240	//272
#define LCD_HCH				0xAA		// 发送给 LCD(迪文) 的帧头字符
#define LCD_LCH1			0xCC		// 发送给 LCD(迪文) 的帧尾字符 1
#define LCD_LCH2			0x33		// 发送给 LCD(迪文) 的帧尾字符 2
#define LCD_LCH3			0xC3		// 发送给 LCD(迪文) 的帧尾字符 3
#define LCD_LCH4			0x3C		// 发送给 LCD(迪文) 的帧尾字符 4

////  CMD  : S--Set, G--Get,  D--Disp,  C--Clr,  X--Xor反色，T--Cut
#define LCD_C_HANDS			0x40		// 握手指令
#define LCD_C_C_SCR			0x52		// 全屏清屏
#define LCD_C_S_BL			0x5F		// 打开背光
#define LCD_C_C_BL			0x5E		// 关闭背光

#define LCD_C_S_COL			0x40		// 设置当前调色板
#define LCD_C_S_DIS			0x41		// 设置字符显示间距
#define LCD_C_G_COL_F		0x43		// 取指定位置颜色到前景色
#define LCD_C_G_COL_B		0x42		// 取指定位置颜色到背景色

#define LCD_C_D_PIC			0x70		// 图片显示，显示保存在终端中的一幅全屏图片
#define LCD_C_D_ICON		0x97		// 从图标库中加载图标显示

#define LCD_C_S_CUR			0x44		// 设置光标
#define LCD_C_D_POT			0x51		// 置点，画点
#define LCD_C_C_POT			0x50		// 删除点
#define LCD_C_D_LINE		0x56		// 画线
#define LCD_C_C_LINE		0x5D		// 删除线

#define LCD_C_D_Rect		0x59		// 画矩形
#define LCD_C_C_Rect		0x69		// 删除矩形
#define LCD_C_D_FRect		0x5A		// 前景色 填充矩形，画实心矩形
#define LCD_C_C_FRect		0x5B		// 背景色 填充矩形，删除实心矩形
#define LCD_C_X_FRect		0x5C		// 指定区域反色显示，再次反色将复原

#define LCD_C_D_ARC			0x57		// 圆弧段显示
#define LCD_C_X_CYC			0x00		// 把指定的圆反色显示
#define LCD_C_D_CYC			0x01		// 用前景色显示圆
#define LCD_C_X_FCYC		0x02		// 把指定的圆域反色显示
#define LCD_C_D_FCYC		0x03		// 用前景色显示圆域

#define LCD_C_T_PICRECT		0x71		// 从指定PIC中剪切一个区域，黏贴到当前显示PIC指定区域


////  FONT  文本显示指令
#define LCD_F_8x8			0x53		// 8*8点阵ASCII字符串
#define LCD_F_16x16			0x54		// 16*16点阵ASCII字符串
#define LCD_F_32x32			0x55		// 32*32点阵ASCII字符串
#define LCD_F_12x12			0x6e		// 12*12点阵ASCII字符串
#define LCD_F_24x24			0x6f		// 24*24点阵ASCII字符串
#define FONT_8				LCD_F_8x8
#define FONT_16				LCD_F_16x16
#define FONT_32				LCD_F_32x32
#define FONT_12				LCD_F_12x12
#define FONT_24				LCD_F_24x24

/****************************************************************************************/
typedef struct
{
	uchar  mCmd;			// 指令
	ushort mFC;				// 前景色
	ushort mBC;				// 背景色
	uchar  mDisX;			// 字符显示间距 x
	uchar  mDisY;			// 字符显示间距 y
	uchar  mFont;			// 字符点阵
}*pLCDSets, mLCDSets;

/****************************************************************************************/
////////   LCD
extern mLCDSets 	gLCDSets;
extern unsigned char  gLCDSStr[80];		// 发送给 LCD 的字符串
extern unsigned char  gLCDRStr[80];		// 从 LCD 接收到的字符串
extern unsigned char  gLCDSLen ;		// 发送给 LCD 的字符数
extern unsigned char  gLCDRLen ;		// 从 LCD 接收到的字符数
extern unsigned char  gLCDCmd;			// 发给 LCD 的指令
extern long  gLCD_FC ;	   				// LCD前景色，默认 白色
extern long  gLCD_BC ;					// LCD背景色，默认 蓝色



/****************************************************************************************
*	  HMI 接口，串口屏  指令屏
*****************************************************************************************/
void LCD_Hands(void);
void LCD_CLR_SCR(void);
void LCD_CLR_SCR_BLK(void);
void LCD_SET_BL(uchar vBL);
void LCD_Disp_PIC(ushort vPicID);
void LCD_BK_On_IfWake(void);
void LCD_Set_Color(long vFC,long vBC);
void LCD_Set_FontDis(char vDX,char vDY);
void LCD_Get_Color(uchar vFCBC, ushort vX, ushort vY);
void LCD_Set_Cursor(uchar vOnOff,ushort vX, ushort vY, uchar vCurW, uchar vCurH);
void LCD_Disp_Point(uchar vOnOff, ushort vX, ushort vY);
void LCD_Disp_Line(uchar vOnOff, ushort vX, ushort vY, ushort vX1, ushort vY1);
void LCD_Disp_Rect(uchar vCmd, ushort vX, ushort vY, ushort vX1, ushort vY1);
void LCD_Disp_Cyc(uchar vCmd, ushort vX, ushort vY, uchar vR);
void LCD_Disp_Arc(uchar vCmd, ushort vX, ushort vY, ushort vR, ushort vAS, ushort vAE);

void LCD_Disp_Float100(uchar vFont, ushort vX, ushort vY, float vData);			// 显示百分号

void LCD_Disp_Txt(uchar vFont, ushort vX, ushort vY, uchar* vPS);
void LCD_Disp_TxtLen(uchar vFont, ushort vX, ushort vY, uchar* vPS,uchar vLen);
void LCD_Disp_Int(uchar vFont, ushort vX, ushort vY, long vData);
void LCD_Disp_Int2B(uchar vFont, ushort vX, ushort vY, long vData);
void LCD_Disp_Intx(uchar vFont, ushort vX, ushort vY, long vData, uchar vLen);
void LCD_Disp_Int4x(uchar vFont, ushort vX, ushort vY, long vData);
void LCD_Disp_Float(uchar vFont, ushort vX, ushort vY, float vData);
void LCD_Disp_FloatPX(uchar vFont, ushort vX, ushort vY, float vData, uchar vFX);
void LCD_Disp_Floatx(uchar vFont, ushort vX, ushort vY, float vData,uchar vLen);

void LCD_Cut_PICRect(ushort vAddr, ushort vPicID, ushort vX, ushort vY, 
		ushort vX1, ushort vY1, ushort vXD, ushort vYD);

/*********************************************************************************************
*	  DGUS 接口，组态屏
**********************************************************************************************/
#if 0
void LCD_GO_Page(ushort vPage);
void LCD_Set_KeyV(uchar vKey);
void LCD_Get_Page(ushort vPage);
void LCD_WR_Data2B(ushort vAddr, ushort vData);	   // 写的数据长度是字节
void LCD_Rd_Data(ushort vAddr, uchar vLen);		// 读的数据长度是字，2字节
void LCD_WR_Str(ushort vAddr, uchar* vStr, uchar vLen);	   // 写字符串
void LCD_WR_TxtByte(ushort vAddr, uchar vChar);	   // 写字符

void LCD_Draw_Clr(ushort vAddr);
void LCD_Draw_Point(ushort vAddr, ushort vX, ushort vY, ushort vColor);
void LCD_Draw_Line(ushort vAddr, ushort vX, ushort vY, ushort vX1, ushort vY1, ushort vColor);
//void LCD_Draw_Line(ushort vAddr, ushort vX, ushort vY, ushort vW, ushort vH, ushort vColor);
void LCD_Draw_Rect(ushort vAddr, ushort vX, ushort vY, ushort vX1, ushort vY1, ushort vColor);
void LCD_Full_Rect(ushort vAddr, ushort vX, ushort vY, ushort vX1, ushort vY1, ushort vColor);
void LCD_Draw_Cycle(ushort vAddr, ushort vX, ushort vY, ushort vRad, ushort vColor);
void LCD_Cut_PICRect(ushort vAddr, ushort vPicID, ushort vX, ushort vY, 
		ushort vW, ushort vH, ushort vXD, ushort vYD, ushort vColor);
void LCD_Disp_ICON(ushort vAddr, ushort vIconID, ushort vX, ushort vY);

#endif


#endif  //__APP_LIJU_H_
