/****************************************************************************
*
*	 APP_LCD 与 LCD 界面显示有关的函数
*
*
*****************************************************************************/

#include "app_lcd.h"
#include "LPC17xx.h"
#include "config.h"

/*********************************************************************************************************************
*
*  图形化表现 按钮及纽子开关 
*
*********************************************************************************************************************/
//// LCD 显示页面中，各输入框位置(x, y)及(x1, y1)
const ushort gLCD_IO_XY[6][4]=				// 纽子开关的图标，依次为： 中，上，下，左，右
{
	{55, 152, 85, 191}, {55, 118, 85, 157}, {55, 188, 85, 227},   {170, 70, 192, 100},   {20, 155, 50, 185}, {90, 155, 120, 185}
//	{55, 155, 85, 185}, {55, 118, 85, 156}, {55, 188, 85, 226}, {20, 155, 50, 185}, {90, 155, 120, 185}
};
const ushort gLCD_KEY_XY[3][4]=				// 按键开关的图标，依次为： 黑，绿，红
{
	{18, 72, 43, 97}, {48, 72, 73, 97}, {78, 72, 103, 97}, 
};
   struct	remote
 {

 int L1;
 int L2;
 int L3;
 int L4;
 int L5;
 int L6;
 int L7; 
 int L8;   
 char message_1;
 char message_2;
 char message_3;
 char message_4;
 char message_5;
 char message_6;  
 char message_7; 
 char message_8;   
 };
 extern  struct remote remote_18c;

mLCDPage gLCDPage;

void ( * gp_lcdtask)(void);
void ( * gp_lcdtaskNext)(void);
unsigned short gLcdFresh = 4;
unsigned short gDataFresh = 0;

	uchar   gPasswordIn[8];
	uchar   N01_BUF;
	uchar   N02_BUF;
 signed short  angle_prif =0 ;
ushort gBKColor = 0x1f;			 // 背景色
ushort gFTColor = 0xffff;			 // 前景色
	ushort gLCDX=0;				 // X
	ushort gLCDY=0;				 // Y
	ushort gLCDX1=0;				 // X1
	ushort gLCDY1=0;				 // Y1
	ushort gLCDW=0;				 // 宽
	ushort gLCDH=0;				 // 高

//unsigned char gWarnTxt[10][15] = {"急停      ", "电池电量低", "通信异常  ", "自检异常  ","无线信号弱"};
unsigned char gWarnTxt[8][18] = {"蓄电池电压低    ","紧急停止        ", "相序错误        ", "                "
				,"                ", "                ", "                ", "                "};


// 延时 x 个指令周期
void delay_xloop(unsigned short vLp)
{
	while(vLp--);
}
/****************************************************************************
*
*  字符串 与 数据 格式转换函数
*
*****************************************************************************/
unsigned long DStrToInt(unsigned char* vInBuf)
{
	unsigned long vLout = 0;
	unsigned char i = 0;
	unsigned char ch = 0;

	for(i=0; i<16; i++)
	{
		ch = *vInBuf++;
		if(ch == 0)
			return vLout;
		vLout = vLout * 10;
		if((ch>='0') && (ch<='9'))
		{
			vLout += ch-'0';
		}		
	}
	return vLout;
}
unsigned long HStrToInt(unsigned char* vInBuf)
{
	unsigned long vLout = 0;
	unsigned char i = 0;
	unsigned char ch = 0;

	for(i=0; i<16; i++)
	{
		ch = *vInBuf++;
		if(ch == 0)
			return vLout;
		vLout <<= 4;
		if((ch>='0') && (ch<='9'))
		{
			ch = ch-'0';
		}		
		if((ch>='A') && (ch<='F'))
		{
			ch = ch - 'A' + 10;
		}		
		if((ch>='a') && (ch<='f'))
		{
			ch = ch - 'a' + 10;
		}	
		vLout += ch;
	}
	return vLout;
}
//// 格式转换，如: "1F342C556d4f3341" --> str[]={0x1F,0x34,0x2C,0x55,0x6D,0x4F,0x33,0x41}
void HBCDtoHStr(unsigned char* vInBuf, unsigned char* vOutBuf)
{
	unsigned char i = 0;
	unsigned char j = 0;
	unsigned char ch = 0;
	unsigned char chOut = 0;

	for(i=0; i<32; )
	{
		i++;
		ch = *vInBuf++;
		if(ch == 0)
			return ;
		if((ch>='0') && (ch<='9'))
		{
			ch = ch-'0';
		}		
		if((ch>='A') && (ch<='F'))
		{
			ch = ch - 'A' + 10;
		}		
		if((ch>='a') && (ch<='f'))
		{
			ch = ch - 'a' + 10;
		}	
		chOut = (chOut<< 4) + ch;
		if((i & 0x01)== 0)		// 偶数个字节
		{
			vOutBuf[j++] = chOut;
			chOut = 0;
		}
	}
}


/****************************************************************************
*
*  LCD 显示界面：主界面
*
*****************************************************************************/

// 电池电量
//const ushort gLCDM_Bat_Tab[3][4]={{295,35, 315,48}, {295, 6, 318, 19}, {295, 55, 318, 68}};
//// 无线信号强弱的图标 ,0~3
const ushort gLCDM_WL_Tab[5][4]={					
	{138,117, 163,139}, {164,117, 189,139}, {190, 117, 215, 139}, {216, 117, 241, 139}, {243, 117, 268, 139}
};

//// 电池电量 
const ushort gLCDM_Bat_Tab[5][4]={
	{137,146, 161,168}, {163,146, 187,168}, {189, 146, 213, 168}, {215, 146, 239, 168}, {243, 146, 266, 168}
};


