/********************************************************************
*  
*  APP_LCD  与 LCD 有关的功能函数
*  功能：
*
********************************************************************/

#ifndef __APP_LCD_H_
#define __APP_LCD_H_


#include "config.h"


#define COLOR_RED_  		0xf800			// 红
#define COLOR_GREEN_  	0x07e0			// 绿
#define COLOR_BLUE_  		0x001f			// 蓝
#define COLOR_YELLW_  	0xffe0			// 黄
#define COLOR_WHITE_		0xffff			// 白

#define PASSWORD_LEN		5			  // 密码长度

#define LCD_FRESH_MS		200				// LCD 刷新周期, ms
#define LCD_FRESH_FREQ	1000/LCD_FRESH_MS				// LCD 刷新频率
#define LCD_FRESH_WARN	LCD_FRESH_FREQ*2				// 告警信息 刷新周期
#define LCD_FRESH_TIME	LCD_FRESH_FREQ/2				// 时间 刷新周期


////  显示页面及切换的函数
typedef struct
{
	uchar  vPageX;			// 页面
	uchar  page;			// 页面
	//uchar  Cmdpage;			// 命令页面
	ushort pic;				// 背景图片
	uchar  key;				// 按键
	uchar  cmdsel;			// 命令选择
	uchar  BDRect;		// 长度标定页面的输入框
	uchar  BDRectSite;	// 输入框内的输入位置
	uchar  sonPage;			// 子页面
	uchar  vRdSite;
	uchar  vSave;			// 保存标定参数标记
}*pLCDPage, mLCDPage;



extern void ( * gp_lcdtask)(void);
extern void ( * gp_lcdtaskNext)(void);


void Disp_mainSPJ(void);
void Disp_ZZT_CtrlStatus(void);
void Disp_ZZT_SetParam(void);
void Disp_GJ(void);
	   
void Disp_PicX(unsigned char vPic);

void Disp_main(void);
void Disp_Input_Password(void);
void Disp_Cmd_Sel(void);
void Disp_Cmd8_SetI2(void);
void Disp_Cmd8_SetI(void);
void Disp_Cmd1_AITime(void);




extern uchar   gPasswordIn[8];
extern unsigned short gLcdFresh ;
extern unsigned short gDataFresh;
extern mLCDPage gLCDPage;
extern ushort gBKColor ;			 // 背景色
extern ushort gFTColor ;			 // 前景色

//// LCD
extern	ushort gLCDX;				 // X
extern	ushort gLCDY;				 // Y
extern	ushort gLCDX1;				 // X1
extern	ushort gLCDY1;				 // Y1
extern	ushort gLCDW;				 // 宽
extern	ushort gLCDH;				 // 高

				   
extern unsigned char N01_BUF;
extern unsigned char N02_BUF;
extern unsigned char gSetAIFlg;
extern unsigned char gSetAIX;
extern unsigned char gSetAIY;
extern unsigned short gAISiteX[9];
extern unsigned short gAISiteY[6];
extern unsigned int  gAISetFlg;		// 设置AI的标记
extern unsigned char gSetAISend ;




#endif  //__APP_LCD_H_
