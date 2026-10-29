/********************************************************************
*
*  APP_IO 与 IO 口有关的函数，key, di, do 等
*  功能：
*
********************************************************************/


#include "app_io.h"
#include "LPC17xx.h"
#include "config.h"



//unsigned char  gPasswordIn[8];
unsigned char  gCmdYKFlg = 1;			// 1--遥控、0--近控

unsigned char  gDOFlag = 0;
unsigned char  gT0SFlag = 0;

unsigned int gTstVal = 100;
unsigned char gMLCDKeyUnlock = 0;		// 主界面解锁按钮
unsigned char gMLCDKeyUnBeep = 0;		// 主界面消音按钮

 unsigned int  gBeepFlg ;
 unsigned int  gWarnFlg ;
unsigned char  gModeSel = 0;

PINCON_TypeDef pGPIOCfg;
// 按键键值，每个键对应一个位
unsigned char gKeyValue = 0;
unsigned char gKeyValue0 = 0;
unsigned char gKeyValue0x = 0;		// 有按键，保持

// DI  输入结果
unsigned char gDIScanFlg[_DI_MAXNUM];
unsigned char gKEYScanFlg[_KEY_MAXNUM];
// DI扫描计数( 去抖 )
unsigned int  gDIScanCnt[_DI_MAXNUM];
unsigned short gKEYScanCnt[_KEY_MAXNUM];
// DO输出值
unsigned char gDOValue[_DO_MAXNUM];

unsigned long  gDIBitV0 = 0;			// 用位定义的两档DI输入结果，1bit -- 1个开关
unsigned long  gDIBitV = 0;			// 用位定义的两档DI输入结果，1bit -- 1个开关，输出给控制器
unsigned int  gDI3BitV = 0;			// 用位定义的三档DI输入结果，2bit -- 1个开关
unsigned int  gDIxBitV = 0;			// 用位定义的档位旋钮DI输入结果，4bit -- 1个开关
unsigned int  gDOBitV = 0;			// 用位定义的Do输出记录，1bit -- 1路DO
unsigned int  gKBitV = 0;				// 用位定义的 K 输入， 1bit -- 1路K
unsigned int  gKValx = 0;				// K 输入的 序号， 1~10

unsigned char gPowerFlg = 0;	 	// 电源标记，0--有线供电，1--电池供电
unsigned char gCellPower = 0;		// 电池电量	0~9
unsigned char gLCDBKLight = 16;	// LCD 背光亮度
unsigned char gLCDBKLight0 = 16;	// LCD 背光亮度
unsigned int  gLCDBKLDly = 0;		// 无摇杆，关闭背光
unsigned int  gLCDBKOnDly = 0;		// 上电到开背光的延时
unsigned char gLCDBKOnFlg = 0;	// LCD 背光开关

unsigned char gLaBaFlg = 0;			// 喇叭，启动和解除急停后，都需先按喇叭，才能起作用
unsigned char gWLAux = 0;				// 无线模块的AUX脚，输入，为高，无线可用，为低，无线忙
unsigned char gArmDSSetFlg = 0;	// 摇杆死区设置标记

unsigned char gJiTing = 0;			// 急停标记
unsigned int  gJiTingCnt = 0;
unsigned int  gTstV = 0;
/*
//// DI扫描(1~3低有效，4~5高有效) 顺序---- 从左到右，
unsigned char gDIScanTable[_DI_MAXNUM][2] = {
	{1, _DI_1},  {1, _DI_2},  {1, _DI_3},  {1, _DI_4},  {1, _DI_5},  {1, _DI_6}, 
	{1, _DI_8},  {1, _DI_7},  {1, _DI_9},  {1, _DI_10},  {2, _DI_11},  {2, _DI_12}, {1,_DI_JiT},     
	{_EOT, _EOT}		  
};
//// DI扫描(1~3低有效，4~5高有效) 顺序---- 从左到右，
unsigned char gDIScanTable[_DI_MAXNUM][2] = {
	{1, _DI_1},  {1, _DI_2},  {2, _DI_11},   {1, _DI_4},  {1, _DI_5},  {1, _DI_6}, 
	{1, _DI_8},  {1, _DI_7},  {1, _DI_9},  {1, _DI_10},  {1, _DI_3},  {2, _DI_12},    
	{_EOT, _EOT}		  
};*/
////  左边外壳上的四个按键用做显示屏的按键，F1~F4  ----  DI2/1/4/3
unsigned char gKEYScanTable[_KEY_MAXNUM][2] = {{0, _DI_1}, {0, _DI_2}, {4, _DI_3}, {0, _DI_4}, {_EOT, _EOT},{_EOT, _EOT}		};
////  DI，用了自锁按钮，所以信号都当做 自复位 型处理
unsigned char gDIScanTable[_DI_MAXNUM][2] = {
	{0, _DI_8}, {0, _DI_7},  {0, _DI_2}, {0, _DI_1},			
	{0, _DI_4}, {4, _DI_3},  {4, _DI_6}, {1, _DI_5},    
	{1, _DI_23}, {0, _DI_24}, {0, _DI_21}, {2, _DI_22}, 
	{0, _DI_25}, {0, _DI_26}, {1, _DI_9}, {1, _DI_10}, 
	{2, _DI_12}, {2, _DI_11}, {2, _DI_18},	{2, _DI_17}, 		// F1~F4 --> DI1~DI4
	{2, _DI_15}, {2, _DI_16}, {2, _DI_19}, {2, _DI_20},  
	{2, _DI_13}, {2, _DI_14}, {1, _DI_24V},
	{_EOT, _EOT}
	  
};		
////  Kx 按钮或多档旋钮
unsigned char gKScanTable[12][2] = {
	{1, _K_1}, {1, _K_2}, {1, _K_3}, {1, _K_4},	{1, _K_5},			
	{1, _K_6}, {1, _K_7}, {1, _K_8}, {1, _K_9},	{1, _K_10},			
	{_EOT, _EOT}
};
//// DO，输出高有效
unsigned char gDOSetTable[_DO_MAXNUM][2] = {
	{3, _DO_1}, {3, _DO_2},	{1, _DO_5}, {0, _DO_3}, {0, _DO_4}, {1, _DO_6}, 
	{_EOT, _EOT} 	  
};



