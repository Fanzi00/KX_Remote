/*******************************************************************************
*   canopen.c
Platform: AT90CAN32
Project : 倾角传感器
Clock F : 内部1MHz，外部12MHz
Software: AVR STUDIO 4.18
Author  : LSG
Version : 12.06.25
Updata  : 
comments:
1、
2、
3、

CANOPEN 协议相关处理函数

*******************************************************************************/

#include "config.h"
#include "canopen.h"
//#include "..\can\LPC1700can.h"


const unsigned char cReadErr[9]="CfgRdErr";

//extern MessageDetail  MessageDetailT;                                          /* 定义发送帧                   */
extern unsigned char gID;
extern unsigned short gCanCyc;

//pSysInfo *pgSysInfo = &gSysInfo;

unsigned int gSysStatus = 0;
unsigned char gCanOpenFlag = 0;
unsigned char  gCanOutTst = 0;
								 
unsigned int   gCanDOTstV = 0;
int fCurve_State = 0;
float fOutput_Y = 0.0;
float ArrayX[_CURVE_N][_CURVE_X];
float ArrayY[_CURVE_N][_CURVE_X];
unsigned char Y_Send[10] ;	
unsigned char GL_Send[10] ;	    unsigned char VKC_Send[10] ;   

//unsigned char gCycleSetCheck[8] = "CYCLExx";
//unsigned int  GRcv44Dly = 0;
unsigned char CAN1_Get_Data(unsigned int vID) ;
	 /*CAN数据缓存*/		  
	 unsigned char CAN1_RBuf[8]={0,0,0,0,0,0,0,0};	//MOb0缓存，1C1F3E02
	 unsigned char CAN_RBuf0[8]={0,0,0,0,0,0,0,0};	//MOb0缓存，1C1F3E02
	 unsigned char CAN_RBuf1[8]={0,0,0,0,0,0,0,0};	//MOb1缓存，1C1F3E03
	 unsigned char CAN_RBuf2[8]={0,0,0,0,0,0,0,0};	//MOb2缓存，1C1F3E23
	 unsigned char CAN_RBuf3[8]={0,0,0,0,0,0,0,0};	//MOb2缓存，1C1F3E23
	 unsigned char CAN_SBuf0[8]={0,0,0,0,0,0,0,0};	//MOb3缓存，1C1F3E0x
	 unsigned char CAN_SBuf1[8]={0,0,0,0,0,0,0,0};	//MOb4缓存，保存外呼按钮灯信息
	 unsigned char CAN_SBuf2[8]={0,0,0,0,0,0,0,0};	//MOb4缓存，保存外呼按钮灯信息
	 unsigned char CAN_SBuf3[8]={0,0,0,0,0,0,0,0};	//MOb4缓存，保存外呼按钮灯信息
unsigned long gCanRID0 = 0;		// RBUF0 对应的 ID 
unsigned long gCanRID1 = 0;		// RBUF1 对应的 ID 
unsigned long gCanRID2 = 0;		// RBUF2 对应的 ID 
unsigned long gCanRID3 = 0;		// RBUF3 对应的 ID 

unsigned int   app10ms_flags = 0;
CAN_MSG MsgBuf_TX1, MsgBuf_TX2; // TX and RX Buffers for CAN message
CAN_MSG MsgBuf_RX1[ID_RCV_NUM];
//CAN_MSG MsgBuf_RX2[ID_RCV_NUM]; // TX and RX Buffers for CAN message

volatile uint32_t CAN1RxDone, CAN2RxDone;
unsigned int gCanRFlg = 0;
unsigned char gRcvYKQ = 0;			// 接收到 遥控器 的数据

unsigned char  T0_50ms = 0;
unsigned char  T0_60ms = 0;
unsigned char  ceSH = 0;
unsigned char  ceSH_BYTE = 0;
unsigned char  ceSH_BYTE_old = 0;
unsigned char  X_TEST_old = 0;	 
int test_x =0;
int test_y =0;
int Y_TEST =0;
int X_TEST =0;
int Y_BAR =0;
int X_BAR =0;  
unsigned char  yk1 = 0;	 
unsigned char  yk2 = 0;	 
unsigned char  yk3 = 0;	 
unsigned char  yk4 = 0;	 
unsigned char  yk5 = 0;	 
unsigned char  yk6 = 0;	 
unsigned char  yk7 = 0;	 
unsigned char  yk8 = 0;	 
											 										    
//extern	 void Can_Send6_tst(void);

#if 1

unsigned long gRcvCanID[2][ID_RCV_NUM];
/****************************************************************************
* 时间: 2008-04-04
* 名称：unsigned char RegisterID(unsigned int vIDParam, unsigned char vIDIndex, unsigned int vOvTime_ms)
* 功能：CAN通讯ID号注册函数
* 入口参数： vIDParam: 需要注册的ID号 
						 vIDIndex: 注册的顺序号  
						 vOvTime_ms：接收超时，单位 ms
* 出口参数：
****************************************************************************/
unsigned char RegisterID(unsigned int vIDParam, unsigned char vIDIndex, unsigned int vOvTime_ms)
{
	unsigned char i = 0;

	if(vIDIndex >= ID_RCV_NUM)
		return 0;
	for(i=0; i<ID_RCV_NUM; i++)
	{
		if(gRcvCanID[0][i] == 0)
		{
			gRcvCanID[0][i] = vIDParam;
			gRcvCanID[1][i] = vOvTime_ms;
			break;
		}
	}
	return 1;
}

