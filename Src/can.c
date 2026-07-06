/*****************************************************************************
 *  can.c:  CAN module API file for NXP LPC17xx Family Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.05.27  ver 1.00    Prelimnary version, first Release
 *
*****************************************************************************/
#include <lpc17xx.h>
#include "config.h"
#include "can.h"

// Receive Queue: one queue for each CAN port
extern CAN_MSG MsgBuf_RX1[ID_RCV_NUM], MsgBuf_RX2[ID_RCV_NUM];
extern volatile uint32_t CAN1RxDone, CAN2RxDone;




unsigned int  CAN_BPS[3] = {BITRATE125K24MHZ, BITRATE250K24MHZ, BITRATE500K24MHZ};

volatile uint32_t CANStatus;

unsigned int  gCanRIDSite  = 0; 

uint32_t CAN1RxCount = 0, CAN2RxCount = 0;
uint32_t CAN1ErrCount = 0, CAN2ErrCount = 0;

unsigned char  gCANReInitFlg = 0;

unsigned char  gCan2InitFlg = 0;

unsigned int gCanSta0 = 0;
unsigned int gCanSta1 = 0;
unsigned int gCanSta0_2 = 0;
unsigned int gCanSta1_2 = 0;

/******************************************************************************
** Function name:		CAN_ISR_Rx1
**
** Descriptions:		CAN Rx1 interrupt handler，接收信息在 MsgBuf_RX1 内
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void CAN_ISR_Rx1( void )
{
  uint32_t * pDest;
  unsigned char vSite = 0;

  // initialize destination pointer
	vSite = Can_ID_Chk(0, CAN1->RID);
	if(0xff == vSite)
	{
		CAN1 -> CMR = 0x04; // release receive buffer
		return;
	}
  
	pDest = (uint32_t *)&MsgBuf_RX1[vSite];
	*pDest = CAN1 -> RFS;  // Frame
	
	pDest++;
	*pDest = CAN1 -> RID; // ID		//change by gongjun
	
	pDest++;
	*pDest = CAN1 -> RDA; // Data A
	
	pDest++;
	*pDest = CAN1 -> RDB; // Data B

	pDest++;
	*pDest = gRcvCanID[1][vSite]; // Over Time
	MsgBuf_RX1[vSite].vRcvs++;
	
	CAN1RxDone = TRUE;
	CAN1 -> CMR = 0x04; // release receive buffer
	return;
}

/******************************************************************************
** Function name:		CAN_ISR_Rx2
**
** Descriptions:		CAN Rx2 interrupt handler
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void CAN_ISR_Rx2( void )
{
	unsigned char vSite = 0;
  uint32_t *pDest;
  /*
	
 // initialize destination pointer
	vSite = Can2_ID_Chk(1, CAN2->RID);
	if(0xff == vSite)
	{
		CAN2 -> CMR = 0x04; // release receive buffer
		return;
	}

  // initialize destination pointer
  pDest = (uint32_t *)&MsgBuf_RX2[vSite];
  *pDest = CAN2 -> RFS;  // Frame

  pDest++;
  *pDest = CAN2 -> RID; // ID

  pDest++;
  *pDest = CAN2 -> RDA; // Data A

  pDest++;
  *pDest = CAN2 -> RDB; // Data B

	pDest++;
 	*pDest = gRcvCan2ID[1][vSite]; // Over Time

  CAN2RxDone = TRUE;
  CAN2 -> CMR = 0x04; // release receive buffer
  */
  return;
}

unsigned char Can_Err_Chk(void)
{
	CAN1ErrCount = (CAN1 -> GSR >> 16 );
	gCanSta0 = CAN1 -> GSR;
	gCanSta1 = CAN1->ICR;
	if (( gCanSta0 & (0xc0) ) || (CAN1ErrCount>0)||(gCanSta1 & 0X0000F4))
	{
		/* The error count includes both TX and RX */
		CAN1ErrCount = 0;
		gCanSta1 = 0;
	//	delay_ms(1);
	//	CAN_Init(gSysInfo[0].vBps);		// 20181101 lsg 
	//	delay_ms(1);
		return 1;
	}
	return 0;
}				   


