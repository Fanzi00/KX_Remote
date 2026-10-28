#include "_Device_config.h"
#include "App_usr.h"
#if (USER_TYPE == KXCAN_GLZYTC_L2X)


Activar_t F = {0};

static DiffCtx gGLZYTCDiff; // 差分刷新上下文（装药台车界面）
static DiffCtx gGLZYTCDiff;   // 差分刷新上下文（调用方持有，每个屏一个）
// 扭子开关图标配置表（App_lcd.c 的 Disp_Rock_Pic 遍历绘制）
// 行格式: {是否显示, 屏幕X, 屏幕Y, gDIBitV移位数, 几档开关(2/3), 是否自锁(0/1), 显示映射(0/1)}
// 两档自锁: 图标仅上/下; 两档自复位: 触发/中; 三档: 上/中/下(自锁自复位同)
// map=1: 上/下反转(交换图标1/2, 中态0不变) —— 实际显示与逻辑相反时置1
// 注: 原 grp(状态缓存组)字段在新模型中未使用(两表均读 gDIBitV)，已弃用; 原 mask/映射方式已折算为 gears/lock/map
// 增删图标: 增删一行；不显示把"是否显示"改为 0；末行 {0xFF} 为表结束标志，勿删
const SwiIconCfg_t gSwiIconTab1[] = {	// 第一排纽子
	{1, 45, 132, 2,  3, 0, 0},   // 三档
	{0, 85, 132, 1,  2, 1, 0},   // 两档自锁(原映射+1)
	{0, 205,132, 8,  3, 0, 1},   // 三档, map反转
	{0, 245,132, 10, 3, 0, 1},   // 三档, map反转
	{0xFF}
};
const SwiIconCfg_t gSwiIconTab2[] = {	// 第二排纽子
	{0, 45, 170, 4,  3, 0, 0},
	{0, 85, 170, 6,  3, 0, 0},
	{0, 205,170, 12, 3, 0, 0},
	{0, 245,170, 28, 3, 0, 0},
	{0xFF}
};
// 旋钮"图标+数值"显示表（App_lcd.c 的 Disp_ZZT_CtrlStatus 遍历绘制，#if _XN_LiuL 时生效）
// 行格式: {显示, 数值X, 数值Y, 显示位数, 圆弧X, 圆弧Y}
// 圆弧和数值绑定：显示=1两者都画，=0都不画；显示值由 Disp_KnobValTab 取 stKnob[i]
// 末行 {0xFF} 为表结束标志，勿删
const KnobValCfg_t gKnobValTab[] = {
	{0,  54, 102+5, 4,  42, 102+12},		// 左旋钮 数值y=vDispLine1+5 圆弧y=vDispLine1+12
	{0, 257, 102+5, 4, 246, 102+12},		// 右旋钮
	{0xFF}
};

KX119_Recv_t KX_119_recv_dis = {0};
// 无线数据解析(接收机回传): 帧0~3=臂距数据
// 无线数据解析(接收机回传): 帧0/5/6 = 臂距数据，3 帧共 12 个 uint16
// 帧内不再放分组号，8 字节 = 4 个 uint16（小端），由帧类型决定是第几组
void WL_Recv(unsigned char vGrp, const unsigned char* buf)
{
	// 收齐 3 组后立即刷屏，不等 200ms 定时刷新
	static uint8_t stGrpGot = 0;
	uint8_t i;

	if(vGrp >= 3)
		return;

	for(i=0; i<4; i++)
		KX_119_recv_dis.data[vGrp*4 + i].u16Data = buf[i*2] | (buf[i*2+1] << 8);

	stGrpGot |= (uint8_t)(1u << vGrp);
	if(stGrpGot == 0x07)			// 3 组都到齐
	{
		stGrpGot = 0;
		gDataFresh = 1;
		gLCDFlg = 1;
	}
}

// 激活按钮设置
void Activar()
{
	static unsigned int forceDly = 0;	// 强制激活长按计时(DI_Scan 1ms 节拍, 6000=6s)
	// 8个侧边按钮和gDIBitV对应的位是固定不变的
	F.F1 = _BitV(gDIBitV, 16);
	F.F2 = _BitV(gDIBitV, 17);
	F.F3 = _BitV(gDIBitV, 18);
	F.F4 = _BitV(gDIBitV, 19);
	F.F5 = _BitV(gDIBitV, 20);
	F.F6 = _BitV(gDIBitV, 21);
	F.F7 = _BitV(gDIBitV, 22);
	F.F8 = _BitV(gDIBitV, 23);
	// 正常激活：F3~F8 任一按下且自检通过(gSelfChkMask==0)
	if(F.F3 || F.F4 || F.F5 || F.F6 || F.F7 || F.F8)
	{
		if(gSelfChkMask == 0)		// 自检异常不允许激活（信号图标红期间持续自检）
			gLaBaFlg = 1;
	}

	// 强制激活：F5~F8 任一长按 6 秒(1ms*6000)绕过自检，强制置激活位；松开即清零计时，须连续按住
	if(F.F5 || F.F6 || F.F7 || F.F8)
	{
		if(forceDly < 6000) forceDly++;
		if(forceDly >= 6000)
			gLaBaFlg = 1;			// 强制激活，不判 gSelfChkMask
	}
	else
	{
		forceDly = 0;
	}
}

