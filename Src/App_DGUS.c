/****************************************************************************
*
*	 APP_DGUS 与 LCD 人机接口 有关的函数
*
*
*****************************************************************************/

#include "APP_DGUS.h"
#include "LPC17xx.h"
#include "config.h"


/****************************************************************************************/
////////   LCD
mLCDSets 	gLCDSets;
unsigned char  gLCDSStr[80];	// 发送给 LCD 的字符串
unsigned char  gLCDRStr[80];	// 从 LCD 接收到的字符串
unsigned char  gLCDSLen = 0;	// 发送给 LCD 的字符数
unsigned char  gLCDRLen = 0;	// 从 LCD 接收到的字符数


/****************************************************************************************
****************************************************************************************
* 时间:  2014-11-24
* 名称：
* 功能：			UART LCD 接口 相关函数
* 说明： MSB方式，即：高字节在前
*
****************************************************************************************
****************************************************************************************/
//// 握手
void LCD_Hands(void)
{
	gLCDSets.mCmd = LCD_C_HANDS;
	gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = LCD_HCH;
	gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
	gLCDSStr[gLCDSLen++] = LCD_LCH1;
	gLCDSStr[gLCDSLen++] = LCD_LCH2;
	gLCDSStr[gLCDSLen++] = LCD_LCH3;
	gLCDSStr[gLCDSLen++] = LCD_LCH4;
	UARTSend(1, gLCDSStr, gLCDSLen);
}

////  全屏清屏，用背景色填充
void LCD_CLR_SCR(void)
{
	gLCDSets.mCmd = LCD_C_C_SCR;
	gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = LCD_HCH;
	gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
	gLCDSStr[gLCDSLen++] = LCD_LCH1;
	gLCDSStr[gLCDSLen++] = LCD_LCH2;
	gLCDSStr[gLCDSLen++] = LCD_LCH3;
	gLCDSStr[gLCDSLen++] = LCD_LCH4;
	UARTSend(1, gLCDSStr, gLCDSLen);
}

////  背光开关、调节
void LCD_SET_BL(uchar vBL)
{
	if(vBL == 0)				// 关闭背光
		gLCDSets.mCmd = LCD_C_C_BL;
	else 						// 背光最大亮度
		gLCDSets.mCmd = LCD_C_S_BL;
	gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = LCD_HCH;
	gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
	if(vBL)
		gLCDSStr[gLCDSLen++] = vBL;
	gLCDSStr[gLCDSLen++] = LCD_LCH1;
	gLCDSStr[gLCDSLen++] = LCD_LCH2;
	gLCDSStr[gLCDSLen++] = LCD_LCH3;
	gLCDSStr[gLCDSLen++] = LCD_LCH4;
	UARTSend(1, gLCDSStr, gLCDSLen);
}
 
////  图片显示
void LCD_Disp_PIC(ushort vPicID)
{
	gLCDPage.vPageX = vPicID;
	
	gLCDSets.mCmd = LCD_C_D_PIC;
	gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = LCD_HCH;
	gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
	if(vPicID>0xff)
		gLCDSStr[gLCDSLen++] = vPicID>>8;
	gLCDSStr[gLCDSLen++] = vPicID;
	gLCDSStr[gLCDSLen++] = LCD_LCH1;
	gLCDSStr[gLCDSLen++] = LCD_LCH2;
	gLCDSStr[gLCDSLen++] = LCD_LCH3;
	gLCDSStr[gLCDSLen++] = LCD_LCH4;
	UARTSend(1, gLCDSStr, gLCDSLen);
	
	if(gLCDBKOnFlg >= 2)
	{
		delay_ms(20);
		gLCDBKOnFlg = 0;
			LCD_SET_BL(gLCDBKLight);
	}

}

//// 设置当前调色板，颜色值一直保持到下次修改，RGB--5：6：5，改单个颜色时，另一个参数需为负数
void LCD_Set_Color(long vFC,long vBC)
{
	if((vFC >= 0) && ( vFC < 0x10000))
		gLCDSets.mFC = vFC;
	if((vBC >= 0) && ( vBC < 0x10000))
		gLCDSets.mBC = vBC;
	gLCDSets.mCmd = LCD_C_S_COL;
	gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = LCD_HCH;
	gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
	gLCDSStr[gLCDSLen++] = gLCDSets.mFC>>8;
	gLCDSStr[gLCDSLen++] = gLCDSets.mFC;
	gLCDSStr[gLCDSLen++] = gLCDSets.mBC>>8;
	gLCDSStr[gLCDSLen++] = gLCDSets.mBC;
	gLCDSStr[gLCDSLen++] = LCD_LCH1;
	gLCDSStr[gLCDSLen++] = LCD_LCH2;
	gLCDSStr[gLCDSLen++] = LCD_LCH3;
	gLCDSStr[gLCDSLen++] = LCD_LCH4;
	UARTSend(1, gLCDSStr, gLCDSLen);
}
//// 设置设置字符间距，一直保持到下次修改
void LCD_Set_FontDis(char vDX,char vDY)
{
	if(vDX < 0x80)
		gLCDSets.mDisX = vDX;
	if(vDY < 0x80)
		gLCDSets.mDisY = vDY;
	gLCDSets.mCmd = LCD_C_S_DIS;
	gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = LCD_HCH;
	gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
	gLCDSStr[gLCDSLen++] = gLCDSets.mDisX;
	gLCDSStr[gLCDSLen++] = gLCDSets.mDisY;
	gLCDSStr[gLCDSLen++] = LCD_LCH1;
	gLCDSStr[gLCDSLen++] = LCD_LCH2;
	gLCDSStr[gLCDSLen++] = LCD_LCH3;
	gLCDSStr[gLCDSLen++] = LCD_LCH4;
	UARTSend(1, gLCDSStr, gLCDSLen);
}