void Curve_N(unsigned char vCHN, float fXin, unsigned char Reset, unsigned char MapNo)
{
	unsigned char i = 0;
	unsigned char j = 0;
  //    Init : BOOL;
	float Temp =0.0;
	float a[_CURVE_X];
	float b[_CURVE_X];
	float OrderX[_CURVE_X];
	float OrderY[_CURVE_X];
	int  Temp2 =0 ;

	if(Reset)
	{
		fCurve_State = 0 ;
		for( i= 0; i< 5; i++)
			{
			OrderX[i] = 0.0;
			OrderY[i] = 0.0;
			a[i] = 0.0;
			b[i] = 0.0;

			fOutput_Y = 0.0 ;
		}
	}
	else
	{
		if( MapNo > _CURVE_X) 
			fCurve_State = -1 ;
		
		for (i=0; i<= (MapNo -1);i++) 
		{
			OrderX[i] = ( ArrayX[vCHN][i] );
			OrderY[i] =( ArrayY[vCHN][i] );
		}
		
		for (i=0; i<= (MapNo -2); i++)
		{
			for (j = i +1; j<= (MapNo -1);j++)
			{
				if (OrderX[i] > OrderX[j])
				{
					Temp 	  = OrderX[j] ;
					OrderX[j] = OrderX[i] ;
					OrderX[i] = Temp;		//OrderX[j] ;(

					Temp      = OrderY[j] ;
					OrderY[j] = OrderY[i] ;
					OrderY[i] = Temp;		//OrderY[j] ;
				}
				else if( OrderX[i] == OrderX[j] )
				{
					if (OrderY[i] != OrderY[j])
					{
						fCurve_State = -1 ;
						return ;
					}
				}
			}
		}
		
		if (fCurve_State == -1)
			return;
		
		for (i=0 ; i<=(MapNo -2); i++)
		{
			j = i +1 ;
			if (OrderX[j] == OrderX[i] )
			{
				a[i] = 0.0 ;
				b[i] = 0.0 ;
			}
			else
			{
				a[i] = (OrderY[j] - OrderY[i]) *10000.0  / (OrderX[j] - OrderX[i]) ;
				b[i] = OrderY[i] - a[i] * OrderX[i] /10000.0 ;
			}
		}

		Temp2 = MapNo -1 ;
		if ((fXin) < OrderX[0] )
			fOutput_Y =  OrderY[0] ;
		else if( (fXin) > OrderX[Temp2] )
			fOutput_Y =  OrderY[Temp2]  ;
		else
		{
			for (i= 0; i<= (MapNo -2); i++)
			{
				j = i +1;
				if (((fXin) >= OrderX[i]) && ((fXin) <= OrderX[j])) 
					fOutput_Y =  a[i] * (fXin) /10000.0  + b[i] ;
			}
		}
	}
	fCurve_State = 1 ;
}


unsigned char Can_ID_Chk(unsigned char vCh, unsigned int vID)
{
	unsigned char i = 0;
	
	for(i=0; i<ID_RCV_NUM; i++)
	{
		if(gRcvCanID[vCh][i] == vID)
		{
			gCanRFlg = 1<<i;
			return i;
		}
	}
	return 0xff;
}

void CAN_Send(unsigned char vChn, unsigned long vID, unsigned char* vpData)
{
	unsigned long canDA = 0;
	unsigned long canDB = 0;

	canDA = *vpData + (*(vpData+1)<<8) + (*(vpData+2)<<16) + (*(vpData+3)<<24);
	canDB = *(vpData+4) + (*(vpData+5)<<8) + (*(vpData+6)<<16) + (*(vpData+7)<<24);
	MsgBuf_TX1.MsgID = vID;
	MsgBuf_TX1.Frame &= ~0x000f0000;		// bit16~19 -- DLC
	MsgBuf_TX1.Frame |= (0x08 << 16);
	if(vID > 0x7ff)
		MsgBuf_TX1.Frame |= (0x80000000);
	else
		MsgBuf_TX1.Frame &= ~(0x80000000);
	MsgBuf_TX1.DataA = canDA;
	MsgBuf_TX1.DataB = canDB;
	//while ( !(CAN1 -> GSR & (1 << 3)) );
	//while(1)
	{
		if ( CAN1_SendMessage( &MsgBuf_TX1 ) == TRUE )
		{
		 	;//break;
		}
	}
}
//// CAN 发送函数
void CAN_SendLen(unsigned char vChn, unsigned long vID, unsigned char* vpData, char vLen)
{
	unsigned long canDA = 0;
	unsigned long canDB = 0;

	canDA = *vpData + (*(vpData+1)<<8) + (*(vpData+2)<<16) + (*(vpData+3)<<24);
	canDB = *(vpData+4) + (*(vpData+5)<<8) + (*(vpData+6)<<16) + (*(vpData+7)<<24);
	MsgBuf_TX1.MsgID = vID;
	MsgBuf_TX1.Frame &= ~0x000f0000;		// bit16~19 -- DLC
	MsgBuf_TX1.Frame |= (vLen << 16);
	if(vID > 0x7ff)
		MsgBuf_TX1.Frame |= (0x80000000);
	else
		MsgBuf_TX1.Frame &= ~(0x80000000);
		MsgBuf_TX1.DataA = canDA;
		MsgBuf_TX1.DataB = canDB;
	//	while ( !(CAN1 -> GSR & (1 << 3)) );
		if ( CAN1_SendMessage( &MsgBuf_TX1 ) == FALSE )
		{
		  ;
		}
}
 #endif

/*=======================================================================================
=========================================================================================
*
* 时间: 2015-05-26
* 名称：CAN 应用函数，配置ID，接收，发送等函数
* 
=========================================================================================
=======================================================================================*/