void IO_Init(void)
{
	unsigned char i = 0;
//	unsigned char v = 0;
	GPIO_TypeDef * pGPIO = NULL;

	GPIOInit();	

	// DI Init	
	for(i=0; i<_DI_MAXNUM; i++)
	{
		if(gDIScanTable[i][0] == _EOT)
		{
			break;
		}
		pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*gDIScanTable[i][0]));
		pGPIO->FIODIR &= ~(0x01 << gDIScanTable[i][1]);
	}
	// DI Init	
	for(i=0; i<12; i++)
	{
		if(gKScanTable[i][0] == _EOT)
		{
			break;
		}
		pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*gKScanTable[i][0]));
		pGPIO->FIODIR &= ~(0x01 << gKScanTable[i][1]);
	}

	// DO Init	
	for(i=0; i<_DO_MAXNUM; i++)
	{
		if(gDOSetTable[i][0] == _EOT)
		{
			break;
		}
		pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*gDOSetTable[i][0]));
		pGPIO->FIODIR |= (0x01 << gDOSetTable[i][1]);
		pGPIO->FIOCLR |= (0x01 << gDOSetTable[i][1]);
	}
	// 电源指示灯
	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*1));
	pGPIO->FIODIR |= (0x01 << _DO_6);
	pGPIO->FIOSET |= (0x01 << _DO_6);
	// 蜂鸣器
	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*1));
	pGPIO->FIODIR |= (0x01 << _DO_FMQI);
	// 急停
	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*1));
	pGPIO->FIODIR &= ~(0x01 << _DI_JiT);
	
	 GPIO0->FIODIR |= DO_CPU_EN;	   		// P0.30, CPU_EN OutPin
	 GPIO0->FIODIR |= DO_LCD_EN;	   		// P0.29, LCD_EN OutPin
	 GPIO1->FIODIR |= DO_WL_EN;	   		// P1.18, LCD_EN OutPin
	_CPU_EN;	
	_LCD_EN;
	_WL_EN;
	// wl aux   P1.0
		pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020));
		pGPIO->FIODIR &= ~(0x01);
	
GPIO0->FIODIR &= ~(1<<_DI_24V);	   			// P0.18, DI_24V InPin			Ver3	20180629  
} 


