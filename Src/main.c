/*****************************************************************************
 *   main.c:  main file for Panel(SanHe) with NXP LPC17xx Family Microprocessors
 *
 *   Copyright(C) 2012, HuNan XiangRui Intelligence
 *   All rights reserved.
 *
 *   History
 *   2013.08.17  ver 1.00    Prelimnary version, first Release
 *
 *    开关面板(SanHe)，Key/Led/DI/DO/CAN
 *
******************************************************************************/
#include <lpc17xx.h>
#include "config.h"
#include "can.h"
#include "timer.h"



extern unsigned int gCanRFlg;
extern unsigned char  gWLFlg;

#define _WL_LOST_DLY 1			// 无线信号连续丢失多少秒后才判定接收机离线(断线延时1s，防瞬时丢帧/信号闪烁误掉线)

unsigned char gPowerFlg0 = 0;
unsigned int  gLCDBC = 0;		// 显示的通用背景色
//unsigned char gTstStr[30];
//unsigned long  gTstRD = 0;

unsigned char  gID = ID_DEFAULT;// 0x58

	
//void BD_Patam_Init(void);
uint Poweron_Chk(void);
void T0S_Prog(void);

	
void delay_ms(unsigned int vDel)
{
	unsigned int i;
	unsigned int j;
	//while(vDel--)
	for(j=0; j<vDel; )
	{
		for(i=0; i<19200; )
			i++;
		j++;
	}
}


void ValInit(void)			//初始化全局变量
{
	
	//取废数据
//	AD_Val_Init();
}

////  5V 检测，针对 万向摇杆， 20180629  Ver3版本， ADC05, ADV[17]  lsg 
unsigned char Power_5V_Chk(unsigned char ChkFlg)
{
	static unsigned char stChkFlg = 0;
	static unsigned short st5VVal[2];					// 上电检测出来的 5V 电压值
	static unsigned int  stChkCnt[2];
	
	unsigned char vCh1 = 0;
	unsigned char vCh2 = 0;
	unsigned short vErrCnt = 400;							// 连续出错的次数/毫秒数 
	unsigned char vRtn = 0;
	unsigned char vStr[9];
	static unsigned short vV50 = 0;
//	static unsigned short vV4x = 0;
//	static unsigned short vV5x = 0;
	static unsigned short vVx100mv = 0;				// 允许波动电压范围对应的 采样值  
	unsigned short vVFY = 1555;								// 5V 分压后的电压值 
	float f100mv = 0.0;												// 5V 的0.1V 对应的采样值
	
	vCh1 = 17;																	// ADC05， ADV[17] 用做 5V 电压检测   20180629  
	vCh2 = 11;
	vRtn = 0;
		f100mv = vVFY;
		f100mv = f100mv / 5000 * 4095 / 3 / 10;
		vV50 = 50 * f100mv; 										// 5.0V 的采样值  
	if((stChkFlg == 0) || (ChkFlg == 0))
	{
//		vV4x = 47 * f100mv;											// 4.7V 的采样值  
//		vV5x = 53 * f100mv; 										// 5.3V 的采样值  
		
		vVx100mv = 2 * f100mv;									// 允许波动 0.2V
		if((ADCRstP[vCh1] > vV50 - vVx100mv) && (ADCRstP[vCh1] < vV50 + vVx100mv))							// 0xc88 -- 4.7V 分压到 2.35V  
			st5VVal[0] = ADCRstP[vCh1];
		else
			st5VVal[0] = vV50;
/*		if((ADCRstP[vCh2] > vV50 - vVx100mv) && (ADCRstP[vCh2] < vV50 + vVx100mv))
			st5VVal[1] = ADCRstP[vCh2];
		else
			st5VVal[1] = vV50;
*/
		stChkFlg = 1;	
		vStr[0] = st5VVal[0];
		vStr[1] = st5VVal[0]>>8;
		vStr[2] = ADCRstP[vCh1];
		vStr[3] = ADCRstP[vCh1]>>8;
		vStr[4] = st5VVal[1];
		vStr[5] = st5VVal[1]>>8;
		vStr[6] = ADCRstP[vCh2];
		vStr[7] = ADCRstP[vCh2]>>8;
		CAN_Send(0, 0x6a0+gID, vStr);
	}
	else														// 检测 5V 电压波动范围不超过 0.2V（采样值132）
	{
		vVx100mv = 3 * f100mv;									// 允许波动 0.3V
		if((ADCRst[vCh1] >= st5VVal[0] + vVx100mv) || (ADCRst[vCh1] <= st5VVal[0] - vVx100mv))
		{
			if(stChkCnt[0] < vErrCnt+200)
				stChkCnt[0]++;
		}
		else
		{
			if(stChkCnt[0] )
				stChkCnt[0]--;
		}
/*		if((ADCRst[vCh2] >= st5VVal[1] + vVx100mv) || (ADCRst[vCh2] <= st5VVal[1] - vVx100mv))
		{
			if(stChkCnt[1] < vErrCnt+200)
				stChkCnt[1]++;
		}
		else
		{
			if(stChkCnt[1] )
				stChkCnt[1]--;
		}
		*/
		if((stChkCnt[0] > vErrCnt))	// && (stChkCnt[1] > vErrCnt))
			vRtn = 1;
	}
	return vRtn;
}
unsigned char vChk5VFlg = 0;				// 5V 检测标记 		20180506  lsg 