//// 取指定位置颜色到背景色/前景色
void LCD_Get_Color(uchar vFCBC, ushort vX, ushort vY)
{
	if(vFCBC == 1)
		gLCDSets.mCmd = LCD_C_G_COL_F;
	if(vFCBC == 0)
		gLCDSets.mCmd = LCD_C_G_COL_B;
	gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = LCD_HCH;
	gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
	gLCDSStr[gLCDSLen++] = vX>>8;
	gLCDSStr[gLCDSLen++] = vX;
	gLCDSStr[gLCDSLen++] = vY>>8;
	gLCDSStr[gLCDSLen++] = vY;
	gLCDSStr[gLCDSLen++] = LCD_LCH1;
	gLCDSStr[gLCDSLen++] = LCD_LCH2;
	gLCDSStr[gLCDSLen++] = LCD_LCH3;
	gLCDSStr[gLCDSLen++] = LCD_LCH4;
	UARTSend(1, gLCDSStr, gLCDSLen);
}


//// 光标显示
void LCD_Set_Cursor(uchar vOnOff,ushort vX, ushort vY, uchar vCurW, uchar vCurH)
{
	gLCDSets.mCmd = LCD_C_S_CUR;
	gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = LCD_HCH;
	gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
	if(vOnOff)
		gLCDSStr[gLCDSLen++] = 1;		// 前景色显示光标
	else
		gLCDSStr[gLCDSLen++] = 0;		// 背景色清除光标
	gLCDSStr[gLCDSLen++] = vX>>8;
	gLCDSStr[gLCDSLen++] = vX;
	gLCDSStr[gLCDSLen++] = vY>>8;
	gLCDSStr[gLCDSLen++] = vY;
	gLCDSStr[gLCDSLen++] = vCurW;
	gLCDSStr[gLCDSLen++] = vCurH;
	gLCDSStr[gLCDSLen++] = LCD_LCH1;
	gLCDSStr[gLCDSLen++] = LCD_LCH2;
	gLCDSStr[gLCDSLen++] = LCD_LCH3;
	gLCDSStr[gLCDSLen++] = LCD_LCH4;
	UARTSend(1, gLCDSStr, gLCDSLen);
}
////  点显示  置点  /  删除点
void LCD_Disp_Point(uchar vOnOff, ushort vX, ushort vY)
{
	if(vOnOff)
		gLCDSets.mCmd = LCD_C_D_POT;
	else
		gLCDSets.mCmd = LCD_C_C_POT;
	gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = LCD_HCH;
	gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
	gLCDSStr[gLCDSLen++] = vX>>8;
	gLCDSStr[gLCDSLen++] = vX;
	gLCDSStr[gLCDSLen++] = vY>>8;
	gLCDSStr[gLCDSLen++] = vY;
	gLCDSStr[gLCDSLen++] = LCD_LCH1;	
	gLCDSStr[gLCDSLen++] = LCD_LCH2;
	gLCDSStr[gLCDSLen++] = LCD_LCH3;
	gLCDSStr[gLCDSLen++] = LCD_LCH4;
	UARTSend(1, gLCDSStr, gLCDSLen);
}

////  指定点连线
void LCD_Disp_Line(uchar vOnOff, ushort vX, ushort vY, ushort vX1, ushort vY1)
{
	if(vOnOff)
		gLCDSets.mCmd = LCD_C_D_LINE;
	else
		gLCDSets.mCmd = LCD_C_C_LINE;
	gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = LCD_HCH;
	gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
	gLCDSStr[gLCDSLen++] = vX>>8;
	gLCDSStr[gLCDSLen++] = vX;
	gLCDSStr[gLCDSLen++] = vY>>8;
	gLCDSStr[gLCDSLen++] = vY;
	gLCDSStr[gLCDSLen++] = vX1>>8;
	gLCDSStr[gLCDSLen++] = vX1;
	gLCDSStr[gLCDSLen++] = vY1>>8;
	gLCDSStr[gLCDSLen++] = vY1;
	gLCDSStr[gLCDSLen++] = LCD_LCH1;	
	gLCDSStr[gLCDSLen++] = LCD_LCH2;
	gLCDSStr[gLCDSLen++] = LCD_LCH3;
	gLCDSStr[gLCDSLen++] = LCD_LCH4;
	UARTSend(1, gLCDSStr, gLCDSLen);
}

