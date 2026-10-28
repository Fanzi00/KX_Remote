/****************************************************************************
*
*	 用户计算 有关的函数
*
*
*****************************************************************************/

#include "APP_usr.h"
#include "LPC17xx.h"
#include "config.h"

extern mOverInfo  gOverInfo;
extern mOvHandInfo gOvHandInfo;

uint32_t gSelfChkMask = 0;		// 自检异常掩码，0=无异常；自检关闭(_SELF_CHK_EN=0)时恒为0
uint8_t gSelfChkOK = 1;			// 自检结果，1=通过；自检关闭时恒为通过

mPswd  gPswd;
mRunInfo gRunInfo;
mBDParam gBDParam[2];			// 本地存储的标定信息	
mBDParam gBDParamRcv[2];	 	// 接收到的标定信息，倾角IO，遥控器
uint32_t gPowerChk = 0;			// 上电检测结果，为0，OK，非0，摇杆或按键故障，所有输出清0
uint32_t gArmOpFLg = 0;			// 摇杆动作的标记，1bit对应一个方向
uint32_t gArmOpFLg0 = 0;			// 摇杆动作的标记，1bit对应一个方向
 
unsigned int  gAISendFlg = 0;	// 下发AI，上电后全部发送，设置时一直发送
unsigned int  gWLSingV = 0;
unsigned int  dead_zone = 0;
unsigned int  gWLRcvChkFail = 0;	// 帧校验失败计数（噪声/错帧，RF 误码）
unsigned int  gWLRcvHdrMiss = 0;	// 缓存中找不到有效帧头计数（噪声/掉线时堆积的垃圾）

// 摇杆从左(J1) 到右(J8)对应的AD采样值存储位置
#if _ARM_XY						// 万向摇杆
	#if _ARM_PG					// P+G
		const unsigned char gArmUpTab[_ARM_MAX] = {7,  3,  13, 8, 5, 1, 0, 0 };
		const unsigned char gArmDownTab[_ARM_MAX]={15, 11, 12, 4, 0, 9, 0, 0 };
	#else
		#if _ARM_XRZN
			const unsigned char gArmUpTab[_ARM_MAX]  = {15, 7, 13, 12, 5, 0 ,  8, 4};		// 万向手柄，先上下 后左右
			const unsigned char gArmDownTab[_ARM_MAX] ={15, 7, 13, 12, 5, 0,   8, 4 };		// 万向手柄，先上下 后左右
		#endif
		#if	 _ARM_KX	
			const unsigned char gArmUpTab[_ARM_MAX]  = {10,6 , 15, 7 ,12,13 , 0, 5 };		// 万向手柄，先上下 后左右
			const unsigned char gArmDownTab[_ARM_MAX] ={ 10,6, 15, 7,  12,13 ,0, 5  };		// 万向手柄，先上下 后左右
																														  
		#else
			const unsigned char gArmUpTab[_ARM_MAX]  = {15, 7, 12, 13, 0, 5 ,  8, 4};		// 万向手柄，先上下 后左右
			const unsigned char gArmDownTab[_ARM_MAX] ={15, 7, 12, 13, 0, 5,   8, 4 };		// 万向手柄，先上下 后左右
		#endif
	#endif
#else									// 直线摇杆
	const unsigned char gArmUpTab[_ARM_MAX]   = {6,  7,  3,  13, 8, 5, 9,  _CNT, _CNT };		// 万向手柄，先上下 后左右
	const unsigned char gArmDownTab[_ARM_MAX] = {10, 15, 11, 12, 4, 0, 1,   _CNT, _CNT };		// 万向手柄，先上下 后左右
#endif

unsigned char gADArmConV[_ARM_MAX+1];		// 摇杆转换后的值(0~124,125,126~250)
unsigned char gADArmSend[_ARM_MAX+1];		// 摇杆的输出值(0~124,125,126~250)，与转换值相同（自检不通过时清0）
unsigned char gBDCmd = 0;
unsigned char gBDCmdAck = 0;