/****************************************************************************
* 时间: 2014-11-15
* 名称：void DI_Scan(void)
* 功能：DI扫描函数
* 入口参数：
* 出口参数：
****************************************************************************/
void DI_Scan(void)
{
	unsigned char i = 0;
	unsigned char v = 0;
	unsigned int  vKTime = 0;
	unsigned int  vKTimeE = 0;
//	unsigned char vtmp = 0;

	GPIO_TypeDef * pGPIO = NULL;
	gDIxBitV = 0;
	gKValx = 0;
	for(i=0; i<12; i++)
	{
		if(gKScanTable[i][0] == _EOT)
		{
			break;
		}
		if(gKScanTable[i][0] == _CNT)
		{
			continue;
		}
		pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*gKScanTable[i][0]));
			v = ((pGPIO->FIOPIN & (0x01 << gKScanTable[i][1])) == (0x01 << gKScanTable[i][1])? 1:0);
			if(v == 0)
			{
				gDIxBitV |= (1<<i);
				gKValx = i+1;
			}
	}	
	for(i=0; i<_DI_MAXNUM; i++)
	{
		if(gDIScanTable[i][0] == _EOT)
		{
			break;
		}
		if(gDIScanTable[i][0] == _CNT)
		{
			continue;
		}
		pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*gDIScanTable[i][0]));
		
		//// 0,2,3 自锁型  F1 ~ F4
		if((i==16) || (i==17) || (i==18) || (i==19))
		{
			v = ((pGPIO->FIOPIN & (0x01 << gDIScanTable[i][1])) == (0x01 << gDIScanTable[i][1])? 1:0);
			if(v == 0)
			{
				vKTime = _DI_SCAN_CNT;
				if(gDIScanCnt[i] < vKTime+2)
					gDIScanCnt[i]++;
				if(gDIScanCnt[i] == vKTime)
				{
					gDIScanFlg[i] = 1;

						gKeyValue ^= (1<<(i-16));
				}
				if(gDIScanCnt[i] >= vKTime)
				{
						gDIBitV0 |= (1<<i);
				}
			}
			else
			{
				if(gDIScanCnt[i] > 0)
					gDIScanCnt[i]>>=1;
				if(gDIScanCnt[i] == 0)
				{
					gDIScanFlg[i] = 0;
							gDIBitV0 &= ~(1<<i);
			}
			}
		}
		else  	// 自复位型
		//// 20,22,23 自锁型  妞子开关，都是自复位模式
		{
			v = ((pGPIO->FIOPIN & (0x01 << gDIScanTable[i][1])) == (0x01 << gDIScanTable[i][1])? 1:0);
			//if(i == 6)								// 外部低有效 , 内部低有效 
			{
				vKTime = _DI_SCAN_CNT;
				vKTimeE = vKTime+2;
				if((i==4) || (i==5) || (i==12) || (i==13) )		// 速凝剂+ -， 排量+ -
					vKTimeE = _DI_DD_SCAN_CNT;
				if(v == 0)
				{
					if((i==4) || (i==5) || (i==12) || (i==13) )		// 速凝剂+ -， 排量+ -
					{
						if((gDIScanCnt[i] > vKTime+1) && (gDIScanCnt[i] < 100))
							gDIScanCnt[i] = 100;
					}
					if(gDIScanCnt[i] < vKTimeE)
						gDIScanCnt[i]++;
					if(gDIScanCnt[i] == vKTime)
					{
						gDIScanFlg[i] = 1;
						gDIBitV0 |= (1<<i);
						}
				}
				else
				{
					if(gDIScanCnt[i] > 0)
						gDIScanCnt[i]-=1;
					if(gDIScanCnt[i] == 0)
					{
						gDIScanFlg[i] = 0;
						gDIBitV0 &= ~(1<<i);
					}
				}
			}
		}

	}
	gCmdYKFlg = 1;			// 1--遥控、0--近控
	
	if(ADCRst[1] > 500)
		gDIBitV0 |= (1<<28);
	else
		gDIBitV0 &= ~(1<<28);
	if(ADCRst[3] > 500)
		gDIBitV0 |= (1<<30);
	else
		gDIBitV0 &= ~(1<<30);
	if(ADCRst[6] > 500)
		gDIBitV0 |= (1<<31);
	else
		gDIBitV0 &= ~(1<<31);
	if(ADCRst[8] > 500)
		gDIBitV0 |= (1<<29);
	else
		gDIBitV0 &= ~(1<<29);

	DI_Correction(); // DI修正

	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020));
	v = ((pGPIO->FIOPIN & (0x01 << _DI_JiT)) == (0x01 << _DI_JiT)? 1:0);
	if(v == 0)
	{
		gJiTingCnt++;
		if(gJiTingCnt > 10)
		{
			gDIBitV0 |= 0x80000000;
			gJiTing = 0x55;
		}
	}
	else
	{
		gDIBitV0 &= ~0x80000000;
		gJiTingCnt >>= 1;
		gJiTing = 0;
	}

	
	gDIBitV = gDIBitV0 ^ 0x8000;
//	if(_BitV(gDIBitV, 15)==0)
//		gDIBitV |= 0x4000;
//	else
//		gDIBitV &= ~(1<<14);
//	gKeyValue = (gDIBitV0>>16) & 0X0F;
	gKeyValue0 = gKeyValue;
	if(	gKeyValue)
	{
		gLCDBKLDly=0;
		gKeyValue0x = gKeyValue;
	}

	#if 0
	if(_BitV(gDIBitV, 20))
		gLaBaFlg = 1;
	if(_BitV(gDIBitV, 18))
		gLaBaFlg = 1;
	if(_BitV(gDIBitV, 21))
		gLaBaFlg = 1;
	if(_BitV(gDIBitV, 19))
		gLaBaFlg = 1;
	if(_BitV(gDIBitV, 22))
		gLaBaFlg = 1;
	if(_BitV(gDIBitV, 23))
		gLaBaFlg = 1;
#else
	Activar();
#endif
	
	if(gPowerChk)
		gLaBaFlg = 0;
}