void Can_ask_rx(void)
{
	if(Can_Err_Chk())
		return;
	
	if(CAN1 -> GSR & 0x01)
	{
		CAN_ISR_Rx1();
	}

}

unsigned char Can2_Err_Chk(void)
{
	CAN2ErrCount = (CAN2 -> GSR >> 16 );
	gCanSta0_2 = CAN2 -> GSR;
	gCanSta1_2 = CAN2->ICR;
	if (( gCanSta0_2 & (0xc0) ) || (CAN2ErrCount>0)||(gCanSta1_2 & 0X0000F4))
	{
		/* The error count includes both TX and RX */
		CAN2ErrCount = 0;
		gCanSta1_2 = 0;
	//	delay_ms(1);
	//	CAN2_Init(gSysInfo[0].vBps2);		// 20181101 lsg 
	//	delay_ms(1);
		return 1;
	}
	return 0;
}				   


void Can2_ask_rx(void)
{
	if(Can2_Err_Chk())
		return;
	
	if(CAN2 -> GSR & 0x01)
	{
		CAN_ISR_Rx2();
	}

}
/*****************************************************************************
** Function name:		CAN_Handler
**
** Descriptions:		CAN interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void CAN_IRQHandler(void)  
{		

  CANStatus = CANCR -> CANRxSR;
  if ( CANStatus & (1 << 8) )
  {
	CAN1RxCount++;
	CAN_ISR_Rx1();
	return;
  }
  //gCANReInitFlg = 1;

	if (( CAN1 -> GSR & (0x000000c0) ) || (CAN1->ICR & 0X00000004))		// 0X000000F4			// 20181101 lsg 
	{
  	//	delay_ms(10);
		CAN_Init(gSysInfo[0].vBps);
	//	delay_ms(20);
	}
  if ( CAN1 -> GSR & (1 << 6 ) )
  {
	/* The error count includes both TX and RX */
	CAN1ErrCount = (CAN1 -> GSR >> 16 );
  }

	if(gCan2InitFlg == 0)
		return;
  if ( CANStatus & (1 << 9) )
  {
	CAN2RxCount++;
	CAN_ISR_Rx2();
	return;
  }
	if (( CAN2 -> GSR & (0x000000c0) ) || (CAN2->ICR & 0X00000004))		// 0X000000F4			// 20181101 lsg 
	{
  	//	delay_ms(10);
		CAN2_Init(gSysInfo[0].vBps);
	//	delay_ms(20);
	}
  if ( CAN2 -> GSR & (1 << 6 ) )
  {
	/* The error count includes both TX and RX */
	CAN2ErrCount = (CAN2 -> GSR >> 16 );
  }

  return;
}

/******************************************************************************
** Function name:		CAN_Init
**
** Descriptions:		Initialize CAN, install CAN interrupt handler
**
** parameters:			bitrate
** Returned value:		true or false, false if initialization failed.
** 
******************************************************************************/
uint32_t CAN_Init( uint32_t can_btr )
{

  CAN1RxDone = CAN2RxDone = FALSE;

  SC -> PCONP |= (1 << 13);// | (1 << 14);	// Enable clock to the peripheral


	PINCON -> PINSEL0	&= 0xFFFFFFF0;	// ysq
	PINCON -> PINSEL0 |= 0x00000005;	//	ysq

	//PINCON -> PINSEL1 |= 0x00003C00;	// ysq

  CAN1 -> MOD = 1;	// Reset CAN
  CAN1 -> IER = 0;	// Disable Receive Interrupt
  CAN1 -> GSR = 0;	// Reset error counter when CANxMOD is in reset
//  CAN1 -> ICR = 0;	// Reset error counter when CANxMOD is in reset

   if(can_btr > 2)
   	can_btr = 1;
  CAN1 -> IER = 5;//0xfd;					// 20181101 lsg 
  CAN1 -> BTR =  CAN_BPS[can_btr&3];
  CAN1 -> MOD = 0x0;	// CAN in normal operation mode

	NVIC_EnableIRQ(CAN_IRQn);

//  CAN2 -> IER = 0xff;		// Enable receive interrupts

  CANAF -> AFMR = 0X03;
  return( TRUE );
}
uint32_t CAN2_Init( uint32_t can_btr )
{

  CAN2RxDone = FALSE;

  SC -> PCONP |= (1 << 14);// | (1 << 14);	// Enable clock to the peripheral


	PINCON -> PINSEL0	&= 0xFFFFF0FF;	// ysq	  P0.4,P0.5  
	PINCON -> PINSEL0 |= 0x00000a00;	//	ysq
//	PINCON -> PINSEL4	&= 0xFFFc3FFF;	//	 	ysq		P2.7,P2.8  
//	PINCON -> PINSEL4 |= 0x00014000;	//		ysq

	//PINCON -> PINSEL1 |= 0x00003C00;	// ysq

  CAN2 -> MOD = 1;	// Reset CAN
  CAN2 -> IER = 0;	// Disable Receive Interrupt
  CAN2 -> GSR = 0;	// Reset error counter when CANxMOD is in reset
//  CAN2 -> ICR = 0;	// Reset error counter when CANxMOD is in reset

   if(can_btr > 2)
   	can_btr = 1;
  CAN2 -> IER = 5;//0xfd;					// 20181101 lsg 
  CAN2 -> BTR =  CAN_BPS[can_btr&3];
  CAN2 -> MOD = 0x0;	// CAN in normal operation mode

	NVIC_EnableIRQ(CAN_IRQn);
	gCan2InitFlg = 1;
//  CAN2 -> IER = 0xff;		// Enable receive interrupts

  CANAF -> AFMR = 0X03;
  return( TRUE );
}

