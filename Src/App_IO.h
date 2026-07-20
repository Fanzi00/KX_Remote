/********************************************************************
*
*  与 IO 口有关的函数，key, di, do 等
*  功能：
*
********************************************************************/

#ifndef ___APP_IO_H_
#define ___APP_IO_H_


#include "config.h"
#include "gpio.h"


#define  _BitV(	v, b)		(((v) & (1<<(b))) == (1<<(b))? 1:0)
#define  _BitVH(v, b)		(((v) & (b)) == (b)? 1:0)


#define  _KEY_MAXNUM	6			// KEY  输入的最大个数
#define  _DI_MAXNUM		32		// DI  输入的最大个数
#define  _DO_MAXNUM		8			// DO输出最大个数

#define  _EOT			0xaa			// End of Table
#define  _CNT			0xCC  		// Continue of Table
#define  _CAN_T			0x20		// 输出到CAN 

#define  _DI_SCAN_CNT	10		// 去抖的计数次数
#define  _DI_DD_SCAN_CNT	300		//  点动信号维持的最小的计数次数
//#define  PASSWORD_LEN	6


////	DI (12, 低有效)	KEY
#define  _DI_1			9			// 		P0.9
#define  _DI_2			8			// 		P0.8
#define  _DI_3			28		// 		P4.28
#define  _DI_4			5			// 		P0.5
#define  _DI_5			17		// 		P1.17
#define  _DI_6			29		// 		P4.29
#define  _DI_7			6			// 		P0.6
#define  _DI_8			7			// 		P0.7
#define  _DI_9			30		// 		P1.14  无
#define  _DI_10			28		// 		P1.28

#define  _DI_11			2			// 		P2.2
#define  _DI_12			3			// 		P2.3
#define  _DI_13			5			// 		P2.5
#define  _DI_14			4			// 		P2.4
#define  _DI_15			9			// 		P2.9
#define  _DI_16			8			// 		P2.8
#define  _DI_17			1			// 		P2.1
#define  _DI_18			0			// 		P2.0
#define  _DI_19			7			// 		P2.7
#define  _DI_20			6			// 		P2.6

#define  _DI_21			11			// 		P0.11
#define  _DI_22			13			// 		P2.13
#define  _DI_23			29			// 		P1.29
#define  _DI_24			10			// 		P0.10
#define  _DI_25			21			// 		P0.21
#define  _DI_26			22			// 		P0.22

#define  _DI_JiT		27			//		P1.27 急停输入脚
//#define  _DI_24V 		31			//		P1.31 24V输入检测
#define  _DI_24V 		18			//	V3	P0.18 24V输入检测     20180629 

////  Kx   10
#define  _K_1				21			//		P1.21
#define  _K_2				22			//		P1.22
#define  _K_3				23			//		P1.23
#define  _K_4				24			//		P1.24
#define  _K_5				25			//		P1.25
#define  _K_6				26			//		P1.26
#define  _K_7				16			//		P1.16
#define  _K_8				15			//		P1.15
#define  _K_9				14			//		P1.14
#define  _K_10			10			//		P1.10


////	DO/ Led
#define  _DO_1			26		// 		P3.26
#define  _DO_2			25		// 		P3.25
#define  _DO_3			29		// 		P0.29
#define  _DO_4			30		// 		P0.30
#define  _DO_5			18		// 		P1.18
#define  _DO_6			19		// 		P1.19

#define  _DO_FMQI		20		// 		P1.20

////  按键： F1~F4	 gKeyValue 中
#define  _F1			0x01	 // 
#define  _F2			0x02	 // 
#define  _F3			0x04	 // 
#define  _F4			0x08	 // 
#define  _F1F2		0x03	 // 
#define  _F2F3		0x06	 // 
#define  _F1F3		0x05	 // 
#define  _F1F4		0x09	 // 	  
 