////  矩形区域显示
void LCD_Disp_Rect(uchar vCmd, ushort vX, ushort vY, ushort vX1, ushort vY1)
{
	if((vCmd==LCD_C_D_Rect)||(vCmd==LCD_C_C_Rect)||(vCmd==LCD_C_D_FRect)||
		(vCmd==LCD_C_C_FRect)||(vCmd==LCD_C_X_FRect))
	{
		gLCDSets.mCmd = vCmd;
		gLCDSLen = 0;	   
		gLCDSStr[gLCDSLen++] = LCD_HCH;
		gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
		gLCDSStr[gLCDSLen++] = vX>>8;
		gLCDSStr[gLCDSLen++] = vX;
		gLCDSStr[gLCDSLen++] = vY>>8;
		gLCDSStr[gLCDSLen++] = vY;
		gLCDSStr[gLCDSLen++] = vX1>>8;
		gLCDSStr[gLCDSLen++] = vX1;
		gLCDSStr[gLCDSLen++] = vY1>>8;
		gLCDSStr[gLCDSLen++] = vY1;
		gLCDSStr[gLCDSLen++] = LCD_LCH1;	
		gLCDSStr[gLCDSLen++] = LCD_LCH2;
		gLCDSStr[gLCDSLen++] = LCD_LCH3;
		gLCDSStr[gLCDSLen++] = LCD_LCH4;
		UARTSend(1, gLCDSStr, gLCDSLen);
	}
}
////  画圆		   
void LCD_Disp_Cyc(uchar vCmd, ushort vX, ushort vY, uchar vR)
{
	if((vCmd==LCD_C_X_CYC)||(vCmd==LCD_C_D_CYC)||
		(vCmd==LCD_C_X_FCYC)||(vCmd==LCD_C_D_FCYC))
	{
		gLCDSets.mCmd = LCD_C_D_ARC;
		gLCDSLen = 0;	   
		gLCDSStr[gLCDSLen++] = LCD_HCH;
		gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
		gLCDSStr[gLCDSLen++] = vCmd;
		gLCDSStr[gLCDSLen++] = vX>>8;
		gLCDSStr[gLCDSLen++] = vX;
		gLCDSStr[gLCDSLen++] = vY>>8;
		gLCDSStr[gLCDSLen++] = vY;
		gLCDSStr[gLCDSLen++] = vR;
		gLCDSStr[gLCDSLen++] = LCD_LCH1;	
		gLCDSStr[gLCDSLen++] = LCD_LCH2;
		gLCDSStr[gLCDSLen++] = LCD_LCH3;
		gLCDSStr[gLCDSLen++] = LCD_LCH4;
		UARTSend(1, gLCDSStr, gLCDSLen);
	}
}
////  画圆弧		   
void LCD_Disp_Arc(uchar vCmd, ushort vX, ushort vY, ushort vR, ushort vAS, ushort vAE )
{
	// 1< vR <0x3ff, 0< AS,AE <0x2d0(0~720, 0.5°)
	gLCDSets.mCmd = LCD_C_D_ARC;
	gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = LCD_HCH;
	gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
	gLCDSStr[gLCDSLen++] = 4;
	gLCDSStr[gLCDSLen++] = vX>>8;
	gLCDSStr[gLCDSLen++] = vX;
	gLCDSStr[gLCDSLen++] = vY>>8;
	gLCDSStr[gLCDSLen++] = vY;
	gLCDSStr[gLCDSLen++] = vR>>8;
	gLCDSStr[gLCDSLen++] = vR;
	gLCDSStr[gLCDSLen++] = vAS>>8;
	gLCDSStr[gLCDSLen++] = vAS;
	gLCDSStr[gLCDSLen++] = vAE>>8;
	gLCDSStr[gLCDSLen++] = vAE;
	gLCDSStr[gLCDSLen++] = LCD_LCH1;	
	gLCDSStr[gLCDSLen++] = LCD_LCH2;
	gLCDSStr[gLCDSLen++] = LCD_LCH3;
	gLCDSStr[gLCDSLen++] = LCD_LCH4;
	UARTSend(1, gLCDSStr, gLCDSLen);
}

/****************************************************************************************
* 时间:  2014-11-24
* 名称：
* 功能：	文本显示 相关函数，带数据格式转换
* 说明： MSB方式，即：高字节在前
*
****************************************************************************************/
//// 显示文本
void LCD_Disp_Txt(uchar vFont, ushort vX, ushort vY, uchar* vPS)
{
	if((vFont==FONT_8)||(vFont==FONT_16)||(vFont==FONT_32)||(vFont==FONT_24)||(vFont==FONT_12))
	{
		gLCDSets.mCmd = vFont;
	}
		gLCDSLen = 0;	   
		gLCDSStr[gLCDSLen++] = LCD_HCH;
		gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
		gLCDSStr[gLCDSLen++] = vX>>8;
		gLCDSStr[gLCDSLen++] = vX;
		gLCDSStr[gLCDSLen++] = vY>>8;
		gLCDSStr[gLCDSLen++] = vY;
		while(*vPS != 0)
			gLCDSStr[gLCDSLen++] = *vPS++;	
		gLCDSStr[gLCDSLen++] = LCD_LCH1;	
		gLCDSStr[gLCDSLen++] = LCD_LCH2;
		gLCDSStr[gLCDSLen++] = LCD_LCH3;
		gLCDSStr[gLCDSLen++] = LCD_LCH4;
		UARTSend(1, gLCDSStr, gLCDSLen);
}
//// 显示文本
void LCD_Disp_TxtLen(uchar vFont, ushort vX, ushort vY, uchar* vPS,uchar vLen)
{
	if((vFont==FONT_8)||(vFont==FONT_16)||(vFont==FONT_32)||(vFont==FONT_24)||(vFont==FONT_12))
	{
		gLCDSets.mCmd = vFont;
	}
		gLCDSLen = 0;	   
		gLCDSStr[gLCDSLen++] = LCD_HCH;
		gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
		gLCDSStr[gLCDSLen++] = vX>>8;
		gLCDSStr[gLCDSLen++] = vX;
		gLCDSStr[gLCDSLen++] = vY>>8;
		gLCDSStr[gLCDSLen++] = vY;
		while(vLen--)
			gLCDSStr[gLCDSLen++] = *vPS++;	
		gLCDSStr[gLCDSLen++] = LCD_LCH1;	
		gLCDSStr[gLCDSLen++] = LCD_LCH2;
		gLCDSStr[gLCDSLen++] = LCD_LCH3;
		gLCDSStr[gLCDSLen++] = LCD_LCH4;
		UARTSend(1, gLCDSStr, gLCDSLen);
}
////  显示整数
void LCD_Disp_Int(uchar vFont, ushort vX, ushort vY, long vData)
{
	char vStr[20];
	uchar vLen = 0;
//	uchar i = 0;

	sprintf(vStr, "%d",	vData);
	vLen = strlen(vStr);
	vStr[vLen] = 0;
	LCD_Disp_Txt(vFont, vX, vY, (uchar*)vStr);
}
////  显示整数
void LCD_Disp_Int2B(uchar vFont, ushort vX, ushort vY, long vData)
{
	char vStr[20];
	uchar vLen = 0;
//	uchar i = 0;

	sprintf(vStr, "%02d",	vData);
	vLen = strlen(vStr);
	vStr[vLen] = 0;
	LCD_Disp_Txt(vFont, vX, vY, (uchar*)vStr);
}
// 按指定长度显示，不足的补空格
void LCD_Disp_Intx(uchar vFont, ushort vX, ushort vY, long vData, uchar vLen)
{
	char vStr[20];
	uchar k = 0;
	uchar i = 0;

	sprintf(vStr, "%d",	vData);
	k = strlen(vStr);
	for(i=k; i<vLen; i++)
		vStr[i] = ' ';
	vStr[i] = 0;
	LCD_Disp_Txt(vFont, vX, vY, (uchar*)vStr);
}
// 按长度4显示，不足的前面补空格
void LCD_Disp_Int4x(uchar vFont, ushort vX, ushort vY, long vData)
{
	char vStr[20];
	uchar k = 0;
//	uchar i = 0;

	if(vData < 10)
		sprintf(vStr, "   %d",	vData);
	else if(vData < 100)
		sprintf(vStr, "  %d",	vData);
	else if(vData < 1000)
		sprintf(vStr, " %d",	vData);
	else //if(vData < 1000)
		sprintf(vStr, "%d",	vData);
	k = strlen(vStr);
	vStr[k] = 0;
	LCD_Disp_Txt(vFont, vX, vY, (uchar*)vStr);
}

