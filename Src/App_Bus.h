/********************************************************************
*  
*  APP_BUS  与 总线 有关的功能函数  IIC SPI CAN UART 等
*  功能：
*
********************************************************************/

#ifndef __APP_BUS_H_
#define __APP_BUS_H_


#include "config.h"



/****************************************************************************************/

///////////////////////////////     IIC 器件地址	 ////////////////////////////										  
#define AT_24_ADDR0			0x06
#define AT_24_ADDR1			0x02
#define X1226_ADDR			0xde	//0xDE

#define AT24C_WRCMD  		0xA0
#define AT24C_RDCMD   		0xA1

///////////////////////////////     外部 EEPROM 空间分配	 ////////////////////////////										  

//#define  EX_STCNT_SITE  	50000
#define  EX_EEPROM_SIZE			0x8000		// 32 KB，平分为 2 块  243C256
#define  EX_SYSINFO_STADDR		0x10		// SysInfo --- 8 Byte
//#define  EX_SYSINFO_STADDR		0x00		// SysInfo --- 8 Byte
#define  EX_OVERTABLE_STADDR	0x20		// 超载信息表 OverInfoTable --- 8 Byte
#define  EX_OV_H_TABLE_STADDR	0x30		// 超载解除按钮操作表OvHandInfoTable --- 8 Byte
#define  EX_WT_STADDR			(0x60)		// worktime ---- 12Byte

#define  EX_BDINFO_STADDR		0x80		// BDInfo1 ---- 64 Byte
#define  EX_BDINFO_STADDR2		(EX_BDINFO_STADDR+0x80)		// BDInfo2 ---- 64 Byte

#define  EX_OVER_INFO_STADDR	0x200		// 超载信息存储起始位置
#define  EX_OVER_INFO_SIZE		32			// 超载信息存储长度，50条记录
#define  EX_OV_H_INFO_STADDR	0x1200		// 超载解除操作存储起始位置
#define  EX_OV_H_INFO_SIZE		16//24		// 超载解除操作存储长度，100条记录

#define  OVER_INFO_NUM			50			// 记录的最大超载次数, < 128 
#define  PAGE_INFO_NUM			5			// 每页显示的记录条数

///////////////////////////////    LCD    
#define  LCD_H1					0x5A
#define  LCD_H2					0xA5


#define  SYSINFO_HEAD0			0x5b
#if _ARM_XY	
	#if _ARM_PG	
		#define  SYSINFO_HEAD1			0xa5				// 0xa5 -- 电流型， 0xaa -- 电压型 
	#else
		#define  SYSINFO_HEAD1			0xa5				// 0xa5 -- 电流型， 0xaa -- 电压型 
	#endif
#else
		#define  SYSINFO_HEAD1			0xb5				// 0xa5 -- 电流型， 0xaa -- 电压型 
#endif

#if _FA_DFS
	#define  SYSINFO2_HEAD0			0x6a
	#define  SYSINFO2_HEAD1			0xa5
#else
	#if _FA_DO
		#define  SYSINFO2_HEAD0			0x6a
		#define  SYSINFO2_HEAD1			0xa8
	#else
		#define  SYSINFO2_HEAD0			0x6a
		#define  SYSINFO2_HEAD1			0xa9
	#endif
#endif

typedef struct
{
	unsigned char vHead[2];
	unsigned char vID;
	unsigned char vBps;

	unsigned char vCycle;				// 发送周期 n * 10 ms
	unsigned char vMType;				// 机型
	unsigned char vVer;					// 版本

	unsigned char vChkSum;
}*pSysInfo,SysInfo;

////  超载信息存储记录表
typedef struct
{
	unsigned char  vHead[2];
	unsigned char  vTotal;					// 总数，<=100
	unsigned char  vSite;					// 当前位置，< vTotal
	unsigned char  vSite2;					// 当前位置2，备用，< vTotal
	unsigned char  vChkSum;
}*pOverTable, mOverTable;

////  超载信息
typedef struct
{
	unsigned char  vHead[2];
	unsigned char  vDTime[6];				// 日期时间
	unsigned short vType;					// 报警字
	unsigned short vFengSu;					// 风速 * 10   <1000
	unsigned short vArc;					// 角度 * 10   <1200
	unsigned short vScope;					// 幅度 * 10   <1000
	unsigned short vEWeight;				// 额重 * 10   <1000
	unsigned short vRWeight;				// 实重 * 10   <1000
	unsigned long  vOvTimes;				// 持续时间 S
	unsigned char  vLiju;					// 力矩
	unsigned char  vFlg;					// 进度标记，1--开始，2--结束
	unsigned char  vrSV;					// 
	unsigned char  vChkSum;					// 校验
}*pOverInfo, mOverInfo;

