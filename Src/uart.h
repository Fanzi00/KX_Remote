/*****************************************************************************
 *   uart.h:  Header file for NXP LPC17xx Family Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.05.27  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __UART_H 
#define __UART_H

#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

#define UARTBUFSIZE		0x40


/***************************************************************************************
*
  不同波特率下，换算出的 DLM/DLL 值有小数，
  配置 UxFDR 可以把小数计入换算中，见PDF的312~315页
*
****************************************************************************************/
#define  UxFDR_72M_57600		0		
#define  UxFDR_72M_115200		0
#define  UxFDR_72M_921600		0x81		// 1.125

#define  UxFDR_24M_57600		0x32		// 1.6666
#define  UxFDR_24M_115200		0x31		// 1.3333
#define  UxFDR_24M_921600		0			// 1.04


extern uint32_t  gUart0RcvFlg;
extern uint32_t  gUart1RcvFlg;

extern volatile uint32_t UART0Status, UART1Status;
extern volatile uint8_t UART0TxEmpty, UART1TxEmpty;
extern volatile uint8_t UART0Buffer[UARTBUFSIZE], UART1Buffer[UARTBUFSIZE];
extern volatile uint32_t UART0Count, UART1Count;
extern volatile uint32_t UART0RcvDly, UART1RcvDly;


uint32_t UARTInit( uint32_t portNum, uint32_t Baudrate );
void UART0_IRQHandler( void );
void UART1_IRQHandler( void );
void UARTSend( uint32_t portNum, uint8_t *BufferPtr, uint32_t Length );

#endif /* end __UART_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
