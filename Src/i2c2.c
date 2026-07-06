/*****************************************************************************
 *   I2C1.c:  I2C1 C file for NXP LPC17xx Family Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.05.26  ver 1.00    Prelimnary version, first Release
 *
*****************************************************************************/
#include "lpc17xx.h"
#include "type.h"
#include "I2C2.h"

volatile uint32_t I2C1MasterState  = I2C1_IDLE;
volatile uint32_t I2C1SlaveState  = I2C1_IDLE;

volatile uint32_t I2C1Cmd;
volatile uint32_t I2C1Mode;

volatile uint8_t I2C1MasterBuffer[I2C1BUFSIZE];
volatile uint8_t I2C1SlaveBuffer[I2C1BUFSIZE];
volatile uint32_t I2C1Count = 0;
volatile uint32_t I2C1ReadLength;
volatile uint32_t I2C1WriteLength;

volatile uint32_t RdIndex1 = 0;
volatile uint32_t WrIndex1 = 1;

unsigned long  gI2C1ErrS = 0;		// Start Err
unsigned long  gI2C1ErrE = 0;		// Stop Err

unsigned char  gI2C1Stat[32];
unsigned short  gI2C1SLen = 0;
/* 
From device to device, the I2C1 communication protocol may vary, 
in the example below, the protocol uses repeated start to read data from or 
write to the device:
For master read: the sequence is: STA,Addr(W),offset,RE-STA,Addr(r),data...STO 
for master write: the sequence is: STA,Addr(W),length,RE-STA,Addr(w),data...STO
Thus, in state 8, the address is always WRITE. in state 10, the address could 
be READ or WRITE depending on the I2C1Cmd.
*/   

/*****************************************************************************
** Function name:		I2C1_IRQHandler
**
** Descriptions:		I2C1 interrupt handler, deal with master mode
**						only.
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void I2C1_IRQHandler(void)  
{
  uint8_t StatValue;

  /* this handler deals with master read and master write only */
  StatValue = I2C1->I2STAT;
//UART0->THR = StatValue;	
//gI2C1SLen &= 0x1ff;
//if(StatValue != 0xf8)
//gI2C1Stat[gI2C1SLen++] = StatValue;
  switch ( StatValue )
  {
	case 0x08:			/* A Start condition is issued. */
	I2C1->I2DAT = I2C1MasterBuffer[0];
	I2C1->I2CONCLR = (I2C1ONCLR_SIC | I2C1ONCLR_STAC);
	I2C1MasterState = I2C1_STARTED;
	break;
	
	case 0x10:			/* A repeated started is issued */
	if ( I2C1Cmd == RD_BIT )
	{
	  I2C1->I2DAT = I2C1MasterBuffer[0] | RD_BIT;
	}
	I2C1->I2CONCLR = (I2C1ONCLR_SIC | I2C1ONCLR_STAC);
	I2C1MasterState = I2C1_RESTARTED;
	break;
	
	case 0x18:			/* Regardless, it's a ACK */
	if ( I2C1MasterState == I2C1_STARTED )
	{
	  //I2C1->I2DAT = I2C1MasterBuffer[1+WrIndex1];
	  I2C1->I2DAT = I2C1MasterBuffer[WrIndex1];
	  WrIndex1++;
	  I2C1MasterState = DATA_ACK1;
	}
	I2C1->I2CONCLR = I2C1ONCLR_SIC;
	break;

	case 0x28:	/* Data byte has been transmitted, regardless ACK or NACK */
	case 0x30:
	if ( WrIndex1 < I2C1WriteLength )
	{   
	  //I2C1->I2DAT = I2C1MasterBuffer[1+WrIndex1]; /* this should be the last one */
	  if ( WrIndex1 < I2C1WriteLength )
	  {   
		I2C1MasterState = DATA_ACK1;
	  }
	  else
	  {
		I2C1MasterState = DATA_NACK1;
		if ( I2C1ReadLength != 0 )
		{
		  I2C1->I2CONSET = I2C1ONSET_STA;	/* Set Repeated-start flag */
		  I2C1MasterState = I2C1_REPEATED_START;
		  break;
		}
	  }
	  I2C1->I2DAT = I2C1MasterBuffer[WrIndex1]; /* this should be the last one */
	  WrIndex1++;
	}
	else
	{
	  if ( I2C1ReadLength != 0 )
	  {
		I2C1->I2CONSET = I2C1ONSET_STA;	/* Set Repeated-start flag */
		I2C1MasterState = I2C1_REPEATED_START;
	  }
	  else
	  {
		I2C1MasterState = DATA_NACK1;
		I2C1->I2CONSET = I2C1ONSET_STO;      /* Set Stop flag */
	  }
	}
	I2C1->I2CONCLR = I2C1ONCLR_SIC;
	break;
	
	case 0x40:	/* Master Receive, SLA_R has been sent */
	if ( 1 == I2C1ReadLength )
		I2C1->I2CONCLR = I2C1ONSET_AA;	/* assert ACK after data is received */
	else
		I2C1->I2CONSET = I2C1ONSET_AA;	/* assert ACK after data is received */
	I2C1->I2CONCLR = I2C1ONCLR_SIC;
	break;
	
	case 0x50:	/* Data byte has been received, regardless following ACK or NACK */
	case 0x58:	

//	I2C1MasterBuffer[3+RdIndex1] = LPC_I2C1->I2DAT;
	I2C1SlaveBuffer[RdIndex1] = I2C1->I2DAT;
	RdIndex1++;
	if ( RdIndex1 < I2C1ReadLength-1 )
	{   
		I2C1MasterState = DATA_ACK1;
	I2C1->I2CONSET = I2C1ONSET_AA;	/* assert ACK after data is received */
	}
	else if ( RdIndex1 == I2C1ReadLength-1 )
 	{
		I2C1MasterState = DATA_ACK1;
		I2C1->I2CONCLR = I2C1ONSET_AA;	/* assert ACK after data is received */
	}
	else
	{
		RdIndex1 = 0;
		I2C1MasterState = DATA_NACK1;
		I2C1->I2CONCLR = I2C1ONSET_AA;	/* assert ACK after data is received */
	}
	I2C1->I2CONCLR = I2C1ONCLR_SIC;
	break;
	
	case 0x20:		/* regardless, it's a NACK */

	case 0x48:
	I2C1->I2CONCLR = I2C1ONCLR_SIC;
	I2C1MasterState = DATA_NACK1;
	break;
	
	case 0x38:		/* Arbitration lost, in this example, we don't
					deal with multiple master situation */
	default:
	I2C1->I2CONCLR = I2C1ONCLR_SIC;	
	break;
  }