void Disp_PicX(unsigned char vPic)
{
	LCD_Disp_PIC(vPic);
}
// 清理界面
void clear_lcd()
{
	// 清空第一栏，-5是为了不清空电池、信号图标
	for(uint8_t i = 0; i < 320/16 - 5; i++)
	{
		LCD_Disp_Txt(FONT_16, i*16, 5, "  ");
	}

	for(uint8_t y = 0; y < 240/16 - 2; y++)
	{
		for(uint8_t x = 0; x < 320/16; x++)
		{
			LCD_Disp_Txt(FONT_16, x*16, y*16+30, "  ");
		}
	}
}
// 立拱模式界面
void LiGong_mode()
{
	static const uint8_t high = 30;        // 行高
	static const uint8_t margL = 10;       // 左边距
	static const uint8_t margR = 10;       // 右边距
	static const uint8_t Vermid = 320 / 2; // 竖中间线
	static const uint8_t Hormid = 240 / 2; // 横中间线

	LCD_Set_Color(COLOR_WHITE_, -1);
	// 模式显示
	if(_BitV(gDIBitV, 0) && _BitV(gDIBitV, 8) && _BitV(gDIBitV, 9))// 中臂&&!立拱&&!凿岩
		LCD_Disp_Txt(FONT_16, Vermid-16*4/2, 5,"模式未选择");
	else
		LCD_Disp_Txt(FONT_16, Vermid-16*4/2, 5,"立拱模式");
	
	LCD_Disp_Txt(FONT_32, Vermid-208/2, Hormid-32/2,"当前臂架:");// 32*4+16+32*2=208
	if(_BitV(gDIBitV, 2))
		LCD_Disp_Txt(FONT_32, Vermid-208/2+32*4+16, Hormid-32/2,"左臂");
	else if(_BitV(gDIBitV, 3))
		LCD_Disp_Txt(FONT_32, Vermid-208/2+32*4+16, Hormid-32/2,"右臂");
	else if(!_BitV(gDIBitV, 2) && !_BitV(gDIBitV, 3))
		LCD_Disp_Txt(FONT_32, Vermid-208/2+32*4+16, Hormid-32/2,"中臂");

}
// 凿岩模式界面
void Drilling_mode()
{
	static const uint8_t high = 30;     // 行高
	static const uint8_t margL = 10;    // 左边距
	static const uint8_t margR = 10;    // 右边距
	static const uint8_t Vermid = 320 / 2; // 竖中间线
	static const uint8_t Hormid = 240 / 2; // 横中间线

	LCD_Set_Color(COLOR_WHITE_, -1);
	// 模式显示
	LCD_Disp_Txt(FONT_16, Vermid-112/2, 5,"凿岩模式(中臂)");//16*6+8*2=112
	// 第一列
	LCD_Disp_Txt(FONT_16, margL, 30+high*0,"臂架压力:");//16*4+8+8*5+8*3=136,每行设置20
	LCD_Disp_Txt(FONT_16, margL, 30+high*1,"冲击压力:");
	LCD_Disp_Txt(FONT_16, margL, 30+high*2,"缓冲压力:");
	LCD_Disp_Txt(FONT_16, margL, 30+high*3,"水路压力:");
	LCD_Disp_Txt(FONT_16, margL, 30+high*4,"泵站:");
	LCD_Disp_Txt(FONT_16, margL, 30+high*5,"水阀:");

	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, 30+high*0,"bar");
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, 30+high*1,"bar");
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, 30+high*2,"bar");
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, 30+high*3,"bar");
	
	// 第二列
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136, 30+high*0,"回转压力:");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136, 30+high*1,"推进压力:");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136, 30+high*2,"润滑压力:");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136, 30+high*3,"注浆压力:");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136, 30+high*4,"空压机:");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136, 30+high*5,"水泵:");

	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*4+8*6, 30+high*0,"bar");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*4+8*6, 30+high*1,"bar");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*4+8*6, 30+high*2,"bar");
	LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*4+8*6, 30+high*3,"bar");
	// 显示数据数字
	LCD_Set_Color(COLOR_YELLW_, -1);
	// 第一列
	LCD_Disp_Intx(FONT_16, margL+16*4+8, 30+high*0, KX_119_recv_press.Press_Boom, 5);
	LCD_Disp_Intx(FONT_16, margL+16*4+8, 30+high*1, KX_119_recv_press.Press_Impact, 5);
	LCD_Disp_Intx(FONT_16, margL+16*4+8, 30+high*2, KX_119_recv_press.Press_Buffer, 5);
	LCD_Disp_Intx(FONT_16, margL+16*4+8, 30+high*3, KX_119_recv_press.Press_Water, 5);
	if(_BitV(gDIBitV, 17)) LCD_Disp_Txt(FONT_16, margL+16*2+8, 30+high*4, "开启"); else LCD_Disp_Txt(FONT_16, margL+16*2+8, 30+high*4, "关闭"); 
	if(_BitV(gDIBitV, 18)) LCD_Disp_Txt(FONT_16, margL+16*2+8, 30+high*5, "开启"); else LCD_Disp_Txt(FONT_16, margL+16*2+8, 30+high*5, "关闭"); 
	// 第二列
	LCD_Disp_Intx(FONT_16, (Vermid*2-margR-136)+16*4+8, 30+high*0, KX_119_recv_press.Press_Rotate, 5);
	LCD_Disp_Intx(FONT_16, (Vermid*2-margR-136)+16*4+8, 30+high*1, KX_119_recv_press.Press_Push, 5);
	LCD_Disp_Intx(FONT_16, (Vermid*2-margR-136)+16*4+8, 30+high*2, KX_119_recv_press.Press_Lube, 5);
	LCD_Disp_Intx(FONT_16, (Vermid*2-margR-136)+16*4+8, 30+high*3, KX_119_recv_press.Press_Grout, 5);
	if(_BitV(gDIBitV, 21)) LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*3+8, 30+high*4, "开启"); else LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*3+8, 30+high*4, "关闭"); 
	if(_BitV(gDIBitV, 22)) LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*2+8, 30+high*5, "开启"); else LCD_Disp_Txt(FONT_16, Vermid*2-margR-136+16*2+8, 30+high*5, "关闭"); 
}