/******************************************************************************************
*******************************************************************************************
*******************************************************************************************
*
* 时间: 2015-6-5
* 名称：LCD  显示交互的相关函数，以页面为基础
* 
*******************************************************************************************
*******************************************************************************************
******************************************************************************************/
/****************************************************************************
*
*  主页面
*
****************************************************************************/
void Key_MainZZT(void)
{
	static unsigned int stCnt = 0;

	// 组合键容错：F1+F3(进状态页)/F1+F4(进密码页) 两键消抖时刻不同步时，
	// gKeyValue 可能只含单个键而被单独处理、导致组合键丢失。用已消抖的实时按键
	// 状态 gDIBitV0（按下且消抖后才置 1：bit16=F1、bit18=F3、bit19=F4）补偿。
	if((gDIBitV0 & (1<<16)) && (gDIBitV0 & (1<<18)) &&
	   (gKeyValue == _F1 || gKeyValue == _F3 || gKeyValue == _F1F3))
		gKeyValue = _F1F3;
	if((gDIBitV0 & (1<<16)) && (gDIBitV0 & (1<<19)) &&
	   (gKeyValue == _F1 || gKeyValue == _F4 || gKeyValue == _F1F4))
		gKeyValue = _F1F4;

	switch(gKeyValue)
	{
		case _F1:		
			gKeyValue &= 0xfE;
			gLCDBKLight+=8;
			if(gLCDBKLight >= 64)
				gLCDBKLight = 8;
			if(gLCDBKLight == 8)
				gLCDBKLight = 16;
			LCD_SET_BL(gLCDBKLight);
			gLCDBKLight0 = gLCDBKLight;
			gLcdFresh = 1;
			break;
		case _F2:
			gKeyValue &= 0xfD;
			break;
		case _F3:							// 单 F3 不进状态页，保留原组合键 F1+F3 进入
		//	gKeyValue &= 0xfb;
	    	gLcdFresh = 1;
		//	gp_lcdtask = Disp_ZZT_CtrlStatus;
			break;
		case _F1F3:
			gKeyValue &= 0xfb;
			gLcdFresh = 3;
			gp_lcdtask = Disp_ZZT_CtrlStatus;
			break;

		case _F4:
	 	//	gKeyValue &= 0xf7;
	 	//	gLcdFresh = 3;
		 //	gp_lcdtask = Disp_GJ;
		 ///	gp_lcdtaskNext = Disp_Cmd_Sel;


			break;
		case _F1F4:
			gKeyValue &= 0xf7;
			gLcdFresh = 3;
			gp_lcdtask = Disp_Input_Password;
			gp_lcdtaskNext = Disp_Cmd_Sel;
			break;
		default:
			gKeyValue = 0;
			break;
	}	 
	if (ceSH_BYTE_old != ceSH_BYTE )
	{
			if (ceSH_BYTE==0   )
			{	
		 	 	gLcdFresh = 3;
			//    gp_lcdtask = Disp_GJ;
				
			}
			else
			{	
		 	 	gLcdFresh = 3;
			   gp_lcdtask = Disp_mainSPJ;
				
			}
	}
	ceSH_BYTE_old = ceSH_BYTE   ;

	if ( X_TEST != X_TEST_old) 
	{  
			if (X_TEST==0   )
			{	
		 	 	gLcdFresh = 3;
			//    gp_lcdtask = Disp_GJ;
				
			}
			else
			{	
		 	 	gLcdFresh = 3;
			   gp_lcdtask = Disp_mainSPJ;
				
			}
			
	}  
	X_TEST_old = X_TEST   ;
	if(_BitV(gDIBitV, 21))
	{
		stCnt++;
		if(stCnt == 10)
			gModeSel ^= 1;
	}
	else
		stCnt = 0;
}
/****************************************************************************
*
*    遥控器状态
*
****************************************************************************/
unsigned int stCSKeyCnt = 0;
void Key_ZZT_CtrlStatus(void)
{
	static unsigned char stRow = 0;			// 第几个摇杆
	Arm_Study();
	if(stStudyFlg || (gp_lcdtask != Disp_ZZT_CtrlStatus))		// 学习中或刚退出学习(F4)，不再走普通状态页按键
		return;
	if(gKeyValue == 0)
		stCSKeyCnt = 0;
	switch(gKeyValue)
	{
		case _F1:			// OK
			gKeyValue &= 0xfe;
			if(stStudyFlg)
			{
				gDIScanCnt[16] = 0;
				break;
			}
			stCSKeyCnt++;
			if(stCSKeyCnt == 2000/LCD_FRESH_MS)
			{
				gArmDSSetFlg ^= 1;
			}
			if(gArmDSSetFlg == 0)
				LCD_Set_Cursor(0,romArmDSDisp[stRow][0],romArmDSDisp[stRow][1]+16, 9, 1);
			else
				LCD_Set_Cursor(1,romArmDSDisp[stRow][0],romArmDSDisp[stRow][1]+16, 9, 1); 
			gDIScanCnt[16] = 0;
			LCD_Disp_Rect(LCD_C_D_Rect,romArmDSDisp[stRow][0]-3,romArmDSDisp[stRow][1]-1,
						romArmDSDisp[stRow][0]+25,romArmDSDisp[stRow][1]+18);
			Arm_RockX();
			break;
		case _F2:			// 向左
			gKeyValue &= 0xfd;
			if(stStudyFlg)
			{
				gDIScanCnt[17] = 0;
				break;
			}
			LCD_Disp_Rect(LCD_C_C_Rect,romArmDSDisp[stRow][0]-3,romArmDSDisp[stRow][1]-1,
						romArmDSDisp[stRow][0]+25,romArmDSDisp[stRow][1]+18);
			stCSKeyCnt++;
			if(stCSKeyCnt == 2)
				stRow++;
			if(stRow >= _ARM_USE)
				stRow = 0;
			LCD_Disp_Rect(LCD_C_D_Rect,romArmDSDisp[stRow][0]-3,romArmDSDisp[stRow][1]-1,
						romArmDSDisp[stRow][0]+25,romArmDSDisp[stRow][1]+18);
			if(gArmDSSetFlg == 0)
				LCD_Set_Cursor(0,romArmDSDisp[stRow][0],romArmDSDisp[stRow][1]+16, 9, 1);
			else
				LCD_Set_Cursor(1,romArmDSDisp[stRow][0],romArmDSDisp[stRow][1]+16, 9, 1); 
			gDIScanCnt[17] = 0;
			break;
		case _F3:			// 向上，加1
			gKeyValue &= 0xfb;
			if(stStudyFlg)
			{
				gDIScanCnt[18] = 0;
				break;
			}
			if(gArmDSSetFlg)
			{
				stCSKeyCnt++;
				if(stCSKeyCnt == 2)
					gBDParam[0].vRockDS[stRow]++;
			}
			if(gBDParam[0].vRockDS[stRow] > 9)
				gBDParam[0].vRockDS[stRow] = 1;
			gDIScanCnt[18] = 0;
			break;
		case _F4:			// 返回
			gKeyValue &= 0xf7;
			gDIScanCnt[19] = 0;
			gLcdFresh = 3;
			gArmDSSetFlg = 0;
			gSaveFlg = 1;
			stRow = 0;
			gp_lcdtask = Disp_mainSPJ;
			break;
		case _F1F4:
			gKeyValue &= 0xf7;
			gLcdFresh = 3;
			gp_lcdtask = Disp_Input_Password;
			gp_lcdtaskNext = Disp_Cmd_Sel;
			break;
		default:
			gKeyValue = 0;
			break;
	}
}
/****************************************************************************
*
*    遥控器 设置参数，高压值
*
****************************************************************************/
/****************************************************************************
*
*  输入密码
*
****************************************************************************/

