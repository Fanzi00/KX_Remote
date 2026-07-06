/****************************************************************************
*
*	 APP_BUS 与 总线、接口 有关的函数  IIC SPI CAN UART  LCD 等
*
*
*****************************************************************************/

#include "app_Bus.h"
//#include "app_Bus2.h"
#include "LPC17xx.h"
#include "config.h"


extern CAN_MSG MsgBuf_RX1[ID_RCV_NUM] ;


mOverTable gOverTable[2];				// 超载信息记录表
mOverTable gOvHandTable[2];			// 超载解除按钮记录表
mOverInfo  gOverInfo;				// 检查和记录‘超载信息’用
mOvHandInfo gOvHandInfo;			// 检查和记录‘超载限制按钮操作’用
mOverInfo  gOverRInfo;				// 读取‘超载信息’用
mOvHandInfo gOvHandRInfo;			// 读取‘超载限制按钮操作’用


/****************************************************************************************/




/****************************************************************************************
****************************************************************************************
* 时间: 2014-11-24
* 名称：
* 功能：				IIC1 相关函数
* 入口参数：
* 出口参数：
****************************************************************************************
****************************************************************************************/

///////////////////////////////////  I2C1  ///////////////////////////////////
////  从slave接收 1 字节数据 
uchar I2C1_Read_B(uchar device,uint address)
{
	unsigned char i=0;
	unsigned char k=0;

	for ( i = 0; i < IIC1BUFSIZE; i++ )	/* clear buffer */
	{
		I2C1MasterBuffer[i] = 0;
	}
	k = 0;
	I2C1ReadLength = 1;
	I2C1MasterBuffer[k++] = device;
	I2C1MasterBuffer[k++] = address>>8;
	I2C1MasterBuffer[k++] = address&0xff;
	I2C1WriteLength = k;
	I2C1Cmd = RD_BIT;
	I2C1Engine();
	return(I2C1SlaveBuffer[0]);
}

////  向slave写入 1 字节数据 
uchar I2C1_Write_B(uchar device,uint address,uchar bytedata)//向slave写入1字节数据 
{
	unsigned char i=0;
	unsigned char k=0;

	for ( i = 0; i < IIC1BUFSIZE; i++ )	/* clear buffer */
	{
		I2C1MasterBuffer[i] = 0;
	}

	k = 0;
	I2C1ReadLength = 0;
	I2C1MasterBuffer[k++] = device;
	I2C1MasterBuffer[k++] = address>>8;
	I2C1MasterBuffer[k++] = address&0xff;
	I2C1MasterBuffer[k++] = bytedata;
	I2C1WriteLength = k;
	I2C1Engine();
//delay_ms(5);
	return 0;
}

////  从slave接收 N 字节数据 
unsigned char I2C1_Read_N(unsigned int device,unsigned int address,
       unsigned char *pRdDat,unsigned char num)
{
	unsigned char k=0;
	unsigned char i=0;

	for ( i = 0; i < IIC1BUFSIZE; i++ )	/* clear buffer */
	{
		I2C1MasterBuffer[i] = 0;
	}
	k = 0;
	I2C1ReadLength = num;
	I2C1MasterBuffer[k++] = device;
	I2C1MasterBuffer[k++] = address>>8;
	I2C1MasterBuffer[k++] = address&0xff;
	I2C1WriteLength = k;
	I2C1Cmd = RD_BIT;
	I2C1Engine();
	for(i=0; i<=num; i++)
		*pRdDat++ = I2C1SlaveBuffer[i]; //I2CMasterBuffer[k+i];
	return i;
}
////  向slave写入 N 字节数据
unsigned char I2C1_Write_N(unsigned int device,unsigned int address,
       unsigned char *pWrDat,unsigned char num)
{
	unsigned char i=0;
	unsigned char k=0;

	for ( i = 0; i < IIC1BUFSIZE; i++ )	/* clear buffer */
	{
		I2C1MasterBuffer[i] = 0;
	}
	k = 0;
	I2C1WriteLength = 3+num;
	I2C1ReadLength = 0;
	I2C1MasterBuffer[k++] = device;
	I2C1MasterBuffer[k++] = address>>8;
	I2C1MasterBuffer[k++] = address&0xff;
	for(i=0; i<num; i++)
		I2C1MasterBuffer[i+k] = *pWrDat++;
	I2C1Engine();
	return i;
}