/******************************************************************
* 时间: 2015-05-26
* 名称：unsigned char Can_RcvID_Cfg(void)
* 功能：配置 CAN通讯接收的ID号
* 入口参数：
			
* 出口参数：
******************************************************************/
unsigned char Can_RcvID_Cfg(void)
{
	unsigned char vIDNum = 0; 
	unsigned char i=0;

	//// 下面函数的 3 个参数分别是 CAN ID，接收序号，接收超时(ms)
	vIDNum += RegisterID(ID_RCV_M1 , i++, 2000);									// 0x7e5， 2000ms 超时，请保留该行	  0
	vIDNum += RegisterID(0x1C8 , i++, 1000);	 	// 1    //GPS-lock-unlock//	1		  
	vIDNum += RegisterID(0x1CA , i++, 1000);	 	// 1    //GPS-lock-unlock//	2		  
 	  

	CAN_SBuf0[0] = gSysStatus;
	CAN_SBuf0[1] = gID;
	CAN_SBuf0[2] = ID_DEFAULT;
	CAN_SBuf0[3] = 0;
	CAN_SendLen(5, ID_SEND_READY+gID, CAN_SBuf0,5);		// AI, 0x700

	return vIDNum;
}

/******************************************************************
* 时间: 2015-05-26
* 名称：unsigned char CanOpen_Prog(void)
* 功能：CAN 配置函数，可配置ID/BPS/CYCLE/角度标定等
* 入口参数：
*			
* 出口参数：
******************************************************************/

unsigned char CanOpen_Prog(unsigned char vFlag)
{
	unsigned char i = 0;
	unsigned char vSBuf[8];
//	unsigned int  vIndex = 0;

	for(i=0; i<8; i++)
		vSBuf[i] = 0;

	if(0 == gCanOpenFlag)
	{
		vSBuf[0] = 0;
		vSBuf[1] = 0;
	//	CAN_SendLen(4, ID_SEND_READY + gID, vSBuf,1);	
		gCanOpenFlag = CANOPEN_MOD_INIT;
	}

	if((0x01 == (vFlag & 0x01)) && (CAN_RBuf0[1] == gID))				// Mob0, ID_RCV_M1 0x7e5
	{
		if(CAN_RBuf0[0] == 0x11)				// Cfg NID
		{
			if((CAN_RBuf0[2] > 0) && (CAN_RBuf0[2] < 128))
			{
				gSysInfo[0].vID = CAN_RBuf0[2];
				//gSysInfo.v1018_1_ID = gSysInfo.vID;
				//gID = gSysInfo.vID;
				gCanOpenFlag = CANOPEN_MOD_SETID;
			}
			vSBuf[0] = 0x11;
			vSBuf[1] = 0;
			CAN_Send(4, ID_ACK_M1, vSBuf);	
			gCanOpenFlag = CANOPEN_MOD_SETID_OK;
		}
		if(CAN_RBuf0[0] == 0x12)				// Cfg CYCLE
		{
			//if((CAN_RBuf0[1] == 0))
			{
				gSysInfo[0].vCycle = CAN_RBuf0[2];
				gCanCyc = gSysInfo[0].vCycle * 10;
				gCanOpenFlag = CANOPEN_MOD_SETCYCL;
			}
			vSBuf[0] = 0x12;
			vSBuf[1] = 0;
			CAN_Send(4, ID_ACK_M1, vSBuf);	
			gCanOpenFlag = CANOPEN_MOD_SETCYCL_OK;
		}
		if(CAN_RBuf0[0] == 0x13)				// Cfg BPS
		{
			//if((CAN_RBuf0[1] == 0))
			{
				gSysInfo[0].vBps = CAN_RBuf0[2];
				if(gSysInfo[0].vBps > 2)
					gSysInfo[0].vBps = 2;
				gCanOpenFlag = CANOPEN_MOD_SETBPS;
			}
			vSBuf[0] = 0x13;
			vSBuf[1] = 0;
			CAN_Send(4, ID_ACK_M1, vSBuf);	
			gCanOpenFlag = CANOPEN_MOD_SETBPS_OK;
		}
		if(CAN_RBuf0[0] == 0x17)				// Save
		{
			gCanOpenFlag = CANOPEN_MOD_SAVE;
			Sys_Save_Info();
		//	Sys_Read_Info();
			gID = gSysInfo[0].vID;
			gSysStatus |= 0x80;
			vSBuf[0] = 0x17;
			vSBuf[1] = 0;
			CAN_Send(4, ID_ACK_M1, vSBuf);	
			gCanOpenFlag = CANOPEN_MOD_SAVE_OK;
		}
	}
	return gCanOpenFlag;
}

/******************************************************************
* 时间: 2015-05-26
* 名称：unsigned char Can_Send_Prog(void)
* 功能：CAN 发送函数，发送 0x180/0x280/0x380/0x480 + ID
* 入口参数：
*			
* 出口参数：
******************************************************************/

