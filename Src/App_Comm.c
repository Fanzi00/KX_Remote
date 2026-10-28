/****************************************************************************
*
*	 APP_Comm 与 通讯 有关的函数  无线通讯，CAN通讯 等
*
*
*****************************************************************************/

#include "app_comm.h"
#include "LPC17xx.h"
#include "config.h"

extern CAN_MSG MsgBuf_RX1[ID_RCV_NUM] ;



unsigned char  gWLRStr[64];
unsigned char  gAISetAck = 0;				// 设置电流时反馈的标记，表示对方接收到了
unsigned char  gWLDLen = 0;					// 无线数据的长度，不包括头尾字段

// UART0 -- 无线
unsigned int gUartDType = 0;		// 无线（遥控器）过来的数据类型：1~4
unsigned char gWLSStr[30];			// 无线发送的数据缓冲
unsigned char gWLSStr2[20];			// 无线发送的数据缓冲
unsigned char gRcvVer[16]="KX119";				// 接收机版本信息


/****************************************************************************************
****************************************************************************************
* 时间: 2015-05-24
* 名称：	Uart0_WL_Rcv()
* 功能：			UART0 接收处理函数，无线通信
* 无线帧结构： 5A A5 Type Data0~8 XOR
* 入口参数：
* 出口参数：
****************************************************************************************
****************************************************************************************/
uchar Uart0_WL_Rcv(void)
{
	uchar i=0;
	uchar vULen = 0;		// UART 接收字符数
	uchar vXOR = 0;			// 
	unsigned short vChk = 0;
	uchar vFlg = 0xff;
	uchar vPageNum = 0;
	uchar vUStr[128] = {0};
	uchar vCh = 0;
	uchar vFrameLen = 0;		// 整帧长度（H1/H2 + LEN 指示），用于按帧拷贝、避免越界
//	uint  vIntD = 0;
//	uchar vType = 0;

	if(gWLRWFlg >= 0x5A)				// 读取 无线模块的当前参数，等待模块返回数据   lsg_wl
	{
		wl_rd_rcv();
		return 0;
	}		

	//// 从环形缓冲中取一帧：关 UART0 收中断后原子读取，ISR 只改 UART0RxHead、解析只改 UART0RxTail，互不踩踏
	NVIC_DisableIRQ(UART0_IRQn);
	vULen = (uint8_t)((UART0RxHead - UART0RxTail) & UART0_RXBUF_MASK);
	if(vULen == 0)
	{
		NVIC_EnableIRQ(UART0_IRQn);
		return 0;
	}
	vFlg = 0xff;
	for(i=0; i<vULen; i++)
	{
		if(UART0_RX_GET(i) == UART_H1)
		{
			if((i+1 < vULen) && (UART0_RX_GET(i+1) == UART_H2))	 // 找到帧头
			{
				vFlg = i;
				break;
			}
		}
	}
	if(vFlg == 0xff)					// 无帧头：缓冲里是噪声/垃圾，整段丢弃避免堆积
	{
		UART0RxTail = (UART0RxTail + vULen) & UART0_RXBUF_MASK;
		gWLRcvHdrMiss++;
		NVIC_EnableIRQ(UART0_IRQn);
		return 0;
	}
	if((vULen - vFlg) < 3)			// 仅有帧头、尚未收到 LEN 字段，等下一批
	{
		NVIC_EnableIRQ(UART0_IRQn);
		return 0;
	}
	//// 关键修复：先按帧内 LEN 字段计算整帧长度，仅拷贝“整帧”，严禁越界写入 vUStr[128]。
	//// 原 13b2ab7 实现拷贝到缓冲末尾（vULen-vFlg 可能 >128），导致栈溢出，把上电/掉线时
	//// 无线模块持续的噪声流当成合法帧解析，出现每次上电都一样的“脏数据”。
	vFrameLen = UART0_RX_GET(vFlg + 2) + 2;		// 帧长 = LEN + H1/H2
	if((vFlg + vFrameLen) > vULen)			// 帧未齐：保留在缓冲，等后续字节到齐
	{
		NVIC_EnableIRQ(UART0_IRQn);
		return 0;
	}
	for(i=0; (i < vFrameLen) && (i < (int)(sizeof(vUStr) - 1)); i++)
		vUStr[i] = UART0_RX_GET(vFlg + i);
	//// 消费整帧后再开中断（ISR 在解析期间不会改写已消费字节）
	UART0RxTail = (UART0RxTail + vFlg + vFrameLen) & UART0_RXBUF_MASK;
	NVIC_EnableIRQ(UART0_IRQn);
	//// 以下沿用原帧解析逻辑（vUStr 已为完整一帧，vUStr[0]=H1）
	i=2;
	vULen  = vUStr[i++];
	gUartDType = vUStr[i++];
//	if((( gUartDType & 0x0f)>6))// || (( gUartDType & 0x0f)<1))		// 数据类型字段错误
//		return 0;
	
			gTXErrDly = 30; 
	vXOR = 0;
	for(i=2; i<vULen; i++)					 	// 检查长度，拷贝数据
	{
		vXOR ^= vUStr[i];
		vChk += vUStr[i];
	}
	if(vXOR != vUStr[i])						// 校验错
		{ gWLRcvChkFail++; return 0; }
	if((uchar)vChk != vUStr[i+1])						// 校验错
		{ gWLRcvChkFail++; return 0; }
	
	i=3;		  // vUStr[3] == Data0
	//vPageNum = gUartDType & 0xf0;
	for(i=0; i<8; i++)
	{
		gWLRStr[i] = vUStr[i+2];
		vUStr[i] = vUStr[i+4];
	}
//	Can_Send6_tst();
	switch(	gUartDType )
	{
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
		case 0x88:
			gAISetAck = gUartDType;
			i=0;
			gRcvVer[i] = vUStr[i];		i++;
			gRcvVer[i] = vUStr[i];		i++;
			gRcvVer[i] = vUStr[i];		i++;
			gRcvVer[i] = vUStr[i];		i++;
			gRcvVer[i] = vUStr[i];		i++;
			gRcvVer[i] = vUStr[i];		i++;
			gRcvVer[i] = vUStr[i];		i++;
			gRcvVer[i] = vUStr[i];		i++;
			gRcvVer[i] = 0;
			break;
		case 0:					 	// 屏显A帧：8字节=4个uint16
			for(i=0; i<8; i++)
				gRunInfo.vRcvWL[i] = vUStr[i];   						// 20230210  
			WL_Recv(0, vUStr);
			break;
		case 5:					 	// 屏显B帧
			WL_Recv(1, vUStr);
			break;
		case 7:					 	// 屏显C帧（Type6 已用于"设置AI的反馈"，见下，故屏显C帧改到 Type7）
			WL_Recv(2, vUStr);
			break;
		case 1:					 	// 控制器的数据  0x280+ykqID	 DI/DO/WARN
			for(i=0; i<8; i++)
				gRunInfo.vRcvWL[8 + i] = vUStr[i];   						// 20230210  

				gRunInfo.vSNJDW = vUStr[0]; 
				gRunInfo.vPaiL = vUStr[1]; 
				gRunInfo.vWarns = vUStr[2];// + vUStr[3]*256; 
				gRunInfo.vLJWkTime = vUStr[6] + vUStr[7]*256; 
				gRunInfo.vLJWkTime <<= 16;
				gRunInfo.vLJWkTime += vUStr[4] + vUStr[5]*256; 
				gWarnFlg = gRunInfo.vWarns;
				//if(gRunInfo.vWarns & 0x80)				// 蓄电池电压低						// 20180509  
				//	gWarnFlg |= 1;
			break;
		case 2:					 	// 控制器的数据
			for(i=0; i<8; i++)
				gRunInfo.vRcvWL[16 + i] = vUStr[i];   						// 20230210
			break;

		case 3: 					// 控制器的数据  0x380+ykqID
			for(i=0; i<8; i++)
				gRunInfo.vRcvWL[24 + i] = vUStr[i];   						// 20230210


		vPageNum = vUStr[7] & 0xf0;
			if(vPageNum <= 0x10)			// 电磁阀电流
			{
				//vCh = (vUStr[1] >> 7) & 1;
				vPageNum = vUStr[1] & 0xf0;
				vCh = vPageNum>>4;					// 反馈值的高4位，位0--上，为1--下
				gRunInfo.vArmI[0+vCh*8] = vUStr[0] + ((vUStr[1]& 0x0f)<<8);
				vPageNum = vUStr[3] & 0xf0;
				vCh = vPageNum>>4;					// 反馈值的高4位，位0--上，为1--下
				gRunInfo.vArmI[1+vCh*8] = vUStr[2] + ((vUStr[3]& 0x0f)<<8);
				vPageNum = vUStr[5] & 0xf0;
				vCh = vPageNum>>4;					// 反馈值的高4位，位0--上，为1--下
				gRunInfo.vArmI[2+vCh*8] = vUStr[4] + ((vUStr[5]& 0x0f)<<8);
				vPageNum = vUStr[7] & 0xf0;
				vCh = vPageNum>>4;					// 反馈值的高4位，位0--上，为1--下
				gRunInfo.vArmI[3+vCh*8] = vUStr[6] + ((vUStr[7]& 0x0f)<<8);
			}
			else if(vPageNum >= 0x80)
			{
				vCh = (vPageNum & 0x70) >> 4;
					gBDParam2[1].vAI[0][vCh] = vUStr[0] + ((vUStr[1]& 0x0f)<<8);
					gBDParam2[1].vAI[1][vCh] = vUStr[2] + ((vUStr[3]& 0x0f)<<8);
					gBDParam2[1].vAI[2][vCh] = vUStr[4] + ((vUStr[5]& 0x0f)<<8);
					gBDParam2[1].vAI[3][vCh] = vUStr[6] + ((vUStr[7]& 0x0f)<<8);
			}
			break;
		case 4:	  					// 控制器的数据  0x480+ykqID
			if(vPageNum <= 0x10)			// 电磁阀电流
			{
				vPageNum = vUStr[1] & 0xf0;
				vCh = vPageNum>>4;					// 反馈值的高4位，位0--上，为1--下
				gRunInfo.vArmI[4+vCh*8] = vUStr[0] + ((vUStr[1]& 0x0f)<<8);
				vPageNum = vUStr[3] & 0xf0;
				vCh = vPageNum>>4;					// 反馈值的高4位，位0--上，为1--下
				gRunInfo.vArmI[5+vCh*8] = vUStr[2] + ((vUStr[3]& 0x0f)<<8);
				vPageNum = vUStr[5] & 0xf0;
				vCh = vPageNum>>4;					// 反馈值的高4位，位0--上，为1--下
				gRunInfo.vArmI[6+vCh*8] = vUStr[4] + ((vUStr[5]& 0x0f)<<8);
				vPageNum = vUStr[7] & 0xf0;
				vCh = vPageNum>>4;					// 反馈值的高4位，位0--上，为1--下
				gRunInfo.vArmI[7+vCh*8] = vUStr[6] + ((vUStr[7]& 0x0f)<<8);
			}
			else if(vPageNum >= 0x80)
			{
				vCh = (vPageNum & 0x70) >> 4;
					gBDParam2[1].vAI[0][vCh] = vUStr[0] + ((vUStr[1]& 0x0f)<<8);
					gBDParam2[1].vAI[1][vCh] = vUStr[2] + ((vUStr[3]& 0x0f)<<8);
					gBDParam2[1].vAI[2][vCh] = vUStr[4] + ((vUStr[5]& 0x0f)<<8);
					gBDParam2[1].vAI[3][vCh] = vUStr[6] + ((vUStr[7]& 0x0f)<<8);
			}
			break;
		case 6:						// 设置AI的反馈
			gRunInfo.vAIPage = vUStr[2];
			gRunInfo.vAIX = vUStr[3]&0x0f;
			gRunInfo.vAIY = (vUStr[3]>>4);
			if(gAISetFlg == 0)			// 上电后发送AI
			{
				if((gSetAIX == gRunInfo.vAIX) && (gSetAIY == gRunInfo.vAIY) )
				{
					gSetAIX += 1;
					if(	gSetAIX > 5)
					{
						gSetAIX = 0;
						if(gSetAIY < 4)
							gSetAIY += 2;
						if(gSetAIY > 4)
							gSetAIY = 4;
					}
					gAISendFlg = gSetAIY * 6 + gSetAIX + 1;
				}
			/*	if((gSetAIX == gRunInfo.vAIX) && (gSetAIY == gRunInfo.vAIY))
				{
					gSetAIX += 1;
					if(	gSetAIX > 5)
					{
						gSetAIX = 0;
						gSetAIY += 2;
					}
					gAISendFlg = gSetAIY * 6 + gSetAIX + 1;
				}	*/
			}
			break;
		default:
			break;
	}
	gWLRcvOk++;
	return gUartDType;
}