//  if((StatValue != 0xf8)&&(StatValue != 0x40))
//  gI2C1Stat[gI2C1SLen++] = I2C1->I2DAT;

}

/*****************************************************************************
** Function name:		I2C1Start
**
** Descriptions:		Create I2C1 start condition, a timeout
**				value is set if the I2C1 never gets started,
**				and timed out. It's a fatal error. 
**
** parameters:			None
** Returned value:		true or false, return false if timed out
** 
*****************************************************************************/
uint32_t I2C1Start( void )
{
  uint32_t timeout = 0;
  uint32_t retVal = FALSE;
 
  /*--- Issue a start condition ---*/
  I2C1->I2CONSET = I2C1ONSET_STA;	/* Set Start flag */
    
  /*--- Wait until START transmitted ---*/
  while( 1 )
  {
	if ( I2C1MasterState == I2C1_STARTED )
	{
	  retVal = TRUE;
	  break;	
	}
	if ( timeout >= MAX_TIMEOUT )
	{
	  retVal = FALSE;
	  break;
	}
	timeout++;
  }
	timeout++;
  return( retVal );
}

/*****************************************************************************
** Function name:		I2C1Stop
**
** Descriptions:		Set the I2C1 stop condition, if the routine
**				never exit, it's a fatal bus error.
**
** parameters:			None
** Returned value:		true or never return
** 
*****************************************************************************/
uint32_t I2C1Stop( void )
{
  I2C1->I2CONSET = I2C1ONSET_STO;  /* Set Stop flag */ 
  I2C1->I2CONCLR = I2C1ONCLR_SIC;  /* Clear SI flag */ 
            
  /*--- Wait for STOP detected ---*/
  while( I2C1->I2CONSET & I2C1ONSET_STO );
  return TRUE;
}

/*****************************************************************************
** Function name:		I2C1Init
**
** Descriptions:		Initialize I2C1 controller
**
** parameters:			I2C1 mode is either MASTER or SLAVE
** Returned value:		true or false, return false if the I2C1
**				interrupt handler was not installed correctly
** 
*****************************************************************************/
uint32_t I2C1Init( uint32_t vI2C1Mode ) 
{
  //SC->PCONP |= (1 << 7);	  // I2C0
  SC->PCONP |= (1 << 19);	  // I2C1

  /* set PIO0.19 and PIO0.20 to I2C1 SDA and SCK */
  /* function to 03 on both SDA and SCK. */
  PINCON->PINSEL1 &= ~0x000003c0;
  PINCON->PINSEL1 |= 0x000003c0;	
  PINCON->PINMODE1 &= ~0x000003c0;	   // NO pull_Up, NO pull_Down
  PINCON->PINMODE1 |= 0x00000280;	
  PINCON->PINMODE_OD0 |= 0x00180000;	   // P0.19/20 as Open drain mode
 
  /*--- Clear flags ---*/
  I2C1->I2CONCLR = I2C1ONCLR_AAC | I2C1ONCLR_SIC | I2C1ONCLR_STAC | I2C1ONCLR_I2ENC;    

  /*--- Reset registers ---*/
  I2C1->I2SCLL   = I2SCLL_SCLL;
  I2C1->I2SCLH   = I2SCLH_SCLH;
  if ( vI2C1Mode == I2C1SLAVE )
  {
	;//I2C1->I2ADR0 = AT24_ADDR;
  }    

  /* Install interrupt handler */
  NVIC_EnableIRQ(I2C1_IRQn);

  I2C1->I2CONSET = I2C1ONSET_I2EN;
  return( TRUE );
}

/*****************************************************************************
** Function name:		I2C1Engine
**
** Descriptions:		The routine to complete a I2C1 transaction
**				from start to stop. All the intermitten
**				steps are handled in the interrupt handler.
**				Before this routine is called, the read
**				length, write length, I2C1 master buffer,
**				and I2C1 command fields need to be filled.
**				see I2C1mst.c for more details. 
**
** parameters:			None
** Returned value:		true or false, return false only if the
**				start condition can never be generated and
**				timed out. 
** 
*****************************************************************************/
uint32_t I2C1Engine( void ) 
{
//	unsigned long I2C1Dly = 0xfffffff;
  I2C1MasterState = I2C1_IDLE;
  RdIndex1 = 0;
  WrIndex1 = 1;
  if ( I2C1Start() != TRUE )
  {
	I2C1Stop();
	//gI2C1ErrS++;
	return ( FALSE );
  }

  while ( 1 )
  {
	if ( I2C1MasterState == DATA_NACK1 )
	{
	  I2C1Stop();
	//	gI2C1ErrE++;
	  break;
	}
/*	I2C1Dly--;
	if(I2C1Dly==0)
	{
	//	I2C1Stop();
		break;
	}  */
  }    
  return ( TRUE );      
}

/******************************************************************************
**                            End Of File
******************************************************************************/