unsigned char gYKQ_WLStr[24];					// 遥控器的数据，发往 无线

/****************************************************************************************/
unsigned int gLCDCloseDly = 300;		// 关闭LCD，秒
unsigned int gTXErrDly = 50;				// 通信异常，秒
unsigned int gTXCANDly = 1200;			// can通信异常，MS

uchar   gSaveFlg = 0;

mWorkTime gWorkTime[2];
mWorkTime gWorkTimeRd[2];


////  电池电量检测  6.9 ~ 7.4V
void Cell_Chk(void)
{
	int vCell = 0;
	int vCell0 = 6900;
/*	
	vCell = (int)(ADCRstP[14] * _V_ADC / _ADC_MAX);		// 电池电压采样值, mV, 82:47
	if(vCell * 2.76 < vCell0)
		gCellPower = 0;
	else
		gCellPower = (vCell * 2.76 - vCell0) / 140;		// 电压分10份
	*/	
	vCell = (int)(ADCRstP[14] * _V_ADC / _ADC_MAX);		// 电池电压采样值, mV, 改分压电阻， 110:47		20180506  LSG  
	if(vCell * 3.34 < vCell0)
		gCellPower = 0;
	else
		gCellPower = (vCell * 3.34 - vCell0) / 100;		// 电压分10份
}

/****************************************************************************************
*
* 时间:  2015-05-24
* 名称：	Arm -- PWM -- AD Feedback
* 功能：	摇杆输出 转换为 PWM 输出，并根据AD反馈调整PWM输出
* 说明： 	摇杆输出：上--0~124, 中--125, 下--126~251
*
****************************************************************************************/
////  摇杆的学习过程，摇杆状态界面下，按下急停，长按F2标定中位，长按 F3 学习最大最小值
	 unsigned int stKeyCnt = 0;
	 unsigned int stStudy = 0;
	 unsigned int stStudyFlg = 0;
