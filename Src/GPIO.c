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


unsigned long  gDIValue = 0;

/*****************************************************************************
** Function name:		GPIO_Init
**
** Descriptions:		GPIO_Init, Init DI, DO
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/

uint32_t GPIOInit( void )
{
	GPIO_TypeDef * pGPIO = NULL;

	//// WP
	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*2)); 		// P2.11
	pGPIO->FIODIR |= (0x01 << IO_WP);
	WP_L;

	//// WP2
	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*2)); 		// P2.12
	pGPIO->FIODIR |= (0x01 << IO_WP2);
	WP2_L;
		
	//// WDI 
	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*1)); 		// P1.4
	pGPIO->FIODIR |= (0x01 << IO_WDI);
	WDT_Feed();

	//// IO_MD0   AS61-MD0
	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*0)); 		// P0.4
	pGPIO->FIODIR |= (0x01 << IO_MD0);
	pGPIO->FIOCLR |= (0x01 << IO_MD0);

//// IO_MD1  AS61-MD1 
	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*1)); 		// P1.1
	pGPIO->FIODIR |= (0x01 << IO_MD1);
	pGPIO->FIOCLR |= (0x01 << IO_MD1);

	//// IO_LC1 
	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*1)); 		// P1.9
	pGPIO->FIODIR |= (0x01 << IO_LC1);

	//// IO_LC2 
	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*1)); 		// P1.8
	pGPIO->FIODIR |= (0x01 << IO_LC2);

	//// IO_BP 
	pGPIO = (GPIO_TypeDef *)((GPIO_BASE + 0x00020*1)); 		// P1.20
	pGPIO->FIODIR |= (0x01 << _DO_BEEP);
	//BEEP_ON;

	WDT_Feed();
	return( TRUE );
}


/*****************************************************************************
** Function name:		WDI_Init
**
** Descriptions:		GPIO_Init, Init WDI OutPin
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void WDT_Init(void)
{
	 GPIO1->FIODIR |= (0x01 << 4);	   		// P1.22, WDI OutPin
	 GPIO1->FIOCLR |= (0x01 << 4);
}


/*****************************************************************************
** Function name:		WDI_Feed
**
** Descriptions:		Feed WDT 
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void WDT_Feed(void)
{
	static unsigned char stWdi = 0;

		GPIO1->FIODIR |= (0x01 << IO_WDI);
	if(stWdi == 0)
	{
		stWdi = 1;
		GPIO1->FIOCLR |= (0x01 << IO_WDI);
	}
	else
	{
		stWdi = 0;
		GPIO1->FIOSET |= (0x01 << IO_WDI);
	}
}


/*****************************************************************************
** Function name:		WDI_Feed
**
** Descriptions:		Feed WDT 
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void TST_Feed(void)
{
	if(GPIO0->FIOPIN & (0x01 << 27))
		GPIO0->FIOCLR |= (0x01 << 27);
	else
		GPIO0->FIOSET |= (0x01 << 27);
}
void Beep_Turn(void)
{
	static unsigned char stBeep = 0;
	if(stBeep ==0)
	{
		stBeep = 1;
		GPIO1->FIOCLR |= (0x01 << IO_FMQ);
		}
	else
	{
		stBeep = 0;
		GPIO1->FIOSET |= (0x01 << IO_FMQ);
		}
	gDOBitV ^= (1<<1);
}

/******************************************************************************
**                            End Of File
******************************************************************************/