/******************************************************************************
** Function name:		CAN_SetACCF_Lookup
**
** Descriptions:		Initialize CAN, install CAN interrupt handler
**
** parameters:			bitrate
** Returned value:		true or false, false if initialization failed.
** 
******************************************************************************/
void CAN_SetACCF_Lookup( void )
{
  uint32_t address = 0;
  uint32_t i;
  uint32_t ID_high, ID_low;

  // Set explicit standard Frame  
  CANAF -> SFF_sa = address;
  for ( i = 0; i < ACCF_IDEN_NUM; i += 2 )
  {
	ID_low = (i << 29) | (EXP_STD_ID << 16);
	ID_high = ((i+1) << 13) | (EXP_STD_ID << 0);
	*((volatile uint32_t *)(CAN_MEM_BASE + address)) = ID_low | ID_high;
	address += 4; 
  }
		
  // Set group standard Frame 
  CANAF -> SFF_GRP_sa = address;
  for ( i = 0; i < ACCF_IDEN_NUM; i += 2 )
  {
	ID_low = (i << 29) | (GRP_STD_ID << 16);
	ID_high = ((i+1) << 13) | (GRP_STD_ID << 0);
	*((volatile uint32_t *)(CAN_MEM_BASE + address)) = ID_low | ID_high;
	address += 4; 
  }
 
  // Set explicit extended Frame 
  CANAF -> EFF_sa = address;
  for ( i = 0; i < ACCF_IDEN_NUM; i++  )
  {
	ID_low = (i << 29) | (EXP_EXT_ID << 0);
	*((volatile uint32_t *)(CAN_MEM_BASE + address)) = ID_low;
	address += 4; 
  }

  // Set group extended Frame 
  CANAF -> EFF_GRP_sa = address;
  for ( i = 0; i < ACCF_IDEN_NUM; i++  )
  {
	ID_low = (i << 29) | (GRP_EXT_ID << 0);
	*((volatile uint32_t *)(CAN_MEM_BASE + address)) = ID_low;
	address += 4; 
  }
   
  // Set End of Table 
  CANAF -> ENDofTable = address;
  return;
}

/******************************************************************************
** Function name:		CAN_SetACCF
**
** Descriptions:		Set acceptance filter and SRAM associated with	
**
** parameters:			ACMF mode
** Returned value:		None
**
** 
******************************************************************************/
void CAN_SetACCF( uint32_t ACCFMode )
{
  switch ( ACCFMode )
  {
	case ACCF_OFF:
	  CANAF -> AFMR = ACCFMode;
	  CAN1 -> MOD = CAN2 -> MOD = 1;	// Reset CAN
	  CAN1 -> IER = CAN2 -> IER = 0;	// Disable Receive Interrupt
	  CAN1 -> GSR = CAN2 -> GSR = 0;	// Reset error counter when CANxMOD is in reset
	break;

	case ACCF_BYPASS:
	  CANAF -> AFMR = ACCFMode;
	break;

	case ACCF_ON:
	case ACCF_FULLCAN:
	  CANAF -> AFMR = ACCF_OFF;
	  CAN_SetACCF_Lookup();
	  CANAF -> AFMR = ACCFMode;
	break;

	default:
	break;
  }
  return;
}