void Arm_Study(void)
{
	unsigned char i;
	unsigned char vKey = (gDIBitV0 >> 16) & 0x0f;
	static unsigned char stLastKey = 0;
	unsigned short vDX = 90;
	unsigned short vDY = 120;

	if(stStudyFlg == 0)
	{
		if(vKey != _F2F3)
		{
			stStudy = 0;
			return;
		}
		if(++stStudy < 3000/LCD_FRESH_MS)
			return;
		memcpy(&gBDParam[1], &gBDParam[0], sizeof(gBDParam[0]));
		stStudyFlg = 1;                 // 位0:学习模式，位1:中位已学，位2:极限已开始
		memset(Y_Send, _ARM_MIDVCan, _ARM_USE);
		memset(VKC_Send, _ARM_MIDVCan, _ARM_USE);
		stStudy = 0;
		stKeyCnt = 0;
		stLastKey = vKey;
		gKeyValue = 0;
		gArmDSSetFlg = 0;
		LCD_Set_Cursor(0, 1, 1, 2, 2);
		LCD_Disp_PIC(8);
		return;
	}
	if(vKey != stLastKey)
	{
		stKeyCnt = 0;
		stLastKey = vKey;
	}
	if(stKeyCnt <= 3000/LCD_FRESH_MS)
		stKeyCnt++;

	if((vKey == _F2) && (stKeyCnt == 3000/LCD_FRESH_MS))
	{
		for(i=0; i<_ARM_USE; i++)
		{
			if(!((_ARM_STUDY_MASK >> i) & 1))
				continue;						// 该轴不参与自学习(未接手柄等)，跳过
			if((ADCRst[gArmUpTab[i]] <= 100) || (ADCRst[gArmUpTab[i]] >= 4090) ||
			   (ADCRst[gArmDownTab[i]] <= 100) || (ADCRst[gArmDownTab[i]] >= 4090))
			{
				LCD_Disp_Txt(FONT_12, vDX, vDY, "手柄采样异常    ");
				return;
			}
		}
		for(i=0; i<_ARM_USE; i++)
		{
			if(!((_ARM_STUDY_MASK >> i) & 1))
				continue;
			gBDParam[1].vRockV[i*2][0] = ADCRst[gArmUpTab[i]];
			gBDParam[1].vRockV[i*2+1][0] = ADCRst[gArmDownTab[i]];
		}
			#if _V_2					// 新的版本，旋钮到最大，采样值最小 0
		#if _XN_LiuL
				if(ADCRst[_XN1_AD_CHN] > 0x800)
				{
					gBDParam[1].vRockV[_XN1_RV_CHN][0] = ADCRst[_XN1_AD_CHN]-10;			// 旋钮的最小值
				}
		#endif		
				if(ADCRst[_XN2_AD_CHN] > 0x800)
				{
					gBDParam[1].vRockV[_XN2_RV_CHN][0] = ADCRst[_XN2_AD_CHN]-10;			// 旋钮的最小值
					LCD_Disp_Txt(FONT_12, vDX, vDY, "中位学习完成    ");
				//	LCD_Disp_Txt(FONT_16, 100, 150, "中位学习完成    ");
				}
			#else					// 旧的版本，旋钮到最大，采样值最大
				if(ADCRst[_XN1_AD_CHN] < 0x100)
				{
					gBDParam[1].vRockV[_XN1_RV_CHN][0] = ADCRst[_XN1_AD_CHN]+10;			// 旋钮的最小值
					LCD_Disp_Txt(FONT_12, vDX, vDY, "中位学习完成    ");
				}
				if(ADCRst[_XN2_AD_CHN] < 0x100)
				{
					gBDParam[1].vRockV[_XN2_RV_CHN][0] = ADCRst[_XN2_AD_CHN]+10;			// 旋钮的最小值
				//	LCD_Disp_Txt(FONT_16, 100, 150, "中位学习完成    ");
				}
			#endif
		stStudyFlg |= 2;
		LCD_Disp_Txt(FONT_12, vDX, vDY, "中位学习完成    ");
	}
	if((vKey == _F3) && (stKeyCnt >= 3000/LCD_FRESH_MS))
	{
		if((stStudyFlg & 4) == 0)
		{
			// 本次极限采集从中位开始，允许重新学习更窄的行程。
			for(i=0; i<_ARM_USE; i++)
			{
				if(!((_ARM_STUDY_MASK >> i) & 1))
					continue;
				gBDParam[1].vRockV[i*2][1] = gBDParam[1].vRockV[i*2][0];
				gBDParam[1].vRockV[i*2+1][1] = gBDParam[1].vRockV[i*2+1][0];
			}
			stStudyFlg |= 4;
		}
		for(i=0; i<_ARM_USE; i++)
		{
			if(!((_ARM_STUDY_MASK >> i) & 1))
				continue;
			if((ADCRst[gArmUpTab[i]] > 100) &&
			   (ADCRst[gArmUpTab[i]] < gBDParam[1].vRockV[i*2][1]))
				gBDParam[1].vRockV[i*2][1] = ADCRst[gArmUpTab[i]];
			if((ADCRst[gArmDownTab[i]] < 4090) &&
			   (ADCRst[gArmDownTab[i]] > gBDParam[1].vRockV[i*2+1][1]))
				gBDParam[1].vRockV[i*2+1][1] = ADCRst[gArmDownTab[i]];
		}
			#if _V_2					// 新的版本，旋钮到最大，采样值最小 0
		#if _XN_LiuL
				if(ADCRst[_XN1_AD_CHN] < gBDParam[1].vRockV[_XN1_RV_CHN][1])
				{
					gBDParam[1].vRockV[_XN1_RV_CHN][1] = ADCRst[_XN1_AD_CHN]+10;			// 旋钮的最大值
				}
		#endif		
				if(ADCRst[_XN2_AD_CHN] < gBDParam[1].vRockV[_XN2_RV_CHN][1])
				{
					gBDParam[1].vRockV[_XN2_RV_CHN][1] = ADCRst[_XN2_AD_CHN]+10;			// 旋钮的最大值
					LCD_Disp_Txt(FONT_12, vDX, vDY, "极限位学习中...");
				//	LCD_Disp_Txt(FONT_16, 100, 150, "极限位学习中...");
				}
			#else					// 旧的版本，旋钮到最大，采样值最大
				if(ADCRst[_XN1_AD_CHN] > gBDParam[1].vRockV[_XN1_RV_CHN][0]+0x800)
				{
					gBDParam[1].vRockV[_XN1_AD_CHN][1] = ADCRst[_XN1_AD_CHN]-10;			// 旋钮的最大值
					LCD_Disp_Txt(FONT_12, vDX, vDY, "极限位学习中...");
				}
				if(ADCRst[_XN2_AD_CHN] > gBDParam[1].vRockV[_XN2_RV_CHN][0]+0x800)
				{
					gBDParam[1].vRockV[_XN2_AD_CHN][1] = ADCRst[_XN2_AD_CHN]-10;			// 旋钮的最大值
				//	LCD_Disp_Txt(FONT_16, 100, 150, "极限位学习中...");
				}
			#endif
		LCD_Disp_Txt(FONT_12, vDX, vDY, "极限位学习中... ");
	}
	if((vKey == _F1) && (stKeyCnt == 3000/LCD_FRESH_MS))
	{
		for(i=0; i<_ARM_USE; i++)
		{
			if(!((_ARM_STUDY_MASK >> i) & 1))
				continue;
			if((stStudyFlg == 1) || (gBDParam[1].vRockV[i*2][1] <= 100) ||
			   (gBDParam[1].vRockV[i*2][1] + 120 >= gBDParam[1].vRockV[i*2][0]) ||
			   (gBDParam[1].vRockV[i*2+1][0] + 120 >= gBDParam[1].vRockV[i*2+1][1]) ||
			   (gBDParam[1].vRockV[i*2+1][1] >= 4090))
			{
				LCD_Disp_Txt(FONT_12, vDX, vDY, "未学习或参数无效");
				return;
			}
		}
		memcpy(&gBDParam[0], &gBDParam[1], sizeof(gBDParam[0]));
		Arm_RockX();
		gSaveFlg = 1;
		LCD_Disp_Txt(FONT_12, vDX, vDY, "参数已提交保存  ");
	}
	if(vKey == _F4)
	{
		gKeyValue = 0;
		gLcdFresh = 3;
		gArmDSSetFlg = 0;
		stStudyFlg = 0;
		stStudy = 0;
		stKeyCnt = 0;
		gp_lcdtask = Disp_mainSPJ;
	}
}