void Can_Prog_Send1(unsigned char vFlag)
{ 
//	unsigned char vCnt = 0;
	unsigned char i = 0;

	if(gCanOutTst == 0)
	{
		if(gJiTing || (gLaBaFlg==0))	  			// 急停		
		{
			CAN_SBuf0[i++] = _ARM_MIDVCan;
			CAN_SBuf0[i++] = _ARM_MIDVCan;
			CAN_SBuf0[i++] = _ARM_MIDVCan;
			CAN_SBuf0[i++] = _ARM_MIDVCan;
			CAN_SBuf0[i++] = _ARM_MIDVCan;//gBDCmd + (gLCDPage.page << 4);
			CAN_SBuf0[i++] = _ARM_MIDVCan;//gDIBitV | gDIBitVxx;
			CAN_SBuf0[i++] = _ARM_MIDVCan;//gDIBitV>>8;
			CAN_SBuf0[i++] = _ARM_MIDVCan;//0x55;		
		}
		else							  		// 无急停
		{
			CAN_SBuf0[i++] = ((unsigned int)gADArmSend[0]) * 255 / 250;
			CAN_SBuf0[i++] = ((unsigned int)gADArmSend[1]) * 255 / 250;
			CAN_SBuf0[i++] = ((unsigned int)gADArmSend[2]) * 255 / 250;
			CAN_SBuf0[i++] = ((unsigned int)gADArmSend[3]) * 255 / 250;
			CAN_SBuf0[i++] = ((unsigned int)gADArmSend[4]) * 255 / 250;
			CAN_SBuf0[i++] = ((unsigned int)gADArmSend[5]) * 255 / 250;
			CAN_SBuf0[i++] = ((unsigned int)gADArmSend[6]) * 255 / 250;
			CAN_SBuf0[i++] = ((unsigned int)gADArmSend[7]) * 255 / 250;
		}
	}
	else
	{
	   CAN_SBuf0[i++] = ADCRst[0];
	   CAN_SBuf0[i++] = ADCRst[0]>>8;
	   CAN_SBuf0[i++] = ADCRst[1];
	   CAN_SBuf0[i++] = ADCRst[1]>>8;
	   CAN_SBuf0[i++] = ADCRst[2];
	   CAN_SBuf0[i++] = ADCRst[2]>>8;
	   CAN_SBuf0[i++] = ADCRst[3];
	   CAN_SBuf0[i++] = ADCRst[3]>>8;
	}
	 if(gPowerFlg==0)  
		CAN_Send(0, ID_SEND_D1+gID, CAN_SBuf0);//UART0Buffer);0x18374840
}

void Can_Prog_Send2(unsigned char vFlag)
{ 
	unsigned char i = 0;
	static unsigned char vX = 0;

	if(gCanOutTst == 1)
	{
		CAN_SBuf3[i++] = ADCRst[4];
		CAN_SBuf3[i++] = ADCRst[4]>>8;
		CAN_SBuf3[i++] = ADCRst[5];
		CAN_SBuf3[i++] = ADCRst[5]>>8;
		CAN_SBuf3[i++] = ADCRst[6];
		CAN_SBuf3[i++] = ADCRst[6]>>8;
		CAN_SBuf3[i++] = ADCRst[7];
		CAN_SBuf3[i++] = ADCRst[7]>>8;
	}
	else
	{
		CAN_SBuf3[i++] = ((unsigned int)gADArmSend[8]) * 255 / 250;
		CAN_SBuf3[i++] = ((unsigned int)gADArmSend[9]) * 255 / 250;
		CAN_SBuf3[i++] = gKValx;
		CAN_SBuf3[i++] = gPower24V;
		CAN_SBuf3[i++] = gBDParam2[0].vAI[4][vX];
		CAN_SBuf3[i++] = gBDParam2[0].vAI[4][vX]>>8;
		CAN_SBuf3[i++] = gBDParam2[0].vAI[5][vX];
		CAN_SBuf3[i++] = (gBDParam2[0].vAI[5][vX]>>8) + (vX<<4);
		vX++;
		if(vX >= 8)
			vX = 0;
	}
	if(gPowerFlg==0)  
		CAN_Send(0, ID_SEND_D2+gID, CAN_SBuf3);
}

void Can_Prog_Send3(unsigned char vFlag)
{ 
	unsigned char i = 0;
	if(gCanOutTst == 0)
	{
		if(gJiTing || (gLaBaFlg==0))	  			// 急停		
		{
			CAN_SBuf1[i++] = gDIBitV;
			CAN_SBuf1[i++] = gDIBitV>>8;
			CAN_SBuf1[i++] = gDIBitV>>16;
			CAN_SBuf1[i++] = gDIBitV>>24;
			CAN_SBuf1[i++] = gDIxBitV;
			CAN_SBuf1[i++] = gDI3BitV;
			CAN_SBuf1[i++] = gDI3BitV>>8;
			if(gJiTing)
				CAN_SBuf1[i++] = 0x55;	
			else
				CAN_SBuf1[i++] = 0x5A;	
		}
		else				     			// 无急停
		{
			CAN_SBuf1[i++] = gDIBitV;
			CAN_SBuf1[i++] = gDIBitV>>8;
			CAN_SBuf1[i++] = gDIBitV>>16;
			CAN_SBuf1[i++] = gDIBitV>>24;
			CAN_SBuf1[i++] = gDIxBitV;
			CAN_SBuf1[i++] = gDIxBitV>>8;
			CAN_SBuf1[i++] = gKeyValue | (gLaBaFlg<<5) ;
			CAN_SBuf1[i++] = gPowerChk;
		}
	}
	else
	{
		CAN_SBuf1[i++] = ADCRst[8];
		CAN_SBuf1[i++] = ADCRst[8]>>8;
		CAN_SBuf1[i++] = ADCRst[9];
		CAN_SBuf1[i++] = ADCRst[9]>>8;
		CAN_SBuf1[i++] = ADCRst[10];
		CAN_SBuf1[i++] = ADCRst[10]>>8;
		CAN_SBuf1[i++] = ADCRst[11];
		CAN_SBuf1[i++] = ADCRst[11]>>8;
	}
	 if(gPowerFlg==0)  
		CAN_Send(0, ID_SEND_D3+gID, CAN_SBuf1);		// 0x18374841
}
void Can_Prog_Send4(unsigned char vFlag)
{ 
	unsigned char i = 0;
	static unsigned char vX = 0;

	if(gCanOutTst == 0)
	{
		if(vX < 8)
		{
			CAN_SBuf2[i++] = gBDParam2[0].vAI[0][vX] ;
			CAN_SBuf2[i++] = gBDParam2[0].vAI[0][vX]>>8;
			CAN_SBuf2[i++] = gBDParam2[0].vAI[1][vX];
			CAN_SBuf2[i++] = gBDParam2[0].vAI[1][vX]>>8;
			CAN_SBuf2[i++] = gBDParam2[0].vAI[2][vX];
			CAN_SBuf2[i++] = gBDParam2[0].vAI[2][vX]>>8;
			CAN_SBuf2[i++] = gBDParam2[0].vAI[3][vX];
			CAN_SBuf2[i++] = (gBDParam2[0].vAI[3][vX]>>8) + (vX<<4);
		}
		if(vX >= 8)
		{
			CAN_SBuf2[0] = gBDParam2[0].vAITime ;
			CAN_SBuf2[1] = gBDParam2[0].vAITime>>8;
			CAN_SBuf2[2] = gBDParam2[0].vVCC ;
			CAN_SBuf2[3] = gBDParam2[0].vRes ;
			CAN_SBuf2[7] = (vX<<4);
		}
		vX++;
		if(vX > 8)
			vX = 0;
	}
	else
	{
		CAN_SBuf2[i++] = ADCRst[12];
		CAN_SBuf2[i++] = ADCRst[12]>>8;
		CAN_SBuf2[i++] = ADCRst[13];
		CAN_SBuf2[i++] = ADCRst[13]>>8;
		CAN_SBuf2[i++] = ADCRst[14];
		CAN_SBuf2[i++] = ADCRst[14]>>8;
		CAN_SBuf2[i++] = ADCRst[15];
		CAN_SBuf2[i++] = ADCRst[15]>>8;
	}
	// if(gPowerFlg==0)  
		CAN_Send(0, ID_SEND_D4+gID, CAN_SBuf2);				// 0x18374845
}