uchar Key_Input_Password(void)
{
	uchar i = 0; 
	uchar rtn = 0;

	switch(gKeyValue)
	{
		case _F1:			// 向上调数字
			gKeyValue &= 0xfe;
			gLcdFresh = 1;
			gLCDPage.BDRectSite++;
			if(gLCDPage.BDRectSite >= PASSWORD_LEN)
				gLCDPage.BDRectSite = 0;
			LCD_Set_Cursor(1,99+gLCDPage.BDRectSite*12, 115, 15, 2);
			break;
		case _F2:			// 向右移位
			gKeyValue &= 0xfd;
			gPasswordIn[gLCDPage.BDRectSite]++;
			if(gPasswordIn[gLCDPage.BDRectSite] > 9)
				gPasswordIn[gLCDPage.BDRectSite] = 0;
			gLcdFresh = 1;
			break;
		case _F3:			// 确定
			gKeyValue &= 0xfb;
			for(i=0; i<PASSWORD_LEN; i++)
			{
				if(gPasswordIn[i] != gPswd.password[i])
				{
					gLcdFresh = 4;
					gLCDPage.cmdsel = 0;
					gp_lcdtask = Disp_mainSPJ;
					return 0;
				}
			}
			gLcdFresh = 4;
			gLCDPage.cmdsel = 0;
			gp_lcdtask = gp_lcdtaskNext;
			rtn =  1;
			break;
		case _F4:			// 取消
			gKeyValue &= 0xf7;
			gLcdFresh = 4;
			gp_lcdtask = Disp_mainSPJ;
			LCD_Set_Cursor(0,99+gLCDPage.BDRectSite*12, 115, 15, 2);
			rtn =  0;
			break;
		default:			// 
			gKeyValue = 0;
			break;
	}
	return rtn;
}
uchar Key_Input_Passwordx(unsigned char vCh)
{
	uchar i = 0; 
	uchar rtn = 0;

	switch(gKeyValue)
	{
		case _F1:			// 向上调数字
			gKeyValue &= 0xfe;
			gLcdFresh = 1;
			gLCDPage.BDRectSite++;
			if(gLCDPage.BDRectSite >= PASSWORD_LEN)
				gLCDPage.BDRectSite = 0;
			LCD_Set_Cursor(1,99+gLCDPage.BDRectSite*12, 115, 15, 2);
			break;
		case _F2:			// 向右移位
			gKeyValue &= 0xfd;
			gPasswordIn[gLCDPage.BDRectSite]++;
			if(gPasswordIn[gLCDPage.BDRectSite] > 9)
				gPasswordIn[gLCDPage.BDRectSite] = 0;
			gLcdFresh = 1;
			break;
		case _F3:			// 确定
			gKeyValue &= 0xfb;
			for(i=0; i<PASSWORD_LEN; i++)
			{
				if(vCh == 1)
				{
					if(gPasswordIn[i] != gPswd.password[i])
					{
						gLcdFresh = 4;
						gLCDPage.cmdsel = 0;
						gp_lcdtask = Disp_mainSPJ;
						return 0;
					}
				}
				if(vCh == 2)
				{
					if(gPasswordIn[i] != gPswd.password2[i])
					{
						gLcdFresh = 4;
						gLCDPage.cmdsel = 0;
						gp_lcdtask = Disp_mainSPJ;
						return 0;
					}
				}
			}
			gLcdFresh = 4;
			gLCDPage.cmdsel = 0;
			gp_lcdtask = gp_lcdtaskNext;
			rtn =  1;
			break;
		case _F4:			// 取消
			gKeyValue &= 0xf7;
			gLcdFresh = 4;
			gp_lcdtask = Disp_mainSPJ;
			LCD_Set_Cursor(0,99+gLCDPage.BDRectSite*12, 115, 15, 2);
			rtn =  0;
			break;
		default:			// 
			gKeyValue = 0;
			break;
	}
	return rtn;
}