////  从无线模块发送遥控器的数据
/****************************************************************************************
****************************************************************************************
* 时间: 2015-05-24
* 名称：	Uart0_WL_Send()
* 功能：		UART0 发送函数，无线通信，电池供电时发送，有线供电时不发送
* 无线帧结构： 5A A5 len Type Data0~8 Sum XOR  (24Byte)
* 入口参数：
* 出口参数：
****************************************************************************************
****************************************************************************************/
// 无线数据
void Uart0_WL_Send(void)
{
	static unsigned char vCh = 0;
	//unsigned char vStr[21];
	unsigned char i = 0;
	unsigned char j = 0;
	unsigned short vChk = 0;
	unsigned short vXor = 0;

	gWLSStr[i++] = UART_H1;
	gWLSStr[i++] = UART_H2;
	gWLSStr[i++] = 0;		 				// len
	gWLSStr[i++] = vCh + 1;			// 1~4
	//gWLSStr[i++] = gBDCmd + (gLCDPage.page << 4);
	j = 0;
	for(j=0; j<gWLDLen; j++)
		gWLSStr[i++] = gYKQ_WLStr[j];

	gWLSStr[2] = i;
	for(j=2; j<i; j++)					 	// 检查长度，拷贝数据
	{
		vXor ^= gWLSStr[j];
		vChk += gWLSStr[j];
	}
	gWLSStr[i++] = (unsigned char)vXor;
	gWLSStr[i++] = (unsigned char)vChk;
			CAN_Send(0, 0x6a0+gID, (unsigned char*)(&gWLSStr[2]));

	UARTSend(0, gWLSStr, i);
}