/****************************************************************************************
****************************************************************************************
* 时间: 2014-11-24
* 名称：
* 功能：			EEPROM 24C256 相关函数
* 入口参数：
* 出口参数：
****************************************************************************************
****************************************************************************************/

//////////////////////////////////  I2C1  ////////////////////////////////////////
unsigned char AT24_Read1_Str(unsigned int byte_addr, unsigned char *buff, unsigned char num)
{
#if 1
    unsigned char i;
	byte_addr &= 0x7fff;
	i = I2C1_Read_N(AT24C_WRCMD + AT_24_ADDR1, byte_addr, buff, num);
#endif
	return i;

}

unsigned char AT24_Write1_Page(unsigned int byte_addr, unsigned char *buff, unsigned char num)
{
    unsigned char i;
	//unsigned char ack = 0;
WP2_L 

	byte_addr &= 0x7fff;
	i = I2C1_Write_N(AT24C_WRCMD + AT_24_ADDR1, byte_addr, buff, num);
	delay_ms(8);
WP2_H
	return i;
}

unsigned long AT24_Read1_ULong(unsigned int byte_addr)
{
    unsigned long i=0;
	unsigned char vStr[6];
	vStr[0] = 0;
	vStr[1] = 0;
	vStr[2] = 0;
	vStr[3] = 0;
	byte_addr &= 0x7fff;
	I2C1_Read_N(AT24C_WRCMD + AT_24_ADDR1, byte_addr, vStr, 4);
	i = (vStr[0]<<24)+ (vStr[1]<<16)+(vStr[2]<<8)+ vStr[3];
	return i;

}
unsigned char AT24_Write1_ULong(unsigned int byte_addr, unsigned long wData)
{
	unsigned char i = 0;
	unsigned char vStr[6];
WP2_L 
	vStr[0] = wData>>24;
	vStr[1] = wData>>16;
	vStr[2] = wData>>8;
	vStr[3] = wData;
	vStr[4] = 0;
	byte_addr &= 0x7fff;
	i = I2C1_Write_N(AT24C_WRCMD + AT_24_ADDR1, byte_addr, vStr, 4);
	delay_ms(8);
WP2_H
	return i;
}

/****************************************************************************************
*
* 时间: 2014-11-24
* 名称：			超载信息的保存和读取
* 功能：			EEPROM_2 24C256 中按 数据结构 读写函数
* 入口参数：
* 出口参数：
*
*****************************************************************************************/

////  保存 超载记录 信息 到EEPROM (AT24C256)  中