// 本地自检内容：按 KXCAN-GLZYTC-L2X.md 的"开机自检"列逐项检查
// 数据源规避竞态：DI 用 gDIBitV0 ^ 0x8000 现算（gDIBitV 发安全帧时被清零）；
// 摇杆用 VKC_Send（CanOpen 10ms 任务刷新，始终新鲜）；位号与接收机 FSJ_DI 完全一致
// 自检异常原因描述（错误码表沿用发射机惯例：30~37=侧边按键bit16~23，50~58=摇杆通道；0=无异常）
const char* SelfChk_Str(uint32_t mask)
{
	switch(mask)
	{
		case 0:		return "              ";
		case 30:	return "F1";
		case 31:	return "泵站启停";
		case 32:	return "空压机启停";
		case 33:	return "使能键F4";
		case 34:	return "打铃";
		case 35:	return "F6";
		case 36:	return "F7";
		case 37:	return "F8";
		
		case 50:	return "右手柄上下";
		case 51:	return "右手柄左右";
		case 52:	return "左手柄上下";
		case 53:	return "左手柄左右";
		case 54:	return "左手顶钮";
		case 55:	return "右手顶钮";
		default:	return "未知";
	}
}

static void SelfChk_Proc(uint32_t* err_mask)
{
	unsigned long vDI = gDIBitV0 ^ 0x8000;		// 必须用 gDIBitV0（原始扫描值），gDIBitV 未连接发安全帧期间被清零

	if(err_mask == NULL)
		return;
	*err_mask = 0;

	// 依次检查，首个异常的错误码写入 *err_mask（错误码表与 SelfChk_Str 一致）
	if(_BitV(vDI, 16))				*err_mask = 30;		// F1
	else if(_BitV(vDI, 17))			*err_mask = 31;		// 泵站启停
	else if(_BitV(vDI, 18))			*err_mask = 32;		// 空压机启停
	else if(_BitV(vDI, 19))			*err_mask = 33;		// 使能键(F4)
	// 打铃/连接通讯响(0x18A[2].4)=激活键，不作自检项（自检时按下它用于激活）
	else if(_BitV(vDI, 21))			*err_mask = 35;		// F6
	else if(_BitV(vDI, 22))			*err_mask = 36;		// F7
	else if(_BitV(vDI, 23))			*err_mask = 37;		// F8
	else if(VKC_Send[4] != 0x7F)	*err_mask = 50;		// 右手柄上下
	else if(VKC_Send[5] != 0x7F)	*err_mask = 51;		// 右手柄左右
	else if(VKC_Send[2] != 0x7F)	*err_mask = 52;		// 左手柄上下
	else if(VKC_Send[3] != 0x7F)	*err_mask = 53;		// 左手柄左右
	else if(VKC_Send[1] <= 99)		*err_mask = 54;		// 左手顶钮按下（VKC_Send原始字节，对齐HNGL判法）
	else if(_BitV(vDI, 30))			*err_mask = 55;		// 右手顶钮 0x18A[3].6
}
void ADC_Correction()
{
	if(ADCRst[9] > 0xA85)
	{
		ADCRst[9] = 0xA85;
	}
	ADC_value[3] = ADCRst[9] * 0x7F / 0xA85;
}

