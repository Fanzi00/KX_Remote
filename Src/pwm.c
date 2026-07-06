/*****************************************************************************
 *   pwm.c:  PWM module file for NXP LPC17xx Family Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.05.26  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#include "lpc17xx.h"
#include "config.h"
#include "pwm.h"


unsigned int  gPwmCycle;
volatile uint32_t match_counter1;	 //match_counter0, 

/******************************************************************************
** Function name:		PWM1_IRQHandler
**
** Descriptions:		PWM1 interrupt handler
**						For now, it only deals with PWM1 match 0
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void PWM1_IRQHandler (void) 
{
  uint32_t regVal;
//  WDT_Feed();

  regVal =  PWM1->IR;
  if ( regVal & MR0_INT )
  {
	match_counter1++;	
  }
   PWM1->IR |= regVal;		/* clear interrupt flag on match 0 */
  return;
}

/******************************************************************************
** Function name:		PWM_Init
**
** Descriptions:		PWM initialization, setup all GPIOs to PWM0~6,
**				reset counter, all latches are enabled, interrupt
**				on PWMMR0, install PWM interrupt to the VIC table.
**
** parameters:			ChannelNum, Duty cycle
** Returned value:		true or fase, if VIC table is full, return false
** 
******************************************************************************/
	uint32_t pwm_pclkdiv, pwm_pclk = PWM_CLK;
uint32_t PWM_Init( uint32_t ChannelNum, uint32_t cycle )
{

	pwm_pclkdiv = (SC->PCLKSEL0 >> 24) & 0x03;
	switch ( pwm_pclkdiv )
	{
		case 0x00:
		default:
		  pwm_pclk = SystemFrequency/4;
		break;
		case 0x01:
		  pwm_pclk = SystemFrequency;
		break; 
		case 0x02:
		  pwm_pclk = SystemFrequency/2;
		break; 
		case 0x03:
		  pwm_pclk = SystemFrequency/8;
		break;
	}
  {
	match_counter1 = 0;
	 SC->PCONP |= (1 << 6);
	 PINCON->PINSEL4 &= ~0x00003fff;
	 PINCON->PINSEL4 |= 0x00001555;	/* set GPIOs for all PWM pins on PWM0 */

	 PWM1->TCR = TCR_RESET;	/* Counter Reset */ 
	 //PWM1->CTCR = 0;
	 //PWM1->CCR = 0;
	 PWM1->PR = 0x0;		/* count frequency:Fpclk */
	 PWM1->MCR = PWMMR0I | PWMMR0R;  /*  ;	 interrupt on PWMMR0, reset on PWMMR0, reset 
								TC if PWM0 matches */		
		pwm_pclk = pwm_pclk/cycle;									
	 PWM1->MR0 = pwm_pclk;		/* set PWM cycle */
	 PWM1->MR1 = 0;
	 PWM1->MR2 = 0;
	 PWM1->MR3 = 0;
	 PWM1->MR4 = 0;
	 PWM1->MR5 = 0;
	 PWM1->MR6 = 0;	
/*	 PWM1->MR1 = cycle * 5/6;
	 PWM1->MR2 = cycle * 2/3;
	 PWM1->MR3 = cycle * 1/2;
	 PWM1->MR4 = cycle * 1/3;
	 PWM1->MR5 = cycle * 1/6;
	 PWM1->MR6 = 0;	
*/ 
	/* all PWM latch enabled    */
	 PWM1->LER = 3;// LER0_EN;
	  PWM1->LER |= LER1_EN | LER2_EN | LER3_EN | LER4_EN | LER5_EN | LER6_EN;
  }

  NVIC_EnableIRQ(PWM1_IRQn);
  return (TRUE);
}

/******************************************************************************
** Function name:		PWM_SetX
**
** Descriptions:		PWM1.x cycle setup
**
** parameters:			Channel number:1~6, PWM cycle °Ù·Ö±È
** Returned value:		None
** 
******************************************************************************/
void PWM_SetX(uint32_t vCh, uint32_t vPre)
{
	switch(vCh)
	{
		case 1:
			PWM1->MR1 = pwm_pclk * vPre / 100;
			PWM1->LER |= LER1_EN;
			break;
		case 2:
			PWM1->MR2 = pwm_pclk * vPre / 100;
			PWM1->LER |= LER2_EN;
			break;
		case 3:
			PWM1->MR3 = pwm_pclk * vPre / 100;
			PWM1->LER |= LER3_EN;
			break;
		case 4:
			PWM1->MR4 = pwm_pclk * vPre / 100;
			PWM1->LER |= LER4_EN;
			break;
		case 5:
			PWM1->MR5 = pwm_pclk * vPre / 100;
			PWM1->LER |= LER5_EN;
			break;
		case 6:
			PWM1->MR6 = pwm_pclk * vPre / 100;
			PWM1->LER |= LER6_EN;
			break;
		default:
			break;
	}
}
/******************************************************************************
** Function name:		PWM_Set
**
** Descriptions:		PWM cycle setup
**
** parameters:			Channel number, PWM cycle, and offset
** Returned value:		None
** 
******************************************************************************/
void PWM_Set(uint32_t cycle)
{
//unsigned int offset = 100;			
  //if ( ChannelNum == 1 )
  {
	// PWM1->MR0 = cycle;		/* set PWM cycle */
	 PWM1->MR1 = pwm_pclk>>1;
	 PWM1->MR2 = pwm_pclk>>2;
	 PWM1->MR3 = pwm_pclk*2/3;
	 PWM1->MR4 = pwm_pclk*5/6;
	 PWM1->MR5 = pwm_pclk*1/3;
	 PWM1->MR6 = pwm_pclk*2/5;
/**/
	/* The LER will be cleared when the Match 0 takes place, in order to
	load and execute the new value of match registers, all the PWMLERs need to
	reloaded. all PWM latch enabled     */
	 PWM1->LER = LER0_EN | LER1_EN | LER2_EN | LER3_EN | LER4_EN | LER5_EN | LER6_EN;
  }
  return;
}

/******************************************************************************
** Function name:		PWM_Start
**
** Descriptions:		Enable PWM by setting the PCR, PTCR registers
**
** parameters:			channel number
** Returned value:		None
** 
******************************************************************************/
void PWM_Start( uint32_t channelNum )
{
  if ( channelNum == 1 )
  {
	/* All single edge, all enable      */
	 PWM1->PCR = PWMENA1 | PWMENA2 | PWMENA3 | PWMENA4 | PWMENA5 | PWMENA6;
	 PWM1->TCR = TCR_CNT_EN | TCR_PWM_EN;	/* counter enable, PWM enable */
  }
  return;
}

/******************************************************************************
** Function name:		PWM_Stop
**
** Descriptions:		Stop all PWM channels
**
** parameters:			channel number
** Returned value:		None
** 
******************************************************************************/
void PWM_Stop( uint32_t channelNum )
{
	if ( channelNum == 1 )
	{
		//	 PWM1->PCR = 0;
		//	 PWM1->TCR = 0x00;		/* Stop all PWMs */
		
		 PWM1->MR0 = gPwmCycle;		/* set PWM cycle */
		 PWM1->MR1 = gPwmCycle;
		 PWM1->MR2 = gPwmCycle;
		 PWM1->MR3 = gPwmCycle;
		 PWM1->MR4 = gPwmCycle;
		 PWM1->MR5 = gPwmCycle;
		 PWM1->MR6 = gPwmCycle;
	
	}
	return;
}

//		PWM_ENA;



/******************************************************************************
**                            End Of File
******************************************************************************/