void Disp_mainSPJ(void)
{
//	unsigned char vFlg = 0;
	unsigned char i=0;
	unsigned char intit =0;
	unsigned short vXX = 0;
	unsigned short vYY = 0;
	static unsigned char stArmOp = 0;
	static unsigned char stLoop = LCD_FRESH_WARN;
	static unsigned char stWLFlg = 55;
	static unsigned char stBATFlg = 55;
	unsigned char vFlg = 0;
	static  int stWarnCnt = 0;
	static  char stCnt = 0;
	float fWKTm = 0.0;
	static  char stPowChk = 0;								// 20180506   lsg 

//		gp_lcdtask = Disp_ZZT_CtrlStatus;
//		return;
	
	if(gLcdFresh >= 3)
	{
		gLcdFresh--;
		LCD_Disp_PIC(31);
		return;
	}
	stLoop++;
	if(stLoop >	LCD_FRESH_WARN)
	{
		stLoop = 0;
		vFlg = 1;
/*		
		gRunInfo.vLJWkTime++;
		gWarnFlg++;
		if(gWarnFlg > 7)
			gWarnFlg = 0;
		*/
	}

	if(gLcdFresh == 2)
	{
		gLcdFresh = 1;
		LCD_Set_Cursor(0,1,1,2,2);
		//gLCDPage.page = gLCDPage.cmdsel;
		LCD_Disp_PIC(31);
		stWLFlg = 55;
		stBATFlg = 55;
	}											//  moment_percentage
	if((gLcdFresh == 1)||(gDataFresh == 1))
	{
		if(gLcdFresh == 1)
			gLcdFresh = 0;
		if(gDataFresh == 1)
			gDataFresh = 0;				   
			LCD_Set_Color(COLOR_BLUE_,-1);
	if(intit==0)
	{
		// LCD_Disp_Txt(FONT_24, 0x13, 0x35, "旋转压力：");
	//	 LCD_Disp_Txt(FONT_24, 0x13, 0x63, "推进压力：");
	//	 LCD_Disp_Txt(FONT_24, 0x13, 0x8f , "冲击压力: ");
															    
	  
	  	intit =1;
	}

#if 1
	static uint8_t swi = 0; // 0表示凿岩界面，1表示立拱界面
	if(!_BitV(gDIBitV, 2) && !_BitV(gDIBitV, 3) && _BitV(gDIBitV, 9))// 切换到中臂时, 且处于凿岩模式
	{
		if(swi == 1)
		{
			swi = 0;
			clear_lcd();
		}
		Drilling_mode();
	}
	else// 左臂或右臂时,只有立拱模式
	{
		if(swi == 0)
		{
			swi = 1;
			clear_lcd();
		}
		LiGong_mode();
	}
		
#else
     	LCD_Disp_Intx(FONT_24, 81, 31, remote_18c.L1, 3);  // 回转 			   
     	LCD_Disp_Intx(FONT_24, 81, 72, remote_18c.L2, 3);  //冲击压力
     	LCD_Disp_Intx(FONT_24, 81, 118, remote_18c.L3, 3);  //推进压力 	   
     	LCD_Disp_Intx(FONT_24, 81, 162, remote_18c.L4, 3);  //油雾  
     	LCD_Disp_Intx(FONT_24, 81, 204, remote_18c.L5, 3);  //水压 
		if( _BitV( remote_18c.L6 ,4) )
			{	LCD_Set_Color(COLOR_YELLW_,-1);	LCD_Disp_Txt(FONT_16, 120, 8, "    空打      ");}  
		else if ( _BitV( remote_18c.L6 ,5))
			{	LCD_Set_Color(COLOR_YELLW_,-1);	LCD_Disp_Txt(FONT_16, 120, 8, "  一级防卡！  ");}   
		else if ( _BitV( remote_18c.L6 ,6))
			{	LCD_Set_Color(COLOR_YELLW_,-1);	LCD_Disp_Txt(FONT_16, 120, 8, "  二级防卡！  ");}   
		else
			{	LCD_Disp_Txt(FONT_16, 120, 8, "              ");}  
		 		LCD_Set_Color(COLOR_BLUE_,-1);				   		 
		if (remote_18c.message_1){	LCD_Disp_Txt(FONT_16, 274, 35, "开启");   	} else {LCD_Disp_Txt(FONT_16, 274, 35, "关闭");  }
		if (remote_18c.message_2){	LCD_Disp_Txt(FONT_16, 274, 69, "开启");   	} else {LCD_Disp_Txt(FONT_16, 274, 69, "关闭");  }
		if (remote_18c.message_3){	LCD_Disp_Txt(FONT_16, 274, 106, "开启");   	} else {LCD_Disp_Txt(FONT_16, 274, 106, "关闭");  }
		if (remote_18c.message_4){	LCD_Disp_Txt(FONT_16, 274, 143, "开启");   	} else {LCD_Disp_Txt(FONT_16, 274, 143, "关闭");  }
		if (remote_18c.message_5){	LCD_Disp_Txt(FONT_16, 274, 177, "开启");   	} else {LCD_Disp_Txt(FONT_16, 274, 177, "关闭");  }
if (remote_18c.message_8) {	 if (remote_18c.message_6){	LCD_Disp_Txt(FONT_16, 200, 219, "高冲");   	} else {LCD_Disp_Txt(FONT_16, 200, 219, "低冲");  }	 }
		if(!remote_18c.message_8){	LCD_Disp_Txt(FONT_16, 200, 219, "     ");   	}
		if (remote_18c.message_7){LCD_Set_Color(COLOR_YELLW_,-1); 	LCD_Disp_Txt(FONT_16, 240, 219, " 自动凿岩 ");   	} else {LCD_Disp_Txt(FONT_16, 240, 219, "         ");  }
#endif
		if(vFlg)
		{
 				 
		if(gPowerChk)
		LCD_Set_Color(COLOR_RED_,-1);
			if(gPowerChk>>16)									// 20180506   lsg 
			{
				LCD_Disp_Txt(FONT_16, 190, 33+24*6, "内部电压异常！");
				stPowChk = 1;
			}
			else if(gPowerChk & 0xffff)
			{
				LCD_Disp_Txt(FONT_16, 190, 33+24*6, "摇杆数据异常！");
				stPowChk = 2;
			}
			else if(stPowChk )
			{
				LCD_Disp_Txt(FONT_16, 190, 33+24*6, "              ");
				stPowChk = 0;
			}
		if(gPowerChk)
		LCD_Set_Color(COLOR_WHITE_,-1);
				
				
			////  无线信号
			vXX = 290;
			vYY = 2;
			if(stWLFlg != gWLSingV)
			{
				i = (unsigned char)(gWLSingV>>1);
				if(gWLSingV > 7)					//  0~10
					i = 3;
				else if(gWLSingV > 4)					//  0~10
					i = 2;
				else if(gWLSingV > 1)					//  0~10
					i = 1;
				else 
					i = 0;
				if(gLaBaFlg == 0)
					i = 0;
				LCD_Cut_PICRect(0, 14, gLCDM_WL_Tab[i][0], gLCDM_WL_Tab[i][1],
					gLCDM_WL_Tab[i][2], gLCDM_WL_Tab[i][3], vXX, vYY);
			}
			//// 电池电量 
			if(gCellPower > 7)					//  0~10
				i = 3;
			else if(gCellPower > 4)					//  0~10
				i = 2;
			else if(gCellPower > 1)					//  0~10
				i = 1;
			else
				i = 0;
			if(stBATFlg != i)
			{
				stBATFlg = i;
				vXX = 264;
				vYY = 2;
				LCD_Cut_PICRect(0, 14, gLCDM_Bat_Tab[i][0], gLCDM_Bat_Tab[i][1],
					gLCDM_Bat_Tab[i][2], gLCDM_Bat_Tab[i][3],vXX, vYY);
			}
		}
	}
	if(gLCDBKOnFlg == 0)
	Key_MainZZT();
}