unsigned short ave_x1(unsigned short ratio)
	{
		const char n = 30 ;
		char i ;
		unsigned long  sum = 0 ;
		unsigned short vMax,vMin,vRtn;
		unsigned char  sMax,sMin;
		static unsigned short ratio_team[40] ;
		static char ptr = 0 ;
		static char firstflg = 10;
	
		if(firstflg>0)
		{
			firstflg --;
			for(i=0; i<n; i++)
				ratio_team[i] = ratio;
		}
		sMax = 0;
		sMin = 0;
		vMax = 0;
		vMin = 0;
		vRtn = 0;
		vMax = 0;
		vMin = 0xffff;
		ratio_team[(ptr++)%n] = ratio ;
		for(i = 0 ; i < n ; i++)
		{
			sum = sum + ratio_team[i] ;
			if(ratio_team[i] > vMax)
			{
				vMax = ratio_team[i];
				sMax = i;
			}
			if(ratio_team[i] < vMin)
			{
				vMin = ratio_team[i];
				sMin = i;
			}
		}
		vRtn = sum/(n);
		ratio_team[sMax] = (vRtn+ratio_team[sMax])>>1;
		ratio_team[sMin] = (vRtn+ratio_team[sMin])>>1;
		sum = sum-vMax;
		sum = sum-vMin;
		vRtn = sum/(n-2);
		
		return (vRtn) ;
		
	}