// 无线数据，发送第几个摇杆的最大、最小电流（两个方向）
void Uart0_WL_SendAI(unsigned char vX)
{
	static unsigned char vCh = 0;
	//unsigned char vStr[21];
	unsigned char i = 0;
	unsigned char j = 0;
	unsigned short vChk = 0;
	unsigned short vXor = 0;

	gWLSStr[i++] = UART_H1;
	gWLSStr[i++] = UART_H2;
	gWLSStr[i++] = 0;		 				// len
	if((vX <= 0X98) && (vX >= 0x80))
	{
		gWLSStr[i++] = vX;// | 0x80;			// 1~4
		if(vX == 0x88)		// 电流爬坡时间
		{
			gWLSStr[i++] = gBDParam2[0].vAITime;
			gWLSStr[i++] = gBDParam2[0].vAITime>>8;
			gWLSStr[i++] = gBDParam2[0].vVCC;
			gWLSStr[i++] = gBDParam2[0].vRes;
			gWLSStr[i++] = 0;
			gWLSStr[i++] = 0;
			gWLSStr[i++] = 0;
			gWLSStr[i++] = 0;
			gWLSStr[i++] = 0;
			gWLSStr[i++] = 0;
			gWLSStr[i++] = 0;
			gWLSStr[i++] = 0;
		}
		else if((vX < 0x88) && (vX >= 0x80))
		{
			vX = vX & 0x0f;
			//gWLSStr[i++] = gBDCmd + (gLCDPage.page << 4);
			gWLSStr[i++] = gBDParam2[0].vAI[0][vX];
			gWLSStr[i++] = gBDParam2[0].vAI[0][vX]>>8;
			gWLSStr[i++] = gBDParam2[0].vAI[1][vX];
			gWLSStr[i++] = gBDParam2[0].vAI[1][vX]>>8;
			gWLSStr[i++] = gBDParam2[0].vAI[2][vX];
			gWLSStr[i++] = gBDParam2[0].vAI[2][vX]>>8;
			gWLSStr[i++] = gBDParam2[0].vAI[3][vX];
			gWLSStr[i++] = gBDParam2[0].vAI[3][vX]>>8;
			gWLSStr[i++] = 0;
			gWLSStr[i++] = 0;
			gWLSStr[i++] = 0;
			gWLSStr[i++] = 0;
		}
		else if((vX < 0x98) && (vX >= 0x90))
		{
			vX = vX & 0x0f;
			//gWLSStr[i++] = gBDCmd + (gLCDPage.page << 4);
			gWLSStr[i++] = gBDParam2[0].vAI2[0][vX];
			gWLSStr[i++] = gBDParam2[0].vAI2[0][vX]>>8;
			gWLSStr[i++] = gBDParam2[0].vAI2[1][vX];
			gWLSStr[i++] = gBDParam2[0].vAI2[1][vX]>>8;
			gWLSStr[i++] = gBDParam2[0].vAI2[2][vX];
			gWLSStr[i++] = gBDParam2[0].vAI2[2][vX]>>8;
			gWLSStr[i++] = gBDParam2[0].vAI2[3][vX];
			gWLSStr[i++] = gBDParam2[0].vAI2[3][vX]>>8;
			gWLSStr[i++] = 0;
			gWLSStr[i++] = 0;
			gWLSStr[i++] = 0;
			gWLSStr[i++] = 0;
		}
	}
	gWLSStr[2] = i;
	for(j=2; j<i; j++)					 	// 检查长度，拷贝数据
	{
		vXor ^= gWLSStr[j];
		vChk += gWLSStr[j];
	}
	gWLSStr[i++] = (unsigned char)vXor;
	gWLSStr[i++] = (unsigned char)vChk;
			CAN_Send(0, 0x600+gID, (unsigned char*)(&gWLSStr[2]));

	UARTSend(0, gWLSStr, i);
}


