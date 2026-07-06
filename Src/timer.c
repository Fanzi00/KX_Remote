/*****************************************************************************
 *   timer.c:  Timer C file for NXP LPC17xx Family Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.05.26  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#include <lpc17xx.h>
#include "config.h"
#include "timer.h"

volatile uint32_t timer0_counter = 0;
volatile uint32_t timer1_counter = 0;

/*
volatile unsigned int   gT0TcntDown = 0;
volatile unsigned int   gT1TcntDown = 0;
volatile unsigned int   gT2TcntDown = 0;
volatile unsigned int   gT3TcntDown = 0;
volatile unsigned int   gT0TcntUp = 0;
volatile unsigned int   gT1TcntUp = 0;
volatile unsigned int   gT2TcntUp = 0;
volatile unsigned int   gT3TcntUp = 0;
volatile unsigned char  gT0CFlag = 0;	//// 电平变化标记
volatile unsigned char  gT1CFlag = 0;
volatile unsigned char  gT2CFlag = 0;
volatile unsigned char  gT3CFlag = 0;

unsigned char  gT0AFlag = 0;			//// 计算角度的标记
unsigned char  gT1AFlag = 0;
unsigned char  gT2AFlag = 0;
unsigned char  gT3AFlag = 0;
*/
//extern unsigned int  GRcv44Dly;

unsigned short  gT0CanCnt = 0;
unsigned char  gCanSFlg = 0;
unsigned char  gT0Flg = 0;				// T0 上升沿标记(100HZ)
unsigned char  gDJDICnt = 0;
unsigned char  gSysPower = 0;
unsigned short  gPowerDly = 0;
unsigned short gCanCyc = 0;
unsigned short gT0SCnt = 0;
unsigned char  gT0SFlg = 0;

unsigned short gLCDCnt = 0;
unsigned char  gLCDFlg = 0;
unsigned short gWLCnt = 0;
unsigned char  gWLFlg = 0;

unsigned int   ts_s = 0;
/*****************************************************************************
** Function name:		delayMs
**
** Descriptions:		Start the timer delay in milo seconds
**						until elapsed
**
** parameters:			timer number, Delay value in milo second			 
** 						
** Returned value:		None
** 
*****************************************************************************/
void delayMs(uint8_t timer_num, uint32_t delayInMs)
{
  if ( timer_num == 0 )
  {
	/*
	* setup timer #0 for delay
	*/
	TIM0 -> TCR = 0x02;		/* reset timer */
	TIM0 -> PR  = 0x00;		/* set prescaler to zero */
	TIM0 -> MR0 = delayInMs * (9000000 / 1000-1);
	TIM0 -> IR  = 0xff;		/* reset all interrrupts */
	TIM0 -> MCR = 0x04;		/* stop timer on match */
	TIM0 -> TCR = 0x01;		/* start timer */
  
	/* wait until delay time has elapsed */
	while (TIM0 -> TCR & 0x01);
  }
  else if ( timer_num == 1 )
  {
	/*
	* setup timer #1 for delay
	*/
	TIM1 -> TCR = 0x02;		/* reset timer */
	TIM1 -> PR  = 0x00;		/* set prescaler to zero */
	TIM1 -> MR0 = delayInMs * (9000000 / 1000-1);
	TIM1 -> IR  = 0xff;		/* reset all interrrupts */
	TIM1 -> MCR = 0x04;		/* stop timer on match */
	TIM1 -> TCR = 0x01;		/* start timer */
  
	/* wait until delay time has elapsed */
	while (TIM1 -> TCR & 0x01);
  }
  return;
}

/******************************************************************************
** Function name:		enable_timer
**
** Descriptions:		Enable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
** 
******************************************************************************/
void enable_timer( uint8_t timer_num )
{
	TIM0 -> TCR = 1;
//	TIM1 -> TCR = 1;
//	TIM2 -> TCR = 1;
//	TIM3 -> TCR = 1;

	return;
}