//// 摇杆死区、变动范围计算
ushort gArmMidSp[_ARM_MAX*2];
ushort gArmEndSp[_ARM_MAX*2];
ushort gArmSp[_ARM_MAX*2];				// 变动范围

void Arm_RockX(void)
{
	uchar i = 0;
	uchar vDS = 0;				// 死区参数
	
	for(i=0; i<_ARM_MAX*2; i++)
	{
		gArmMidSp[i] = 0;
		gArmEndSp[i] = 0;
		vDS = gBDParam[0].vRockDS[i>>1];
		if((vDS<1) || (vDS>9))
			vDS = 2;
		if(gBDParam[0].vRockV[i][1] < gBDParam[0].vRockV[i][0]-100)
		{
			gArmMidSp[i] = (gBDParam[0].vRockV[i][0] - gBDParam[0].vRockV[i][1]) * (0.06*(1+vDS));
			gArmEndSp[i] = (gBDParam[0].vRockV[i][0] - gBDParam[0].vRockV[i][1]) * 0.05;
			gArmSp[i] = (gBDParam[0].vRockV[i][0] - gBDParam[0].vRockV[i][1]) - gArmMidSp[i] - gArmEndSp[i];
		}
		else if(gBDParam[0].vRockV[i][1] > gBDParam[0].vRockV[i][0]+100)
		{
			gArmMidSp[i] = (gBDParam[0].vRockV[i][1] - gBDParam[0].vRockV[i][0]) * (0.06*(1+vDS));
			gArmEndSp[i] = (gBDParam[0].vRockV[i][1] - gBDParam[0].vRockV[i][0]) * 0.05;
			gArmSp[i] = (gBDParam[0].vRockV[i][1] - gBDParam[0].vRockV[i][0]) - gArmMidSp[i] - gArmEndSp[i];
		}
	}
}

////  把摇杆的采样值转换为0~250的值发送出去，万向手柄，从左到右，先上下后左右，对于 ARM 1~6
	unsigned long  armX = 0;
	unsigned short armV = 0;