void Sys_Save_OverInfo(unsigned char vSite)
{
	unsigned int i = 0;
	unsigned char  k = 0;
	unsigned char  iEEAck = 0;
	unsigned short vChk = 0;

	gLenE = sizeof(gOverInfo);
	for(k=0; k<2; k++)
	{
		//gLenE = SYSLEN_SIZE;
		gSSite = (EX_EEPROM_SIZE>>1)*k + EX_OVER_INFO_STADDR + vSite * EX_OVER_INFO_SIZE;
		vChk = 0;
		pL = (unsigned char*)(&gOverInfo);
		for(i=0; i<gLenE-1; i++)
		{
			vChk += *pL;
			pL++;
		}
		gOverInfo.vChkSum = vChk;
		pL = (unsigned char*)(&gOverInfo);
		i = AT24_Write_Page((gSSite), pL, gLenE);
		iEEAck += (i<<gk);
	}
}
////  读取存储在EEPROM (AT24C256) 中的 超载记录 信息
unsigned char  Sys_Read_OverInfo(unsigned char vSite)
{
	unsigned short i = 0;
	unsigned char  j = 0;
	unsigned char  ChkFlg = 0;
	unsigned char  vExEEprom = 0;
	unsigned char  vRdCnt = 0;
	unsigned char  Flg2 = 0;

	ChkFlg = 0;
	gLenE = sizeof(gOverRInfo);
	for(j=0; j<2; )
	{
		gSSite = (EX_EEPROM_SIZE>>1)*j + EX_OVER_INFO_STADDR + vSite * EX_OVER_INFO_SIZE;
		pL = (unsigned char*)(&gOverRInfo);

		vExEEprom = AT24_Read_Str((gSSite), pL, gLenE);
		pL = (unsigned char*)(&gOverRInfo);
		gChkE = 0;
		for(i=0; i<gLenE-1; i++)
		{
			gChkE += *pL;
			pL++;
		}
		if((unsigned char)gChkE == gOverRInfo.vChkSum)
		{
			if((gOverRInfo.vHead[0]== SYSINFO_HEAD0)&& (gOverRInfo.vHead[1]== SYSINFO_HEAD1))
				Flg2 = 1;
		}

		vRdCnt++;
		if(vExEEprom && Flg2)
		{
			return 0;
		}	
		if(vRdCnt >= 0x03)
		{
			ChkFlg |= (1<<j);	
			j++;
			vRdCnt = 0;
		}	
	}
	return ChkFlg;
}

////  保存 超载记录 信息 到EEPROM (AT24C256)  中
void Sys_Save_OverHInfo(unsigned char vSite)
{
	unsigned int i = 0;
	unsigned char  k = 0;
	unsigned char  iEEAck = 0;
	unsigned short vChk = 0;

	gLenE = sizeof(gOvHandInfo);
	for(k=0; k<2; k++)
	{
		//gLenE = SYSLEN_SIZE;
		gSSite = (EX_EEPROM_SIZE>>1)*k + EX_OV_H_INFO_STADDR + vSite * EX_OV_H_INFO_SIZE;
		vChk = 0;
		pL = (unsigned char*)(&gOvHandInfo);
		for(i=0; i<gLenE-1; i++)
		{
			vChk += *pL;
			pL++;
		}
		gOvHandInfo.vChkSum = vChk;
		pL = (unsigned char*)(&gOvHandInfo);
		i = AT24_Write_Page((gSSite), pL, gLenE);
		iEEAck += (i<<gk);
	}
}
////  读取存储在EEPROM (AT24C256) 中的 超载记录 信息
unsigned char  Sys_Read_OverHInfo(unsigned char vSite)
{
	unsigned short i = 0;
	unsigned char  j = 0;
	unsigned char  ChkFlg = 0;
	unsigned char  vExEEprom = 0;
	unsigned char  vRdCnt = 0;
	unsigned char  Flg2 = 0;

	ChkFlg = 0;
	gLenE = sizeof(gOvHandRInfo);
	for(j=0; j<2; )
	{
		gSSite = (EX_EEPROM_SIZE>>1)*j + EX_OV_H_INFO_STADDR + vSite * EX_OV_H_INFO_SIZE;
		pL = (unsigned char*)(&gOvHandRInfo);

		vExEEprom = AT24_Read_Str((gSSite), pL, gLenE);
		pL = (unsigned char*)(&gOvHandRInfo);
		gChkE = 0;
		for(i=0; i<gLenE-1; i++)
		{
			gChkE += *pL;
			pL++;
		}
		if((unsigned char)gChkE == gOvHandRInfo.vChkSum)
		{
			if((gOvHandRInfo.vHead[0]== SYSINFO_HEAD0)&& (gOvHandRInfo.vHead[1]== SYSINFO_HEAD1))
				Flg2 = 1;
		}

		vRdCnt++;
		if(vExEEprom && Flg2)
		{
			return 0;
		}	
		if(vRdCnt >= 0x03)
		{
			ChkFlg |= (1<<j);	
			j++;
			vRdCnt = 0;
		}	
	}
	return ChkFlg;
}

