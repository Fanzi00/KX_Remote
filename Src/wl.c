/*****************************************************************************
 *   wdt.c:  Watchdog C file for NXP LPC17xx Family Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.05.27  ver 1.00    Prelimnary version, first Release
 *
*****************************************************************************/
#include "LPC17xx.h"
#include "config.h"
#include "wl.h"

////  XRZNWR  0x: 58 52 5A 4E 57 52  
unsigned char  gWLInfo[8];							// 无线参数（全部），用来读写
unsigned char  gWLCmd = 0;							// 无线命令
unsigned short  gWLParam = 0;						// 无线参数，新
unsigned char  gWLRWFlg = 0;

	 unsigned char stBKClose = 0;
unsigned char  gPower24V = 1;						// 是否 24V 供电，24V供电时，不考虑电池，不关无线

/*****************************************************************************
** Function name:		无线配置
**
** Descriptions:		需要在 CANOPEN的接收、UART的接收、主循环中调用不同的函数，备注 lsg_wl
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/

void wl_reset(void)
{
	GPIO_TypeDef * pGPIO = NULL;
	//// IO_MD0 
	GPIO0->FIODIR |= (0x01 << IO_MD0); 		// P0.4
	GPIO0->FIOSET |= (0x01 << IO_MD0);
	//// IO_MD1 
	GPIO1->FIODIR |= (0x01 << IO_MD1);		// P1.1
	GPIO1->FIOSET |= (0x01 << IO_MD1);

//	UARTInit(0, 115200);
//	UARTInit(1, 115200);
		delay_ms(10);

	//// IO_MD0 
	GPIO0->FIOCLR |= (0x01 << IO_MD0);
	//// IO_MD1 
	GPIO1->FIOCLR |= (0x01 << IO_MD1);	
		delay_ms(10);
}


uint32_t wl_rd( void )
{
	unsigned char i = 0;
	unsigned char vStr[10];
	GPIO_TypeDef * pGPIO = NULL;


	//// IO_MD0 
	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*0)); 		// P0.4
	pGPIO->FIODIR |= (0x01 << IO_MD0);
	pGPIO->FIOSET |= (0x01 << IO_MD0);
	//// IO_MD1 
	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*1)); 		// P1.1
	pGPIO->FIODIR |= (0x01 << IO_MD1);
	pGPIO->FIOSET |= (0x01 << IO_MD1);
		delay_ms(10);

	if(gWLRWFlg == 0x5A)				// 读取
	{
		i = 0;
		UARTInit(0, 9600);
		delay_ms(20);
		vStr[i++] = 0xc1;
		vStr[i++] = 0xc1;
		vStr[i++] = 0xc1;
			UARTSend(0, vStr, i);
		gWLRWFlg = 0x5E;
	}

	return 0;
}
uint32_t wl_wr( void )
{
	unsigned char i = 0;
	unsigned char vStr[10];
	GPIO_TypeDef * pGPIO = NULL;


	//// IO_MD0 
	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*0)); 		// P0.4
	pGPIO->FIODIR |= (0x01 << IO_MD0);
	pGPIO->FIOSET |= (0x01 << IO_MD0);
	//// IO_MD1 
	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*1)); 		// P1.1
	pGPIO->FIODIR |= (0x01 << IO_MD1);
	pGPIO->FIOSET |= (0x01 << IO_MD1);
		delay_ms(10);

	if(gWLRWFlg == 0x55)				// 设置
	{
		i = 0;
		UARTInit(0, 9600);
		delay_ms(20);

		if(gWLInfo[0] == 0xc0)
		{
			if(gWLCmd == 0x81)
			{
				gWLInfo[3] &= 0xf8;
				gWLInfo[3] |= gWLParam & 7;
			}
			UARTSend(0, gWLInfo, 6);
		}
		delay_ms(20);
		gWLRWFlg = 0x5A;
	}
	return 0;
}

void WL_Set(void)
{
	if((CAN_RBuf0[4] == 'W'))						// 	设置无线参数，模块地址，通道，空速，串口速率等
	{
		gWLCmd = CAN_RBuf0[5];															// 配置的命令字， 0x81--空速
		gWLParam = (CAN_RBuf0[6]) | (CAN_RBuf0[7]<<8);			// 参数
		gWLRWFlg = 0x55;
		//wl_wr();
	}
	if((CAN_RBuf0[4] == 'R'))						// 	读取无线参数，频率，跳频信道数，跳频序列
	{
		gWLRWFlg = 0x5A;
		//wl_rd();
	}
}

void wl_set_param(void)
{
	if(gWLRWFlg == 0x55)				// 设置
		wl_wr();
	if(gWLRWFlg == 0x5A)				// 读取
		wl_rd();
}

void wl_rd_rcv(void)
{
	unsigned char i=0;
	unsigned char k=0;
	unsigned char l=0;
	unsigned char vULen = 0;		// UART 接收字符数
	unsigned char vCStr[10];

	if(gWLRWFlg >= 0x5A)				// 读取 无线模块的当前参数，等待模块返回数据
	{
		vULen = (uint8_t)((UART0RxHead - UART0RxTail) & UART0_RXBUF_MASK);
		if(vULen >= 7)
		{
			for(i=0; i<vULen; i++)
			{
				if(UART0_RX_GET(i) == 0xC0)
				{
					if((i + 7) > vULen)		// 0xC0 帧共 7 字节，不足则等下一批（环形缓冲不会丢，不越界）
						break;
					k = 0;
					l = i;
					vCStr[k++] = 'X';
					vCStr[k++] = 'R';
					vCStr[k++] = UART0_RX_GET(i++);
					vCStr[k++] = UART0_RX_GET(i++);
					vCStr[k++] = UART0_RX_GET(i++);
					vCStr[k++] = UART0_RX_GET(i++);
					vCStr[k++] = UART0_RX_GET(i++);
					vCStr[k++] = UART0_RX_GET(i++);
					CAN_Send(0, ID_ACK_M1, vCStr);
					k=0;
					i=l;
					gWLInfo[k++] = UART0_RX_GET(i++);
					gWLInfo[k++] = UART0_RX_GET(i++);
					gWLInfo[k++] = UART0_RX_GET(i++);
					gWLInfo[k++] = UART0_RX_GET(i++);
					gWLInfo[k++] = UART0_RX_GET(i++);
					gWLInfo[k++] = UART0_RX_GET(i++);
					gWLInfo[k++] = 0;
					gWLRWFlg = 0x5f;
					UART0RxTail = (UART0RxTail + vULen) & UART0_RXBUF_MASK;
					break;
				}
			}
		}
	}
}

void wl_lcd_power(void)
{
	static unsigned char stJTFlg = 0;
	static unsigned int stPowerLow = 0;

		if(gJTDly > 10*1000)
			_CPU_DIS;
		if(gJTDly >= 5000)					// 急停 或 电池欠压 关 LCD 电源
		{
			_LCD_DIS;
			_WL_DIS;
		}
		if(gLCDBKLDly >= _LCD_OFF_DLY)	// 无操作时，关LCD电源
		{
				delay_ms(2);
				stBKClose = 1;
				_LCD_DIS;
		}
		//// 急停， 关闭屏，停止发送无线数据
		if((gJiTing == 0x55))				// || lsg_www
		{
				if(gLCDBKLight > 0)
					gLCDBKLight0 = gLCDBKLight;
				gLCDBKLight = 0;
				if((gJiTing == 0x55) && (gJTDly < 5))
					LCD_SET_BL(gLCDBKLight);
				gLaBaFlg = 0;
				gLCDBKOnFlg = 1;
				gJTDly++;
			stJTFlg = 1;
		}
		else
		{
			if(stJTFlg == 1)
			{
				stJTFlg = 0;
				_LCD_EN;
				_WL_EN;
				delay_ms(800);
				gLCDBKLight = gLCDBKLight0;
				gLCDBKOnFlg = 2;
				//LCD_SET_BL(gLCDBKLight);
				gLcdFresh = 2;
			}
			gJTDly = 0;
		}		
		
	//	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020));
	//	gPower24V = ((GPIO1->FIOPIN & (0x01 << _DI_24V)) == (0x01 << _DI_24V)? 1:0);
		gPower24V = ((GPIO0->FIOPIN & (0x01 << _DI_24V)) == (0x01 << _DI_24V)? 1:0);     // Ver3 P0.18	20180629 
		if((gCellPower == 0) && (gPower24V==0))  	// 电池供电时，欠压保护    g24VFlg
		{
			stPowerLow++;
			if(stPowerLow > 4000)
			{
				_LCD_DIS;
				_WL_SLEEP;
				_CPU_DIS;
				_WL_DIS;
			}
		}
		if(gKeyValue0x)							// 按键，打开LCD电源
		{
			gLCDBKLDly = 0;
			if((stBKClose == 1) || ((gLCDBKLight0==0) &&(gKeyValue0x&0xfe)))
			{
				stBKClose = 0;
					_LCD_EN;
					_WL_EN;
				gLCDBKOnDly = 800;
				//	delay_ms(800);
				if(gLCDBKLight0 == 0)
					gLCDBKLight0 = 16;
				gLCDBKLight = gLCDBKLight0;
					LCD_SET_BL(gLCDBKLight);
					gLCDBKOnFlg = 2;
					//LCD_SET_BL(gLCDBKLight);
					gLcdFresh = 2;
			}
			gKeyValue0x = 0;
		}
			
}

void T0S_Prog(void)
{
	unsigned char i=0;
	unsigned char vFlg = 0;
	
		Cell_Chk();			  // 电池电量检查
		//// 是否有摇杆动作
	/*
		for(i=0; i<_ARM_USE; i++)
		{
			if(gADArmSend[i] != _ARM_MIDV)
				vFlg++;
		}
	*/
		////  LCD按键动作
		if(gKeyValue0x)
			vFlg = 1;
		//gKeyValue0x = 0;
		if(vFlg == 0)
		{
			gLCDBKLDly++;
			if(gLCDBKLDly >= _LCD_OFF_DLY)
			{
				if(gLCDBKLDly == _LCD_OFF_DLY)
					gLCDBKLight0 = gLCDBKLight;
				gLCDBKLight = 8;
				LCD_SET_BL(gLCDBKLight);
				delay_ms(10);
				_LCD_DIS;
				stBKClose = 1;
					gLCDBKOnFlg = 1;
			}
		}
		else
		{
			/*
			CAN_SBuf1[0] = 0X11;
			CAN_SBuf1[1] = gKeyValue0;
		CAN_Send(0, 0x567, CAN_SBuf1);
			if(stBKClose == 1)
			{
				stBKClose = 0;
				gLCDBKLight = gLCDBKLight0;
				LCD_SET_BL(gLCDBKLight);
			}
			gLCDBKLDly = 0;
			*/
		}
}
/******************************************************************************
**                            End Of File
******************************************************************************/