unsigned short Arm_xy_conv(unsigned char vCh)
{
	unsigned short vRtn = _ARM_MIDV;
	
	if((ADCRst[gArmUpTab[vCh]] > 10) && (ADCRst[gArmUpTab[vCh]] < 4090))
	{
		if(ADCRst[gArmUpTab[vCh]] < gBDParam[0].vRockV[vCh*2][0] - gArmMidSp[vCh*2])
		{
			armX = gBDParam[0].vRockV[vCh*2][0] - ADCRst[gArmUpTab[vCh]] - gArmMidSp[vCh*2];
			if(armX > gArmSp[vCh*2])
				armX =  gArmSp[vCh*2];
			armV = armX * _ARM_MIDV / gArmSp[vCh*2];
			vRtn = _ARM_MIDV - armV;
		}
	}
	if((ADCRst[gArmDownTab[vCh]] > 10) && (ADCRst[gArmDownTab[vCh]] < 4090))
	{
		if(ADCRst[gArmDownTab[vCh]] > gBDParam[0].vRockV[vCh*2+1][0] + gArmMidSp[vCh*2+1])
		{
			armX = ADCRst[gArmDownTab[vCh]] - (gBDParam[0].vRockV[vCh*2+1][0] + gArmMidSp[vCh*2+1]);
			if(armX > gArmSp[vCh*2+1])
				armX =  gArmSp[vCh*2+1];
			armV = armX * _ARM_MIDV / gArmSp[vCh*2+1];
			vRtn = armV + _ARM_MIDV;
		}
	}
		return vRtn;
}
unsigned short Arm_x_conv(unsigned char vCh)
{
	uchar vMidV = _ARM_MIDV;
	uchar vArmUp = 0;
	uchar vArmDown = 0;
	ushort vArmMidSpace = _ARM_MID_SPACE;			// 中位死区，小于它，为0
	ushort vArmEndSpace = _ARM_END_SPACE;			// 末端死区，大于它，为125


		vArmUp = 0;
	vArmMidSpace = gBDParam[0].vRockV[vCh*2][0] + gArmMidSp[vCh*2];
	vArmEndSpace = gBDParam[0].vRockV[vCh*2][1] - gArmEndSp[vCh*2];
		if((ADCRst[gArmUpTab[vCh]] > vArmMidSpace) && (ADCRst[gArmUpTab[vCh]] < 4000))
		{
			if(ADCRst[gArmUpTab[vCh]] >= vArmEndSpace)
				vArmUp = vMidV;
			else									   
				vArmUp = (ADCRst[gArmUpTab[vCh]] - vArmMidSpace)* vMidV /(vArmEndSpace - vArmMidSpace);
		}
		vArmDown = 0;
	vArmMidSpace = gBDParam[0].vRockV[vCh*2+1][0] + gArmMidSp[vCh*2+1];
	vArmEndSpace = gBDParam[0].vRockV[vCh*2+1][1] - gArmEndSp[vCh*2+1];
		if((ADCRst[gArmDownTab[vCh]] >= vArmMidSpace) && (ADCRst[gArmDownTab[vCh]] < 4000))
		{
			if(ADCRst[gArmDownTab[vCh]] > vArmEndSpace)
				vArmDown = vMidV ;
			else
				vArmDown = (ADCRst[gArmDownTab[vCh]] - vArmMidSpace)* vMidV /(vArmEndSpace - vArmMidSpace);
		}
		if((vArmUp==0) && (vArmDown==0))  	// 中位	 0
		{
		//	gADArmConV[i] = 0;
			gADArmConV[vCh] = vMidV;
			gArmOpFLg &= ~(3<<(vCh*2));
		}
		else if(vArmUp > 0)//vArmDown)	   		// 上  124~0
		{
		//	gADArmConV[i] = vArmUp;						// 1~125
			gADArmConV[vCh] = vMidV - vArmUp;				//  124~0
			gArmOpFLg &= ~(3<<(vCh*2));
			gArmOpFLg |= (1<<(vCh*2));
		}
		else if(vArmDown > 0)				// 下  126~250
		{
			gADArmConV[vCh] = vArmDown + vMidV;
			gArmOpFLg &= ~(3<<(vCh*2));
			gArmOpFLg |= (2<<(vCh*2));
		}
		return gADArmConV[vCh];
}
	