// 自检状态机，10ms 节拍：0=上电延时300ms → 1=自检循环 → 2=正常运行
// 以屏上信号图标为判据（图标红 = 未激活gLaBaFlg==0 或 无线信号gWLSingV<=1，见 App_lcd.c）：
// 图标红：持续自检，按了立刻报、松开立刻消；自检异常则禁止激活（见 Activar）
// 图标非红（已激活且在线）：正常操控不做自检；若带异常则回自检循环，全部松开才恢复
void SelfChk_main(void)
{
	static unsigned char status = 0;
	static unsigned char dly = 0;

	switch(status)
	{
		case 0: // 上电延时 30*10ms
			if(dly++ > 30)
			{
				dly = 0;
				status = 1;
			}
			break;
		case 1: // 自检循环，全部松开/回中才通过；未连接也检测，异常原因显示在屏幕
			SelfChk_Proc(&gSelfChkMask);
			if(gSelfChkMask == 0)
			{
				gSelfChkOK = 1;						// 自检通过，开始发正常数据
				status = 2;
			}
			break;
		case 2: // 正常运行
			if(gLaBaFlg != 0 && gWLSingV > 1)	// 信号图标非红：已激活且在线，正常操控，不做自检
			{
				if(gSelfChkMask != 0)				// 带着异常（如掉线期间按过键）恢复：回自检循环，全部松开才恢复
					status = 1;
				break;
			}
			SelfChk_Proc(&gSelfChkMask);			// 信号图标红（未激活或链接不到接收机）：持续自检
			if(gSelfChkMask != 0)
				gSelfChkOK = 0;						// 有异常禁止激活（见 Activar）
			else
				gSelfChkOK = 1;
			break;
		default:
			status = 0;
			break;
	}
}

// 自检反馈
uint8_t showWarn(uint32_t t)
{
	if(t != 0)
	{
		return 1;
	}
	return 0;
}

// 清理界面
void clear_lcd(void)
{
	// 清空第一栏，-5是为了不清空电池、信号图标
	for(uint8_t i = 0; i < 320/16 - 5; i++)
	{
		LCD_Disp_Txt(FONT_16, i*16, 5, (uchar*)"  ");
	}

	for(uint8_t y = 0; y < 240/16 - 2; y++)
	{
		for(uint8_t x = 0; x < 320/16; x++)
		{
			LCD_Disp_Txt(FONT_16, x*16, y*16+30, (uchar*)"  ");
		}
	}
}

// 装药台车界面: 左臂0x1C0/中臂0x1D0/右臂0x1E0 各显示左/右/上/前距, 单位mm
// 静态层：仅绘制固定单位（进页面时画一次）
void display_static(void)
{
	static const uint8_t high = 30;        // 行高
	static const uint8_t margL = 10;       // 左边距
	static const uint8_t Title = 30;       // 第一行的高度

	LCD_Set_Color(COLOR_WHITE_, -1); // 字体 设置为白色
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*0, (uchar*)"mm");
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*1, (uchar*)"mm");
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*2, (uchar*)"mm");
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*3, (uchar*)"mm");
}