////  显示小数，显示 1 位小数
void LCD_Disp_Float(uchar vFont, ushort vX, ushort vY, float vData)
{
	char vStr[20];
	uchar vLen = 0;
//	uchar i = 0;

	sprintf(vStr, "%4.1f ",	vData);
	vLen = strlen(vStr);
	vStr[vLen] = 0;
	LCD_Disp_Txt(vFont, vX, vY, (uchar*)vStr);
}
void LCD_Disp_Float100(uchar vFont, ushort vX, ushort vY, float vData)			// 显示百分号
{
	char vStr[20];
	uchar vLen = 0;
//	uchar i = 0;

	sprintf(vStr, "%4.1f    ",	vData);
	vStr[4] = '%';
//	vLen = strlen(vStr);
	vStr[5] = 0;
	LCD_Disp_Txt(vFont, vX, vY, (uchar*)vStr);
}
void LCD_Disp_Floatx(uchar vFont, ushort vX, ushort vY, float vData,uchar vLen)
{
	char vStr[20];
	uchar k = 0;
	uchar i = 0;

	sprintf(vStr, "%8.1f",	vData);
	k = strlen(vStr);
	for(i=k; i<vLen; i++)
		vStr[i] = ' ';
	vStr[i] = 0;
	LCD_Disp_Txt(vFont, vX, vY, (uchar*)vStr);
}

////  显示小数，显示 x 位小数
void LCD_Disp_FloatPX(uchar vFont, ushort vX, ushort vY, float vData, uchar vFX)
{
	char vStr[20];

	switch(vFX)
	{
		case 1:
			sprintf(vStr, "%8.1f",	vData);
			break;
		case 2:
			sprintf(vStr, "%8.2f",	vData);
			break;
		case 3:
			sprintf(vStr, "%8.3f",	vData);
			break;
		case 4:
			sprintf(vStr, "%8.4f",	vData);
			break;
		case 5:
			sprintf(vStr, "%8.5f",	vData);
			break;
		case 6:
			sprintf(vStr, "%8.6f",	vData);
			break;
		default:
			sprintf(vStr, "%8.2f",	vData);
			break;
	}
	LCD_Disp_Txt(vFont, vX, vY, (uchar*)vStr);
}

