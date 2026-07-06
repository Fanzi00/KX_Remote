/*****************************************************************************
 *   pwm.h:  Header file for NXP LPC17xx Family Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.5.25  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __PWM_H 
#define __PWM_H



#define PWM_CLK			(4000000UL/4)		// PWM 周期，	IRC_OSC = 4000000UL
#define PWM_CYCLE		165					// 电磁阀的频率 150~180
#define PWM_OFFSET		200

#define MR0_INT			(1 << 0	)
#define MR1_INT			(1 << 1	)
#define MR2_INT			(1 << 2	)
#define MR3_INT			(1 << 3	)
#define MR4_INT			(1 << 8	)
#define MR5_INT			(1 << 9	)
#define MR6_INT			(1 << 10)

#define TCR_CNT_EN		0x00000001
#define TCR_RESET		0x00000002
#define TCR_PWM_EN		0x00000008

#define PWMMR0I			(1 << 0	)
#define PWMMR0R			(1 << 1	)
#define PWMMR0S			(1 << 2	)
#define PWMMR1I			(1 << 3	)
#define PWMMR1R			(1 << 4	)
#define PWMMR1S			(1 << 5	)
#define PWMMR2I			(1 << 6	)
#define PWMMR2R			(1 << 7	)
#define PWMMR2S			(1 << 8	)
#define PWMMR3I			(1 << 9	)
#define PWMMR3R			(1 << 10 )
#define PWMMR3S			(1 << 11 )
#define PWMMR4I			(1 << 12 )
#define PWMMR4R			(1 << 13 )
#define PWMMR4S			(1 << 14 )
#define PWMMR5I			(1 << 15 )
#define PWMMR5R			(1 << 16 )
#define PWMMR5S			(1 << 17 )
#define PWMMR6I			(1 << 18 )
#define PWMMR6R			(1 << 19 )
#define PWMMR6S			(1 << 20 )

#define PWMSEL2			(1 << 2	)
#define PWMSEL3			(1 << 3	)
#define PWMSEL4			(1 << 4	)
#define PWMSEL5			(1 << 5	)
#define PWMSEL6			(1 << 6	)
#define PWMENA1			(1 << 9	)
#define PWMENA2			(1 << 10 )
#define PWMENA3			(1 << 11 )
#define PWMENA4			(1 << 12 )
#define PWMENA5			(1 << 13 )
#define PWMENA6			(1 << 14 )

#define LER0_EN			(1 << 0	)
#define LER1_EN			(1 << 1	)
#define LER2_EN			(1 << 2	)
#define LER3_EN			(1 << 3	)
#define LER4_EN			(1 << 4	)
#define LER5_EN			(1 << 5	)
#define LER6_EN			(1 << 6	)

#if 1

//// PWMx Set cycle and Enable Output
#define PWM1_SET(cycle)	{ PWM1->MR1 = cycle; PWM1->LER |= LER1_EN; }
#define PWM2_SET(cycle)	{ PWM1->MR2 = cycle; PWM1->LER |= LER2_EN; }
#define PWM3_SET(cycle)	{ PWM1->MR3 = cycle; PWM1->LER |= LER3_EN; }
#define PWM4_SET(cycle)	{ PWM1->MR4 = cycle; PWM1->LER |= LER4_EN; }
#define PWM5_SET(cycle)	{ PWM1->MR5 = cycle; PWM1->LER |= LER5_EN; }
#define PWM6_SET(cycle)	{ PWM1->MR6 = cycle; PWM1->LER |= LER6_EN; }

#define PWM1_EN		PWM1->PCR |= PWMENA1;
#define PWM2_EN		PWM1->PCR |= PWMENA2;
#define PWM3_EN		PWM1->PCR |= PWMENA3;
#define PWM4_EN		PWM1->PCR |= PWMENA4;
#define PWM5_EN		PWM1->PCR |= PWMENA5;
#define PWM6_EN		PWM1->PCR |= PWMENA6;

#define PWM1_DIS		PWM1->PCR &= ~PWMENA1;
#define PWM2_DIS		PWM1->PCR &= ~PWMENA2;
#define PWM3_DIS		PWM1->PCR &= ~PWMENA3;
#define PWM4_DIS		PWM1->PCR &= ~PWMENA4;
#define PWM5_DIS		PWM1->PCR &= ~PWMENA5;
#define PWM6_DIS		PWM1->PCR &= ~PWMENA6;

//// PWMx Output disable, GPIO disable
#define PWM1_OFF	{ PWM1->PCR &= ~PWMENA1; PINCON->PINSEL4 &= ~(0x03<<0); PINCON->PINSEL3 &= ~(0x03<<4);  }
#define PWM2_OFF	{ PWM1->PCR &= ~PWMENA2; PINCON->PINSEL4 &= ~(0x03<<2); PINCON->PINSEL3 &= ~(0x03<<8);  }
#define PWM3_OFF	{ PWM1->PCR &= ~PWMENA3; PINCON->PINSEL4 &= ~(0x03<<4); PINCON->PINSEL3 &= ~(0x03<<10);  }
#define PWM4_OFF	{ PWM1->PCR &= ~PWMENA4; PINCON->PINSEL4 &= ~(0x03<<6); PINCON->PINSEL3 &= ~(0x03<<14);  }
#define PWM5_OFF	{ PWM1->PCR &= ~PWMENA5; PINCON->PINSEL4 &= ~(0x03<<8); PINCON->PINSEL3 &= ~(0x03<<16);  }
#define PWM6_OFF	{ PWM1->PCR &= ~PWMENA6; PINCON->PINSEL4 &= ~(0x03<<10); PINCON->PINSEL3 &= ~(0x03<<20); }
//// PWMx A/B Pin enable
#define PWM1_A	{ PINCON->PINSEL4 |= (0x01<<0); }	  // P2.0
#define PWM1_B	{ PINCON->PINSEL3 |= (0x02<<4); }	  // P1.18
#define PWM2_A	{ PINCON->PINSEL4 |= (0x01<<2); }	  // P2.1
#define PWM2_B	{ PINCON->PINSEL3 |= (0x02<<8); }	  // P1.20
#define PWM3_A	{ PINCON->PINSEL4 |= (0x01<<4); }	  // P2.2
#define PWM3_B	{ PINCON->PINSEL3 |= (0x02<<10); }	  // P1.21
#define PWM4_A	{ PINCON->PINSEL4 |= (0x01<<6); }	  // P2.3
#define PWM4_B	{ PINCON->PINSEL3 |= (0x02<<14); }	  // P1.23
#define PWM5_A	{ PINCON->PINSEL4 |= (0x01<<8); }	  // P2.4
#define PWM5_B	{ PINCON->PINSEL3 |= (0x02<<16); }	  // P1.24
#define PWM6_A	{ PINCON->PINSEL4 |= (0x01<<10); }	  // P2.5
#define PWM6_B	{ PINCON->PINSEL3 |= (0x02<<20); }	  // P1.26

#endif


#define PWM_ENA		PWM1->TCR = TCR_CNT_EN | TCR_PWM_EN;	/* counter enable, PWM enable */



extern uint32_t PWM_Init( uint32_t channelNum, uint32_t cycle );
//extern void PWM_Set( uint32_t channelNum, uint32_t cycle, uint32_t offset );
extern void PWM_Set(uint32_t cycle); /* PWM cycle setup  */
extern void PWM_Start( uint32_t channelNum );  /* All single edge, all enable */
extern void PWM_Stop( uint32_t channelNum );   /* Stop all PWM channels		  */
extern void PWM_SetX(uint32_t vCh, uint32_t vPre);





extern unsigned int  gPwmCycle;



#endif /* end __PWM_H */
/****************************************************************************
**                            End Of File
****************************************************************************/