void Arm_Conv(void)
{
	unsigned char i = 0;
	unsigned long vNum = 0;
	unsigned short vXN_Val;
	
	for(i=0; i<ADC_NUM; i++)
		ADCRst[i] = AD_Bessel(i, ADCRstP[i] );
	
	for(i=0; i<_ARM_USE; i++)
	{
		gADArmSend[i] = _ARM_MIDV;
			armX = 0;

			if (i==0 || i==1)
			{
				gADArmSend[i] = 250 - Arm_xy_conv(i);
			}
			else 
			{
				 gADArmSend[i] = Arm_x_conv(i);
			}
/*
#if _ARM_XY			
		#if _ARM_PG	
				gADArmSend[i] = Arm_xy_conv(i);			// PG
		#else
				gADArmSend[i] = 250 - Arm_xy_conv(i);
		#endif		
#else
		gADArmSend[i] = Arm_x_conv(i);
#endif	
*/	
		
	}
#if _V_2					// 新的版本，旋钮到最大，采样值最小 0
	vXN_Val = (ADCRst[_XN1_AD_CHN]);
	if( vXN_Val >= gBDParam[0].vRockV[_XN1_RV_CHN][0] )
		gADArmSend[_ARM_MAX-2] = 0;
	else if(vXN_Val <= gBDParam[0].vRockV[_XN1_RV_CHN][1] )
		gADArmSend[_ARM_MAX-2] = _ARM_MIDV*2;
	else
	{
		vNum = (gBDParam[0].vRockV[_XN1_RV_CHN][0] - vXN_Val) * _ARM_MIDV * 2;
		gADArmSend[_ARM_MAX-2] = vNum / (gBDParam[0].vRockV[_XN1_RV_CHN][0] - gBDParam[0].vRockV[_XN1_RV_CHN][1]);
	}
	
	vXN_Val = (ADCRst[_XN2_AD_CHN]);
	if( vXN_Val >= gBDParam[0].vRockV[_XN2_RV_CHN][0] )
		gADArmSend[_ARM_MAX-1] = 0;
	else if(vXN_Val <= gBDParam[0].vRockV[_XN2_RV_CHN][1] )
		gADArmSend[_ARM_MAX-1] = _ARM_MIDV*2;
	else
	{
		vNum = (gBDParam[0].vRockV[_XN2_RV_CHN][0] - vXN_Val) * _ARM_MIDV * 2;
		gADArmSend[_ARM_MAX-1] = vNum / (gBDParam[0].vRockV[_XN2_RV_CHN][0] - gBDParam[0].vRockV[_XN2_RV_CHN][1]);
	}
#else	
	vXN_Val = (ADCRst[_XN1_AD_CHN]);
	if( vXN_Val <= gBDParam[0].vRockV[_XN1_RV_CHN][0] )
		gADArmSend[_ARM_MAX-2] = 0;
	else if(vXN_Val >= gBDParam[0].vRockV[_XN1_RV_CHN][1] )
		gADArmSend[_ARM_MAX-2] = _ARM_MIDV*2;
	else
	{
		vNum = (vXN_Val - gBDParam[0].vRockV[_XN1_RV_CHN][0]) * _ARM_MIDV * 2;
		gADArmSend[_ARM_MAX-2] = 250 - vNum / (gBDParam[0].vRockV[_XN1_RV_CHN][1] - gBDParam[0].vRockV[_XN1_RV_CHN][0]);
	}
	
	vXN_Val = (ADCRst[_XN2_AD_CHN]);
	if( vXN_Val <= gBDParam[0].vRockV[_XN2_RV_CHN][0] )
		gADArmSend[_ARM_MAX-1] = 0;
	else if(vXN_Val >= gBDParam[0].vRockV[_XN2_RV_CHN][1] )
		gADArmSend[_ARM_MAX-1] = _ARM_MIDV*2;
	else
	{
		vNum = (vXN_Val - gBDParam[0].vRockV[_XN2_RV_CHN][0]) * _ARM_MIDV * 2;
		gADArmSend[_ARM_MAX-1] = 250 - vNum / (gBDParam[0].vRockV[_XN2_RV_CHN][1] - gBDParam[0].vRockV[_XN2_RV_CHN][0]);
	}
#endif
/*
	vXN_Val = ave_x1(ADCRst[_XN1_AD_CHN]);
	if( vXN_Val <= gBDParam[0].vRockV[14][0] )
		gADArmSend[_ARM_MAX] = 0;
	else if(vXN_Val >= gBDParam[0].vRockV[14][1] )
		gADArmSend[_ARM_MAX] = _ARM_MIDV*2;
	else
	{
		vNum = (vXN_Val - gBDParam[0].vRockV[14][0]) * _ARM_MIDV * 2;
		gADArmSend[_ARM_MAX] = vNum / (gBDParam[0].vRockV[14][1] - gBDParam[0].vRockV[14][0]);
	}
	*/
}
