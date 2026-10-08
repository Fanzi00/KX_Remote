/********************************************************************
*  
*  力矩有关的功能函数
*  功能：
*
********************************************************************/

#ifndef __APP_USR_H_
#define __APP_USR_H_


#include "config.h"

#define  _V_2				1				// Ver2 新版本

#define  _SYS_YKJK	1				// 1--遥控，0--近控

#define  _ARM_XRZN		0				// 自制摇杆
#define  _ARM_XY		1				// XY型摇杆，万向摇杆
#define  _ARM_KX		1				// 康旭自定义			 
#define  _ARM_PG		0				// 是否为 P+G 摇杆
#define  _FA_DFS		0				// 是否为 DFS 阀
#define  _FA_DO			0				// 是否为 DO 阀



#define  _ARM_MAX		10				// 最大摇杆数
#define  _ARM_ROCK	4					// 摇杆数

#if _ARM_XY
	#define  _ARM_USE		(_ARM_ROCK*2)				// 现在使用的摇杆通道数
#else
	#define  _ARM_USE		(_ARM_ROCK)					// 现在使用的摇杆通道数
#endif

#if (_ARM_ROCK==4)
	#define _ARM_4R_XY		1
#else
	#define _ARM_4R_XY		0
#endif

#define  _YKQ_TYPE		"XT3:3.2GD "					// D--DO, F--DFS, H--HAW


////  ADC参数定义
#define  _V_ADC			2950			// mV, ADC采样电压
#define  _ADC_MAX		4096			// ADC 最大采样值


#define  _ARM_MID_SPACE			0x1d0	// 摇杆的中位死区
#define  _ARM_END_SPACE			0xE50	// 摇杆的末端死区

#define 	_ARM_MIDV					127
#define 	_ARM_MIDVCan			127
#define 	_ARM_gl_MIDVCan			0X3F
						   
#define  _XN_AD_CHN				9			// 旋钮所在的 AD 通道
#define  _XN1_AD_CHN			10			// 旋钮所在的 AD 通道
#define  _XN2_AD_CHN			9				// 旋钮所在的 AD 通道
#define  _XN1_RV_CHN			18			// 旋钮所在的 标定参数 通道
#define  _XN2_RV_CHN			19			// 旋钮所在的 标定参数 通道

////  密码
typedef struct 
{
	uchar   vHead[2];

	// 密码
	uchar   password[8];
	uchar   password2[8];

	ushort  vChkSum;					// 校验和
}*pPswd, mPswd;

////  标定的参数
typedef struct 
{
	uchar   vHead[2];
	//// 摇杆的高、中、低位采样值
	ushort  vRockV[20][2];						// 摇杆的首末端有效值 
	uchar   vRockDS[_ARM_MAX];				// 摇杆 中位 死区，1~9 *0.05
	
	ushort  vChkSum;					// 校验和
}*pBDParam, mBDParam;
////  标定2的参数   电磁阀的电流
typedef struct 
{
	uchar   vHead[2];
	ushort  vAITime;					// 电流爬坡时间
	ushort  vVCC;							// 电压
	ushort  vRes;							// 阀块电阻
 	// 电磁阀的最大/最小电流
	ushort  vAI[6][8];  			// 0,2 ---- Max, 1,3 ---- Min  
	ushort  vAI2[6][8];  			// 0,2 ---- Max, 1,3 ---- Min  
	ushort  vChkSum;					// 校验和
}*pBDParam2, mBDParam2;

typedef struct 
{
	uchar   vHead[2];
	ushort  vRsv;
	uint    vWorkTime;					// 累计工作时间，分钟
	ushort   vWorkTime2;					// 累计工作时间，分钟

	ushort  vChkSum;					// 校验和
}*pWorkTime, mWorkTime;

typedef struct 
{
	unsigned char  vRcvSt;				// 接收机状态
	
	unsigned char  vRcvWL[40];		// 从无线接受到的数据，接收机发送过来
	
	unsigned char  vSNJDW;				// 速凝剂档位
	unsigned char  vPaiL;					// 排量
	unsigned short vWarns;				// 故障代码
	unsigned long  vLJWkTime;			// 累计工作时间
	unsigned short vAITime;				// 电流爬坡时间，mS
	unsigned short vAITimeRcv;			// 电流爬坡时间，mS
	unsigned char  vSetFlg;					// 设置的标记
	
	
	// 控制器发过来的数据	0x05
	unsigned short vArmI[16];		// 反馈电流
	unsigned char vRcvDI[2];		// 控制器的DI信息，输入
	unsigned char vRcvDO[2];		// 控制器的DO信息，输出反馈
	unsigned short vRcvWarn;		// 控制器的告警信息，
	unsigned int  vWTime;			// 控制器的工作时间，

	// 无线模块参数
	unsigned char  vWLSingle;		// 无线信号强弱,20~110
	unsigned char  vWLSingle0;		// 无线信号强弱,20~110

	// 通信ID
	unsigned char  vRcvID[4];
	// AI发送有关
	unsigned char  vAIX;
	unsigned char  vAIY;
	unsigned char  vAIPage;

}*pRunInfo, mRunInfo;


/************************************ 变量 *************************************************/

extern uchar   gSaveFlg ;
extern uint32_t gPowerChk;			// 上电检测结果，为0，OK，非0，摇杆或按键故障，所有输出清0
extern	 unsigned int stStudyFlg ;
extern uint32_t gSelfChkMask;		
extern uint8_t gSelfChkOK;

extern unsigned char gBDCmd ;
extern unsigned char gBDCmdAck;

extern mBDParam gBDParam[2];
extern mBDParam gBDParamRcv[2];	 	// 接收到的标定信息，倾角IO，遥控器
extern mBDParam2 gBDParam2[2];
extern mPswd  gPswd;

extern mRunInfo gRunInfo;
extern unsigned char gADArmConV[_ARM_MAX+1];		// 摇杆转换后的值(0~124,125,126~250)
extern unsigned char gADArmSend[_ARM_MAX+1];		// 摇杆的输出值（0~124,125,126~250
extern const unsigned char gArmUpTab[_ARM_MAX];
extern const unsigned char gArmDownTab[_ARM_MAX];
extern ushort gArmMidSp[_ARM_MAX*2];


extern const unsigned short romArmValDisp[8][2];	// 摇杆值
extern  unsigned short romArmDSDisp[8][2];		// 死区值 1~9

extern mWorkTime gWorkTime[2];
extern mWorkTime gWorkTimeRd[2];

extern unsigned int gLCDCloseDly ;		// 关闭LCD，秒
extern unsigned int gTXErrDly;
extern unsigned int gTXCANDly;			// can通信异常，MS
extern unsigned char gChaoZOv ;

extern unsigned int  gAISendFlg;		// 下发AI，上电后全部发送，设置时一直发送
extern unsigned int  gWLSingV ;extern unsigned int  gWLMachSingV ;extern unsigned int  dead_zone ;
	   

extern unsigned char gYKQ_WLStr[24];					// 遥控器的数据，发往 无线

extern unsigned int  gWLRcvs ;
extern unsigned int  gWLRcvOk ;
extern unsigned int  gMachRcvOk ;
extern unsigned int  gJTDly ;

/************************************ 函数 *************************************************/


void Arm_Conv(void);
void Cell_Chk(void);
void BitV_Chk(void);

void Arm_Study(void);
void Arm_RockX(void);


#endif  //__APP_LIJU_H_