/****************************************************************************
*
*  菜单选择
*
****************************************************************************/
void Key_Cmd_Sel(void)
{
	if(gLcdFresh == 2)
	{
	//	gLCDPage.cmdsel = 0;
		gLcdFresh = 1;
		memcpy((char*)(&gBDParam[1]), (char*)(&gBDParam[0]), sizeof(gBDParam[1]));
	}
	switch(gKeyValue)
	{
		case _F1:			// 向上
			gKeyValue &= 0xfe;
			gLCDPage.cmdsel++;
			if(gLCDPage.cmdsel >= 3)
				gLCDPage.cmdsel = 0;
			gLCDPage.page = gLCDPage.cmdsel+1;
			//gLCDPage.Cmdpage = 0;

			break;
		case _F2:			// 向下
			gKeyValue &= 0xfd;
			if(gLCDPage.cmdsel == 0)
				gLCDPage.cmdsel = 2;
			else
				gLCDPage.cmdsel--;
			gLCDPage.page = gLCDPage.cmdsel+1;
			//gLCDPage.Cmdpage = 0;
			break;
		case _F3:			// 确定
			gKeyValue &= 0xfb;
			gLcdFresh = 4;
			switch(gLCDPage.cmdsel)
			{
				case 0:			// 设置爬坡时间
					gp_lcdtask = Disp_Cmd1_AITime;
					break;
				case 1:			// 设置电流
					gp_lcdtask = Disp_Cmd8_SetI;//Disp_Input_Password;
					break;
				case 2:			// 设置电流
					gp_lcdtask = Disp_Cmd8_SetI2;//Disp_Input_Password;
					break;
				default:
					break;
			}

			break;	 
		case _F4:			// 返回
			gKeyValue &= 0xf7;
			gLcdFresh = 4;
			gLCDPage.cmdsel = 0;
			gLCDPage.page = 0;
			gBDCmd = 0;
			gp_lcdtask = Disp_mainSPJ;
			break;
		default:
			gKeyValue = 0;
			break;
	}		

}
void Key_Cmd1_AITime(void)
{
	switch(gKeyValue)
	{
		case _F1:			// 向下
			gKeyValue &= 0xfe;
			if(gLCDPage.BDRect == 0)
			{
				if(gBDParam2[0].vAITime < 10)
					gBDParam2[0].vAITime = 0;
				else
					gBDParam2[0].vAITime -= 10;
			}
			if(gLCDPage.BDRect == 1)
			{
				if(gBDParam2[0].vRes > 0)
					gBDParam2[0].vRes -= 1;
			}
			if(gLCDPage.BDRect == 2)
			{
				if(gBDParam2[0].vVCC > 0)
					gBDParam2[0].vVCC -= 1;
			}
			break;
		case _F2:			// 向下
			gKeyValue &= 0xfd;
			if(gLCDPage.BDRect == 0)
			{
				if(gBDParam2[0].vAITime < 10000)
					gBDParam2[0].vAITime += 10;
			}
			if(gLCDPage.BDRect == 1)
			{
				if(gBDParam2[0].vRes < 50)
					gBDParam2[0].vRes += 1;
			}
			if(gLCDPage.BDRect == 2)
			{
				if(gBDParam2[0].vVCC < 30)
					gBDParam2[0].vVCC += 1;
			}
			break;
		case _F3:			// 确定
			gKeyValue &= 0xfb;
			gRunInfo.vSetFlg = 1;
				gSetAISend = 0x88;
			gAISetAck = 0;
			gLCDPage.vSave = 1;
			gLCDPage.BDRect++;
			if(gLCDPage.BDRect >= 3)
				gLCDPage.BDRect = 0;

			break;
		case _F4:			// 返回
			gKeyValue &= 0xf7;
			gLcdFresh = 4;
			gLCDPage.cmdsel = 0;
			gLCDPage.page = 0;
			gBDCmd = 0;
			gp_lcdtask = Disp_Cmd_Sel;
			break;
		default:
			gKeyValue = 0;
			break;
	}
}

