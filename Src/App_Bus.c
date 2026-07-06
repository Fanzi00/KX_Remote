/****************************************************************************
*
*	 APP_BUS 与 总线、接口 有关的函数  IIC SPI CAN UART  LCD 等
*
*
*****************************************************************************/

#include "app_Bus.h"
#include "LPC17xx.h"
#include "config.h"


//extern CAN_MSG MsgBuf_RX1[ID_RCV_NUM], MsgBuf_RX2;


SysInfo gSysInfo[2];
mBDParam2 gBDParam2[2];

/****************************************************************************************/
////////   IIC 
// RTC
SysTime  gSysTime;
SysTime  gSysTime0;

uchar x_year=1;
uchar x_month=2;
uchar x_day=3;
uchar x_hour=4;
uchar x_minute=5;
uchar x_second=6; 
uchar x_y2k = 1;
uchar x_week = 2;
uchar x12026_st = 0;


unsigned char lpFlg = 0;
unsigned char gk = 0;
unsigned char* pL;

// EX_EEPROM
unsigned int gSSite = 0;
unsigned int gLenE = 0;
unsigned short gChkE = 0;


void Delay(uint t) 
{ 
    uint i; 
    for(;t>0;t--) 
        for(i=0;i<19206;i++) 
            ;                         
} 
unsigned char BCD2HEX(unsigned char val)  //BCD转换为Byte 
{ 
	unsigned char i; 
	i= val&0x0f; 
	val >>= 4; 
	val &= 0x0f; 
	val *= 10; 
	i += val; 

	return i; 
} 

unsigned char bbcd(unsigned char i)//单字节B码转bcd码子程序 
{ 
	unsigned char a,b; 
	a=i/10; 
	b=i%10; 
	return(a*16+b); 
} 



/****************************************************************************************
****************************************************************************************
* 时间: 2014-11-24
* 名称：
* 功能：			时钟 X1226 相关函数
* 入口参数：
* 出口参数：
****************************************************************************************
****************************************************************************************/
uchar ReadRTC_ST(void) 			   //读时钟状态 
{
	short vRd = 0;

	vRd = I2C_Read_B(X1226_ADDR, 0x003f);
	if(vRd == -1)
		return 1;
	x12026_st = vRd;
	vRd = I2C_Read_B(X1226_ADDR, 0x0037);
	if(vRd == -1)
		return 1;
	x_y2k = vRd;
	return 0;
}
uchar ReadRTC(void) 			   //读时钟 
{
	unsigned char vRTCStr[10];
	unsigned char i=0;

	I2C_Read_N(X1226_ADDR, 0x0030, vRTCStr, 8);
	i=0;
	gSysTime.vSec = BCD2HEX(vRTCStr[i++]);
	gSysTime.vMin = BCD2HEX(vRTCStr[i++]); 
	gSysTime.vHour = BCD2HEX(vRTCStr[i++]&0x7f); 
	gSysTime.vDay = BCD2HEX(vRTCStr[i++]); 
	gSysTime.vMon = BCD2HEX(vRTCStr[i++]);
	gSysTime.vYear = BCD2HEX(vRTCStr[i++]);
	gSysTime.vWeek = BCD2HEX(vRTCStr[i++]);
	gSysTime.vY2K = BCD2HEX(vRTCStr[i++]);
	return 0;
}
//************************实数写入，带格式转换 
uchar WriteRTC(uchar nian,uchar yue,uchar ri,uchar shi,uchar fen,uchar miao) //写时钟 
{  
	uchar clock_temp; 
	uchar errCnt = 0;
//	uchar wStr[50];
//WP_L 

	//wStr[i++] = 0x02;
	errCnt += I2C_Write_B(0xDE,0x003F,0x02); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x06); 
	clock_temp=0; 
	errCnt += I2C_Write_B(0xDE,0x0010,clock_temp); 
	  
	Delay(10); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x02); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x06); 
	clock_temp=0x98; 
	errCnt += I2C_Write_B(0xDE,0x0011,clock_temp); 
	  
	Delay(10); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x02); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x06); 
	clock_temp=0; 
	errCnt += I2C_Write_B(0xDE,0x0012,clock_temp); 
	  
	Delay(10); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x02); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x06); 
	clock_temp=0; 
	errCnt += I2C_Write_B(0xDE,0x0013,clock_temp); 

	Delay(10); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x02); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x06);  
	clock_temp=bbcd(miao); 
	errCnt += I2C_Write_B(0xDE,0x0030,clock_temp);//sec 
	  
	Delay(6); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x02); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x06); 
	clock_temp=bbcd(fen);  
	errCnt += I2C_Write_B(0xDE,0x0031,clock_temp);    //min 
	  
	Delay(6); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x02); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x06); 
	clock_temp=bbcd(shi); 
	errCnt += I2C_Write_B(0xDE,0x0032,clock_temp + 0x80);//hou 24小时制 
	  
	Delay(6); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x02); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x06); 
	clock_temp=bbcd(ri); 
	errCnt += I2C_Write_B(0xDE,0x0033,clock_temp);     //date 
	  
	Delay(6); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x02); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x06); 
	clock_temp=bbcd(yue); 
	errCnt += I2C_Write_B(0xDE,0x0034,clock_temp);    //x_month 
	  
	Delay(6); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x02); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x06);  
	clock_temp=bbcd(nian); 
	errCnt += I2C_Write_B(0xDE,0x0035,clock_temp); //x_year 
	  
	Delay(6); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x02); 
	errCnt += I2C_Write_B(0xDE,0x003F,0x06); 
	//clock_temp=bbcd(0x20);  
	errCnt += I2C_Write_B(0xDE,0x0037,0x20); //2000 