////  超载解除按钮信息
typedef struct
{
	unsigned char  vHead[2];
	unsigned char  vDTimeS[6];				// 起始日期时间
	unsigned char  vDTimeE[6];				// 结束日期时间
	unsigned char  vFLg;					// 进度标记，1--开始，2--结束
	unsigned char  vChkSum;					// 校验
}*pOvHandInfo, mOvHandInfo;

////  系统时间
typedef struct 
{
	unsigned char  vSec;				// 秒
	unsigned char  vMin;				// 分
	unsigned char  vHour;				// 时
	unsigned char  vDay;				// 日
	unsigned char  vMon;				// 月
	unsigned char  vYear;				// 年
	unsigned char  vWeek;				// 年
	unsigned char  vY2K;				// 世纪，19/ 20 h
}*pSysTime, SysTime;

////  存储信息
typedef struct
{
	unsigned short vSNum;				// 存储的序号
	unsigned short vHollSum;			// HOLL 变化的累计值
	unsigned short vHollSumTst;			// HOLL 变化的累计值
	unsigned char  vCyc;				// 圈数
	unsigned short vADLen;				// 最后的AD 值
	unsigned char  vChkSum;					// 校验
}*pSaveInfo, SaveInfo;


extern SysInfo gSysInfo[2];
extern mOverTable gOverTable[2];			// 超载信息记录表
extern mOverTable gOvHandTable[2];			// 超载解除按钮记录表
extern mOverInfo  gOverInfo;			// 超载信息结构
extern mOvHandInfo gOvHandInfo;			// 超载解除信息结构
extern mOverInfo  gOverRInfo;				// 读取‘超载信息’用
extern mOvHandInfo gOvHandRInfo;			// 读取‘超载限制按钮操作’用


extern unsigned int gSSite ;
extern unsigned int gLenE ;
extern unsigned short gChkE ;
// UART0 -- 无线
extern unsigned int gUartDType ;		// 无线（遥控器）过来的数据类型：1~4
extern unsigned char gk ;
extern unsigned char* pL;

extern unsigned char gID;
extern unsigned int  gCanSCycle;
extern SysTime  gSysTime;
extern SysTime  gSysTime0;
extern uchar x12026_st ;


/****************************************************************************************/


uchar I2C_Write_B(uchar device,uint address,uchar bytedata);//向slave写入1字节数据 
uchar I2C_Read_B(uchar device,uint address);

uchar ReadRTC_ST(void); 			   //读时钟状态 
uchar ReadRTC(void); 			   //读时钟 
uchar WriteRTC(uchar nian,uchar yue,uchar ri,uchar shi,uchar fen,uchar miao); //写时钟 
void x12027_chk(void);

unsigned char AT24_Read_Str(unsigned int byte_addr, unsigned char *buff, unsigned char num);
unsigned char AT24_Write_Page(unsigned int byte_addr, unsigned char *buff, unsigned char num);
//unsigned char  Sys_Read_Len(void);
//void Sys_Write_Len(unsigned char vPowerFlg);

unsigned char  Sys_Read_BD2(void);
void BD_Patam_Init(void);
void Sys_Param_Init(unsigned char vID);
unsigned char  Sys_Read_BD(void);
void Sys_Write_BD(void);
unsigned char  Sys_Read_WT(void);
void Sys_Write_WT(void);
void Sys_Write_BD2(void);

unsigned char  Sys_Read_Info(void);
void Sys_Save_Info(void);

//void Sys_Save_Over(uchar* vPStr, uint vSite, uchar vLen);
//unsigned char  Sys_Read_Over(uchar* vPStr, uint vSite, uchar vLen);
void Sys_Save_OverHT(void);
unsigned char  Sys_Read_OverHT(void);
void Sys_Save_OverT(void);
unsigned char  Sys_Read_OverT(void);
void Sys_Save_OverInfo(unsigned char vSite);
unsigned char  Sys_Read_OverInfo(unsigned char vSite);
void Sys_Save_OverHInfo(unsigned char vSite);
unsigned char  Sys_Read_OverHInfo(unsigned char vSite);

uchar Uart0_WL_Rcv(void);
void  Uart0_WL_Send(void);
void Uart0_WL_KYL(uchar vCmd, uchar vData);
uchar Uart0_WL_KYL_Rcv(void);
void Uart0_WL_SendAI(unsigned char vX);

uchar Uart1_Prog(void);
unsigned long AT24_Read_ULong(unsigned int byte_addr);
unsigned char AT24_Write_ULong(unsigned int byte_addr, unsigned long wData);

unsigned char AT24_Write1_Page(unsigned int byte_addr, unsigned char *buff, unsigned char num);
unsigned char AT24_Read1_Str(unsigned int byte_addr, unsigned char *buff, unsigned char num);
unsigned long AT24_Read1_ULong(unsigned int byte_addr);
unsigned char AT24_Write1_ULong(unsigned int byte_addr, unsigned long wData);



#endif  //__APP_BUS_H_