void Disp_Rock_Pic(unsigned char reFlg)
{
		unsigned char vTmpV = 0;
	unsigned char vTmpV0 = 0;
	unsigned short vXX = 0;
	unsigned short vYY = 0;
	unsigned char  vPicX = 14;
	static unsigned char stDisp = 0;
	static unsigned long  stDIBitV = 0;
	static unsigned long  stDIBitV1 = 0;
	static unsigned long  stDIBitV2 = 0;

			////////////////////////////////////////  左右两端的按钮，急停  ////////////////////////////////////////
			//////////////////////////////  左  
		if(stDisp == 0)
		{
			vXX = 2;
			vYY = 105;
			vTmpV = (gDIBitV>>16) & 0x01;
			vTmpV0 = (stDIBitV>>16) & 0x01;
			if((vTmpV != vTmpV0) || reFlg)
			{
				LCD_Cut_PICRect(0, vPicX, gLCD_KEY_XY[vTmpV][0], gLCD_KEY_XY[vTmpV][1],
					gLCD_KEY_XY[vTmpV][2], gLCD_KEY_XY[vTmpV][3], vXX, vYY);
			}
			vYY += 25;
			vTmpV = (gDIBitV>>17) & 0x01;
			vTmpV0 = (stDIBitV>>17) & 0x01;
			if((vTmpV != vTmpV0) || reFlg)
			{
				LCD_Cut_PICRect(0, vPicX, gLCD_KEY_XY[vTmpV][0], gLCD_KEY_XY[vTmpV][1],
					gLCD_KEY_XY[vTmpV][2], gLCD_KEY_XY[vTmpV][3], vXX, vYY);
			}
			vYY += 25;
			vTmpV = (gDIBitV>>18) & 0x01;
			vTmpV0 = (stDIBitV>>18) & 0x01;
			if((vTmpV != vTmpV0) || reFlg)
			{
				LCD_Cut_PICRect(0, vPicX, gLCD_KEY_XY[vTmpV][0], gLCD_KEY_XY[vTmpV][1],
					gLCD_KEY_XY[vTmpV][2], gLCD_KEY_XY[vTmpV][3], vXX, vYY);
			}
			vYY += 25;
			vTmpV = (gDIBitV>>19) & 0x01;
			vTmpV0 = (stDIBitV>>19) & 0x01;
			if((vTmpV != vTmpV0) || reFlg)
			{
				LCD_Cut_PICRect(0, vPicX, gLCD_KEY_XY[vTmpV][0], gLCD_KEY_XY[vTmpV][1],
					gLCD_KEY_XY[vTmpV][2], gLCD_KEY_XY[vTmpV][3], vXX, vYY);
			}
		}
			//////////////////////////////  右  
		if(stDisp == 0)
		{
			vXX = 320-26;
			vYY = 105;
			vTmpV = (gDIBitV>>20) & 0x01;
			vTmpV0 = (stDIBitV>>20) & 0x01;
			if((vTmpV != vTmpV0) || reFlg)
			{
				LCD_Cut_PICRect(0, vPicX, gLCD_KEY_XY[vTmpV][0], gLCD_KEY_XY[vTmpV][1],
					gLCD_KEY_XY[vTmpV][2], gLCD_KEY_XY[vTmpV][3], vXX, vYY);
			}
			vYY += 25;
			vTmpV = (gDIBitV>>21) & 0x01;
			vTmpV0 = (stDIBitV>>21) & 0x01;
			if((vTmpV != vTmpV0) || reFlg)
			{
				LCD_Cut_PICRect(0, vPicX, gLCD_KEY_XY[vTmpV][0], gLCD_KEY_XY[vTmpV][1],
					gLCD_KEY_XY[vTmpV][2], gLCD_KEY_XY[vTmpV][3], vXX, vYY);
			}
			vYY += 25;
			vTmpV = (gDIBitV>>22) & 0x01;
			vTmpV0 = (stDIBitV>>22) & 0x01;
			if((vTmpV != vTmpV0) || reFlg)
			{
				LCD_Cut_PICRect(0, vPicX, gLCD_KEY_XY[vTmpV][0], gLCD_KEY_XY[vTmpV][1],
					gLCD_KEY_XY[vTmpV][2], gLCD_KEY_XY[vTmpV][3], vXX, vYY);
			}
			vYY += 25;
			vTmpV = (gDIBitV>>23) & 0x01;
			vTmpV0 = (stDIBitV>>23) & 0x01;
			if((vTmpV != vTmpV0) || reFlg)
			{
				LCD_Cut_PICRect(0, vPicX, gLCD_KEY_XY[vTmpV][0], gLCD_KEY_XY[vTmpV][1],
					gLCD_KEY_XY[vTmpV][2], gLCD_KEY_XY[vTmpV][3], vXX, vYY);
			}
			vYY = 5;
			vTmpV = 0;
			if(gJiTing == 0x55)
			{
				vTmpV = 2;
			}
	//			LCD_Cut_PICRect(0, vPicX, gLCD_KEY_XY[vTmpV][0], gLCD_KEY_XY[vTmpV][1],
	//				gLCD_KEY_XY[vTmpV][2], gLCD_KEY_XY[vTmpV][3], vXX, vYY);
		}
		if(stDisp == 0)
		{
			////////////////////////////////////////  第一排纽子  ////////////////////////////////////////
			vXX = 45;
			vYY = 132;
			vTmpV = (gDIBitV >> 2) & 0x03;
			vTmpV0 = (stDIBitV1 >> 2) & 0x03;
			if((vTmpV != vTmpV0) || reFlg)
			{
				//vTmpV = (vTmpV + 1) ^ 3;					// 两档纽子，占一位 
				LCD_Cut_PICRect(0, vPicX, gLCD_IO_XY[vTmpV][0], gLCD_IO_XY[vTmpV][1],
					gLCD_IO_XY[vTmpV][2], gLCD_IO_XY[vTmpV][3], vXX, vYY);
			}
			vXX += 40;
			vTmpV = (gDIBitV >> 1) & 0x01;
			vTmpV0 = (stDIBitV1 >> 1) & 0x01;
			if((vTmpV != vTmpV0) || reFlg)
			{
					vTmpV += 1;
				LCD_Cut_PICRect(0, vPicX, gLCD_IO_XY[vTmpV][0], gLCD_IO_XY[vTmpV][1],
					gLCD_IO_XY[vTmpV][2], gLCD_IO_XY[vTmpV][3], vXX, vYY);
			}
			
			vXX += 120;
			vTmpV = (gDIBitV>>8) & 0x03;
			vTmpV0 = (stDIBitV1>>8) & 0x03;
			if((vTmpV != vTmpV0) || reFlg)
			{
				if(vTmpV == 1)
					vTmpV = 2;
				else if(vTmpV == 2)
					vTmpV = 1;
				LCD_Cut_PICRect(0, vPicX, gLCD_IO_XY[vTmpV][0], gLCD_IO_XY[vTmpV][1],
					gLCD_IO_XY[vTmpV][2], gLCD_IO_XY[vTmpV][3], vXX, vYY);
			}
			vXX += 40;
			vTmpV = (gDIBitV>>10) & 0x03;
			vTmpV0 = (stDIBitV1>>10) & 0x03;
			if((vTmpV != vTmpV0) || reFlg)
			{
				if(vTmpV == 1)
					vTmpV = 2;
				else if(vTmpV == 2)
					vTmpV = 1;
				LCD_Cut_PICRect(0, vPicX, gLCD_IO_XY[vTmpV][0], gLCD_IO_XY[vTmpV][1],
					gLCD_IO_XY[vTmpV][2], gLCD_IO_XY[vTmpV][3], vXX, vYY);
			}
		//delay_ms(3);
		}
		
		if(stDisp == 0)
		{
			////////////////////////////////////////  第二排纽子  ////////////////////////////////////////
			vXX = 45;
			vYY = 170;
			vTmpV = (gDIBitV>>4) & 0x03;
			vTmpV0 = (stDIBitV2>>4) & 0x03;
			if((vTmpV != vTmpV0) || reFlg)
			{
				//vTmpV = (vTmpV + 1) ^ 3;
				if(vTmpV > 2)
					vTmpV = 0;

				LCD_Cut_PICRect(0, vPicX, gLCD_IO_XY[vTmpV][0], gLCD_IO_XY[vTmpV][1],
					gLCD_IO_XY[vTmpV][2], gLCD_IO_XY[vTmpV][3], vXX, vYY);
			}
		
			vXX += 40;
			vTmpV = (gDIBitV>>6) & 0x03;
			vTmpV0 = (stDIBitV2>>6) & 0x03;
			if((vTmpV != vTmpV0) || reFlg)
			{
				LCD_Cut_PICRect(0, vPicX, gLCD_IO_XY[vTmpV][0], gLCD_IO_XY[vTmpV][1],
					gLCD_IO_XY[vTmpV][2], gLCD_IO_XY[vTmpV][3], vXX, vYY);
			}
			vXX += 120;
			vTmpV = (gDIBitV>> 12) & 0x03;
			vTmpV0 = (stDIBitV2>> 12) & 0x03;
			if((vTmpV != vTmpV0) || reFlg)
			{
				if(vTmpV == 1)
					vTmpV = 2;
				else if(vTmpV == 2)
					vTmpV = 1;
				else
					vTmpV = 0;
				LCD_Cut_PICRect(0, vPicX, gLCD_IO_XY[vTmpV][0], gLCD_IO_XY[vTmpV][1],
					gLCD_IO_XY[vTmpV][2], gLCD_IO_XY[vTmpV][3], vXX, vYY);
			}
			vXX += 40;
			vTmpV = (gDIBitV>>28) & 0x03;
			vTmpV0 = (stDIBitV2>>28) & 0x03;
			if((vTmpV != vTmpV0) || reFlg)
			{
				LCD_Cut_PICRect(0, vPicX, gLCD_IO_XY[vTmpV][0], gLCD_IO_XY[vTmpV][1],
					gLCD_IO_XY[vTmpV][2], gLCD_IO_XY[vTmpV][3], vXX, vYY);
			}
			
			////////////////////////////////////////  第三排纽子  ////////////////////////////////////////
/*
		delay_ms(3);
			vXX = 125;
			vYY = 170;
			vTmpV = (gDIBitV>>14) & 0x01;
			vTmpV0 = (stDIBitV2>>14) & 0x01;
			if((vTmpV != vTmpV0) || reFlg)
			{
				vTmpV = (vTmpV + 1) ^ 3;					// 两档纽子，占一位 
				LCD_Cut_PICRect(0, vPicX, gLCD_IO_XY[vTmpV][0], gLCD_IO_XY[vTmpV][1],
					gLCD_IO_XY[vTmpV][2], gLCD_IO_XY[vTmpV][3], vXX, vYY);
			}
			vXX += 40;
			vTmpV = (gDIBitV>>13) & 0x01;
			vTmpV0 = (stDIBitV2>>13) & 0x01;
			if((vTmpV != vTmpV0) || reFlg)
			{
				vTmpV = (vTmpV + 1) ^ 3;					// 两档纽子，占一位 
				LCD_Cut_PICRect(0, vPicX, gLCD_IO_XY[vTmpV][0], gLCD_IO_XY[vTmpV][1],
					gLCD_IO_XY[vTmpV][2], gLCD_IO_XY[vTmpV][3], vXX, vYY);
			}
*/			
		}
		
		if(stDisp == 0)
			stDIBitV = gDIBitV;
		if(stDisp == 0)
			stDIBitV1 = gDIBitV;
		if(stDisp == 0)
			stDIBitV2 = gDIBitV;
	//	stDisp++;
		if(stDisp > 2)
			stDisp = 0;
}