void LCD_Cut_PICRect(ushort vAddr, ushort vPicID, ushort vX, ushort vY, 
		ushort vX1, ushort vY1, ushort vXD, ushort vYD)
{
	gLCDSets.mCmd = LCD_C_T_PICRECT;
	gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = LCD_HCH;
	gLCDSStr[gLCDSLen++] = gLCDSets.mCmd;
	gLCDSStr[gLCDSLen++] = vPicID;
	gLCDSStr[gLCDSLen++] = vX>>8;
	gLCDSStr[gLCDSLen++] = vX;
	gLCDSStr[gLCDSLen++] = vY>>8;
	gLCDSStr[gLCDSLen++] = vY;
	gLCDSStr[gLCDSLen++] = vX1>>8;
	gLCDSStr[gLCDSLen++] = vX1;
	gLCDSStr[gLCDSLen++] = vY1>>8;
	gLCDSStr[gLCDSLen++] = vY1;
	gLCDSStr[gLCDSLen++] = vXD>>8;
	gLCDSStr[gLCDSLen++] = vXD;
	gLCDSStr[gLCDSLen++] = vYD>>8;
	gLCDSStr[gLCDSLen++] = vYD;

	gLCDSStr[gLCDSLen++] = LCD_LCH1;	
	gLCDSStr[gLCDSLen++] = LCD_LCH2;
	gLCDSStr[gLCDSLen++] = LCD_LCH3;
	gLCDSStr[gLCDSLen++] = LCD_LCH4;
	UARTSend(1, gLCDSStr, gLCDSLen);
}
/********************************************************************************************
*********************************************************************************************
*
*	DGUS 接口屏的相关指令，组态屏
*
*********************************************************************************************
********************************************************************************************/
#if 0
void LCD_GO_Page(ushort vPage)
{
	gLCDPage.page = vPage;

		CAN_SBuf0[6] = vPage;
		gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = 0x5a;
	gLCDSStr[gLCDSLen++] = 0xa5;
	gLCDSStr[gLCDSLen++] = 0x04;
	gLCDSStr[gLCDSLen++] = 0x80;			// Write CMD
	gLCDSStr[gLCDSLen++] = 0x03;			// REG
	gLCDSStr[gLCDSLen++] = vPage>>8;		// PICTURE 
	gLCDSStr[gLCDSLen++] = vPage;
	UARTSend(1, gLCDSStr, gLCDSLen);
}
void LCD_Set_KeyV(uchar vKey)
{
		gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = 0x5a;
	gLCDSStr[gLCDSLen++] = 0xa5;
	gLCDSStr[gLCDSLen++] = 0x03;
	gLCDSStr[gLCDSLen++] = 0x80;			// Write CMD
	gLCDSStr[gLCDSLen++] = 0x4f;			// REG
	gLCDSStr[gLCDSLen++] = vKey;			// PICTURE 
//	gLCDSStr[gLCDSLen++] = vPage;
	UARTSend(1, gLCDSStr, gLCDSLen);
}

void LCD_Get_Page(ushort vPage)
{
		gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = 0x5a;
	gLCDSStr[gLCDSLen++] = 0xa5;
	gLCDSStr[gLCDSLen++] = 0x03;
	gLCDSStr[gLCDSLen++] = 0x81;			// Read CMD
	gLCDSStr[gLCDSLen++] = 0x03;			// REG
	gLCDSStr[gLCDSLen++] = 0x02;			// 2 Byte
	UARTSend(1, gLCDSStr, gLCDSLen);
}

void LCD_WR_Data2B(ushort vAddr, ushort vData)	   // 写的数据长度是字节
{
		gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = 0x5a;
	gLCDSStr[gLCDSLen++] = 0xa5;
	gLCDSStr[gLCDSLen++] = 0x05;
	gLCDSStr[gLCDSLen++] = 0x82;				// Write Data
	gLCDSStr[gLCDSLen++] = vAddr>>8;			// Address 
	gLCDSStr[gLCDSLen++] = vAddr;
	gLCDSStr[gLCDSLen++] = vData>>8;			// Data 
	gLCDSStr[gLCDSLen++] = vData;
	UARTSend(1, gLCDSStr, gLCDSLen);
	gLCDSLen = 0;	   
}
void LCD_Rd_Data(ushort vAddr, uchar vLen)		// 读的数据长度是字，2字节
{
		gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = 0x5a;
	gLCDSStr[gLCDSLen++] = 0xa5;
	gLCDSStr[gLCDSLen++] = 0x03+vLen;
	gLCDSStr[gLCDSLen++] = 0x83;				// Read Data
	gLCDSStr[gLCDSLen++] = vAddr>>8;			// Address 
	gLCDSStr[gLCDSLen++] = vAddr;
	gLCDSStr[gLCDSLen++] = vLen;				// n WORD
	UARTSend(1, gLCDSStr, gLCDSLen);
}

void LCD_WR_TxtByte(ushort vAddr, uchar vChar)	   // 写字符
{
		gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = 0x5a;
	gLCDSStr[gLCDSLen++] = 0xa5;
	gLCDSStr[gLCDSLen++] = 0x00;
	gLCDSStr[gLCDSLen++] = 0x82;				// Write Data
	gLCDSStr[gLCDSLen++] = vAddr>>8;			// Address 
	gLCDSStr[gLCDSLen++] = vAddr;
	gLCDSStr[gLCDSLen++] = vChar;				// Data 
	gLCDSStr[gLCDSLen++] = 0;				// Data 
	gLCDSStr[2] = gLCDSLen-3; 					// 
	UARTSend(1, gLCDSStr, gLCDSLen);
	gLCDSLen = 0;	   
}
void LCD_WR_Str(ushort vAddr, uchar* vStr, uchar vLen)	   // 写字符串
{
		gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = 0x5a;
	gLCDSStr[gLCDSLen++] = 0xa5;
	gLCDSStr[gLCDSLen++] = 0x00;
	gLCDSStr[gLCDSLen++] = 0x82;				// Write Data
	gLCDSStr[gLCDSLen++] = vAddr>>8;			// Address 
	gLCDSStr[gLCDSLen++] = vAddr;
	while(vLen--)
	gLCDSStr[gLCDSLen++] = *vStr++;			// Data 
	//gLCDSStr[gLCDSLen++] = *vStr;
	gLCDSStr[2] = gLCDSLen-3; 					// 
	UARTSend(1, gLCDSStr, gLCDSLen);
	gLCDSLen = 0;	   
}