void Over_Table_Init(void)
{
	unsigned char *pStr = (unsigned char*)(&gOverTable);
	unsigned char i = 0;
	unsigned char vLen = 0;
	unsigned short vChk = 0;

	gOverTable[0].vHead[0] = SYSINFO_HEAD0;
	gOverTable[0].vHead[1] = SYSINFO_HEAD1;
	gOverTable[0].vTotal = 0;
	gOverTable[0].vSite = 99; 				// 
	gOverTable[0].vSite2 = 0;

	vLen = sizeof(gOverTable[0]);
	for(i=0; i<vLen-1; i++)
	{
		vChk += *pStr;
		pStr++;
	}
	gOverTable[0].vChkSum = (unsigned char)vChk;

}

////  保存 超载表 信息 到EEPROM (AT24C256)  中
void Sys_Save_OverT(void)
{
	unsigned int i = 0;
	unsigned char  k = 0;
	unsigned char  iEEAck = 0;
	unsigned short vChk = 0;

	gLenE = sizeof(gOverTable[0]);
	memcpy((unsigned char*)(&gOverTable[1]), (unsigned char*)(&gOverTable[0]), gLenE);
	for(k=0; k<2; k++)
	{
		//gLenE = SYSLEN_SIZE;
		gSSite = (EX_EEPROM_SIZE>>1)*k + EX_OVERTABLE_STADDR;
		vChk = 0;
		pL = (unsigned char*)(&gOverTable[k]);
		for(i=0; i<gLenE-1; i++)
		{
			vChk += *pL;
			pL++;
		}
		gOverTable[k].vChkSum = vChk;
		pL = (unsigned char*)(&gOverTable[k]);
		i = AT24_Write_Page((gSSite), pL, gLenE);
		iEEAck += (i<<gk);
	}

}

////  读取存储在EEPROM (AT24C256) 中的 超载表 信息
unsigned char  Sys_Read_OverT(void)
{
	unsigned short i = 0;
	unsigned char  j = 0;
	unsigned char  ChkFlg = 0;
	unsigned char  vExEEprom = 0;
	unsigned char  vRdCnt = 0;
	unsigned char  Flg2 = 0;

	ChkFlg = 0;
	gLenE = sizeof(gOverTable);
	for(j=0; j<2; )
	{
		gSSite = (EX_EEPROM_SIZE>>1)*j + EX_OVERTABLE_STADDR;
		pL = (unsigned char*)(&gOverTable[j]);

		vExEEprom = AT24_Read_Str((gSSite), pL, gLenE);
		pL = (unsigned char*)(&gOverTable[j]);
		gChkE = 0;
		for(i=0; i<gLenE-1; i++)
		{
			gChkE += *pL;
			pL++;
		}
		if((unsigned char)gChkE == gOverTable[j].vChkSum)
		{
			if((gOverTable[j].vHead[0]== SYSINFO_HEAD0)&& (gOverTable[j].vHead[1]== SYSINFO_HEAD1))
				Flg2 = 1;
		}

		vRdCnt++;
		if(vExEEprom && Flg2)
		{
			if((unsigned char)gChkE != gOverTable[j].vChkSum)
				ChkFlg |= (1<<j);	
			j++;
			vRdCnt = 0;
		}	
		if(vRdCnt >= 0x03)
		{
			ChkFlg |= (1<<j);	
			j++;
			vRdCnt = 0;
		}	
	}
	switch(ChkFlg)
	{
		case 2: 			// gOverTable[0] is OK
			memcpy((unsigned char*)(&gOverTable[1]), (unsigned char*)(&gOverTable[0]), gLenE);
			Sys_Save_OverT();
			break;
		case 1:				// gOverTable[1] is OK
			memcpy((unsigned char*)(&gOverTable[0]), (unsigned char*)(&gOverTable[1]), gLenE);
			Sys_Save_OverT();
			break;
		case 0: 			// gSysInfo[0], gSysInfo[1] is OK
			break;
		case 3:				// gSysInfo[0], gSysInfo[[1] error
			Over_Table_Init();
			Sys_Save_OverT();
			break;
		default:
			break;
	}
	return ChkFlg;
}