/******************************************************************************
** Function name:		CAN1_SendMessage
**
** Descriptions:		Send message block to CAN1	
**
** parameters:			pointer to the CAN message
** Returned value:		true or false, if message buffer is available,
**						message can be sent successfully, return TRUE,
**						otherwise, return FALSE.
** 
******************************************************************************/
uint32_t CAN1_SendMessage( CAN_MSG *pTxBuf )
{
  uint32_t CANStatus;
  uint32_t sendDly = 3000;

//  if(Can_Err_Chk())
//	  return FALSE;
  
  while (( (CAN1 -> GSR & (1 << 5)) ) && (sendDly--));				// 20181101 lsg 
  if(sendDly == 0)
		return (FALSE);
	
  CANStatus = CAN1 -> SR;
  if ( CANStatus & 0x00000004 )
  {
	CAN1 -> TFI1 = pTxBuf->Frame & 0xC00F0000;
	CAN1 -> TID1 = pTxBuf->MsgID;
	CAN1 -> TDA1 = pTxBuf->DataA;
	CAN1 -> TDB1 = pTxBuf->DataB;
	CAN1 -> CMR |= 0x21;
	return ( TRUE );
  }
  else if ( CANStatus & 0x00000400 )
  {
	CAN1 -> TFI2 = pTxBuf->Frame & 0xC00F0000;
	CAN1 -> TID2 = pTxBuf->MsgID;
	CAN1 -> TDA2 = pTxBuf->DataA;
	CAN1 -> TDB2 = pTxBuf->DataB;
	CAN1 -> CMR |= 0x41;
	return ( TRUE );
  }
  else if ( CANStatus & 0x00040000 )
  {	
	CAN1 -> TFI3 = pTxBuf->Frame & 0xC00F0000;
	CAN1 -> TID3 = pTxBuf->MsgID;
	CAN1 -> TDA3 = pTxBuf->DataA;
	CAN1 -> TDB3 = pTxBuf->DataB;
	CAN1 -> CMR |= 0x81;
	return ( TRUE );
  }
  return ( FALSE );
}

uint32_t CAN2_SendMessage( CAN_MSG *pTxBuf )
{
  uint32_t CANStatus;
  uint32_t sendDly = 3000;

//  if(Can_Err_Chk())
//	  return FALSE;
  
  while (( (CAN2 -> GSR & (1 << 5)) ) && (sendDly--));		// 20181101 lsg 
  if(sendDly == 0)
		return (FALSE);
  
  CANStatus = CAN2 -> SR;
  if ( CANStatus & 0x00000004 )
  {
	CAN2 -> TFI1 = pTxBuf->Frame & 0xC00F0000;
	CAN2 -> TID1 = pTxBuf->MsgID;
	CAN2 -> TDA1 = pTxBuf->DataA;
	CAN2 -> TDB1 = pTxBuf->DataB;
	CAN2 -> CMR |= 0x21;
	return ( TRUE );
  }
  else if ( CANStatus & 0x00000400 )
  {
	CAN2 -> TFI2 = pTxBuf->Frame & 0xC00F0000;
	CAN2 -> TID2 = pTxBuf->MsgID;
	CAN2 -> TDA2 = pTxBuf->DataA;
	CAN2 -> TDB2 = pTxBuf->DataB;
	CAN2 -> CMR |= 0x41;
	return ( TRUE );
  }
  else if ( CANStatus & 0x00040000 )
  {	
	CAN2 -> TFI3 = pTxBuf->Frame & 0xC00F0000;
	CAN2 -> TID3 = pTxBuf->MsgID;
	CAN2 -> TDA3 = pTxBuf->DataA;
	CAN2 -> TDB3 = pTxBuf->DataB;
	CAN2 -> CMR |= 0x81;
	return ( TRUE );
  }
  return ( FALSE );
}

/******************************************************************************
**                            End Of File
******************************************************************************/
