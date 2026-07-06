/*****************************************************************************
 *   adc.h:  Header file for NXP LPC17xx Family Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.05.25  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __ADC_H 
#define __ADC_H


#define ADC_BESSEL		0		// Bessel

/* If Burst mode is enabled, make sure interrupt flag is set. */
#define ADC_INTERRUPT_FLAG	1	/* 1 is interrupt driven, 0 is polling */
#define BURST_MODE			0   /* Burst mode works in interrupt driven mode only. */

#define ADC_OFFSET		0x10
#define ADC_INDEX		4

#define ADC_DONE		0x80000000
#define ADC_OVERRUN		0x40000000
#define ADC_ADINT		0x00010000

#define ADC_NUM			18		/* for LPCxxxx */						// Ver3  20180629  
//#define ADC_CLK			1000000		/* set to 1Mhz */

#define ADC_CLK			1000000 		/* set to 88khz, x/8/65 Hz/Chl */


////  Bessel 滤波参数定义

/*
//0.02
#define _IQ_A0		0.005587102
#define _IQ_A1		0.011174203
#define _IQ_A2		0.005587102
#define _IQ_B1		1.735662879
#define _IQ_B2		-0.758011286
//0.01
#define _IQ_A0		0.001491842
#define _IQ_A1		0.002983684
#define _IQ_A2		0.001491842
#define _IQ_B1		1.864734364
#define _IQ_B2		-0.870701733
*/
//0.05
#define _IQ_A0		0.02902242
#define _IQ_A1		0.05804484
#define _IQ_A2		0.02902242
#define _IQ_B1		1.382979163
#define _IQ_B2		-0.499068843

/*
//0.1
#define _IQ_A0		0.088510744
#define _IQ_A1		0.177021488
#define _IQ_A2		0.088510744
#define _IQ_B1		0.892359002
#define _IQ_B2		-0.246401978

*/

#define _A0			(_IQ_A0)
#define _A1			(_IQ_A1)
#define _A2			(_IQ_A2)
#define _B1			(_IQ_B1)
#define _B2			(_IQ_B2)


#define  HC4052_XY0		{LC1_L; LC2_L;}
#define  HC4052_XY1		{LC1_L; LC2_H;}
#define  HC4052_XY2		{LC1_H; LC2_L;}
#define  HC4052_XY3		{LC1_H; LC2_H;}


extern 	unsigned int   ADCBsl[ADC_NUM];
extern 	unsigned int   ADCRst[ADC_NUM];
extern  unsigned int   ADCRstP[ADC_NUM];			// ADC 的平均值
extern 	 unsigned int  gADValue;
extern 	 unsigned int   gADChl;
extern 	 unsigned int  gADConvFlg;		// AD 采样完成的标记


extern void ADC_StartCnv (void);
void ADC_StopCnv (void);

extern void ADC_Bessel(unsigned int chn, unsigned int vADvalue);
extern unsigned long ADCInit( unsigned long ADC_Clk );
extern unsigned long ADC0Read( unsigned char channelNum );
extern void ADCBurstRead( void );
extern void AD_Start_Chl(unsigned char vChl);
extern unsigned int AD_Bessel(unsigned char chn, unsigned int ADValue);

#endif /* end __ADC_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