void Can_Prog_Send5(unsigned char vFlag)
{ 
	unsigned char i = 0;
	static unsigned char vX = 0;

	if(gCanOutTst == 0)
	{
		if(vX < 8)
		{
			CAN_SBuf3[i++] = gBDParam2[0].vAI[4][vX] ;
			CAN_SBuf3[i++] = gBDParam2[0].vAI[4][vX]>>8;
			CAN_SBuf3[i++] = gBDParam2[0].vAI[5][vX];
			CAN_SBuf3[i++] = gBDParam2[0].vAI[5][vX]>>8;
			CAN_SBuf3[i++] = gCellPower;	//gBDParam2[0].vAI[2][vX];
			CAN_SBuf3[i++] = ADCRstP[14];	//gBDParam2[0].vAI[2][vX]>>8;
			CAN_SBuf3[i++] = ADCRstP[14]>>8;	//gBDParam2[0].vAI[3][vX];
			CAN_SBuf3[i++] = 0 + (vX<<4);	//(gBDParam2[0].vAI[3][vX]>>8) + (vX<<4);
		}
		if(vX >= 8)
		{
			CAN_SBuf3[0] = gBDParam2[0].vAITime ;
			CAN_SBuf3[1] = gBDParam2[0].vAITime>>8;
			CAN_SBuf3[4] = gCellPower;
			CAN_SBuf3[5] = ADCRstP[14];	//gBDParam2[0].vAI[2][vX]>>8;
			CAN_SBuf3[6] = ADCRstP[14]>>8;	//gBDParam2[0].vAI[3][vX];
			CAN_SBuf3[7] = (vX<<4);
		}
		vX++;
		if(vX > 8)
			vX = 0;
	}
	else
	{
		CAN_SBuf3[i++] = gArmMidSp[0];	// gLCDSStr[0];
		CAN_SBuf3[i++] = gArmMidSp[1];	// gLCDSStr[1];
		CAN_SBuf3[i++] = gArmMidSp[2];	// gLCDSStr[2];
		CAN_SBuf3[i++] = gArmMidSp[3];	// gLCDSStr[3];
		CAN_SBuf3[i++] = gArmMidSp[4];	// gLCDSStr[4];
		CAN_SBuf3[i++] = gArmMidSp[5];	// gLCDSStr[5];
		CAN_SBuf3[i++] = gArmMidSp[6];	// gLCDSStr[6];
		CAN_SBuf3[i++] = gArmMidSp[7];	// gLCDSStr[7];
	}
	// if(gPowerFlg==0)  
	CAN_Send(0, 0x580+gID, CAN_SBuf3);
 
}

void Can_Send6_tst(void)
{
	char i=0;
	CAN_SBuf3[i++] = gWLRStr[0];
	CAN_SBuf3[i++] = gWLRStr[1];
	CAN_SBuf3[i++] = gWLRStr[2];
	CAN_SBuf3[i++] = gWLRStr[3];
	CAN_SBuf3[i++] = gWLRStr[4];
	CAN_SBuf3[i++] = gWLRStr[5];
	CAN_SBuf3[i++] = gWLRStr[6];
	CAN_SBuf3[i++] = gWLRStr[7];
	CAN_Send(0, 0x680+gID, CAN_SBuf3);

	 i=0 ;
	CAN_SBuf3[i++] = gLaBaFlg;
	CAN_SBuf3[i++] = gPowerChk;
	CAN_SBuf3[i++] = 0;
	CAN_SBuf3[i++] = 0;
	CAN_SBuf3[i++] = 0;
	CAN_SBuf3[i++] = 0;
	CAN_SBuf3[i++] =0;
	CAN_SBuf3[i++] = 0;
	CAN_Send(0, 0x221, CAN_SBuf3);


}	    

 struct	remote
 {
 int L1;
 int L2;
 int L3;
 int L4;
 int L5;
 int L6;
 int L7; 
 int L8;   
 char message_1;
 char message_2;
 char message_3;
 char message_4;
 char message_5;
 char message_6;  
 char message_7; 
 char message_8;   
 };
 struct remote remote_18c;