/****************************************************************************
*
*    遥控器状态
*
****************************************************************************/
void Key_Cmd2_CtrlStatus(void)
{
	switch(gKeyValue)
	{
		case _F1:			// 向上
			gKeyValue &= 0xfe;
			break;
		case _F2:			// 向上
			gKeyValue &= 0xfd;
		//	gTstV ^= 0xfff;
			break;
		case _F3:			// 向上
			gKeyValue &= 0xfb;
			break;
		case _F4:			// 向上
			gKeyValue &= 0xf7;
			gLcdFresh = 3;
			//gLCDPage.cmdsel = 
			gp_lcdtask = Disp_Cmd_Sel;
			break;
		default:
			break;
	}
}


/****************************************************************************
*
*    比例电磁阀电流设置
*
****************************************************************************/
void Key_Cmd8_SetI(void)
{
	static unsigned char stSeted = 0;
	static unsigned char stSetSave = 0;
	
	if(gKeyValue == 0)
		return;
	//LCD_Set_Color(COLOR_RED_,-1);
	//LCD_Get_Color(0,5,5);

	switch(gKeyValue)
	{
		case _F1:			// 向右
			gKeyValue &= 0xfe;
			gLcdFresh = 1;
			if(gSetAIFlg == 0)
			{
				gSetAIX++;
				if(gSetAIX >= _ARM_USE)
					gSetAIX = 0;
			}
			if(gSetAIFlg == 1)
			{
				gLCDPage.BDRectSite++;
				if(gLCDPage.BDRectSite == 4)
					gLCDPage.BDRectSite = 1;
				LCD_Set_Cursor(1,gAISiteX[gSetAIX]+gLCDPage.BDRectSite*8,gAISiteY[gSetAIY]+15, 9, 1); 
			}

			break;
		case _F2:			// 向上移位
			gKeyValue &= 0xfd;
			gLcdFresh = 1;
			if(gSetAIFlg == 0)
			{
				if(gSetAIY == 0)
					gSetAIY = 4;
				gSetAIY--;
			}
			if(gSetAIFlg == 1)
			{
				stSeted = 1;
				switch(gLCDPage.BDRectSite)
				{
					case 0:
						gBDParam2[1].vAI[gSetAIY][gSetAIX] += 1000;
						if(gBDParam2[1].vAI[gSetAIY][gSetAIX] >= 2000)
							gBDParam2[1].vAI[gSetAIY][gSetAIX] -= 2000;
						break;
					case 1:
						if(gBDParam2[1].vAI[gSetAIY][gSetAIX]%1000 >= 900)
							gBDParam2[1].vAI[gSetAIY][gSetAIX] -= 900;
						else
							gBDParam2[1].vAI[gSetAIY][gSetAIX]+=100;
						break;
					case 2:
						if(gBDParam2[1].vAI[gSetAIY][gSetAIX]%100 >= 90)
							gBDParam2[1].vAI[gSetAIY][gSetAIX] -= 90;
						else
							gBDParam2[1].vAI[gSetAIY][gSetAIX]+=10;
						break;
					case 3:
						if(gBDParam2[1].vAI[gSetAIY][gSetAIX]%10 == 9)
							gBDParam2[1].vAI[gSetAIY][gSetAIX] -= 9;
						else
							gBDParam2[1].vAI[gSetAIY][gSetAIX]+=1;
						break;
				}
			}
			break;
		case _F3:			// 确定
			gKeyValue &= 0xfb;
			gLcdFresh = 1;
			gLCDPage.BDRectSite = 1;
			if(gSetAIFlg && stSeted)
			{
				gSetAISend = 0x80 + gSetAIX;
				stSetSave = 1;
			}
			gSetAIFlg ^= 1;
			stSeted = 0;
			gBDParam2[0].vAI[gSetAIY][gSetAIX] = gBDParam2[1].vAI[gSetAIY][gSetAIX];
			if(gSetAIFlg == 0)
				LCD_Set_Cursor(0,gAISiteX[gSetAIX],gAISiteY[gSetAIY]+15, 9, 1);
			else
				LCD_Set_Cursor(1,gAISiteX[gSetAIX],gAISiteY[gSetAIY]+15, 9, 1); 
			break;
		case _F4:			// 取消
			gKeyValue &= 0xf7;

			gLcdFresh = 4;
			gAISetFlg = 0;
			gAISendFlg = 0;
			gSetAIX = 0;
			gSetAIY = 0;
			if(stSetSave)
			{
				gLCDPage.vSave = 2;
				stSetSave = 0;
			}
			gp_lcdtask = Disp_Cmd_Sel;
			break;
		default:
			gKeyValue = 0;
			break;
	}		
}

