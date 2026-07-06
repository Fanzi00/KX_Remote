/*****************************************************************************
 *   adc.c:  ADC module file for NXP LPC17xx Family Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.05.25  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#include "lpc17xx.h"
#include "config.h"
#include "adc.h"

volatile uint32_t ADC0Value[ADC_NUM];
volatile uint32_t ADC0IntDone = 0;
volatile unsigned  stAdChn = 0;


// Bessel
unsigned int  gX0[ADC_NUM];
unsigned int  gX1[ADC_NUM];
unsigned int  gX2[ADC_NUM];
unsigned int  gY1[ADC_NUM];
unsigned int  gY2[ADC_NUM];
float   gADFValue[ADC_NUM];
float gA = 1.0;		//0.9911;
float gB = 0.0;		//0.00552;unsigned int   ADCRst[ADC_NUM];
unsigned int   ADCBsl[ADC_NUM];
unsigned int   ADCRst[ADC_NUM];
unsigned int   ADCRstP[ADC_NUM];			// ADC 的平均值

unsigned char  gXYSel = 0;
unsigned int   gADConvFlg = 0;		// AD 采样完成的标记


#if BURST_MODE
volatile uint32_t channel_flag; 
#endif

#if ADC_INTERRUPT_FLAG
/******************************************************************************
** Function name:		ADC_IRQHandler
**
** Descriptions:		ADC interrupt handler
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void ADC_IRQHandler (void)  
{
	static char CnvDlys = 0;
	static char CnvNum = 0;
	static unsigned char stFlg = 1;
	uint32_t regVal;
	unsigned char i = 0;  

  //stAdChn = (ADC -> ADGDR>>24)&0x07;
	regVal = ADC -> ADSTAT;		/* Read ADC will clear the interrupt */
	ADC -> ADCR &= 0xFFFFFF00;
	ADC -> ADCR |= (1 << (stAdChn));		//(1 << 24)
	if ( regVal & 0x0000FF00 )	/* check OVERRUN error first */
	{
		regVal = (regVal & 0x0000FF00) >> 0x08;
	/* if overrun, just read ADDR to clear */
	/* regVal variable has been reused. */
 #if 1
	switch ( regVal )
	{
		case 0x01:
			regVal = ADC -> ADDR0;
			break;
		case 0x02:
			regVal = ADC -> ADDR1;
			break;
		case 0x04:
			regVal = ADC -> ADDR2;
			break;
		case 0x08:
			regVal = ADC -> ADDR3;
			break;
		
		default:
			break;
	}
#else
	  regVal = ADC -> ADGDR;
#endif
	return;
  }
    
	if ( regVal & ADC_ADINT )
	{
	#if 1
		switch ( regVal & 0xFF )	/* check DONE bit */
		{
			case 0x01:
				CnvDlys++;
				if(stFlg)
					ADC0Value[gXYSel*4+0] += ( ADC -> ADDR0 >> 4 ) & 0xFFF;
				else
					regVal = ADC -> ADDR0;
				break;
			case 0x02:
				if(stFlg)
					ADC0Value[gXYSel*4+1] += ( ADC -> ADDR1 >> 4 ) & 0xFFF;
				else
					regVal = ADC -> ADDR1;
				break;
			case 0x04:
				if(stFlg)
					ADC0Value[gXYSel*4+2] += ( ADC -> ADDR2 >> 4 ) & 0xFFF;
				else
					regVal = ADC -> ADDR2;
				break;
			case 0x08:
				if(stFlg)
				{
					ADC0Value[gXYSel*4+3] += ( ADC -> ADDR3 >> 4 ) & 0xFFF;
					gXYSel++;
					if(gXYSel >= 4)			// gXYSel = 0/1/2/3
					{
						CnvNum++;
						gXYSel = 0;
					}
					if(gXYSel == 0)
						HC4052_XY0
					if(gXYSel == 1)
						HC4052_XY1
					if(gXYSel == 2)
						HC4052_XY2
					if(gXYSel == 3)
						HC4052_XY3
					stFlg = 0;
					stFlg = 0;
					stFlg = 0;
					stFlg = 0;
				}
				else
				{
					regVal = ADC -> ADDR3;
					stFlg = 1;
				}
				break;
			default:
			break;
		}
		#else
		   ADC0Value[stAdChn] += (ADC -> ADGDR>>4) & 0xFFF;
		   if(stAdChn == 3)
			CnvNum++;
		#endif
  	}
	if(CnvNum >= 8)	   			// 16次取平均
	{
		for(i=0; i<ADC_NUM; i++)
		{
			ADCRstP[i] = (ADC0Value[i] >> 3);
			ADC0Value[i] = 0;
		//	ADCRst[i] = ADCRstP[i] ;
		}
		CnvNum = 0;
		gADConvFlg++;
		WDT_Feed();
	}
//  ADC -> ADCR &= 0xF8FFFFFF;	/* stop ADC now */ 
//  ADC0IntDone = 1;
	ADC -> ADCR |= 0x01000000 ;
	stAdChn++;
	stAdChn &= 0x03;
}
#endif