////  DO 控制信号
#define  DO_LED1			 (1<<_DO_1)			// 红灯
#define  DO_LED2			 (1<<_DO_2)
#define  DO_LED3			 _DO_3
#define  DO_LED4			 _DO_4
#define  DO_LED5			 _DO_5
#define  DO_LED6			 _DO_6

#define  _POW_ON			GPIO3->FIOSET |= DO_LED2;
#define  _POW_OFF			GPIO3->FIOCLR |= DO_LED2;
#define  _RED_ON			GPIO3->FIOSET |= DO_LED1;
#define  _RED_OFF			GPIO3->FIOCLR |= DO_LED1;



#define  DO_CPU_EN		(1<<30)						// P0.30
#define  DO_LCD_EN		(1<<29)						// P0.29
#define	 _CPU_EN		GPIO0->FIOCLR |= DO_CPU_EN;
#define	 _CPU_DIS		GPIO0->FIOSET |= DO_CPU_EN;
#define	 _LCD_EN		GPIO0->FIOSET |= DO_LCD_EN;
#define	 _LCD_DIS		GPIO0->FIOCLR |= DO_LCD_EN;

#define  DO_WL_EN			(1<<18)						// P1.18
#define	 _WL_EN			GPIO1->FIOSET |= DO_WL_EN;
#define	 _WL_DIS		GPIO1->FIOCLR |= DO_WL_EN;


extern unsigned char gDOValue[_DO_MAXNUM];
extern unsigned char gDIScanFlg[_DI_MAXNUM];

extern unsigned char gKeyValue ;
extern unsigned char gKeyValue0 ;
extern unsigned char gKeyValue0x ;		// 有按键，保持

extern unsigned char gPowerFlg;			// 电源标记，0--有线供电，1--电池供电
extern unsigned char gCellPower;		// 电池电量	0~9

extern unsigned char gCmdYKFlg;			// 1--遥控、0--近控
extern unsigned char gWLAux ;				// 无线模块的AUX脚，输入，为高，无线可用，为低，无线忙
extern unsigned char gArmDSSetFlg;	// 摇杆死区设置标记

extern unsigned int  gBeepFlg ;
extern unsigned int  gWarnFlg ;
extern unsigned char  gModeSel ;

extern unsigned long  gDIBitV0 ;			// 用位定义的两档DI输入结果，1bit -- 1个开关
extern unsigned long  gDIBitV ;			// 用位定义的两档DI输入结果，1bit -- 1个开关，输出给控制器
extern unsigned int  gDI3BitV ;			// 用位定义的三档DI输入结果，2bit -- 1个开关
extern unsigned int  gDIxBitV ;			// 用位定义的档位旋钮DI输入结果，4bit -- 1个开关
extern unsigned int  gDOBitV ;			// 用位定义的DO输出记录，1bit -- 1路DO
extern unsigned int  gKBitV ;				// 用位定义的 K 输入， 1bit -- 1路K
extern unsigned int  gKValx ;				// K 输入的 序号， 1~10

extern unsigned char gJiTing ;			// 急停标记
extern unsigned char gLCDBKLight;		// LCD 背光亮度
extern unsigned char gLCDBKLight0;	// LCD 背光亮度
extern unsigned int  gLCDBKLDly;		// 无摇杆，关闭背光
extern unsigned int  gLCDBKOnDly ;		// 上电到开背光的延时
extern unsigned char gLCDBKOnFlg ;	// LCD 背光开关
extern unsigned char gLaBaFlg ;			// 喇叭，启动和解除急停后，都需先按喇叭，才能起作用

extern unsigned int  gTstV;


extern void delay_ms(unsigned int vDel);

void IO_Init(void);
void DI_Scan(void) ;


void Key_Main(void);
uchar Key_Input_Password(void);
uchar Key_Input_Passwordx(unsigned char vCh);
void Key_Cmd_Sel(void);

void Key_Cmd8_SetI2(void);
void Key_Cmd8_SetI(void);
void Key_Cmd1_AITime(void);
void Key_MainZZT(void);
void Key_ZZT_CtrlStatus(void);


#endif  //___APP_IO_H_