/******************************************************************************
** Function name:		disable_timer
**
** Descriptions:		Disable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
** 
******************************************************************************/
void disable_timer( uint8_t timer_num )
{
  if ( timer_num == 0 )
  {
	TIM0 -> TCR = 0;
  }
  else
  {
	TIM1 -> TCR = 0;
  }
  return;
}

/******************************************************************************
** Function name:		reset_timer
**
** Descriptions:		Reset timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
** 
******************************************************************************/
void reset_timer( uint8_t timer_num )
{
  uint32_t regVal;

  if ( timer_num == 0 )
  {
	regVal = TIM0 -> TCR;
	regVal |= 0x02;
	TIM0 -> TCR = regVal;
  }
  else
  {
	regVal = TIM1 -> TCR;
	regVal |= 0x02;
	TIM1 -> TCR = regVal;
  }
  return;
}
//  定时中断
void Timer0Cfg(void)
{
	TIM0->TCR = 0x02;
	TIM0->MR0 = TIME_INTERVAL;
	TIM0->MCR = 3;
	NVIC_EnableIRQ(TIMER0_IRQn);
	
}

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void TIMER0_IRQHandler (void) 
{
	static unsigned int	gT0SCnt2=0;
	TIM0 -> IR = 1;			/* clear interrupt flag */
		gT0Flg = 1;

	gT0SCnt2++;
		if(gT0SCnt2 == 1499)
		{
				if(gWLSingV==0)
				{	_RED_ON;	}
			}
		if(gT0SCnt2 == 1500)
		{
				if(gWLSingV==0)
				{	_RED_OFF;	}
			}

	if(gT0SCnt2 == 1999)
	{
				if((gCellPower==0) || (gWLSingV==0))
				{	_RED_ON;	}
				else
				{	_POW_ON;	}
	}
	if(gT0SCnt2 >= 2000)
	{
		gT0SCnt2 = 0;
			_POW_OFF;
			_RED_OFF;
	}

	gT0SCnt++;
	if(gT0SCnt >= 1000)
	{
		gT0SCnt = 0;
		gT0SFlg = 1;
	}

	gLCDCnt++;
	if(gLCDCnt >= LCD_FRESH_MS)
	{
		gLCDCnt = 0;
		gLCDFlg = 1;
	}
	gWLCnt++;
	if(gWLCnt >= 100)
	{
		gWLCnt = 0;
		gWLFlg ++;
	}
	DI_Scan();

	gT0CanCnt++;
	if(gT0CanCnt == gCanCyc - 20)
		gCanSFlg |= 16;
	if(gT0CanCnt == gCanCyc - 15)
		gCanSFlg |= 8;
	if(gT0CanCnt == gCanCyc - 10)
		gCanSFlg |= 2;
	if(gT0CanCnt == gCanCyc - 5)
		gCanSFlg |= 4;
	if(gT0CanCnt >= gCanCyc)
	{
		gCanSFlg |= 1;
		gT0CanCnt = 0;
	}

	if(UART0RcvDly)
	{
		UART0RcvDly--;
		if(UART0RcvDly == 1)
			gUart0RcvFlg = 1;
	}

	if(gLCDBKOnDly)							// lsg_wl
		gLCDBKOnDly--;
	if(gTXCANDly)
		gTXCANDly--;

				ts_s++ ;
		if (ts_s>9)
		{
			app10ms_flags = 1 ;	
			ts_s = 0 ;
		}
}



/******************************************************************************
** Function name:		init_timer
**
** Descriptions:		Initialize timer, set timer interval, reset timer,
**						install timer interrupt handler
**
** parameters:			timer number and timer interval
** Returned value:		true or false, if the interrupt handler can't be
**						installed, return false.
** 
******************************************************************************/
uint32_t init_timer ( void ) 
{

	Timer0Cfg();
	//Timer1Cfg();
	//Timer2Cfg();
	//Timer3Cfg();

	return (TRUE);
}

/******************************************************************************
**                            End Of File
******************************************************************************/
