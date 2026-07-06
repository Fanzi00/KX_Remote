/*****************************************************************************
 *   wdt.h:  Header file for NXP LPC214x Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.09.01  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __GPIO_H 
#define __GPIO_H




////	DI (2)
#define  _DJ_DI			15		// P1.15	µçÔ´µçÑ¹¼ì²â½Å
#define  GET_DJDI		(GPIO1->FIOPIN & (0x01 << _DJ_DI)) == (0x01 << _DJ_DI)? 1:0



//// 	IO_ÍâÎ§Æ÷¼þ
#define  IO_WP			11		// p2.11 24C04 WP
#define  IO_WP2			12		// p2.12 24C04 WP
#define  IO_WDI			4		// P1.4	 WDI

#define  IO_MD0			4		// P0.4	 AS61-MD0
#define  IO_MD1			1		// P1.1	 AS61-MD1
#define  IO_AUX			0		// P1.0	 AS61-AUX
#define  IO_LC1			9		// P1.9	 AD Select Ctrl_1
#define  IO_LC2			8		// P1.8	 AD Select Ctrl_2

//  ·äÃùÆ÷
#define  IO_FMQ			20	// P1.20 ·äÃùÆ÷
//#define  DO_BEEP			20			// P1.20£¬ÄÚ²¿·äÃùÆ÷
#define  DO_BEEP_H		GPIO1->FIOSET |= (0x01 << IO_FMQ);
#define  DO_BEEP_L		GPIO1->FIOCLR |= (0x01 << IO_FMQ);

#define  IO_LaBa			4	// P1.20 ·äÃùÆ÷
//#define  DO_BEEP			20			// P1.20£¬ÄÚ²¿·äÃùÆ÷
#define  DO_LABA_H		GPIO2->FIOSET |= (0x01 << IO_LaBa);
#define  DO_LABA_L		GPIO2->FIOCLR |= (0x01 << IO_LaBa);

////	IO_Ctrl
#define  WP_H			GPIO2->FIOSET |= (0x01 << IO_WP);
#define  WP_L			GPIO2->FIOCLR |= (0x01 << IO_WP);
#define  WP2_H			GPIO2->FIOSET |= (0x01 << IO_WP2);
#define  WP2_L			GPIO2->FIOCLR |= (0x01 << IO_WP2);
#define  LC1_H			GPIO1->FIOSET |= (0x01 << IO_LC1);
#define  LC1_L			GPIO1->FIOCLR |= (0x01 << IO_LC1);
#define  LC2_H			GPIO1->FIOSET |= (0x01 << IO_LC2);
#define  LC2_L			GPIO1->FIOCLR |= (0x01 << IO_LC2);
#define  MD0_H			GPIO0->FIOSET |= (0x01 << IO_MD0);
#define  MD0_L			GPIO0->FIOCLR |= (0x01 << IO_MD0);
#define  MD1_H			GPIO1->FIOSET |= (0x01 << IO_MD1);
#define  MD1_L			GPIO1->FIOCLR |= (0x01 << IO_MD1);


////	DO (12 + 5)
//  Êä³ö¸ßÓÐÐ§
#define  _DO_BEEP		20		// p1.20	ÑïÉùÆ÷Êä³ö½Å
////	SET DO
#define  BEEP_ON		GPIO1->FIOSET |= (0x01 << _DO_BEEP)  	// SET
#define  BEEP_OFF		GPIO1->FIOCLR |= (0x01 << _DO_BEEP)  	// SET
//#define  BEEP_TURN		GPIO1->FIOPIN ^= (0x01 << _DO_BEEP)

#define  PIN_TURN_TST	GPIO0->FIOPIN ^= (0x01 << 27)



 
extern uint32_t GPIOInit( void );
extern uint32_t Get_DI(void);
extern void WDT_Init(void);
extern void WDT_Feed(void);

extern void TST_Feed(void);
void Beep_Turn(void);


#endif /* end __GPIO_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