/*****************************************************************************
** Function name:		main
**
** Descriptions:		main routine for CAN module test
**
** parameters:			None
** Returned value:		int
** 
*****************************************************************************/
  unsigned int bsl_lp = 200;
	unsigned char vMPowerFlg = 0;
unsigned char gWLDly_lp = 1;
unsigned char gAISendChk = 0;				// 发送的 AI 序号
unsigned char gAISendOK = 0;				// 上电后是否发送完 AI
unsigned int  gWLRcvs = 0;
unsigned int  gWLRcvOk = 0;
unsigned int  gJTDly = 0;
static unsigned char stWLHad = 0;		// 接收机上一秒是否在线(用于掉线边沿检测)
static unsigned char stWLLostDly = 0;	// 无线信号丢失计时(秒)，连续丢失达 _WL_LOST_DLY 才判定离线

int main( void )
{
//	uint32_t counter = 0;
	static unsigned char stAIi = 0;
	static unsigned char stJTFlg = 0;
	unsigned char i = 0;
	unsigned char vFlg = 0;
	unsigned int vChkCnt = 0;
	
	delay_ms(10);
	SystemInit();

	Sys_Param_Init(ID_DEFAULT);
	IO_Init();
  /* Please note, this PCLK is set in the target.h file. 
   The bit timing is based on the 
  setting of the PCLK, if different PCLK is used, please read can.h carefully 
  and set your CAN bit timing accordingly. */  

	BD_Patam_Init();
	I2CInit(I2CMASTER);
				WDT_Feed();
	Sys_Read_BD();
				WDT_Feed();
	Sys_Read_BD2();
				WDT_Feed();
	init_timer( ); // 10ms	
//		CAN_Init( gSysInfo[0].vBps);
	BEEP_ON;
	delay_ms(10);
	BEEP_OFF;
	WDT_Feed();

	UARTInit(0, 115200);
	UARTInit(1, 115200);

#if 1
	ADCInit( ADC_CLK );
	CAN_Init( gSysInfo[0].vBps);
	BEEP_ON;
	delay_ms(20);
	Can_RcvID_Cfg();
	enable_timer(0);
	ADC_StartCnv();
	//		CAN_Send(0, 0x780+gID, CAN_SBuf3);
	BEEP_OFF;

#endif
	UARTInit(0, 115200);
	gID = ID_DEFAULT;
 	gCanCyc = gSysInfo[0].vCycle * 10;
	Arm_RockX();
	for(i=0; i<20; i++)
	{
		delay_ms(100);
		WDT_Feed();		
			Arm_Conv();									// 20180506  lsg   5V  
	}
	LCD_SET_BL(gLCDBKLight);
	Disp_PicX(1);// 显示Logo
	for(i=0; i<20; i++)
	{
		delay_ms(100);
		WDT_Feed();		
			Arm_Conv();
		vFlg = Poweron_Chk();						// 20190321 lsg add 
		if(vFlg)
			vChkCnt++;
	}
			Arm_Conv();
	if(vChkCnt > 15)
		gPowerChk = 1;
	// 20180426  lsg 
			vChk5VFlg = Power_5V_Chk(0);								// 20180506  lsg   5V  
			if(vChk5VFlg)
				gPowerChk |= 0x10000;
			else
				gPowerChk &= ~0x10000;
	gWLDly_lp = 1;
	gLaBaFlg = 0;
	gLcdFresh = 5;
	if(gPowerChk)
			gp_lcdtask = Disp_ZZT_CtrlStatus;
	else
			gp_lcdtask = Disp_mainSPJ;
		
	while ( 1 )
	{
		Can_ask_rx();				// CAN 异常诊断

		if(gT0Flg == 1)				// T0 , 1 ms
		{
			gT0Flg = 0;
				WDT_Feed();
			if(gWLRWFlg )				// 设置			lsg_wl
				wl_set_param();
			wl_lcd_power();			// 电源控制 	lsg_wl

			vChk5VFlg = Power_5V_Chk(1);								// 20180506  lsg   5V  
			if(vChk5VFlg)
				gPowerChk |= 0x10000;
			else
				gPowerChk &= ~0x10000;

				
			#if _SYS_YKJK	  		// 1--遥控
				if(gWLFlg && (gCmdYKFlg == _SYS_YKJK) )
				{
					gWLFlg = 0;
					Arm_Conv();						//  摇杆数据转换
						YKQ_Data_WL(0);				//  数据转换到无线
					if(gTXCANDly)
						;
					else   
						{
						WDT_Feed();
								gWLDly_lp = 1;
								gWLFlg = 0;
								if(gJTDly < 5*1000)
								 	Uart0_WL_Send();
						}
					}
				 
			#endif		
		}

		if(gADConvFlg)
		{
			gADConvFlg = 0;
			Arm_Conv();
		}

		////if(gWLFlg && (gPowerFlg || (gTXCANDly==0)) )
		if(	gUart0RcvFlg == 1)			 // 收到UART0 数据，无线 通信
		{
			gUart0RcvFlg = 0;
				WDT_Feed();
			gWLRcvs++;
			Uart0_WL_Rcv();
		}

		if(gT0SFlg)
		{
			gT0SFlg = 0;
			{
				unsigned int rawSing = gWLRcvOk;
				// 信号格显示值低通平滑：帧数上升立即跟上，下降缓慢(~1/秒)，避免 RF 瞬时衰落导致信号格乱跳/误判掉线；掉线判定仍用原始值 rawSing，逻辑不变
				if(rawSing >= gWLSingV)	gWLSingV = rawSing;
				else if(gWLSingV > 0)	gWLSingV--;
				gWLRcvOk = 0;
				T0S_Prog();
				if(rawSing == 0)
				{
					//// 本秒未收到接收机数据：累计丢失时长，连续丢满 _WL_LOST_DLY 秒才清激活态
					if(stWLLostDly < _WL_LOST_DLY)
						stWLLostDly++;
					if(stWLLostDly >= _WL_LOST_DLY)
					{
						wl_reset();
						gLaBaFlg = 0;		// 连续丢满 _WL_LOST_DLY 秒才清激活态，恢复后需手动重新激活（关闭自动重连）
						stWLHad = 0;		// 标记接收机离线
					}
				}
				else
				{
					stWLLostDly = 0;		// 收到数据，清除信号丢失计时
					stWLHad = 1;		// 标记接收机在线（已关闭自动重连：掉线后需手动重新激活）
				}
			}
		}
		// 收到CAN数据
		if( CAN1RxDone == TRUE )		 
		{
			CAN1RxDone = FALSE;
			Can_Prog_Rcv(gCanRFlg);
		}
		// 发送 CAN 数据
		if(app10ms_flags) //10ms 运行一次
		{
			Can_Send_Prog();
		//	Can_Prog_Send(0);			// 转换数据
			app10ms_flags = 0;
			app10ms() ;
#ifdef _SELF_CHK_EN			// 自检功能开关：用 #ifdef 控制，在 User/_Device_config.h 中定义即开启、#undef 即关闭
			SelfChk_main() ;
#endif
		}
		if(gLCDFlg)
		{
			gLCDFlg = 0;
			gDataFresh = 1;
			if(gLCDBKOnDly == 0)
				gp_lcdtask();			  // 显示、按键 任务
		}
		if(gSaveFlg == 1)
		{
			gSaveFlg = 2;
			Sys_Write_BD();
		}
		if(gLCDPage.vSave)
		{
			gLCDPage.vSave = 0;
			Sys_Write_BD2();
		}
	}
}

//// 上电检查，主要检测摇杆是否中位，必须都在中位才认定无故障
uint Poweron_Chk(void)
{
	unsigned char i=0;
	unsigned int vChk = 0;

	for(i=0; i<_ARM_USE; i++)			// 摇杆中位检测，上电时所有摇杆均应在中位
	{
	//	if(gADArmSend[i] != _ARM_MIDV) //  hudh
	//		vChk |= 1<<(i);	
		/*
		//if(ADCRst[gArmUpTab[i]] > _ARM_MID_SPACE)
		if(ADCRst[gArmUpTab[i]] < gBDParam[0].vRockV[i*2][0] - gArmMidSp[i*2])
			vChk |= 1<<(i);
		//if(ADCRst[gArmDownTab[i]] > _ARM_MID_SPACE)
		if(ADCRst[gArmDownTab[i]] > gBDParam[0].vRockV[i*2+1][0] + gArmMidSp[i*2+1])
			vChk |= 1<<(i+8);
		*/
	}
	//if(gDIBitV & 0x1f)					// 按钮输入检测，上电时所有按键均应无效
	//	vChk |= (gDIBitV & 0x0f)<<16;

	return vChk;	
}