//WP_H 

	return errCnt;

} 
void x12027_chk(void)
{
	ReadRTC_ST();
	//IIC_Stop(); //发送STOP 信号		   
	delay_ms(5); 
	ReadRTC();
	x_year = gSysTime.vYear;
	x_month = gSysTime.vMon;
	x_day = gSysTime.vDay;
	x_hour = gSysTime.vHour;
	x_minute = gSysTime.vMin;
	x_second = gSysTime.vSec;

	ReadRTC_ST();
	if((x12026_st & 0x01) == 0x01)		// 时钟失效
	{
		if(x_year == 0)			// 没有设置时间，设默认时间
			WriteRTC(15,5,1,8,8,8);
		else
			WriteRTC(x_year,x_month,x_day,x_hour,x_minute,x_second);
		delay_ms(5); 
	}
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
unsigned char AT24_Read_Str(unsigned int byte_addr, unsigned char *buff, unsigned char num)
{
#if 1
    unsigned char i;
	byte_addr &= 0x7fff;
	i = I2C_Read_N(AT24C_WRCMD + AT_24_ADDR0, byte_addr, buff, num);
#endif
	return i;

}

unsigned char AT24_Write_Page(unsigned int byte_addr, unsigned char *buff, unsigned char num)
{
    unsigned char i;
	//unsigned char ack = 0;
WP_L 

	byte_addr &= 0x7fff;
	i = I2C_Write_N(AT24C_WRCMD + AT_24_ADDR0, byte_addr, buff, num);
	delay_ms(8);
WP_H
	return i;
}

unsigned long AT24_Read_ULong(unsigned int byte_addr)
{
    unsigned long i=0;
	unsigned char vStr[6];
	vStr[0] = 0;
	vStr[1] = 0;
	vStr[2] = 0;
	vStr[3] = 0;
	byte_addr &= 0x7fff;
	I2C_Read_N(AT24C_WRCMD + AT_24_ADDR0, byte_addr, vStr, 4);
	i = (vStr[0]<<24)+ (vStr[1]<<16)+(vStr[2]<<8)+ vStr[3];
	return i;

}
unsigned char AT24_Write_ULong(unsigned int byte_addr, unsigned long wData)
{
	unsigned char i = 0;
	unsigned char vStr[6];
WP_L 
	vStr[0] = wData>>24;
	vStr[1] = wData>>16;
	vStr[2] = wData>>8;
	vStr[3] = wData;
	vStr[4] = 0;
	byte_addr &= 0x7fff;
	i = I2C_Write_N(AT24C_WRCMD + AT_24_ADDR0, byte_addr, vStr, 4);
	delay_ms(8);
WP_H
	return i;
}



/****************************************************************************************
*
* 时间: 2014-11-24
* 名称：			标定信息的保存和读取
* 功能：			EEPROM 24C256 中按 数据结构 读写函数
* 入口参数：
* 出口参数：
*
*****************************************************************************************/


void Sys_Param_Init(unsigned char vID)
{
	unsigned char *pStr = (unsigned char*)(&gSysInfo);
	unsigned char i = 0;
	unsigned char vLen = 0;
	unsigned short vChk = 0;

	gSysInfo[0].vHead[0] = SYSINFO_HEAD0;
	gSysInfo[0].vHead[1] = SYSINFO_HEAD1;
	gSysInfo[0].vID = vID;
	gSysInfo[0].vBps = 1; 				// 0--125K, 1--250K, 2--500K
	gSysInfo[0].vCycle = 10;

	gSysInfo[0].vMType = 0;
	gSysInfo[0].vVer = 20;
	gSysInfo[0].vChkSum = 0;
	vLen = sizeof(gSysInfo[0]);
	for(i=0; i<vLen-1; i++)
	{
		vChk += *pStr;
		pStr++;
	}
	gSysInfo[0].vChkSum = (unsigned char)vChk;
}

void WT_Patam_Init(void)
{
//	uchar i = 0;

	memset((unsigned char*)(&gWorkTime[0]), 0, sizeof(gWorkTime[0]));

	gWorkTime[0].vHead[0]= SYSINFO_HEAD0;
	gWorkTime[0].vHead[1]= SYSINFO_HEAD1;

	gWorkTime[0].vRsv = 0;
	gWorkTime[0].vWorkTime = 0;
	gWorkTime[0].vWorkTime2 = 0;
	memcpy((unsigned char*)(&gWorkTime[1]), (unsigned char*)(&gWorkTime[0]), sizeof(gWorkTime[0]));

}
	 unsigned char gWkTimeSave = 0;
void Sys_Write_WT(void)
{
	unsigned int i = 0;
	unsigned char  k = 0;
	unsigned char  iEEAck = 0;
	unsigned int  vSSite = 0;

	gLenE = sizeof(gWorkTime[0]);
	memcpy((unsigned char*)(&gWorkTime[1]), (unsigned char*)(&gWorkTime[0]), gLenE);
	if(gWkTimeSave == 0)
	{
		for(k=0; k<2; k++)
		{
			//gLenE = SYSLEN_SIZE;
			gSSite = (EX_EEPROM_SIZE>>1)*k + EX_WT_STADDR;
			gWorkTime[k].vChkSum = 0;
			pL = (unsigned char*)(&gWorkTime[k]);
			for(i=0; i<gLenE-2; i++)
			{
				gWorkTime[k].vChkSum += *pL;
				pL++;
			}
			pL = (unsigned char*)(&gWorkTime[k]);
			i = AT24_Write_Page((gSSite), pL, gLenE);
			iEEAck += (i<<gk);
		}
	}
	else if(gWkTimeSave == 1)
	{
	 	for(k=0; k<2; k++)
		{
			//gLenE = SYSLEN_SIZE;
			vSSite = (EX_EEPROM_SIZE>>1) - EX_WT_STADDR - 0x80;
	
			gSSite = (EX_EEPROM_SIZE>>1)*k + EX_WT_STADDR + vSSite;
			gWorkTime[k].vChkSum = 0;
			pL = (unsigned char*)(&gWorkTime[k]);
			for(i=0; i<gLenE-2; i++)
			{
				gWorkTime[k].vChkSum += *pL;
				pL++;
			}
			pL = (unsigned char*)(&gWorkTime[k]);
			i = AT24_Write_Page((gSSite), pL, gLenE);
			iEEAck += (i<<gk);
		}
	}
	 gWkTimeSave ^= 1;
}

////  读取存储在EEPROM (AT24C256) 中的信息		gWorkTimeRd
unsigned char  Sys_Read_WT(void)
{
	unsigned short i = 0;
	unsigned char  j = 0;
	unsigned char  ChkFlg = 0;
	unsigned char  ChkFlg2 = 0;
	unsigned char  vExEEprom = 0;
	unsigned char  vRdCnt = 0;
	unsigned char  Flg2 = 0;
	unsigned int  vSSite = 0;

	ChkFlg = 0;
	gLenE = sizeof(gWorkTime[0]);
	for(j=0; j<2; )
	{
				Flg2 = 0;
		gSSite = (EX_EEPROM_SIZE>>1)*j + EX_WT_STADDR;
		pL = (unsigned char*)(&gWorkTime[j]);

		vExEEprom = AT24_Read_Str((gSSite), pL, gLenE);
		pL = (unsigned char*)(&gWorkTime[j]);
		gChkE = 0;
		for(i=0; i<gLenE-2; i++)
		{
			gChkE += *pL;
			pL++;
		}
		if(gChkE == gWorkTime[j].vChkSum)
		{
			if((gWorkTime[j].vHead[0]== SYSINFO_HEAD0)&&(gWorkTime[j].vHead[1]== SYSINFO_HEAD1))
				Flg2 = 1;
		}

		vRdCnt++;
		if(vExEEprom && Flg2)
		{
			if(gChkE != gWorkTime[j].vChkSum)
				ChkFlg |= (1<<j);	
			j++;
			vRdCnt = 0;
		}	
		if(vRdCnt >= 0x02)
		{
			ChkFlg |= (1<<j);	
			j++;
			vRdCnt = 0;
		}	
		//delay_xms(1);
	}

	switch(ChkFlg)
	{
		case 2: 			// gWorkTime[0] is OK
			memcpy((unsigned char*)(&gWorkTime[1]), (unsigned char*)(&gWorkTime[0]), gLenE);
			//Sys_Write_BD2();
			break;
		case 1:				// gWorkTime[1] is OK
			memcpy((unsigned char*)(&gWorkTime[0]), (unsigned char*)(&gWorkTime[1]), gLenE);
			//Sys_Write_BD2();
			break;
		case 0: 			// gWorkTime[0], gWorkTime[1] is OK
			break;
		case 3:				// gWorkTime[0], gWorkTime[[1] error
		//	BD2_Patam_Init();
		//	Sys_Write_BD2();
			break;
		default:
			break;
	}

 			vRdCnt = 0;
	for(j=0; j<2; )
	{
				Flg2 = 0;
				vSSite = (EX_EEPROM_SIZE>>1) - EX_WT_STADDR - 0x80;
		gSSite = (EX_EEPROM_SIZE>>1)*j + EX_WT_STADDR + vSSite;
		pL = (unsigned char*)(&gWorkTimeRd[j]);

		vExEEprom = AT24_Read_Str((gSSite), pL, gLenE);
		pL = (unsigned char*)(&gWorkTimeRd[j]);
		gChkE = 0;
		for(i=0; i<gLenE-2; i++)
		{
			gChkE += *pL;
			pL++;
		}
		if(gChkE == gWorkTimeRd[j].vChkSum)
		{
			if((gWorkTimeRd[j].vHead[0]== SYSINFO_HEAD0)&&(gWorkTimeRd[j].vHead[1]== SYSINFO_HEAD1))
				Flg2 = 1;
		}

		vRdCnt++;
		if(vExEEprom && Flg2)
		{
			if(gChkE != gWorkTimeRd[j].vChkSum)
				ChkFlg2 |= (1<<j);	
			j++;
			vRdCnt = 0;
		}	
		if(vRdCnt >= 0x02)
		{
			ChkFlg |= (1<<j);	
			j++;
			vRdCnt = 0;
		}	
		//delay_xms(1);
	}

	switch(ChkFlg2)
	{
		case 2: 			// gWorkTime[0] is OK
			memcpy((unsigned char*)(&gWorkTimeRd[1]), (unsigned char*)(&gWorkTimeRd[0]), gLenE);
			//Sys_Write_BD2();
			break;
		case 1:				// gWorkTime[1] is OK
			memcpy((unsigned char*)(&gWorkTimeRd[0]), (unsigned char*)(&gWorkTimeRd[1]), gLenE);
			//Sys_Write_BD2();
			break;
		case 0: 			// gWorkTime[0], gWorkTime[1] is OK
			break;
		case 3:				// gWorkTime[0], gWorkTime[[1] error
			//BD2_Patam_Init();
			//Sys_Write_BD2();
			break;
		default:
			break;
	}
	if(	ChkFlg == 3)
	{
		if(	ChkFlg2 == 3)			 // 新、旧的都出错
		{
 			WT_Patam_Init();
			Sys_Write_WT();
		}
	}
	if(gWorkTimeRd[0].vWorkTime > gWorkTime[0].vWorkTime)	// gWorkTimeRd[0] is new
 	{
			memcpy((unsigned char*)(&gWorkTime[0]), (unsigned char*)(&gWorkTimeRd[0]), gLenE);
			memcpy((unsigned char*)(&gWorkTime[1]), (unsigned char*)(&gWorkTimeRd[0]), gLenE);
			gWkTimeSave = 0;
	}
	else
			gWkTimeSave = 1;
	return ChkFlg;
}

void Pswd_Init(void)
{
	gPswd.password[0] = 5;
	gPswd.password[1] = 1;
	gPswd.password[2] = 8;
	gPswd.password[3] = 5;
	gPswd.password[4] = 1;
	gPswd.password[5] = 8;

	gPswd.password2[0] = 8;
	gPswd.password2[1] = 0;
	gPswd.password2[2] = 2;
	gPswd.password2[3] = 1;
	gPswd.password2[4] = 6;
	gPswd.password2[5] = 8;
}
void BD_Patam_Init(void)
{
	unsigned char i = 0;
	memset((unsigned char*)(&gBDParam[0]), 0, sizeof(gBDParam[0]));

	gBDParam[0].vHead[0]= SYSINFO_HEAD0;
	gBDParam[0].vHead[1]= SYSINFO_HEAD1;
#if _ARM_XY	
		#if _ARM_PG	
			////  P+G 摇杆
			for(i=0; i<_ARM_MAX; i++)
			{
				gBDParam[0].vRockV[i*2][0] = 0x7D0;
				gBDParam[0].vRockV[i*2][1] = 0x200;
				gBDParam[0].vRockV[i*2+1][0] = 0x7D0;
				gBDParam[0].vRockV[i*2+1][1] = 0xFE0;
					gBDParam[0].vRockDS[i] = 3;
			}
		#else
			for(i=0; i<_ARM_MAX; i++)
			{
					gBDParam[0].vRockV[i*2][0] = 0x7a0;
					gBDParam[0].vRockV[i*2][1] = 0x200;
					gBDParam[0].vRockV[i*2+1][0] = 0x7a0;
					gBDParam[0].vRockV[i*2+1][1] = 0xd01;
					gBDParam[0].vRockDS[i] = 3;
			}
		#endif	
		#if _V_2					// 新的版本，旋钮到最大，采样值最小 0
			gBDParam[0].vRockV[_XN1_RV_CHN][1] = 0x40;				// 旋钮的最小值
			gBDParam[0].vRockV[_XN1_RV_CHN][0] = 0xa00;				// 旋钮的最大值
			gBDParam[0].vRockV[_XN2_RV_CHN][1] = 0x40;				// 旋钮的最小值
			gBDParam[0].vRockV[_XN2_RV_CHN][0] = 0xa00;				// 旋钮的最大值
		#else
			gBDParam[0].vRockV[_XN1_RV_CHN][0] = 0x40;				// 旋钮的最小值
			gBDParam[0].vRockV[_XN1_RV_CHN][1] = 0xa00;			// 旋钮的最大值
			gBDParam[0].vRockV[_XN2_RV_CHN][0] = 0x40;				// 旋钮的最小值
			gBDParam[0].vRockV[_XN2_RV_CHN][1] = 0xa00;			// 旋钮的最大值
		#endif
#else
		for(i=0; i<_ARM_MAX; i++)
		{
				gBDParam[0].vRockV[i*2][0] = 0x180;
				gBDParam[0].vRockV[i*2][1] = 0x880;
				gBDParam[0].vRockV[i*2+1][0] = 0x181;
				gBDParam[0].vRockV[i*2+1][1] = 0x901;
		}
		gBDParam[0].vRockV[_XN1_RV_CHN][0] = 0x40;				// 旋钮的最小值
		gBDParam[0].vRockV[_XN1_RV_CHN][1] = 0x900;			// 旋钮的最大值
#endif	

	memcpy((unsigned char*)(&gBDParam[1]), (unsigned char*)(&gBDParam[0]), sizeof(gBDParam[0]));
}



////  保存信息 到EEPROM (AT24C256)  中
void Sys_Write_BD(void)
{
	unsigned int i = 0;
	unsigned char  k = 0;
	unsigned char  iEEAck = 0;

	gLenE = sizeof(gBDParam[0]);
	memcpy((unsigned char*)(&gBDParam[1]), (unsigned char*)(&gBDParam[0]), gLenE);
	for(k=0; k<2; k++)
	{
		//gLenE = SYSLEN_SIZE;
		gSSite = (EX_EEPROM_SIZE>>1)*k + EX_BDINFO_STADDR;
		gBDParam[k].vChkSum = 0;
		pL = (unsigned char*)(&gBDParam[k]);
		for(i=0; i<gLenE-2; i++)
		{
			gBDParam[k].vChkSum += *pL;
			pL++;
		}
		pL = (unsigned char*)(&gBDParam[k]);
		i = AT24_Write_Page((gSSite), pL, gLenE);
		iEEAck += (i<<gk);
	}
}

////  读取存储在EEPROM (AT24C256) 中的信息
unsigned char  Sys_Read_BD(void)
{
	unsigned short i = 0;
	unsigned char  j = 0;
	unsigned char  ChkFlg = 0;
	unsigned char  vExEEprom = 0;
	unsigned char  vRdCnt = 0;
	unsigned char  Flg2 = 0;

	Pswd_Init();
	ChkFlg = 0;
	gLenE = sizeof(gBDParam[0]);
	for(j=0; j<2; )
	{
		gSSite = (EX_EEPROM_SIZE>>1)*j + EX_BDINFO_STADDR;
		pL = (unsigned char*)(&gBDParam[j]);

		vExEEprom = AT24_Read_Str((gSSite), pL, gLenE);
		pL = (unsigned char*)(&gBDParam[j]);
		gChkE = 0;
		for(i=0; i<gLenE-2; i++)
		{
			gChkE += *pL;
			pL++;
		}
		if(gChkE == gBDParam[j].vChkSum)
		{
			if((gBDParam[j].vHead[0]== SYSINFO_HEAD0)&&	(gBDParam[j].vHead[1]== SYSINFO_HEAD1))
				Flg2 = 1;
		}

		vRdCnt++;
		if(vExEEprom && Flg2)
		{
			if(gChkE != gBDParam[j].vChkSum)
				ChkFlg |= (1<<j);	
			j++;
			vRdCnt = 0;
		}	
		if(vRdCnt >= 0x02)
		{
			ChkFlg |= (1<<j);	
			j++;
			vRdCnt = 0;
		}	
		//delay_xms(1);
	}
//  ChkFlg = 3;
	switch(ChkFlg)
	{
		case 2: 			// gBDParam[0] is OK
			memcpy((unsigned char*)(&gBDParam[1]), (unsigned char*)(&gBDParam[0]), gLenE);
			Sys_Write_BD();
			break;
		case 1:				// gBDParam[1] is OK
			memcpy((unsigned char*)(&gBDParam[0]), (unsigned char*)(&gBDParam[1]), gLenE);
			Sys_Write_BD();
			break;
		case 0: 			// gBDParam[0], gBDParam[1] is OK
			break;
		case 3:				// gBDParam[0], gBDParam[[1] error
			BD_Patam_Init();
			Sys_Write_BD();
			break;
		default:
			break;
	}

	return ChkFlg;
}


void BD2_Patam_Init(void)
{
	unsigned char i=0;
	memset((unsigned char*)(&gBDParam2[0]), 0, sizeof(gBDParam2[0]));

	gBDParam2[0].vHead[0]= SYSINFO2_HEAD0;
	gBDParam2[0].vHead[1]= SYSINFO2_HEAD1;
	
	gBDParam2[0].vAITime = 500;
	gBDParam2[0].vVCC = 24;
	gBDParam2[0].vRes = 24;
#if _FA_DFS
	for(i=0; i<8; i++)
	{
		gBDParam2[0].vAI[0][i] = 250;			// DFS阀，中位电压 为 VCC/2
		gBDParam2[0].vAI[1][i] = 450;			// DFS阀，中位电压 为 VCC/2
		gBDParam2[0].vAI[2][i] = 550;			// DFS阀，中位电压 为 VCC/2
		gBDParam2[0].vAI[3][i] = 750;			// DFS阀，中位电压 为 VCC/2
		gBDParam2[0].vAI2[0][i] = 250;			// DFS阀，中位电压 为 VCC/2
		gBDParam2[0].vAI2[1][i] = 450;			// DFS阀，中位电压 为 VCC/2
		gBDParam2[0].vAI2[2][i] = 550;			// DFS阀，中位电压 为 VCC/2
		gBDParam2[0].vAI2[3][i] = 750;			// DFS阀，中位电压 为 VCC/2
	}

#else	
	#if _FA_DO
		for(i=0; i<_ARM_USE; i++)
		{
			gBDParam2[0].vAI[0][i] = 600;			// 最大电流
			gBDParam2[0].vAI[1][i] = 700;			// 最小电流
			gBDParam2[0].vAI[2][i] = 600;			// 最大电流
			gBDParam2[0].vAI[3][i] = 700;			// 最小电流
			gBDParam2[0].vAI2[0][i] = 600;			// 最大电流
			gBDParam2[0].vAI2[1][i] = 700;			// 最小电流
			gBDParam2[0].vAI2[2][i] = 600;			// 最大电流
			gBDParam2[0].vAI2[3][i] = 700;			// 最小电流
		}
	#else
		for(i=0; i<_ARM_USE; i++)
		{
			gBDParam2[0].vAI[0][i] = 600;			// 最大电流
			gBDParam2[0].vAI[1][i] = 200;			// 最小电流
			gBDParam2[0].vAI[2][i] = 600;			// 最大电流
			gBDParam2[0].vAI[3][i] = 200;			// 最小电流
			gBDParam2[0].vAI2[0][i] = 600;			// 最大电流
			gBDParam2[0].vAI2[1][i] = 200;			// 最小电流
			gBDParam2[0].vAI2[2][i] = 600;			// 最大电流
			gBDParam2[0].vAI2[3][i] = 200;			// 最小电流
		}
		for(i=_ARM_USE-2; i<_ARM_USE; i++)
		{
			gBDParam2[0].vAI[0][i] = 650;			// 最大电流
			gBDParam2[0].vAI[1][i] = 450;			// 最小电流
			gBDParam2[0].vAI[2][i] = 650;			// 最大电流
			gBDParam2[0].vAI[3][i] = 450;			// 最小电流
			gBDParam2[0].vAI2[0][i] = 650;			// 最大电流
			gBDParam2[0].vAI2[1][i] = 450;			// 最小电流
			gBDParam2[0].vAI2[2][i] = 650;			// 最大电流
			gBDParam2[0].vAI2[3][i] = 450;			// 最小电流
		}
	#endif
#endif	
	memcpy((unsigned char*)(&gBDParam2[1]), (unsigned char*)(&gBDParam2[0]), sizeof(gBDParam2[0]));
}



////  保存信息 到EEPROM (AT24C256)  中
void Sys_Write_BD2(void)
{
	unsigned int i = 0;
	unsigned char  k = 0;
	unsigned char  iEEAck = 0;

	gLenE = sizeof(gBDParam2[0]);
	memcpy((unsigned char*)(&gBDParam2[1]), (unsigned char*)(&gBDParam2[0]), gLenE);
	for(k=0; k<2; k++)
	{
		//gLenE = SYSLEN_SIZE;
		gSSite = (EX_EEPROM_SIZE>>1)*k + EX_BDINFO_STADDR2;
		gBDParam2[k].vChkSum = 0;
		pL = (unsigned char*)(&gBDParam2[k]);
		for(i=0; i<gLenE-2; i++)
		{
			gBDParam2[k].vChkSum += *pL;
			pL++;
		}
		pL = (unsigned char*)(&gBDParam2[k]);
		i = AT24_Write_Page((gSSite), pL, gLenE);
		iEEAck += (i<<gk);
	}
}

////  读取存储在EEPROM (AT24C256) 中的信息
unsigned char  Sys_Read_BD2(void)
{
	unsigned short i = 0;
	unsigned char  j = 0;
	unsigned char  ChkFlg = 0;
	unsigned char  vExEEprom = 0;
	unsigned char  vRdCnt = 0;
	unsigned char  Flg2 = 0;

	ChkFlg = 0;
	gLenE = sizeof(gBDParam2[0]);
	for(j=0; j<2; )
	{
		gSSite = (EX_EEPROM_SIZE>>1)*j + EX_BDINFO_STADDR2;
		pL = (unsigned char*)(&gBDParam2[j]);

		vExEEprom = AT24_Read_Str((gSSite), pL, gLenE);
		pL = (unsigned char*)(&gBDParam2[j]);
		gChkE = 0;
		for(i=0; i<gLenE-2; i++)
		{
			gChkE += *pL;
			pL++;
		}
		if(gChkE == gBDParam2[j].vChkSum)
		{
			if((gBDParam2[j].vHead[0]== SYSINFO2_HEAD0)&&	(gBDParam2[j].vHead[1]== SYSINFO2_HEAD1))
				Flg2 = 1;
		}

		vRdCnt++;
		if(vExEEprom && Flg2)
		{
			if(gChkE != gBDParam2[j].vChkSum)
				ChkFlg |= (1<<j);	
			j++;
			vRdCnt = 0;
		}	
		if(vRdCnt >= 0x02)
		{
			ChkFlg |= (1<<j);	
			j++;
			vRdCnt = 0;
		}	
		//delay_xms(1);
	}
//  ChkFlg = 3;
	switch(ChkFlg)
	{
		case 2: 			// gBDParam[0] is OK
			memcpy((unsigned char*)(&gBDParam2[1]), (unsigned char*)(&gBDParam2[0]), gLenE);
			Sys_Write_BD2();
			break;
		case 1:				// gBDParam[1] is OK
			memcpy((unsigned char*)(&gBDParam2[0]), (unsigned char*)(&gBDParam2[1]), gLenE);
			Sys_Write_BD2();
			break;
		case 0: 			// gBDParam[0], gBDParam[1] is OK
			break;
		case 3:				// gBDParam[0], gBDParam[[1] error
			BD2_Patam_Init();
			Sys_Write_BD2();
			break;
		default:
			break;
	}
	return ChkFlg;
}


/****************************************************************************************
*
* 时间: 2014-11-24
* 名称：			系统信息的保存和读取
* 功能：			EEPROM 24C256 中按 数据结构 读写函数
* 入口参数：
* 出口参数：
*
*****************************************************************************************/

////  保存信息 到EEPROM (AT24C256)  中
void Sys_Save_Info(void)
{
	unsigned int i = 0;
	unsigned char  k = 0;
	unsigned char  iEEAck = 0;
	unsigned short vChk = 0;

	gLenE = sizeof(gSysInfo[0]);
	memcpy((unsigned char*)(&gSysInfo[1]), (unsigned char*)(&gSysInfo[0]), gLenE);
	for(k=0; k<2; k++)
	{
		//gLenE = SYSLEN_SIZE;
		gSSite = (EX_EEPROM_SIZE>>1)*k + EX_SYSINFO_STADDR;
		vChk = 0;
		pL = (unsigned char*)(&gSysInfo[k]);
		for(i=0; i<gLenE-1; i++)
		{
			vChk += *pL;
			pL++;
		}
		gSysInfo[k].vChkSum	= vChk;
		pL = (unsigned char*)(&gSysInfo[k]);
		i = AT24_Write_Page((gSSite), pL, gLenE);
		iEEAck += (i<<gk);
	}
}


////  读取存储在EEPROM (AT24C256) 中的信息
unsigned char  Sys_Read_Info(void)
{
	unsigned short i = 0;
	unsigned char  j = 0;
	unsigned char  ChkFlg = 0;
	unsigned char  vExEEprom = 0;
	unsigned char  vRdCnt = 0;
	unsigned char  Flg2 = 0;

	ChkFlg = 0;
	gLenE = sizeof(gSysInfo[0]);
	for(j=0; j<2; )
	{
		gSSite = (EX_EEPROM_SIZE>>1)*j + EX_SYSINFO_STADDR;
		pL = (unsigned char*)(&gSysInfo[j]);

		vExEEprom = AT24_Read_Str((gSSite), pL, gLenE);
		pL = (unsigned char*)(&gSysInfo[j]);
		gChkE = 0;
		for(i=0; i<gLenE-1; i++)
		{
			gChkE += *pL;
			pL++;
		}
		if((unsigned char)gChkE == gSysInfo[j].vChkSum)
		{
			if((gSysInfo[j].vHead[0]== SYSINFO_HEAD0)&&	(gSysInfo[j].vHead[1]== SYSINFO_HEAD1))
				Flg2 = 1;
		}

		vRdCnt++;
		if(vExEEprom && Flg2)
		{
			if((unsigned char)gChkE != gSysInfo[j].vChkSum)
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
		//delay_xms(1);
	}
	//ChkFlg = 3;
	switch(ChkFlg)
	{
		case 2: 			// gSysInfo[0] is OK
			memcpy((unsigned char*)(&gSysInfo[1]), (unsigned char*)(&gSysInfo[0]), gLenE);
			Sys_Save_Info();
			break;
		case 1:				// gSysInfo[1] is OK
			memcpy((unsigned char*)(&gSysInfo[0]), (unsigned char*)(&gSysInfo[1]), gLenE);
			Sys_Save_Info();
			break;
		case 0: 			// gSysInfo[0], gSysInfo[1] is OK
			break;
		case 3:				// gSysInfo[0], gSysInfo[[1] error
			Sys_Param_Init(ID_DEFAULT);
			Sys_Save_Info();
			break;
		default:
			break;
	}
	gCanCyc = gSysInfo[0].vCycle * 10;
	gSysStatus |= ChkFlg;

	return ChkFlg;
}