////  清除
void LCD_Draw_Clr(ushort vAddr)
{
		gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = 0x5a;
	gLCDSStr[gLCDSLen++] = 0xa5;
	gLCDSStr[gLCDSLen++] = 0; 					// 长度，可最后填充
	gLCDSStr[gLCDSLen++] = 0x82; 				// 写数据存储区指令
	gLCDSStr[gLCDSLen++] = vAddr>>8; 			// DGUS组态中的基本图形地址
	gLCDSStr[gLCDSLen++] = vAddr; 				// 
	gLCDSStr[gLCDSLen++] = 0; 					// 基本图形的画点指令	0001，存入vAddr
	gLCDSStr[gLCDSLen++] = 1; 					// 
	gLCDSStr[gLCDSLen++] = 0; 					// 基本图形的画矩形个数	000x，存入vAddr+1
	gLCDSStr[gLCDSLen++] = 1; 					// 
	gLCDSStr[gLCDSLen++] = 0; 			// 基本图形的X0，存入vAddr+3
	gLCDSStr[gLCDSLen++] = 0; 				// 
	gLCDSStr[gLCDSLen++] = 0; 			// 基本图形的Y0，存入vAddr+4
	gLCDSStr[gLCDSLen++] = 0; 				// 
	gLCDSStr[gLCDSLen++] = 0; 			// 基本图形的颜色，存入vAddr+2
	gLCDSStr[gLCDSLen++] = 0; 				// 
	gLCDSStr[gLCDSLen++] = 0xFF; 				// 结束基本图形
	gLCDSStr[gLCDSLen++] = 0; 					// 
	gLCDSStr[2] = gLCDSLen-3; 					// 
	UARTSend(1, gLCDSStr, gLCDSLen);
}
////  画点
void LCD_Draw_Point(ushort vAddr, ushort vX, ushort vY, ushort vColor)
{
		gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = 0x5a;
	gLCDSStr[gLCDSLen++] = 0xa5;
	gLCDSStr[gLCDSLen++] = 0; 					// 长度，可最后填充
	gLCDSStr[gLCDSLen++] = 0x82; 				// 写数据存储区指令
	gLCDSStr[gLCDSLen++] = vAddr>>8; 			// DGUS组态中的基本图形地址
	gLCDSStr[gLCDSLen++] = vAddr; 				// 
	gLCDSStr[gLCDSLen++] = 0; 					// 基本图形的画点指令	0001，存入vAddr
	gLCDSStr[gLCDSLen++] = 1; 					// 
	gLCDSStr[gLCDSLen++] = 0; 					// 基本图形的画矩形个数	000x，存入vAddr+1
	gLCDSStr[gLCDSLen++] = 1; 					// 
	gLCDSStr[gLCDSLen++] = vX>>8; 			// 基本图形的X0，存入vAddr+3
	gLCDSStr[gLCDSLen++] = vX; 				// 
	gLCDSStr[gLCDSLen++] = vY>>8; 			// 基本图形的Y0，存入vAddr+4
	gLCDSStr[gLCDSLen++] = vY; 				// 
	gLCDSStr[gLCDSLen++] = vColor>>8; 			// 基本图形的颜色，存入vAddr+2
	gLCDSStr[gLCDSLen++] = vColor; 				// 
	gLCDSStr[gLCDSLen++] = 0xFF; 				// 结束基本图形
	gLCDSStr[gLCDSLen++] = 0; 					// 
	gLCDSStr[2] = gLCDSLen-3; 					// 
	UARTSend(1, gLCDSStr, gLCDSLen);
}
////  画线
void LCD_Draw_Line(ushort vAddr, ushort vX, ushort vY, ushort vX1, ushort vY1, ushort vColor)
{
		gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = 0x5a;
	gLCDSStr[gLCDSLen++] = 0xa5;
	gLCDSStr[gLCDSLen++] = 0x13; 					// 长度，可最后填充
	gLCDSStr[gLCDSLen++] = 0x82; 				// 写数据存储区指令
	gLCDSStr[gLCDSLen++] = vAddr>>8; 			// DGUS组态中的基本图形地址
	gLCDSStr[gLCDSLen++] = vAddr; 				// 
	gLCDSStr[gLCDSLen++] = 0; 					// 基本图形的画矩形指令	0002，存入vAddr
	gLCDSStr[gLCDSLen++] = 2; 					// 
	gLCDSStr[gLCDSLen++] = 0; 					// 基本图形的画矩形个数	000x，存入vAddr+1
	gLCDSStr[gLCDSLen++] = 1; 					// 
	gLCDSStr[gLCDSLen++] = vColor>>8; 			// 基本图形的颜色，存入vAddr+2
	gLCDSStr[gLCDSLen++] = vColor; 				// 
	gLCDSStr[gLCDSLen++] = vX>>8; 			// 基本图形的X0，存入vAddr+3
	gLCDSStr[gLCDSLen++] = vX; 				// 
	gLCDSStr[gLCDSLen++] = vY>>8; 			// 基本图形的Y0，存入vAddr+4
	gLCDSStr[gLCDSLen++] = vY; 				// 
	gLCDSStr[gLCDSLen++] = (vX1)>>8; 			// 基本图形的X1，存入vAddr+5
	gLCDSStr[gLCDSLen++] = (vX1); 			// 
	gLCDSStr[gLCDSLen++] = (vY1)>>8; 			// 基本图形的Y1，存入vAddr+6
	gLCDSStr[gLCDSLen++] = (vY1); 			// 
	gLCDSStr[gLCDSLen++] = 0xFF; 				// 结束基本图形
	gLCDSStr[gLCDSLen++] = 0; 					// 
	gLCDSStr[2] = gLCDSLen-3; 					// 
	UARTSend(1, gLCDSStr, gLCDSLen);
}
////  画矩形
void LCD_Draw_Rect(ushort vAddr, ushort vX, ushort vY, ushort vX1, ushort vY1, ushort vColor)
{
		gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = 0x5a;
	gLCDSStr[gLCDSLen++] = 0xa5;
	gLCDSStr[gLCDSLen++] = 0; 					// 长度，可最后填充
	gLCDSStr[gLCDSLen++] = 0x82; 				// 写数据存储区指令
	gLCDSStr[gLCDSLen++] = vAddr>>8; 			// DGUS组态中的基本图形地址
	gLCDSStr[gLCDSLen++] = vAddr; 				// 
	gLCDSStr[gLCDSLen++] = 0; 					// 基本图形的画矩形指令	0003，存入vAddr
	gLCDSStr[gLCDSLen++] = 3; 					// 
	gLCDSStr[gLCDSLen++] = 0; 					// 基本图形的画矩形个数	000x，存入vAddr+1
	gLCDSStr[gLCDSLen++] = 1; 					// 
	gLCDSStr[gLCDSLen++] = vX>>8; 			// 基本图形的X0，存入vAddr+3
	gLCDSStr[gLCDSLen++] = vX; 				// 
	gLCDSStr[gLCDSLen++] = vY>>8; 			// 基本图形的Y0，存入vAddr+4
	gLCDSStr[gLCDSLen++] = vY; 				// 
	gLCDSStr[gLCDSLen++] = (vX1)>>8; 			// 基本图形的X1，存入vAddr+5
	gLCDSStr[gLCDSLen++] = (vX1); 			// 
	gLCDSStr[gLCDSLen++] = (vY1)>>8; 			// 基本图形的Y1，存入vAddr+6
	gLCDSStr[gLCDSLen++] = (vY1); 			// 
	gLCDSStr[gLCDSLen++] = vColor>>8; 			// 基本图形的颜色，存入vAddr+2
	gLCDSStr[gLCDSLen++] = vColor; 				// 
	gLCDSStr[gLCDSLen++] = 0xFF; 				// 结束基本图形
	gLCDSStr[gLCDSLen++] = 0; 					// 
	gLCDSStr[2] = gLCDSLen-3; 					// 
	UARTSend(1, gLCDSStr, gLCDSLen);
}
////  填充矩形区域
void LCD_Full_Rect(ushort vAddr, ushort vX, ushort vY, ushort vX1, ushort vY1, ushort vColor)
{
		gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = 0x5a;
	gLCDSStr[gLCDSLen++] = 0xa5;
	gLCDSStr[gLCDSLen++] = 0; 					// 长度，可最后填充
	gLCDSStr[gLCDSLen++] = 0x82; 				// 写数据存储区指令
	gLCDSStr[gLCDSLen++] = vAddr>>8; 			// DGUS组态中的基本图形地址
	gLCDSStr[gLCDSLen++] = vAddr; 				// 
	gLCDSStr[gLCDSLen++] = 0; 					// 基本图形的填充矩形指令	0004，存入vAddr
	gLCDSStr[gLCDSLen++] = 4; 					// 
	gLCDSStr[gLCDSLen++] = 0; 					// 基本图形的画矩形个数	000x，存入vAddr+1
	gLCDSStr[gLCDSLen++] = 1; 					// 
	gLCDSStr[gLCDSLen++] = vX>>8; 			// 基本图形的X0，存入vAddr+3
	gLCDSStr[gLCDSLen++] = vX; 				// 
	gLCDSStr[gLCDSLen++] = vY>>8; 			// 基本图形的Y0，存入vAddr+4
	gLCDSStr[gLCDSLen++] = vY; 				// 
	gLCDSStr[gLCDSLen++] = (vX1)>>8; 			// 基本图形的X1，存入vAddr+5
	gLCDSStr[gLCDSLen++] = (vX1); 			// 
	gLCDSStr[gLCDSLen++] = (vY1)>>8; 			// 基本图形的Y1，存入vAddr+6
	gLCDSStr[gLCDSLen++] = (vY1); 			// 
	gLCDSStr[gLCDSLen++] = vColor>>8; 			// 基本图形的颜色，存入vAddr+2
	gLCDSStr[gLCDSLen++] = vColor; 				// 
	gLCDSStr[gLCDSLen++] = 0xFF; 				// 结束基本图形
	gLCDSStr[gLCDSLen++] = 0; 					// 
	gLCDSStr[2] = gLCDSLen-3; 					// 
	UARTSend(1, gLCDSStr, gLCDSLen);
}