//const unsigned short romArmValDisp[6][2]={{25,147},{178,147},{25,175},{178,175},{178,175},{0,0}};
//const unsigned short romArmValDisp[8][2]={{50,93},{95,93},{140,93},{185,93},{230,93},{275,93},{313,93}};
const unsigned short romArmValDisp[8][2]={{40,58},{75,58},{110,58},{145,58},{180,58},{215,58},{250,58} ,{285,58}};	// 摇杆值
 unsigned short romArmDSDisp[8][2]={{40,81},{75,81},{110,81},{145,81},{180,81},{215,81},{250,81} ,{285,81}};		// 死区值 1~9
const unsigned char romDI_DispDO_TabZZT[8] = {0,1, 2, 3,4,5,99};
// 显示遥控按键
void Disp_ZZT_CtrlStatus(void)
{
	unsigned char i = 0;
//	unsigned char k = 0;
	unsigned char j = 0;
	unsigned char vLen = 0;
	char vStr[8];
	unsigned short vDispLine1 = 102;
	unsigned short vDispLine2 = 116;
	unsigned short vDispLine3 = 130;
	static unsigned char stPowChk = 0;								// 20180506   lsg 
//	unsigned char ReDrawFlg =0;
	static	unsigned char ReDrawFlg =0;
	char  vArmConV[8];
	unsigned short vDX = 120;
	unsigned short vDY = 116;

	if(gLcdFresh > 2)
	{
		gLcdFresh--;
		LCD_Disp_PIC(7);
		return;
	}
	if(gLcdFresh == 2)
	{
		//LCD_Set_Color(COLOR_RED_,-1);
		LCD_Get_Color(0,5,5);
		gLcdFresh = 1;
		LCD_Disp_PIC(7);
		#if _XN_LiuL
			//LCD_Disp_Txt(FONT_16, 2, vDispLine1, "旋钮:");
			LCD_Disp_Arc(1, 42, vDispLine1+8, 8, 80, 640);
		#endif
//			LCD_Disp_Arc(1, 245, vDispLine1+8, 8, 80, 640);
			//LCD_Disp_Txt(FONT_16, 240, vDispLine1, "旋钮:");
			LCD_Disp_Txt(FONT_12, 120, vDispLine1, _YKQ_TYPE);
			LCD_Disp_Txt(FONT_12, 180, vDispLine1, gRcvVer);
			//LCD_Disp_Txt(FONT_16, 100, vDispLine2, "DI: ");
			LCD_Disp_Rect(LCD_C_D_Rect,romArmDSDisp[0][0]-3,romArmDSDisp[0][1]-1,
						romArmDSDisp[0][0]+25,romArmDSDisp[0][1]+18);
		ReDrawFlg = 3;
		Disp_Rock_Pic(ReDrawFlg);
	} 
	
	if((gLcdFresh == 1)||(gDataFresh == 1))
	{
		if(gLcdFresh == 1)
			gLcdFresh = 0;
		if(gDataFresh == 1)
			gDataFresh = 0;
		LCD_Set_Color(COLOR_WHITE_,-1);
		LCD_Get_Color(0,6,6);
		for(i=0; i<_ARM_USE; i++)
		{
			gADArmConV[i] = gADArmSend[i] * 255 / 250;
			//sprintf(vStr, "%d", (unsigned char)((gADArmConV[i]))); 
			 sprintf(vStr, "%d", (unsigned char)((Y_Send[i]))); 
			
			vLen = strlen(vStr);
			for(j=vLen; j<4; j++)
				vStr[j] = ' ';
			vStr[j] = 0;
			LCD_Disp_Txt(FONT_16, romArmValDisp[i][0], romArmValDisp[i][1], (unsigned char*)vStr);
			LCD_Disp_Int(FONT_16, romArmDSDisp[i][0], romArmDSDisp[i][1], gBDParam[0].vRockDS[i]);
		}
		//if(gLCDPage.vPageX == 8)
		{
		#if _XN_LiuL
			gADArmConV[_ARM_MAX-2] = gADArmSend[_ARM_MAX-2] * 255 / 250;
			LCD_Disp_Intx(FONT_16, 54, vDispLine1, gADArmConV[_ARM_MAX-2], 3);						// 刷动
			gADArmConV[_ARM_MAX-1] = gADArmSend[_ARM_MAX-1] * 255 / 250;
			LCD_Disp_Intx(FONT_16, 257, vDispLine1, gADArmConV[_ARM_MAX-1], 3);						// 刷动
		#else
			gADArmConV[_ARM_MAX-1] = gADArmSend[_ARM_MAX-1] * 255 / 250;
//			LCD_Disp_Intx(FONT_16, 257, vDispLine1, gADArmConV[_ARM_MAX-1], 3);						// 刷动
		#endif
		}

		LCD_Set_Color(COLOR_RED_,-1);
			if(gPowerChk>>16)									// 20180506   lsg 
			{
				LCD_Disp_Txt(FONT_12, vDX, vDY, "内部电压异常！  ");
				stPowChk = 1;
			}
			else if(gPowerChk & 0xffff)
			{
				LCD_Disp_Txt(FONT_12, vDX, vDY, "摇杆数据异常！  ");
				stPowChk = 2;
			}
			else //if(stPowChk )
			{
				for(i=0; i<5; i++)
				{
					if(i==4)
						sprintf(vStr, "%02X ", (unsigned char)(gDIxBitV)); 
					else
						sprintf(vStr, "%02X ", (unsigned char)(gDIBitV>>(i*8))); 
					vLen = strlen(vStr);
					//vStr[3] = 0;
					LCD_Disp_Txt(FONT_12, vDX+i*20, vDY, (unsigned char*)vStr);
				}
				//LCD_Disp_Txt(FONT_12, vDX, vDY, "               ");
				stPowChk = 0;
			}		
		LCD_Set_Color(COLOR_WHITE_,-1);
			
		////  图形化 按钮 和 纽子开关
		Disp_Rock_Pic(ReDrawFlg);
		if(ReDrawFlg)
			ReDrawFlg--;
	}
	if(gLCDBKOnFlg == 0)
		Key_ZZT_CtrlStatus();	
}


