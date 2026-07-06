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


#define  _WL_SLEEP	{ GPIO0->FIOSET |= IO_MD0;  GPIO1->FIOSET |= IO_MD1;  }				// 无线休眠
#define  _WL_WAKEUP	{ GPIO0->FIOCLR |= IO_MD0;  GPIO1->FIOCLR |= IO_MD1;  }				// 无线唤醒

#define  _LCD_OFF_DLY (10 * 30)				// (3 * 60) 秒  持续无操作时，关LCD电源

extern unsigned char  gWLInfo[8];							// 无线参数（全部），用来读写
extern unsigned char  gWLCmd ;							// 无线命令
extern unsigned short  gWLParam ;						// 无线参数，新
extern unsigned char  gWLRWFlg ;

extern	 unsigned char stBKClose ;
extern  unsigned char  gPower24V ;						// 是否 24V 供电，24V供电时，不考虑电池，不关无线

uint32_t wl_rd( void );
uint32_t wl_wr( void );
void wl_rd_rcv(void);
void WL_Set(void);
void wl_set_param(void);

void T0S_Prog(void);
void wl_lcd_power(void);
void wl_reset(void);


/******************************************************************************
**                            End Of File
******************************************************************************/