void CAN_receive_data(void)
{
		  
	unsigned char  rdCan = 0;
	unsigned char  i = 0;

	 
			rdCan = CAN1_Get_Data(0x1C8);			   										   
			if(rdCan == 1)					// 0x2FD 返回值 0~ID_RCV_NUM ：接收到指定的CAN 数据
			{   
				  	Y_TEST  =  CAN1_RBuf[1]  ;
					X_TEST = CAN1_RBuf[0]   ;
				  	Y_BAR  =  CAN1_RBuf[3]   ;
					X_BAR = CAN1_RBuf[4] + CAN1_RBuf[5] *256  ;
				
					   				
			}	
			else														// 返回值 > ID_RCV_NUM，异常，0xFF：超时，0xEE：不存在该 ID
			{										 	 
					  ;
			}
	 
			rdCan = CAN1_Get_Data(0x1CA);			   										   
			if(rdCan == 2)					// 0x2FD 返回值 0~ID_RCV_NUM ：接收到指定的CAN 数据
			{   
				  	yk1  =  CAN1_RBuf[0]  ;
					yk2 =   CAN1_RBuf[1]   ;
				  	yk3  =  CAN1_RBuf[2]   ;
					yk4 =   CAN1_RBuf[3] ;
				  	yk5  =  CAN1_RBuf[4]  ;
					yk6 =   CAN1_RBuf[5]   ;
				  	yk7  =  CAN1_RBuf[6]   ;
					yk8 =   CAN1_RBuf[7] ;
				
					   				
			}	
			else														// 返回值 > ID_RCV_NUM，异常，0xFF：超时，0xEE：不存在该 ID
			{	
				  	yk1  = 0  ;
					yk2 =   0   ;
				  	yk3  =  0  ;
					yk4 =  0 ;
				  	yk5  = 0  ;
					yk6 =  0   ;
				  	yk7  = 0   ;
					yk8 =   0 ;
				
			}

}
void app10ms(void)
{
	 
	unsigned char i = 0;
	
	 CAN_receive_data() ;
	for(i=0; i<8; i++)
	{
	   KX_Send_joysend(i) ;

	}
	
    KX_Send_joylogic(Y_Send  ,8 ) ;


	
	for(i=0; i<8; i++)
	{
	   KX_127_63(VKC_Send,i) ;

	}		
	
			    
					remote_18c.L1           =	 gRunInfo.vRcvWL[0] + yk1 ; 
					remote_18c.L2           =	 gRunInfo.vRcvWL[1] + yk2 ; 
					remote_18c.L3           =	 gRunInfo.vRcvWL[2] + yk3 ; 
					remote_18c.L4           =	 gRunInfo.vRcvWL[3] + yk4 ;  
					remote_18c.L5           =	 gRunInfo.vRcvWL[4] + yk5 ; 
					remote_18c.L6           =	 gRunInfo.vRcvWL[5] + yk6 ; 
					remote_18c.L7           =	 gRunInfo.vRcvWL[6] + yk7 ; 
					remote_18c.L8           =	 gRunInfo.vRcvWL[7]  + yk8 ;   
					 ceSH_BYTE =   _BitV(	remote_18c.L6 	, 7) ;						  
					remote_18c.message_1      =	 _BitV(remote_18c.L7, 0) ;
					remote_18c.message_2	  =	 _BitV(remote_18c.L7, 1) ;
					remote_18c.message_3	  =	 _BitV(remote_18c.L7, 2) ;
					remote_18c.message_4	  =	 _BitV(remote_18c.L7, 3) ;
					remote_18c.message_5	  =	 _BitV(remote_18c.L7, 4) ;
					remote_18c.message_6	  =	 _BitV(remote_18c.L7, 5) ;
					remote_18c.message_7	  =	 _BitV(remote_18c.L7, 6) ;
					remote_18c.message_8	  =	 _BitV(remote_18c.L7, 7) ;
					       


} 
void Can_Send_Prog(void)
{		    
	T0_50ms++ ;	  T0_60ms++ ;
	if(T0_50ms >5)
	{	
	 				
		CAN_SBuf1[0] = Y_Send[0];	 
		CAN_SBuf1[1] = Y_Send[1] ;	 
		CAN_SBuf1[2] = Y_Send[2];
		CAN_SBuf1[3] = Y_Send[3] ;
		CAN_SBuf1[4] = Y_Send[4];  
		CAN_SBuf1[5] = Y_Send[5]  ;     		  
		 CAN_SBuf1[6] = ADCRst[1]; // 2下
		 CAN_SBuf1[7] = ADCRst[1]>>8;
	   	CAN_Send(0, 0x188, CAN_SBuf1);		
				 			    							
		CAN_SBuf1[0] =dead_zone;	 
		CAN_SBuf1[1] = ADCRst[0] >>8;	 
		CAN_SBuf1[2] = ADCRst[ 1];
		CAN_SBuf1[3] = ADCRst[ 1] >>8;
		CAN_SBuf1[4] = ADCRst[2];  
		CAN_SBuf1[5] = ADCRst[2] >>8 ;  
		CAN_SBuf1[6] = ADCRst[3]   ; 
		CAN_SBuf1[7] = ADCRst[3] >>8 ; 	
		 
	   	CAN_Send(0, 0x189, CAN_SBuf1);		
		   
		T0_50ms =0 ;
		 		
	}	
	if (T0_60ms >10)
	{ 
		CAN_SBuf1[0] = gDIBitV;
		CAN_SBuf1[1] = gDIBitV>>8;
		CAN_SBuf1[2] = gDIBitV>>16;
		CAN_SBuf1[3] = gDIBitV>>24;
		CAN_SBuf1[4] =  ADCRst[4];
		CAN_SBuf1[5] =  ADCRst[4]>>8;	// 
		CAN_SBuf1[6] =   ADCRst[5] ;
		CAN_SBuf1[7] =   ADCRst[5]>>8 ;
	   	CAN_Send(0, 0x18A, CAN_SBuf1);		//
													  
   				
		CAN_SBuf1[0] = ADCRst[6];	 
		CAN_SBuf1[1] = ADCRst[6] >>8;	 
		CAN_SBuf1[2] = ADCRst[ 7];
		CAN_SBuf1[3] = ADCRst[ 7] >>8;
		CAN_SBuf1[4] = ADCRst[8];  
		CAN_SBuf1[5] = ADCRst[8] >>8 ;  
		CAN_SBuf1[6] = ADCRst[9]   ; 
		CAN_SBuf1[7] = ADCRst[9] >>8 ; 													 
	   	CAN_Send(0, 0x18B, CAN_SBuf1);		//
	 
		CAN_SBuf1[0] = ADCRst[10];	 
		CAN_SBuf1[1] = ADCRst[10] >>8;	 
		CAN_SBuf1[2] = ADCRst[11];
		CAN_SBuf1[3] = ADCRst[11] >>8;
		CAN_SBuf1[4] = ADCRst[12];  
		CAN_SBuf1[5] = ADCRst[12] >>8 ;  
		CAN_SBuf1[6] = ADCRst[13]   ; 
		CAN_SBuf1[7] = ADCRst[13] >>8 ; 													 
	   	CAN_Send(0, 0x18C, CAN_SBuf1);		//
		CAN_SBuf1[0] = ADCRst[14];	 
		CAN_SBuf1[1] = ADCRst[14] >>8;	 
		CAN_SBuf1[2] = ADCRst[15];
		CAN_SBuf1[3] = ADCRst[15] >>8;
		CAN_SBuf1[4] = ADCRst[16];  
		CAN_SBuf1[5] = ADCRst[16] >>8 ;  
		CAN_SBuf1[6] = ADCRst[13]   ; 
		CAN_SBuf1[7] = ADCRst[13] >>8 ; 													 
	   	CAN_Send(0, 0x18D, CAN_SBuf1);		//
	 
		  
		T0_60ms =0 ;

	}
}
 			 