/****************************************************************************
*
*  LCD 显示界面：输入密码页面
*
*****************************************************************************/
// 显示臂架界面
void Disp_GJ(void)
{
	unsigned char i=0;
	unsigned char intit =0;
	unsigned short vXX = 0;
	unsigned short vYY = 0;
	static unsigned char stArmOp = 0;
	static unsigned char stLoop = LCD_FRESH_WARN;
	static unsigned char stWLFlg = 55;
	static unsigned char stBATFlg = 55;
	unsigned char vFlg = 0;
	static  int stWarnCnt = 0;
	static  char stCnt = 0;
	float fWKTm = 0.0;
	static  char stPowChk = 0;								// 20180506   lsg 

//		gp_lcdtask = Disp_ZZT_CtrlStatus;
//		return;
	
	if(gLcdFresh >= 3)
	{
		gLcdFresh--;
		LCD_Disp_PIC(31);
		return;
	}
	stLoop++;
	if(stLoop >	LCD_FRESH_WARN)
	{
		stLoop = 0;
		vFlg = 1;
/*		
		gRunInfo.vLJWkTime++;
		gWarnFlg++;
		if(gWarnFlg > 7)
			gWarnFlg = 0;
		*/
	}

	if(gLcdFresh == 2)
	{
		gLcdFresh = 1;
		LCD_Set_Cursor(0,1,1,2,2);
		//gLCDPage.page = gLCDPage.cmdsel;
		LCD_Disp_PIC(31);
		stWLFlg = 55;
		stBATFlg = 55;
	}											//  moment_percentage
	if((gLcdFresh == 1)||(gDataFresh == 1))
	{
		if(gLcdFresh == 1)
			gLcdFresh = 0;
		if(gDataFresh == 1)
			gDataFresh = 0;				   
			LCD_Set_Color(COLOR_WHITE_,-1);
	 if(intit==0)
	 {												 
	  	intit =1;
	  }	
	  	
     
		if(vFlg)
		{
 			  
			    if (remote_18c.L1>0){N01_BUF=1;N02_BUF=0;} else if(remote_18c.L4>0)  {N02_BUF=1;N01_BUF=0;} else {N01_BUF=0;N02_BUF=0; }
					if ( N02_BUF ==1) 			{	LCD_Set_Color(COLOR_YELLW_,-1);	LCD_Disp_Txt(FONT_24, 87, 97, "                         ");} // HUDH  
				else if (	remote_18c.L1 == 0 )  	{	LCD_Set_Color(COLOR_YELLW_,-1);	LCD_Disp_Txt(FONT_24, 87, 97, " 1号未连接臂架        ");} // HUDH
				else if (remote_18c.L1 == 1)  	{	LCD_Set_Color(COLOR_YELLW_,-1);	LCD_Disp_Txt(FONT_24, 87, 97, " 1号连接A(左)臂架     ");} // HUDH
				else if  (remote_18c.L1 ==2)	{	LCD_Set_Color(COLOR_YELLW_,-1);	LCD_Disp_Txt(FONT_24, 87, 97, " 1号连接B(右)臂架     ");} // HUDH   
				else if  (remote_18c.L1 ==3)	{	LCD_Set_Color(COLOR_YELLW_,-1);	LCD_Disp_Txt(FONT_24, 87, 97, " 1号连接C(中)臂架     ");} // HUDH 
				
				 
				 		 if ( N01_BUF ==1)
					  			{	LCD_Set_Color(COLOR_YELLW_,-1);	LCD_Disp_Txt(FONT_24, 87, 147, "                         ");} // HUDH  
						else if (	remote_18c.L4 == 0 )  
						    	{	LCD_Set_Color(COLOR_YELLW_,-1);	LCD_Disp_Txt(FONT_24, 87, 147, " 2号未连接臂架        ");} // HUDH
						else if (remote_18c.L4 == 1) 
					 	{	LCD_Set_Color(COLOR_YELLW_,-1);	LCD_Disp_Txt(FONT_24, 87, 147, " 2号连接A(左)臂架     ");} // HUDH
						else if  (remote_18c.L4 ==2)
							{	LCD_Set_Color(COLOR_YELLW_,-1);	LCD_Disp_Txt(FONT_24, 87, 147, " 2号连接B(右)臂架     ");} // HUDH   
						else if  (remote_18c.L4 ==3)
							{	LCD_Set_Color(COLOR_YELLW_,-1);	LCD_Disp_Txt(FONT_24, 87, 147, " 2号连接C(中)臂架     ");} // HUDH  
			angle_prif = (short)(remote_18c.L2 + remote_18c.L3*256)  ;
			if (angle_prif<0)
			{
				dead_zone++ ;
			}	   	     			 
			   										  
     		if(remote_18c.L5==0){		LCD_Disp_Txt(FONT_24,14, 32, " 吊篮角度 ° ") ; LCD_Disp_FloatPX(FONT_24,154, 32, ( angle_prif)/10.0,1);  LCD_Disp_Txt(FONT_24,87, 62, "                   ");}
			   										  
     		if(remote_18c.L5==1){	LCD_Disp_Txt(FONT_24,87, 62, " C(中)臂操作箱开启 ") ; 	LCD_Disp_Txt(FONT_24,14, 32, "            ") ; 	LCD_Disp_Txt(FONT_24,154, 32, "            ") ;}
									 
				 
		if(gPowerChk)
		LCD_Set_Color(COLOR_RED_,-1);
			if(gPowerChk>>16)									// 20180506   lsg 
			{
				LCD_Disp_Txt(FONT_16, 190, 33+24*6, "内部电压异常！");
				stPowChk = 1;
			}
			else if(gPowerChk & 0xffff)
			{
				LCD_Disp_Txt(FONT_16, 190, 33+24*6, "摇杆数据异常！");
				stPowChk = 2;
			}
			else if(stPowChk )
			{
				LCD_Disp_Txt(FONT_16, 190, 33+24*6, "              ");
				stPowChk = 0;
			}
		if(gPowerChk)
		LCD_Set_Color(COLOR_WHITE_,-1);
				
				
			////  无线信号
			vXX = 290;
			vYY = 2;
			if(stWLFlg != gWLSingV)
			{
				i = (unsigned char)(gWLSingV>>1);
				if(gWLSingV > 7)					//  0~10
					i = 3;
				else if(gWLSingV > 4)					//  0~10
					i = 2;
				else if(gWLSingV > 1)					//  0~10
					i = 1;
				else 
					i = 0;
				if(gLaBaFlg == 0)
					i = 0;
				LCD_Cut_PICRect(0, 14, gLCDM_WL_Tab[i][0], gLCDM_WL_Tab[i][1],
					gLCDM_WL_Tab[i][2], gLCDM_WL_Tab[i][3], vXX, vYY);
			}
			//// 电池电量 
			if(gCellPower > 7)					//  0~10
				i = 3;
			else if(gCellPower > 4)					//  0~10
				i = 2;
			else if(gCellPower > 1)					//  0~10
				i = 1;
			else
				i = 0;
			if(stBATFlg != i)
			{
				stBATFlg = i;
				vXX = 264;
				vYY = 2;
				LCD_Cut_PICRect(0, 14, gLCDM_Bat_Tab[i][0], gLCDM_Bat_Tab[i][1],
					gLCDM_Bat_Tab[i][2], gLCDM_Bat_Tab[i][3],vXX, vYY);
			}
		}
	}
	if(gLCDBKOnFlg == 0)
	Key_MainZZT();
}