////  画圆
void LCD_Draw_Cycle(ushort vAddr, ushort vX, ushort vY, ushort vRad, ushort vColor)
{
		gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = 0x5a;
	gLCDSStr[gLCDSLen++] = 0xa5;
	gLCDSStr[gLCDSLen++] = 0; 					// 长度，可最后填充
	gLCDSStr[gLCDSLen++] = 0x82; 				// 写数据存储区指令
	gLCDSStr[gLCDSLen++] = vAddr>>8; 			// DGUS组态中的基本图形地址
	gLCDSStr[gLCDSLen++] = vAddr; 				// 
	gLCDSStr[gLCDSLen++] = 0; 					// 基本图形的画圆指令	0005，存入vAddr
	gLCDSStr[gLCDSLen++] = 5; 					// 
	gLCDSStr[gLCDSLen++] = 0; 					// 基本图形的画矩形个数	000x，存入vAddr+1
	gLCDSStr[gLCDSLen++] = 1; 					// 
	gLCDSStr[gLCDSLen++] = vX>>8; 			// 基本图形的X0，存入vAddr+3
	gLCDSStr[gLCDSLen++] = vX; 				// 
	gLCDSStr[gLCDSLen++] = vY>>8; 			// 基本图形的Y0，存入vAddr+4
	gLCDSStr[gLCDSLen++] = vY; 				// 
	gLCDSStr[gLCDSLen++] = vRad>>8; 			// 基本图形的圆的半径，存入vAddr+5
	gLCDSStr[gLCDSLen++] = vRad; 			// 
	gLCDSStr[gLCDSLen++] = vColor>>8; 			// 基本图形的颜色，存入vAddr+2
	gLCDSStr[gLCDSLen++] = vColor; 				// 
	gLCDSStr[gLCDSLen++] = 0xFF; 				// 结束基本图形
	gLCDSStr[gLCDSLen++] = 0; 					// 
	gLCDSStr[2] = gLCDSLen-3; 					// 
	UARTSend(1, gLCDSStr, gLCDSLen);
}

