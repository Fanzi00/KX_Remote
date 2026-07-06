/*****************************************************************************
 *   I2C1.h:  Header file for NXP LPC17xx Family Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.05.26  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __I2C1_H 
#define __I2C1_H

#define I2C1BUFSIZE			0x80
#define MAX_TIMEOUT		0x000FFFFF

#define I2C1MASTER		0x01
#define I2C1SLAVE		0x02

/* For more info, read Philips's LM95 datasheet 
#define LM75_ADDR		0x90
#define LM75_TEMP		0x00
#define LM75_CONFIG		0x01
#define LM75_THYST		0x02
#define LM75_TOS		0x03
											  */


#define RD_BIT			0x01

#define I2C1_IDLE			0
#define I2C1_STARTED			1
#define I2C1_RESTARTED		2
#define I2C1_REPEATED_START	3
#define DATA_ACK1			4
#define DATA_NACK1			5

#define I2C1ONSET_I2EN		0x00000040  /* I2C1 Control Set Register */
#define I2C1ONSET_AA			0x00000004
#define I2C1ONSET_SI			0x00000008
#define I2C1ONSET_STO		0x00000010
#define I2C1ONSET_STA		0x00000020

#define I2C1ONCLR_AAC		0x00000004  /* I2C1 Control clear Register */
#define I2C1ONCLR_SIC		0x00000008
#define I2C1ONCLR_STAC		0x00000020
#define I2C1ONCLR_I2ENC		0x00000040

#define I2DAT_I2C1			0x00000000  /* I2C1 Data Reg */
#define I2ADR_I2C1			0x00000000  /* I2C1 Slave Address Reg */
#define I2SCLH_SCLH			0x00000078  /* I2C1 SCL Duty Cycle High Reg   0x00000080 */
#define I2SCLL_SCLL			0x00000078  /* I2C1 SCL Duty Cycle Low Reg   0x00000080*/



extern unsigned char   gI2C1Stat[32];
extern unsigned short  gI2C1SLen;

extern unsigned long  gI2C1ErrS;		// Start Err
extern unsigned long  gI2C1ErrE;		// Stop Err


extern volatile uint32_t I2C1MasterState;
extern volatile uint32_t I2C1SlaveState;

extern volatile uint32_t I2C1Cmd;
extern volatile uint32_t I2C1Mode;

extern volatile uint8_t I2C1MasterBuffer[I2C1BUFSIZE];
extern volatile uint8_t I2C1SlaveBuffer[I2C1BUFSIZE];
extern volatile uint32_t I2C1Count ;
extern volatile uint32_t I2C1ReadLength;
extern volatile uint32_t I2C1WriteLength;

extern volatile uint32_t RdIndex ;
extern volatile uint32_t WrIndex ;


extern void I2C1_IRQHandler( void );
extern uint32_t I2C1Init( uint32_t I2C1Mode );
extern uint32_t I2C1Start( void );
extern uint32_t I2C1Stop( void );
extern uint32_t I2C1Engine( void );

#endif /* end __I2C1_H */
/****************************************************************************
**                            End Of File
*****************************************************************************/