/****************************************************************************
*
*  LCD 显示界面：输入密码页面
*
*****************************************************************************/
void Disp_Input_Password(void)
{
	uchar i = 0;
	uchar vStr[PASSWORD_LEN+5];

	if(gLcdFresh > 2)
	{
		gLcdFresh--;
		LCD_Disp_PIC(6);
		return;
	}
	for(i=0; i<PASSWORD_LEN; i++)
		vStr[i] = '*';
	if(gLcdFresh == 2)
	{
		gLcdFresh = 1;
		LCD_Disp_PIC(6);
		LCD_Set_Color(COLOR_RED_,-1);
		LCD_Get_Color(0,85,95);
		gLCDPage.BDRect = 0;
		gLCDPage.BDRectSite	= 0;
		gPasswordIn[0] = 0;
		vStr[0] = '0';
		vStr[i++] = ' ';
		vStr[i++] = 0;
		LCD_Disp_TxtLen(FONT_24, 100, 84, vStr, PASSWORD_LEN);
		for(i=0; i<PASSWORD_LEN; i++)
		{
			gPasswordIn[i] = 0;
		}
			LCD_Set_Cursor(1,99+gLCDPage.BDRectSite*12, 115, 15, 2);
	}
	if(gLcdFresh == 1)
	{
	//	LCD_Disp_PIC(8);
		gLcdFresh = 0;
		for(i=0; i<PASSWORD_LEN; i++)
		{
			if(i == gLCDPage.BDRectSite)
				vStr[i] = gPasswordIn[i]+'0';
			}
		LCD_Disp_TxtLen(FONT_24, 100, 84, vStr, PASSWORD_LEN);
	}
	if(gLCDBKOnFlg == 0)
	Key_Input_Password();
}
/****************************************************************************
*
*  LCD 显示界面：菜单页面，选择进入不同的功能页面
*
*****************************************************************************/
void Disp_Cmd_Sel(void)
{				
	static uchar stSel = 9;
	if(gLcdFresh > 3)
	{
		gLcdFresh--;
		return;
	}
	if(gLcdFresh == 3)
	{
		gLcdFresh--;
		LCD_Disp_PIC(9);
		return;
	}
	if(gLcdFresh == 2)
	{
		LCD_Set_Color(COLOR_WHITE_,-1);
		LCD_Get_Color(0,6,6);
		LCD_Set_Cursor(0,1,1,2,2);
		gLcdFresh = 2;
		stSel = 9;
		gLCDPage.cmdsel = 0;
		gLCDPage.page = gLCDPage.cmdsel+1;
		//gLCDPage.Cmdpage = 0;
		LCD_Disp_PIC(9);
	} 
	LCD_Set_Color(COLOR_WHITE_,-1);
		LCD_Get_Color(0,6,6);
	//	LCD_Disp_PIC(2);
	if(stSel != gLCDPage.cmdsel)
	{
		LCD_Disp_Rect(LCD_C_C_Rect,6,38+27*(stSel), LCD_WIDTH-6, 38+27*(stSel+1));
		stSel = gLCDPage.cmdsel;
		LCD_Disp_Rect(LCD_C_D_Rect,6,38+27*(stSel), LCD_WIDTH-6, 38+27*(stSel+1));
	}
	if(gLCDBKOnFlg == 0)
	Key_Cmd_Sel();
//	gLcdFresh = 0;
}

unsigned short gParamSiteY[8]=			// 参数显示位置的的Y值
		{34,59,84,109,109,0,0,0};
void Disp_Cmd1_AITime(void)
{
	static unsigned char stInputSite = 0;
	if(gLcdFresh > 2)
	{
		gLcdFresh--;
		LCD_Disp_PIC(5);
		return;
	}
	if(gLcdFresh == 2)
	{
		LCD_Set_Color(COLOR_WHITE_,-1);
		LCD_Get_Color(0,6,6);
		LCD_Set_Cursor(0,1,1,2,2);
		gLcdFresh = 1;
		gLCDPage.BDRect = 0;
		LCD_Disp_PIC(5);
			LCD_Disp_Intx(FONT_16, 140, 65, gBDParam2[0].vRes,2);					// 电阻  Ω
			LCD_Disp_Intx(FONT_16, 140, 90, gBDParam2[0].vVCC,2);					// 电压  V
			LCD_Disp_Rect(LCD_C_D_Rect,122,gParamSiteY[stInputSite],
					205,gParamSiteY[stInputSite+1]);
	}
	if((gLcdFresh == 1)||(gDataFresh == 1))
	{
		if(stInputSite != gLCDPage.BDRect)
		{
			LCD_Disp_Rect(LCD_C_C_Rect,122,gParamSiteY[stInputSite],
					205,gParamSiteY[stInputSite+1]);
			delay_ms(1);
			stInputSite = gLCDPage.BDRect;
			LCD_Disp_Rect(LCD_C_D_Rect,122,gParamSiteY[stInputSite],
					205,gParamSiteY[stInputSite+1]);
		}
		if(gLcdFresh == 1)
			gLcdFresh = 0;
		if(gDataFresh == 1)
			gDataFresh = 0;
		if(gLCDPage.BDRect == 0)
			LCD_Disp_Intx(FONT_16, 140, 40, gBDParam2[0].vAITime,4);			// 爬坡时间，mS
		if(gLCDPage.BDRect == 1)
			LCD_Disp_Intx(FONT_16, 140, 65, gBDParam2[0].vRes,2);					// 电阻  Ω
		if(gLCDPage.BDRect == 2)
			LCD_Disp_Intx(FONT_16, 140, 90, gBDParam2[0].vVCC,2);					// 电压  V
	}
	Key_Cmd1_AITime();
}

