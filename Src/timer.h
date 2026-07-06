/*****************************************************************************
 *   timer.h:  Header file for NXP LPC17xx Family Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.05.26  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __TIMER_H 
#define __TIMER_H

#include "config.h"


#define Cclk			(Fextal * 8)
#define Fcclk 			( Cclk / 4)
	
#define TIME_INTERVAL	(Fcclk/1000 - 1)

#define TIME_PR_V		4			// ·ÖÆµ




extern unsigned char  gT0Flg;
extern unsigned char  gSysPower;
extern unsigned short  gPowerDly;
extern unsigned char  gCanSFlg;
extern unsigned char  gT0SFlg ;

extern unsigned short gLCDCnt ;
extern unsigned char  gLCDFlg ;
extern unsigned short gCanCyc;

extern unsigned short gT0SCnt ;

extern void delayMs(uint8_t timer_num, uint32_t delayInMs);
extern uint32_t init_timer( void );
extern void enable_timer( uint8_t timer_num );
extern void disable_timer( uint8_t timer_num );
extern void reset_timer( uint8_t timer_num );
extern void TIMER0_IRQHandler (void);
extern void TIMER1_IRQHandler (void);

#endif /* end __TIMER_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