/******************************************************************
* 时间: 2015-05-26
* 名称：unsigned char Can_Prog_Rcv(void)
* 功能：CAN 接收函数，接收 0x180/0x280/0x380/0x480 + Rcv_ID / QJ_ID
* 入口参数：
*			
* 出口参数：
******************************************************************/

void Can_Prog_Rcv(unsigned int vFlag)
{ 
	unsigned char vCh = 0;
		
		vCh = 0;
	if((1<<vCh) == (vFlag & (1<<vCh)))	  //  CFG
	{
		vFlag &= ~(1<<vCh);
		CAN_RBuf0[0] = (MsgBuf_RX1[vCh].DataA >> 0) & 0xff;
		CAN_RBuf0[1] = (MsgBuf_RX1[vCh].DataA >> 8) & 0xff;
		CAN_RBuf0[2] = (MsgBuf_RX1[vCh].DataA >> 16) & 0xff;
		CAN_RBuf0[3] = (MsgBuf_RX1[vCh].DataA >> 24) & 0xff;
		CAN_RBuf0[4] = (MsgBuf_RX1[vCh].DataB >> 0) & 0xff;
		CAN_RBuf0[5] = (MsgBuf_RX1[vCh].DataB >> 8) & 0xff;
		CAN_RBuf0[6] = (MsgBuf_RX1[vCh].DataB >> 16) & 0xff;
		CAN_RBuf0[7] = (MsgBuf_RX1[vCh].DataB >> 24) & 0xff;

		if((CAN_RBuf0[0]==0xf1)&&(CAN_RBuf0[1]==0xf2)&&(CAN_RBuf0[2]==0xf3)&&(CAN_RBuf0[3]==0xf4))
		{
			if((CAN_RBuf0[4]==0xf5)&&(CAN_RBuf0[5]==0xf6)&&(CAN_RBuf0[6]==gID))
				gCanOutTst = CAN_RBuf0[7];
			if((CAN_RBuf0[4]==0xff))
			{
				gCanOutTst = CAN_RBuf0[5];
				gCanDOTstV = CAN_RBuf0[6] + (CAN_RBuf0[7]<<8);
			}
		}
		else
			CanOpen_Prog(1);
	}
}