/****************************************************************************
*
*  LCD 显示界面：遥控器端口状态
*
*****************************************************************************/
/****************************************************************************
*
*  LCD 显示界面：电磁阀最大最小电流设置
*
*****************************************************************************/
//unsigned short gAISiteX[8]=			// 最大电流显示位置的的X值
//		{40,86,132,178,224,270,316,0};
//unsigned short gAISiteY[6]=				// 最小电流显示位置的的Y值
//		{75, 120, 150, 190,0};
unsigned short gAISiteX[9]=	{35,70,105,140,175,210,245,280,315};		// 最大电流显示位置的的X值
unsigned short gAISiteY[6]=				// 最小电流显示位置的的Y值
		{75, 120, 150, 190,0};

unsigned char gSetAIFlg = 0;
unsigned char gSetAIX = 0;
unsigned char gSetAIY = 0;
unsigned char gSetAIX0 = 0;
unsigned char gSetAIY0 = 0;
unsigned int  gAISetFlg = 0;		// 设置AI的标记
unsigned char gSetAISend = 0;

void Disp_Cmd8_SetI(void)
{
	unsigned char i=0;
	unsigned char j=0; 
	static unsigned char stDly = 50;

	if(gLcdFresh > 2)
	{
		gLcdFresh--;
		LCD_Disp_PIC(4);
		return;
	}
	if(gLcdFresh == 2)
	{
		gAISetFlg = 1;
		gLcdFresh = 1;
		gLCDPage.BDRect = 0;
		gLCDPage.BDRectSite	= 0;
		gLCDPage.key = 0;
		LCD_Disp_PIC(4);
		LCD_Set_Color(COLOR_WHITE_,-1);
		LCD_Get_Color(0,6,6);
		LCD_Set_Cursor(0,1,1,2,2);
		for(i=0; i<2; i++)
		{
			for(j=0; j<8; j++)
			{
				LCD_Disp_Int4x(FONT_16, gAISiteX[j], gAISiteY[i], gBDParam2[0].vAI[i][j]);
			}
		}
			delay_ms(30);	
		for(i=2; i<4; i++)
		{
			for(j=0; j<8; j++)
			{
				LCD_Disp_Int4x(FONT_16, gAISiteX[j], gAISiteY[i], gBDParam2[0].vAI[i][j]);
			}
		}
		gSetAIFlg = 0;
		gSetAIX = 0;
		gSetAIY = 0;	
		gAISendFlg = 0;
		gSetAIX0 = 0;
		gSetAIY0 = 0;	
		stDly = 50;
	//		LCD_Disp_Rect(LCD_C_D_Rect,gAISiteX[gSetAIX0]-5,gAISiteY[gSetAIY0]-2,
	//				gAISiteX[gSetAIX0+1]-5,gAISiteY[gSetAIY0]+18);
		gSetAIY0 = 1;	
	//	delay_ms(1);	

	}

	if(gLCDBKOnFlg == 0)
	Key_Cmd8_SetI();
	if(gLcdFresh == 1)
	{
		gLcdFresh = 0;
		LCD_Disp_Int4x(FONT_16, gAISiteX[gSetAIX], gAISiteY[gSetAIY], gBDParam2[1].vAI[gSetAIY][gSetAIX]);
		if((gSetAIX != gSetAIX0) || (gSetAIY != gSetAIY0))
		{
			LCD_Disp_Rect(LCD_C_C_Rect,gAISiteX[gSetAIX0]-3,gAISiteY[gSetAIY0]-2,
					gAISiteX[gSetAIX0+1]-2,gAISiteY[gSetAIY0]+18);
			gSetAIX0 = gSetAIX;
			gSetAIY0 = gSetAIY;
			delay_ms(stDly);
			stDly = 1;	
			LCD_Disp_Rect(LCD_C_D_Rect,gAISiteX[gSetAIX0]-3,gAISiteY[gSetAIY0]-2,
					gAISiteX[gSetAIX0+1]-2,gAISiteY[gSetAIY0]+18);
			delay_ms(1);	
		}
	}
}


void Disp_Cmd8_SetI2(void)
{
	unsigned char i=0;
	unsigned char j=0; 
	static unsigned char stDly = 50;

	if(gLcdFresh > 2)
	{
		gLcdFresh--;
		LCD_Disp_PIC(4);
		return;
	}
	if(gLcdFresh == 2)
	{
		gAISetFlg = 1;
		gLcdFresh = 1;
		gLCDPage.BDRect = 0;
		gLCDPage.BDRectSite	= 0;
		gLCDPage.key = 0;
		LCD_Disp_PIC(4);
		LCD_Set_Color(COLOR_WHITE_,-1);
		LCD_Get_Color(0,6,6);
		LCD_Set_Cursor(0,1,1,2,2);
		LCD_Disp_Txt(FONT_24, 208, 2, "B ");
		for(i=0; i<2; i++)
		{
			for(j=0; j<8; j++)
			{
				LCD_Disp_Int4x(FONT_16, gAISiteX[j], gAISiteY[i], gBDParam2[0].vAI2[i][j]);
			}
		}
			delay_ms(30);	
		for(i=2; i<4; i++)
		{
			for(j=0; j<8; j++)
			{
				LCD_Disp_Int4x(FONT_16, gAISiteX[j], gAISiteY[i], gBDParam2[0].vAI2[i][j]);
			}
		//	delay_ms(1);	
		}
		gSetAIFlg = 0;
		gSetAIX = 0;
		gSetAIY = 0;	
		gAISendFlg = 0;
		gSetAIX0 = 0;
		gSetAIY0 = 0;	
		stDly = 50;
	//		LCD_Disp_Rect(LCD_C_D_Rect,gAISiteX[gSetAIX0]-5,gAISiteY[gSetAIY0]-2,
	//				gAISiteX[gSetAIX0+1]-5,gAISiteY[gSetAIY0]+18);
		gSetAIY0 = 1;	
	//	delay_ms(1);	

	}

	if(gLCDBKOnFlg == 0)
	Key_Cmd8_SetI2();
	if(gLcdFresh == 1)
	{
		gLcdFresh = 0;
		LCD_Disp_Int4x(FONT_16, gAISiteX[gSetAIX], gAISiteY[gSetAIY], gBDParam2[1].vAI2[gSetAIY][gSetAIX]);
		if((gSetAIX != gSetAIX0) || (gSetAIY != gSetAIY0))
		{
			LCD_Disp_Rect(LCD_C_C_Rect,gAISiteX[gSetAIX0]-3,gAISiteY[gSetAIY0]-2,
					gAISiteX[gSetAIX0+1]-2,gAISiteY[gSetAIY0]+18);
			gSetAIX0 = gSetAIX;
			gSetAIY0 = gSetAIY;
			delay_ms(stDly);
			stDly = 1;	
			LCD_Disp_Rect(LCD_C_D_Rect,gAISiteX[gSetAIX0]-3,gAISiteY[gSetAIY0]-2,
					gAISiteX[gSetAIX0+1]-2,gAISiteY[gSetAIY0]+18);
			delay_ms(1);	
		}
	}
}