void Over_HTable_Init(void)
{
	unsigned char *pStr = (unsigned char*)(&gOvHandTable);
	unsigned char i = 0;
	unsigned char vLen = 0;
	unsigned short vChk = 0;

	gOvHandTable[0].vHead[0] = SYSINFO_HEAD0;
	gOvHandTable[0].vHead[1] = SYSINFO_HEAD1;
	gOvHandTable[0].vTotal = 0;
	gOvHandTable[0].vSite = 0; 				// 0--125K, 1--250K, 2--500K
	gOvHandTable[0].vSite2 = 0;

	vLen = sizeof(gOvHandTable[0]);
	for(i=0; i<vLen-1; i++)
	{
		vChk += *pStr;
		pStr++;
	}
	gOvHandTable[0].vChkSum = (unsigned char)vChk;

}

////  保存 超载解除操作表 信息 到EEPROM (AT24C256)  中
void Sys_Save_OverHT(void)
{
	unsigned int i = 0;
	unsigned char  k = 0;
	unsigned char  iEEAck = 0;
	unsigned short vChk = 0;

	gLenE = sizeof(gOvHandTable[0]);
	memcpy((unsigned char*)(&gOvHandTable[1]), (unsigned char*)(&gOvHandTable[0]), gLenE);
	for(k=0; k<2; k++)
	{
		//gLenE = SYSLEN_SIZE;
		gSSite = (EX_EEPROM_SIZE>>1)*k + EX_OV_H_TABLE_STADDR;
		vChk = 0;
		pL = (unsigned char*)(&gOvHandTable[k]);
		for(i=0; i<gLenE-1; i++)
		{
			vChk += *pL;
			pL++;
		}
		gOvHandTable[k].vChkSum = vChk;
		pL = (unsigned char*)(&gOvHandTable[k]);
		i = AT24_Write_Page((gSSite), pL, gLenE);
		iEEAck += (i<<gk);
	}

}

////  读取存储在EEPROM (AT24C256) 中的 超载解除操作表 信息
unsigned char  Sys_Read_OverHT(void)
{
	unsigned short i = 0;
	unsigned char  j = 0;
	unsigned char  ChkFlg = 0;
	unsigned char  vExEEprom = 0;
	unsigned char  vRdCnt = 0;
	unsigned char  Flg2 = 0;

	ChkFlg = 0;
	gLenE = sizeof(gOvHandTable);
	for(j=0; j<2; )
	{
		gSSite = (EX_EEPROM_SIZE>>1)*j + EX_OV_H_TABLE_STADDR;
		pL = (unsigned char*)(&gOvHandTable[j]);

		vExEEprom = AT24_Read_Str((gSSite), pL, gLenE);
		pL = (unsigned char*)(&gOvHandTable[j]);
		gChkE = 0;
		for(i=0; i<gLenE-1; i++)
		{
			gChkE += *pL;
			pL++;
		}
		if((unsigned char)gChkE == gOvHandTable[j].vChkSum)
		{
			if((gOvHandTable[j].vHead[0]== SYSINFO_HEAD0)&& (gOvHandTable[j].vHead[1]== SYSINFO_HEAD1))
				Flg2 = 1;
		}

		vRdCnt++;
		if(vExEEprom && Flg2)
		{
			if((unsigned char)gChkE != gOvHandTable[j].vChkSum)
				ChkFlg |= (1<<j);	
			j++;
			vRdCnt = 0;
		}	
		if(vRdCnt >= 0x03)
		{
			ChkFlg |= (1<<j);	
			j++;
			vRdCnt = 0;
		}	
	}
	switch(ChkFlg)
	{
		case 2: 			// gOverTable[0] is OK
			memcpy((unsigned char*)(&gOvHandTable[1]), (unsigned char*)(&gOvHandTable[0]), gLenE);
			Sys_Save_OverHT();
			break;
		case 1:				// gOverTable[1] is OK
			memcpy((unsigned char*)(&gOvHandTable[0]), (unsigned char*)(&gOvHandTable[1]), gLenE);
			Sys_Save_OverHT();
			break;
		case 0: 			// gSysInfo[0], gSysInfo[1] is OK
			break;
		case 3:				// gSysInfo[0], gSysI96nfo[[1] error
			Over_HTable_Init();
			Sys_Save_OverHT();
			break;
		default:
			break;
	}
	return ChkFlg;
}