/****************************************************************************************
****************************************************************************************
* 时间: 2015-05-24
* 名称：	YKQ_Data_WL()
* 功能：		生产无线发送的数据到缓冲区
* 无线帧结构： 5A A5 len Type Data0~17 XOR
* 入口参数：
* 出口参数：
****************************************************************************************
****************************************************************************************/
uint8_t ADC_value[5] = {0};
void YKQ_Data_WL(unsigned char vFlg)
{
	unsigned char i = 0;
//	unsigned char k = 0;
	ADC_Correction();
		if(gJiTing || stStudyFlg || (gLaBaFlg==0))	  			// 急停、学习中、未激活均输出中位
		{			
			gDIBitV = 0;
			gDIxBitV = 0;
			gYKQ_WLStr[i++] = _ARM_MIDV;			// Arm1
			gYKQ_WLStr[i++] = _ARM_MIDV;
			gYKQ_WLStr[i++] = _ARM_MIDV;
			gYKQ_WLStr[i++] = _ARM_MIDV;
			gYKQ_WLStr[i++] = _ARM_MIDV;
			gYKQ_WLStr[i++] = _ARM_MIDV;
			gYKQ_WLStr[i++] = _ARM_MIDV;
			gYKQ_WLStr[i++] = _ARM_MIDV;			// Arm 8
			gYKQ_WLStr[i++] = 0;			// ad1
			gYKQ_WLStr[i++] = 0;			// ad2
			gYKQ_WLStr[i++] = 0;			// ad3
			gYKQ_WLStr[i++] = gDIBitV;				// 两档妞子  1bit -- 1个开关，
			gYKQ_WLStr[i++] = gDIBitV >> 8;
			gYKQ_WLStr[i++] = gDIBitV >>16;				// 三档妞子  2bit -- 1个开关
			gYKQ_WLStr[i++] = gDIBitV >>24;
			gYKQ_WLStr[i++] = gDIxBitV;				// 档位旋钮DI输入结果，4bit -- 1个开关
			gYKQ_WLStr[i++] = gDIxBitV >> 8;
			gYKQ_WLStr[i++] = gModeSel  | (gLaBaFlg<<5) | (0x80*_ARM_4R_XY);//gDIxBitV >> 8;
			if(gJiTing)
				gYKQ_WLStr[i++] = 0x55;
			else
				gYKQ_WLStr[i++] = 0xaa;
		}
		else
		{
			
			
			gYKQ_WLStr[i++] = VKC_Send[0] ; // gADArmSend[0];
			gYKQ_WLStr[i++] = VKC_Send[1] ; //gADArmSend[1];			// Arm1
			gYKQ_WLStr[i++] = VKC_Send[2] ; //gADArmSend[2];
			gYKQ_WLStr[i++] = VKC_Send[3] ; //gADArmSend[3];
			gYKQ_WLStr[i++] = VKC_Send[4] ; //gADArmSend[4];
			gYKQ_WLStr[i++] = VKC_Send[5] ;//gADArmSend[5];
			gYKQ_WLStr[i++] = VKC_Send[6] ;//gADArmSend[6];
			gYKQ_WLStr[i++] = VKC_Send[7] ;//gADArmSend[7];			// Arm 8
			gYKQ_WLStr[i++] = ADC_value[0] ;//gADArmSend[8];			// ad1
			gYKQ_WLStr[i++] = ADC_value[1] ;//gADArmSend[9];			// ad2
			gYKQ_WLStr[i++] = ADC_value[2] ;//gADArmSend[10];			// ad2	10
			gYKQ_WLStr[i++] = gDIBitV;						// 两档妞子  1bit -- 1个开关，
			gYKQ_WLStr[i++] = gDIBitV >> 8;
			gYKQ_WLStr[i++] = gDIBitV >> 16;			
			gYKQ_WLStr[i++] = gDIBitV >> 24;
			gYKQ_WLStr[i++] = ADC_value[3];
			gYKQ_WLStr[i++] = ADC_value[4];
			gYKQ_WLStr[i++] = gModeSel | (gLaBaFlg<<5)  | (0x80*_ARM_4R_XY);//gDIxBitV >> 8;
			gYKQ_WLStr[i++] = 0;
		}
		gWLDLen = i;
}