// 动态层：臂名/标签/数值走差分刷新（只发变化字段），自检警告保留直刷
void display_dynamic(void)
{
	static const uint8_t high = 30;        // 行高
	static const uint8_t margL = 10;       // 左边距
	static const uint8_t margT = 5;		   // 上边距
	static const uint8_t Vermid = 320 / 2; // 竖中间线
	static const uint8_t Title = 30; // 第一行的高度
	static const uint8_t LwordLenth = 8 * 3 + 16 * 2; // 左侧文字长度

	// 臂选择: gDIBitV bit2=左臂 bit3=右臂 皆0=中臂
	uint8_t flag = 0;
	if(_BitV(gDIBitV, 2)) // 左臂
		flag = 0;
	else if(_BitV(gDIBitV, 3)) // 右臂
		flag = 2;
	else // 中臂
		flag = 1;

	if(flag == 0)
	{
		LCD_Set_Color(COLOR_WHITE_, -1);
		Diff_Txt(&gGLZYTCDiff, 0, Vermid-32/2, margT, (uchar*)"左臂", FONT_16);// 第一行显示
		Diff_Txt(&gGLZYTCDiff, 1, margL, Title+high*0, (uchar*)"1#左距:", FONT_16);
		Diff_Txt(&gGLZYTCDiff, 2, margL, Title+high*1, (uchar*)"1#右距:", FONT_16);
		Diff_Txt(&gGLZYTCDiff, 3, margL, Title+high*2, (uchar*)"1#上距:", FONT_16);
		Diff_Txt(&gGLZYTCDiff, 4, margL, Title+high*3, (uchar*)"1#前距:", FONT_16);

		LCD_Set_Color(COLOR_YELLW_, -1);
		Diff_Int(&gGLZYTCDiff, 5, margL+LwordLenth, Title+high*0, KX_119_recv_dis.data[0].u16Data, 5, FONT_16, 0);
		Diff_Int(&gGLZYTCDiff, 6, margL+LwordLenth, Title+high*1, KX_119_recv_dis.data[1].u16Data, 5, FONT_16, 0);
		Diff_Int(&gGLZYTCDiff, 7, margL+LwordLenth, Title+high*2, KX_119_recv_dis.data[2].u16Data, 5, FONT_16, 0);
		Diff_Int(&gGLZYTCDiff, 8, margL+LwordLenth, Title+high*3, KX_119_recv_dis.data[3].u16Data, 5, FONT_16, 0);
	}
	else if(flag == 1)
	{
		LCD_Set_Color(COLOR_WHITE_, -1);
		Diff_Txt(&gGLZYTCDiff, 0, Vermid-32/2, margT, (uchar*)"中臂", FONT_16);// 第一行显示
		Diff_Txt(&gGLZYTCDiff, 1, margL, Title+high*0, (uchar*)"2#左距:", FONT_16);
		Diff_Txt(&gGLZYTCDiff, 2, margL, Title+high*1, (uchar*)"2#右距:", FONT_16);
		Diff_Txt(&gGLZYTCDiff, 3, margL, Title+high*2, (uchar*)"2#上距:", FONT_16);
		Diff_Txt(&gGLZYTCDiff, 4, margL, Title+high*3, (uchar*)"2#前距:", FONT_16);

		LCD_Set_Color(COLOR_YELLW_, -1);
		Diff_Int(&gGLZYTCDiff, 5, margL+LwordLenth, Title+high*0, KX_119_recv_dis.data[4].u16Data, 5, FONT_16, 0);
		Diff_Int(&gGLZYTCDiff, 6, margL+LwordLenth, Title+high*1, KX_119_recv_dis.data[5].u16Data, 5, FONT_16, 0);
		Diff_Int(&gGLZYTCDiff, 7, margL+LwordLenth, Title+high*2, KX_119_recv_dis.data[6].u16Data, 5, FONT_16, 0);
		Diff_Int(&gGLZYTCDiff, 8, margL+LwordLenth, Title+high*3, KX_119_recv_dis.data[7].u16Data, 5, FONT_16, 0);
	}
	else
	{
		LCD_Set_Color(COLOR_WHITE_, -1);
		Diff_Txt(&gGLZYTCDiff, 0, Vermid-32/2, margT, (uchar*)"右臂", FONT_16);// 第一行显示
		Diff_Txt(&gGLZYTCDiff, 1, margL, Title+high*0, (uchar*)"3#左距:", FONT_16);
		Diff_Txt(&gGLZYTCDiff, 2, margL, Title+high*1, (uchar*)"3#右距:", FONT_16);
		Diff_Txt(&gGLZYTCDiff, 3, margL, Title+high*2, (uchar*)"3#上距:", FONT_16);
		Diff_Txt(&gGLZYTCDiff, 4, margL, Title+high*3, (uchar*)"3#前距:", FONT_16);

		LCD_Set_Color(COLOR_YELLW_, -1);
		Diff_Int(&gGLZYTCDiff, 5, margL+LwordLenth, Title+high*0, KX_119_recv_dis.data[8].u16Data, 5, FONT_16, 0);
		Diff_Int(&gGLZYTCDiff, 6, margL+LwordLenth, Title+high*1, KX_119_recv_dis.data[9].u16Data, 5, FONT_16, 0);
		Diff_Int(&gGLZYTCDiff, 7, margL+LwordLenth, Title+high*2, KX_119_recv_dis.data[10].u16Data, 5, FONT_16, 0);
		Diff_Int(&gGLZYTCDiff, 8, margL+LwordLenth, Title+high*3, KX_119_recv_dis.data[11].u16Data, 5, FONT_16, 0);
	}

	// 自检异常提示：保留直刷（可靠显示，不参与差分）
	uint8_t strBuf[32];
	uint32_t mask = gSelfChkMask;
	if(showWarn(mask))
	{
		LCD_Set_Color(COLOR_RED_, -1);
		snprintf((char*)strBuf, sizeof(strBuf), "自检异常:%s", SelfChk_Str(mask));
		LCD_Disp_Txt(FONT_16, margL, Title+high*5, strBuf);
	}
	else
	{
		LCD_Disp_Txt(FONT_16, margL, Title+high*5, "                        ");
	}

	Diff_EndFrame(&gGLZYTCDiff);
}

// 主要逻辑：每 N 帧刷新动态数据
void MainLogic(void)
{
    display_dynamic();
}

// 进页面：绘制静态文字并请求整页全量（前 2 帧强制全发，之后仅发变化字段）
void MainLogic_Enter(void)
{
    display_static();
    Diff_PageEnter(&gGLZYTCDiff);
}

#endif // (USER_TYPE == KXCAN_GLZYTC_L2X)