/****************************************************************************************
****************************************************************************************
* 时间: 2014-11-24
* 名称：	Uart1_Prog()
* 功能：			UART1 接收处理函数，LCD
* 无线帧结构： 
* 入口参数：
* 出口参数：
****************************************************************************************
****************************************************************************************/
uchar DGUS_GetData(unsigned short vCmd, unsigned int vAddr, unsigned int vData)
{
	switch(vAddr)
	{
		case 0x200:
			
			break;
		case 0x201:
			
			break;
	}
	return 0;
}

uchar Uart1_Prog(void)
{
	uchar i=0;
	uchar vULen = 0;		// UART 接收字符数
	uchar vRLen = 0;		// DGUS 屏返回数据字段长度
	uchar vFlg = 0xff;
	uchar vUStr[128];

	//gUART1RcvFlg = 0;
	vULen = UART1Count;
	for(i=0; i<vULen; i++)
	{
		if(UART1Buffer[i] == LCD_H1)
		{
			if(UART1Buffer[i+1] == LCD_H2)		 // 找到帧头
			{
				vFlg = i;
				break;
			}
		}	 
	}
	if(vFlg < vULen)							 // 检查长度，拷贝数据
	{
		for(i=0; i<vULen; i++)
			vUStr[i] = UART1Buffer[vFlg+i];
	}
	else
		return 0;
	i=2;
	vRLen = vUStr[i++];
	if((vFlg + vRLen + 2) > vULen)		// 接收数据的长度不够
		return 0;

	UART1Count = 0;
	gDGUSRtnCmd = vUStr[i++];
	if(	gDGUSRtnCmd == 0x81)			// 读寄存器的DGUS屏应答， ADR--1B，长度为字节（1Byte）
	{
		gDGUSRtnAddr = vUStr[i++];
		gDGUSRtnLen = vUStr[i++];
		if(gDGUSRtnLen == 1)			// 读一个字节
			gDGUSRtnData = vUStr[i++];
		else if(gDGUSRtnLen == 2)		// 读两个字节
		{
			gDGUSRtnData = vUStr[i++];
			gDGUSRtnData = (gDGUSRtnData<<8) + vUStr[i++];
		}
	}
	if(	gDGUSRtnCmd == 0x83)			// 读变量存储区的DGUS屏应答， ADR--2B，长度为字（2Byte）
	{
		gDGUSRtnAddr = vUStr[i++];
		gDGUSRtnAddr = (gDGUSRtnAddr<<8) + vUStr[i++];
		gDGUSRtnLen = vUStr[i++];
		if(gDGUSRtnLen == 1)			// 读数据的长度，单位：字，2Byte
		{
			gDGUSRtnData = vUStr[i++];
			gDGUSRtnData = (gDGUSRtnData<<8) + vUStr[i++];
		}
		else if(gDGUSRtnLen == 2)		// 读取长度为 2 Word
		{
			gDGUSRtnData = vUStr[i++];
			gDGUSRtnData = (gDGUSRtnData<<8) + vUStr[i++];
			gDGUSRtnData = (gDGUSRtnData<<8) + vUStr[i++];
			gDGUSRtnData = (gDGUSRtnData<<8) + vUStr[i++];
		}
	}

	DGUS_GetData(gDGUSRtnCmd, gDGUSRtnAddr, gDGUSRtnData);

	return gDGUSRtnCmd;
}

// UART1Buffer[UART1Count]