/*****************************************************************************
** Function name:		ADCInit
**
** Descriptions:		initialize ADC channel
**
** parameters:			ADC clock rate
** Returned value:		true or false
** 
*****************************************************************************/
unsigned long ADCInit( unsigned long vADC_Clk )
{
	unsigned long pclkdiv, pclk;

	stAdChn = 1;
	
	/* Enable CLOCK into ADC controller */
	SC->PCONP |= (1 << 12);

	/*  the related pins are set to ADC inputs, AD0.0~3--p0.23/24/25/26 */

	PINCON->PINSEL1 &= ~0x003fC000;	/* P0.23~26, A0.0~3, function 01 */
	PINCON->PINSEL1 |=  0x00154000;
	HC4052_XY0;

	/* By default, the PCLKSELx value is zero, thus, the PCLK for
	all the peripherals is 1/4 of the SystemFrequency. */
	/* Bit 24~25 is for UART0 */
	pclkdiv = (SC->PCLKSEL0 >> 24) & 0x03;
	switch ( pclkdiv )
	{
		case 0x00:
		default:
		  pclk = SystemFrequency/4;
		break;
		case 0x01:
		  pclk = SystemFrequency;
		break; 
		case 0x02:
		  pclk = SystemFrequency/2;
		break; 
		case 0x03:
		  pclk = SystemFrequency/8;
		break;
	}

	ADC->ADCR = ( 0x001 << 0 ) | 	/* SEL=0xff,select channel 0~7 on ADC0~7 */
		( ( pclk  / vADC_Clk - 1 ) << 8 ) |  /* CLKDIV = Fpclk / ADC_Clk - 1 */ 
		( 0 << 16 ) | 		/* BURST = 1, BURST, hardware controlled */
		( 0 << 17 ) |  		/* CLKS = 0, 11 clocks/10 bits */
		( 1 << 21 ) |  		/* PDN = 1, normal operation */
		( 0 << 24 );  		/* START = 0 A/D conversion stops */
	//	( 0 << 27 );		/* EDGE = 0 (CAP/MAT singal falling,trigger A/D conversion) */ 

  /* If POLLING, no need to do the following */
#if ADC_INTERRUPT_FLAG
	ADC->ADINTEN = 0x10f;		/* Enable all interrupts */
	NVIC_EnableIRQ(ADC_IRQn);
#endif

  //ADC->ADCR |=  (1<<24);                    

  return (TRUE);
}

void ADC_StartCnv (void) {
  ADC->ADCR &= ~(7<<24);                    
  ADC->ADCR |=  (1<<24);                    
}
void ADC_StopCnv (void) {
  ADC->ADCR &= ~(7<<24);     
  }
/*****************************************************************************
** Function name:		ADC0Read
**
** Descriptions:		Read ADC0 channel
**
** parameters:			Channel number
** Returned value:		Value read, if interrupt driven, return channel #
** 
*****************************************************************************/
unsigned long ADC0Read( unsigned char channelNum )
{
#if !ADC_INTERRUPT_FLAG
  unsigned long regVal, ADC_Data;
#endif

  /* channel number is 0 through 7 */
  if ( channelNum >= ADC_NUM )
  {
	channelNum = 0;		/* reset channel number to 0 */
  }
  ADC->ADCR &= 0xFFFFFF00;
  ADC->ADCR |= (1 << 24) | (1 << channelNum);	
				/* switch channel,start A/D convert */
#if !ADC_INTERRUPT_FLAG
  while ( 1 )			/* wait until end of A/D convert */
  {
	regVal = *(volatile unsigned long *)(ADC_BASE 
			+ ADC_OFFSET + ADC_INDEX * channelNum);
	/* read result of A/D conversion */
	if ( regVal & ADC_DONE )
	{
	  break;
	}
  }	
        
  ADC->ADCR &= 0xF8FFFFFF;	/* stop ADC now */    
  if ( regVal & ADC_OVERRUN )	/* save data when it's not overrun, otherwise, return zero */
  {
	return ( 0 );
  }
  ADC_Data = ( regVal >> 4 ) & 0xFFF;
  return ( ADC_Data );	/* return A/D conversion value */
#else
  return ( channelNum );	/* if it's interrupt driven, the ADC reading is 
							done inside the handler. so, return channel number */
#endif
}

/*****************************************************************************
** Function name:		ADC0BurstRead
**
** Descriptions:		Use burst mode to convert multiple channels once.
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void ADCBurstRead( void )
{
  if ( ADC->ADCR & (0x7<<24) )
  {
	ADC->ADCR &= ~(0x7<<24);
  }
  /* Test channel 5,6,7 using burst mode because they are not shared
  with the JTAG pins. */
  ADC->ADCR &= ~0xFF;
  /* Read all channels, 0 through 7. */
  ADC->ADCR |= (0xFF);
  ADC->ADCR |= (0x1<<16);	/* Set burst mode and start A/D convert */
  return;						/* the ADC reading is done inside the 
								handler, return 0. */
}



unsigned int AD_Bessel(unsigned char chn, unsigned int ADValue)
{
	unsigned char i = 0;
	static unsigned char stInit = 0;
	static unsigned char InitFlg[ADC_NUM] ;
		static unsigned short ratio0[ADC_NUM] ;

	if(stInit == 0)
	{
		for(i=0; i<ADC_NUM; i++)
		{
			InitFlg[i] = 20;
			ratio0[i] = 0;
		}
		stInit = 1;
	}
	ratio0[chn] = ADValue;	

	if(InitFlg[chn])
	{
		if(InitFlg[chn] == 1)
			gX2[chn] = gX1[chn] = gX0[chn] = gY1[chn] = gY2[chn] =  ADValue;
		InitFlg[chn]--;
		return ADValue;
	}
		gX2[chn] = gX1[chn];
		gX1[chn] = gX0[chn];
		gX0[chn] = ADValue;
		gADFValue[chn] = _IQ_A0 * gX0[chn];
		gADFValue[chn] += _IQ_A1 * gX1[chn];
		gADFValue[chn] += _IQ_A2 * gX2[chn];
		gADFValue[chn] += _IQ_B1 * gY1[chn];
		gADFValue[chn] += _IQ_B2 * gY2[chn];
		gY2[chn] = gY1[chn];
		gY1[chn] = (unsigned int)gADFValue[chn];
		return gY1[chn];
}
/*********************************************************************************
**                            End Of File
*********************************************************************************/
