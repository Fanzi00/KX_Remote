/********************************************************************
*  
*  APP_BUS  与 总线 有关的功能函数  IIC SPI CAN UART 等
*  功能：
*
********************************************************************/

#ifndef __APP_COMM_H_
#define __APP_COMM_H_


#include "config.h"



/****************************************************************************************/

//////////////////////////////     UART 无线 帧头定义		/////////////////////////////
#define  UART_H1				0xFE
#define  UART_H2				0xFD
#define  KYL_H1				0x55
#define  KYL_H2				0xAA
#define  UART_LEN				12			// 无线数据的长度 2+1+8+1=12



extern unsigned int gSSite ;
extern unsigned int gLenE ;
extern unsigned short gChkE ;
// UART0 -- 无线
extern unsigned int gUartDType ;		// 无线（遥控器）过来的数据类型：1~4
extern unsigned char gk ;
extern unsigned char* pL;

extern unsigned char gID;
extern unsigned int  gCanSCycle;

extern unsigned char gRcvVer[16];				// 接收机版本信息

extern unsigned char  gWLRStr[64];
extern unsigned char  gAISetAck ;				// 设置电流时反馈的标记，表示对方接收到了
extern uint8_t ADC_value[5];
/****************************************************************************************/

void Can_Prog_Send(unsigned char vFlag);
void Can_Prog_Send1(unsigned char vFlag);
void Can_Prog_Send2(unsigned char vFlag);
void Can_Prog_Send3(unsigned char vFlag);
void Can_Prog_Send4(unsigned char vFlag);
void Can_Prog_Rcv(unsigned int vFlag);


uchar Uart0_WL_Rcv(void);
void  Uart0_WL_Send(void);
void Uart0_WL_KYL(uchar vCmd, uchar vData);
uchar Uart0_WL_KYL_Rcv(void);
void Uart0_WL_SendAI(unsigned char vX);
void YKQ_Data_WL(unsigned char vFlg);



#endif  //__APP_BUS_H_