////  从接收缓冲区中读取指定ID的数据（CANx_RBuf），并返回状态：0xFF：超时, 0xEE：没配置接收该ID，其他值：接收序号
unsigned char CAN1_Get_Data(unsigned int vID)
{
	unsigned char i = 0;
	unsigned char vCh = 0;
	
	for(i=0; i<8; i++)
		CAN1_RBuf[i] = 0;
	for(i=0; i<ID_RCV_NUM; i++)
	{
		if(gRcvCanID[0][i] == vID)
		{
			vCh = i;
			CAN1_RBuf[0] = (MsgBuf_RX1[vCh].DataA >> 0) & 0xff;
			CAN1_RBuf[1] = (MsgBuf_RX1[vCh].DataA >> 8) & 0xff;
			CAN1_RBuf[2] = (MsgBuf_RX1[vCh].DataA >> 16) & 0xff;
			CAN1_RBuf[3] = (MsgBuf_RX1[vCh].DataA >> 24) & 0xff;
			CAN1_RBuf[4] = (MsgBuf_RX1[vCh].DataB >> 0) & 0xff;
			CAN1_RBuf[5] = (MsgBuf_RX1[vCh].DataB >> 8) & 0xff;
			CAN1_RBuf[6] = (MsgBuf_RX1[vCh].DataB >> 16) & 0xff;
			CAN1_RBuf[7] = (MsgBuf_RX1[vCh].DataB >> 24) & 0xff;
			if(MsgBuf_RX1[vCh].vEmpty == 0)
				return 0xFF;
			else
				return i;
		}
	}
	return 0xEE;	
}


/******************************************************************
* 时间: 2015-05-26
* 名称：unsigned char CanOpen_Prog(void)
* 功能：CAN 配置函数，可配置ID/BPS/CYCLE/角度标定等
* 入口参数：
*			
* 出口参数：
******************************************************************/

void  KX_127_63(unsigned char Y_Send_buf[] ,unsigned char vNO   )	// 0 1 
{
						    
 													   
	  if (abs( Y_Send_buf[vNO] - _ARM_MIDVCan ) > 0  )
	  {
	  		 if (Y_Send_buf[vNO] >_ARM_MIDVCan )
			 {
			 	 GL_Send[vNO]= 62-abs( Y_Send_buf[vNO] - _ARM_MIDVCan )*62.0/127.0  ;	
			 }
			 else
			 {	  
			 	 GL_Send[vNO]= abs( Y_Send_buf[vNO] - _ARM_MIDVCan )*62.0/127.0 +64  ;	
			 		
			 }
	  		
	  }
	  else
	  {
	  		GL_Send[vNO] = 63 ;
	  }
	   
}


/******************************************************************
* 时间: 2015-05-26
* 名称：unsigned char CanOpen_Prog(void)
* 功能：CAN 配置函数，可配置ID/BPS/CYCLE/角度标定等
* 入口参数：
*			
* 出口参数：
******************************************************************/

void  KX_Send_joylogic(unsigned char Y_Send_buf[] ,unsigned char vNO   )	// 0 1 
{

	unsigned char i = 0 , NO_Result=0; char vc = 0 ;
	float q_y_link = 0.0 ;
	for(i=0; i<4; i++)
	{
	   vc = 2*i ;
	   if (abs( Y_Send_buf[vc+1] - _ARM_MIDVCan ) > 0  )
	   {    
		   q_y_link =  abs( Y_Send_buf[vc] - _ARM_MIDVCan ) /  abs( Y_Send_buf[vc+1] - _ARM_MIDVCan )  ;
		 
			if (q_y_link >1.0)
				NO_Result =vc ;
			else if	(q_y_link <1.0)
				NO_Result = vc+1 ;
			else
			   NO_Result =vc+8  ;
		}
		else
		{  
			if ( abs( Y_Send_buf[vc] - _ARM_MIDVCan ) >0 )

				 NO_Result = vc ;
			else
				 NO_Result = vc+8 ;
		
		}
		
		if (NO_Result==vc)
		{
			VKC_Send[NO_Result]	= Y_Send_buf[NO_Result] ;
			VKC_Send[NO_Result+1]	= _ARM_MIDVCan ;
		}
	  
		if (NO_Result==vc+1)
		{	 
			VKC_Send[NO_Result]	= Y_Send_buf[NO_Result] ;
			VKC_Send[NO_Result-1]	=_ARM_MIDVCan ;
	
		}
		if (NO_Result==vc+8)	 
		{
			VKC_Send[NO_Result-8]	= Y_Send_buf[NO_Result-8] ;
			VKC_Send[NO_Result-7]	= Y_Send_buf[NO_Result-7] ;
		}						  
	}
 		

	 
	   
}

/******************************************************************
* 时间: 2015-05-26
* 名称：unsigned char CanOpen_Prog(void)
* 功能：CAN 配置函数，可配置ID/BPS/CYCLE/角度标定等
* 入口参数：
*			
* 出口参数：
******************************************************************/
void KX_Send_joysend(unsigned char vCH)	// 0 1 
{
		 
			 	if (ADCRst[gArmUpTab[vCH]]>100)
				{
				//	if (vCH==0)
					{
						  ArrayX[1][0] =  0xd35	;
						  ArrayX[1][1] =  0xBCB	 ;
						  ArrayX[1][2] =  0x7DC	  ;
						  ArrayX[1][3] =  0x3F9	  ;
						  ArrayX[1][4] =  0x1Fa	  ;		   						 
							 	 
						  ArrayY[1][0] =  0; 	 
						  ArrayY[1][1] =  30;  	 
						  ArrayY[1][2] =  127;  	 
						  ArrayY[1][3] =  200; 	 
						  ArrayY[1][4] =  255; 
					  }
			 
						Curve_N(1 , ADCRst[gArmUpTab[vCH]]  , 0,5 ) ;		// 
						if ( abs(ADCRst[gArmUpTab[vCH]] -0x7DC)<120 )  //中位误差处理。
							  Y_Send[vCH] = _ARM_MIDVCan ;
						else							   
								Y_Send[vCH] = fOutput_Y ;
					}
					else
					{
						 	Y_Send[vCH] = _ARM_MIDVCan ;
						
					}
			 
 
}
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