void Key_Cmd8_SetI2(void)
{
	static unsigned char stSeted = 0;
	static unsigned char stSetSave = 0;
	
	if(gKeyValue == 0)
		return;
	//LCD_Set_Color(COLOR_RED_,-1);
	//LCD_Get_Color(0,5,5);

	switch(gKeyValue)
	{
		case _F1:			// 向右
			gKeyValue &= 0xfe;
			gLcdFresh = 1;
			if(gSetAIFlg == 0)
			{
				gSetAIX++;
				if(gSetAIX >= _ARM_USE)
					gSetAIX = 0;
			}
			if(gSetAIFlg == 1)
			{
				gLCDPage.BDRectSite++;
				if(gLCDPage.BDRectSite == 4)
					gLCDPage.BDRectSite = 1;
				LCD_Set_Cursor(1,gAISiteX[gSetAIX]+gLCDPage.BDRectSite*8,gAISiteY[gSetAIY]+15, 9, 1); 
			}

			break;
		case _F2:			// 向上移位
			gKeyValue &= 0xfd;
			gLcdFresh = 1;
			if(gSetAIFlg == 0)
			{
				if(gSetAIY == 0)
					gSetAIY = 4;
				gSetAIY--;
			}
			if(gSetAIFlg == 1)
			{
				stSeted = 1;
				switch(gLCDPage.BDRectSite)
				{
					case 0:
						gBDParam2[1].vAI2[gSetAIY][gSetAIX] += 1000;
						if(gBDParam2[1].vAI2[gSetAIY][gSetAIX] >= 2000)
							gBDParam2[1].vAI2[gSetAIY][gSetAIX] -= 2000;
						break;
					case 1:
						if(gBDParam2[1].vAI2[gSetAIY][gSetAIX]%1000 >= 900)
							gBDParam2[1].vAI2[gSetAIY][gSetAIX] -= 900;
						else
							gBDParam2[1].vAI2[gSetAIY][gSetAIX]+=100;
						break;
					case 2:
						if(gBDParam2[1].vAI2[gSetAIY][gSetAIX]%100 >= 90)
							gBDParam2[1].vAI2[gSetAIY][gSetAIX] -= 90;
						else
							gBDParam2[1].vAI2[gSetAIY][gSetAIX]+=10;
						break;
					case 3:
						if(gBDParam2[1].vAI2[gSetAIY][gSetAIX]%10 == 9)
							gBDParam2[1].vAI2[gSetAIY][gSetAIX] -= 9;
						else
							gBDParam2[1].vAI2[gSetAIY][gSetAIX]+=1;
						break;
				}
			}
			break;
		case _F3:			// 确定
			gKeyValue &= 0xfb;
			gLcdFresh = 1;
			gLCDPage.BDRectSite = 1;
			if(gSetAIFlg && stSeted)
			{
				gSetAISend = 0x90 + gSetAIX;
				stSetSave = 1;
			}
			gSetAIFlg ^= 1;
			stSeted = 0;
			gBDParam2[0].vAI2[gSetAIY][gSetAIX] = gBDParam2[1].vAI2[gSetAIY][gSetAIX];
			if(gSetAIFlg == 0)
				LCD_Set_Cursor(0,gAISiteX[gSetAIX],gAISiteY[gSetAIY]+15, 9, 1);
			else
				LCD_Set_Cursor(1,gAISiteX[gSetAIX],gAISiteY[gSetAIY]+15, 9, 1); 
			break;
		case _F4:			// 取消
			gKeyValue &= 0xf7;

			gLcdFresh = 4;
			gAISetFlg = 0;
			gAISendFlg = 0;
			gSetAIX = 0;
			gSetAIY = 0;
			if(stSetSave)
			{
				gLCDPage.vSave = 2;
				stSetSave = 0;
			}
			gp_lcdtask = Disp_Cmd_Sel;
			break;
		default:
			gKeyValue = 0;
			break;
	}		
}