////  图片区域剪切、黏贴，从(vX,vY) 剪切到 (vXD, vYD)
void LCD_Cut_PICRect(ushort vAddr, ushort vPicID, ushort vX, ushort vY, 
		ushort vW, ushort vH, ushort vXD, ushort vYD, ushort vColor)
{
		gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = 0x5a;
	gLCDSStr[gLCDSLen++] = 0xa5;
	gLCDSStr[gLCDSLen++] = 0; 					// 长度，可最后填充
	gLCDSStr[gLCDSLen++] = 0x82; 				// 写数据存储区指令
	gLCDSStr[gLCDSLen++] = vAddr>>8; 			// DGUS组态中的基本图形地址
	gLCDSStr[gLCDSLen++] = vAddr; 				// 
	gLCDSStr[gLCDSLen++] = 0; 					// 基本图形的图片区域剪切指令	0006，存入vAddr
	gLCDSStr[gLCDSLen++] = 6; 					// 
	gLCDSStr[gLCDSLen++] = 0; 					// 基本图形的画矩形个数	000x，存入vAddr+1
	gLCDSStr[gLCDSLen++] = 1; 					// 
	gLCDSStr[gLCDSLen++] = vPicID; 				// 待剪切的 PIC_ID

	gLCDSStr[gLCDSLen++] = vX>>8; 				// 待剪切图形的X0，存入vAddr+3
	gLCDSStr[gLCDSLen++] = vX; 					// 
	gLCDSStr[gLCDSLen++] = vY>>8; 				// 待剪切图形的Y0，存入vAddr+4
	gLCDSStr[gLCDSLen++] = vY; 					// 
	gLCDSStr[gLCDSLen++] = (vX+vW)>>8; 			// 待剪切图形的X1，存入vAddr+5
	gLCDSStr[gLCDSLen++] = (vX+vW); 			// 
	gLCDSStr[gLCDSLen++] = (vY+vH)>>8; 			// 待剪切图形的Y1，存入vAddr+6
	gLCDSStr[gLCDSLen++] = (vY+vH); 			// 
	gLCDSStr[gLCDSLen++] = (vXD)>>8; 			// 待黏贴图形的X0，存入vAddr+7
	gLCDSStr[gLCDSLen++] = (vXD); 				// 
	gLCDSStr[gLCDSLen++] = (vYD)>>8; 			// 待黏贴图形的Y0，存入vAddr+8
	gLCDSStr[gLCDSLen++] = (vYD); 				// 
	//gLCDSStr[gLCDSLen++] = vColor>>8; 			// 基本图形的颜色，存入vAddr+2
	//gLCDSStr[gLCDSLen++] = vColor; 				// 
	gLCDSStr[gLCDSLen++] = 0xFF; 				// 结束基本图形
	gLCDSStr[gLCDSLen++] = 0; 					// 
	gLCDSStr[2] = gLCDSLen-3; 					// 
	UARTSend(1, gLCDSStr, gLCDSLen);
}

////  显示图标
void LCD_Disp_ICON(ushort vAddr, ushort vIconID, ushort vX, ushort vY)
{
		gLCDSLen = 0;	   
	gLCDSStr[gLCDSLen++] = 0x5a;
	gLCDSStr[gLCDSLen++] = 0xa5;
	gLCDSStr[gLCDSLen++] = 0; 					// 长度，可最后填充
	gLCDSStr[gLCDSLen++] = 0x82; 				// 写数据存储区指令
	gLCDSStr[gLCDSLen++] = vAddr>>8; 			// DGUS组态中的基本图形地址
	gLCDSStr[gLCDSLen++] = vAddr; 				// 
	gLCDSStr[gLCDSLen++] = 0; 					// 显示图标指令	0007，存入vAddr
	gLCDSStr[gLCDSLen++] = 7; 					// 
	gLCDSStr[gLCDSLen++] = 0; 					// 基本图形的画矩形个数	000x，存入vAddr+1
	gLCDSStr[gLCDSLen++] = 1; 					// 

	gLCDSStr[gLCDSLen++] = vX>>8; 				// 图标显示的X0，存入vAddr+3
	gLCDSStr[gLCDSLen++] = vX; 					// 
	gLCDSStr[gLCDSLen++] = vY>>8; 				// 图标显示的Y0，存入vAddr+4
	gLCDSStr[gLCDSLen++] = vY; 					// 
	gLCDSStr[gLCDSLen++] = vIconID>>8; 				// 图标ID
	gLCDSStr[gLCDSLen++] = vIconID; 				// 图标ID
	gLCDSStr[gLCDSLen++] = 0xFF; 				// 结束基本图形
	gLCDSStr[gLCDSLen++] = 0; 					// 
	gLCDSStr[2] = gLCDSLen-3; 					// 
	UARTSend(1, gLCDSStr, gLCDSLen);
}








#endif

