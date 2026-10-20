#include "_Device_config.h"
#include "App_usr.h"
#if (USER_TYPE == KX119_GLMZ213_L3X)

Activar_t F = {0};
// 扭子开关图标配置表（App_lcd.c 的 Disp_SwiIconRow 遍历绘制）
// 行格式: {是否显示, 屏幕X, 屏幕Y, gDIBitV移位数, 几档开关(2/3), 是否自锁(0/1), 显示映射(0/1)}
// 两档自锁: 图标仅上/下; 两档自复位: 触发/中; 三档: 上/中/下(自锁自复位同)
// map=1: 上/下反转(交换图标1/2, 中态0不变) —— 实际显示与逻辑相反时置1
// shift 对应 gDIBitV 位号（与接收机 FSJ_DI 位布局一致：见接收机 .c 注释）
// gDIBitV 位号(=接收机 FSJ_DI 位号, 依据《213遥控器测试结果》实测 gDIBitV 布局)：
//   水泵=0 搅拌=1 注浆正/反=2/3 水阀开/关=4/5 换钎=15(开=1) 空压机=28/29(开=1,两bit联动)
//   电机启停=16 钻臂=17 锚杆=18 吊篮=19 F5~F8=20~23 启动=24 手柄2/3顶钮=30
//   注水/注浆/水阀 三档开关: bit(bit) 与 bit(bit+1) 为高/低两DI，两bit均0为中位
// 面板布局(协议"发射机布局说明")：
//   第一排: 1.1 水泵(2档自锁)  1.2 水阀(3档自复位)  1.4 换钎(2档自锁)
//   第二排: 2.1 注浆(3档自锁)  2.2 搅拌(2档自锁)     2.4 空压机(2档自锁)
//   右侧还有 1 个旋钮 (推进调压, 无线帧 byte15 = ADCRst[9] 缩放 0~127)
// 增删图标: 增删一行；末行 {0xFF} 为表结束标志，勿删
const SwiIconCfg_t gSwiIconTab1[] = {	// 第一排纽子
	{1, 45, 132, 0,  2, 1, 0},   // 1.1 水泵 两档自锁 (bit0: 关=1, 开=0)
	{1, 85, 132, 4,  3, 0, 1},   // 1.2 水阀 三档自复位 (bit4=开, bit5=关, 中位两bit均0)
	{0, 205,132, 0,  2, 0, 0},   // 1.3 无扭子开关(隐藏)
	{1, 245,132, 15, 2, 1, 1},   // 1.4 换钎 两档自锁 (bit15: 开=1, 关=0)
	{0xFF}
};
const SwiIconCfg_t gSwiIconTab2[] = {	// 第二排纽子
	{1, 45, 170, 2,  3, 1, 1},   // 2.1 注浆 三档自锁 (bit2=正, bit3=反, 中位两bit均0)
	{1, 85, 170, 1,  2, 1, 0},   // 2.2 搅拌 两档自锁 (bit1: 关=1, 开=0)
	{0, 205,170, 0,  2, 0, 0},   // 2.3 无扭子开关(隐藏)
	{1, 245,170, 29, 2, 1, 1},   // 2.4 空压机 两档自锁 (bit28/29: 开=1, 关=0)
	{0xFF}
};
// 行格式: {显示, ADCRst的AD通道, 数值X, 数值Y, 显示位数, 圆弧X, 圆弧Y}
// 圆弧和数值绑定：显示=1两者都画，=0都不画；显示的是AD原始采样值(0~4095)
// 末行 {0xFF} 为表结束标志，勿删
const KnobValCfg_t gKnobValTab[] = {
	{0, _XN1_AD_CHN,  54, 102+5, 4,  42, 102+12},		// 左旋钮 数值y=vDispLine1+5 圆弧y=vDispLine1+12
	{1, _XN2_AD_CHN, 257, 102+5, 4, 246, 102+12},		// 右旋钮
	{0xFF}
};

KX119_Recv_t KX_119_recv_dis = {0};
// 无线数据解析(接收机回传): 帧0=0x220, 帧1=0x221, 帧2=0x222，每帧 8 字节 = 4 个 uint16（小端）
// 0x220: B0=工作模式, B1~2=回转压力, B3~4=冲击压力, B5~6=推进压力, B7=水压
// 0x221: B0=润滑气压, B1~2=预应力, B3~4=锁定扭矩, B5=孔数, B6~7=钻孔深度
// 0x222: B0~1=锚杆深度, B2~3=注浆压力, B4~5=注浆量, B6~7=未使用
void WL_Recv(unsigned char vGrp, const unsigned char* buf)
{
    if(vGrp > 2)   // GLMZ213 仅 0x220/0x221/0x222 三帧反馈，其余组忽略
        return;

    if(vGrp == 0)
    {
        KX_119_recv_dis.data[0].u8Data[0] = buf[0];                  // 工作模式
        KX_119_recv_dis.data[0].u8Data[1] = buf[7];                  // 水压
        KX_119_recv_dis.data[1].u16Data   = buf[1] | (buf[2] << 8);  // 回转压力
        KX_119_recv_dis.data[2].u16Data   = buf[3] | (buf[4] << 8);  // 冲击压力
        KX_119_recv_dis.data[3].u16Data   = buf[5] | (buf[6] << 8);  // 推进压力
    }
    else if(vGrp == 1)
    {
        KX_119_recv_dis.data[4].u8Data[0] = buf[0];                  // 润滑气压
        KX_119_recv_dis.data[4].u8Data[1] = buf[5];                  // 孔数
        KX_119_recv_dis.data[5].u16Data   = buf[1] | (buf[2] << 8);  // 预应力
        KX_119_recv_dis.data[6].u16Data   = buf[3] | (buf[4] << 8);  // 锁定扭矩
        KX_119_recv_dis.data[7].u16Data   = buf[6] | (buf[7] << 8);  // 钻孔深度
    }
    else // vGrp == 2
    {
        KX_119_recv_dis.data[8].u16Data   = buf[0] | (buf[1] << 8);  // 锚杆深度
        KX_119_recv_dis.data[9].u16Data   = buf[2] | (buf[3] << 8);  // 注浆压力
        KX_119_recv_dis.data[10].u16Data  = buf[4] | (buf[5] << 8);  // 注浆量
        KX_119_recv_dis.data[11].u16Data  = 0;                       // 未使用
    }

    gDataFresh = 1;
    gLCDFlg = 1;
}

// 激活按钮设置
void Activar()
{
    static unsigned int forceDly = 0;   // 强制激活长按计时(DI_Scan 1ms 节拍, 6000=6s)，仅本函数内使用

    F.F1 = _BitV(gDIBitV, 16);
    F.F2 = _BitV(gDIBitV, 17);
    F.F3 = _BitV(gDIBitV, 18);
    F.F4 = _BitV(gDIBitV, 19);
    F.F5 = _BitV(gDIBitV, 20);
    F.F6 = _BitV(gDIBitV, 21);
    F.F7 = _BitV(gDIBitV, 22);
    F.F8 = _BitV(gDIBitV, 23);
    // gDIBitV 位映射已按《213遥控器测试结果》确认：16=电机 17=钻臂 18=锚杆 19=吊篮 20~23=F5~F8

    // 正常激活：F3~F8 任一按下且自检通过(gSelfChkMask==0)
    if(F.F3 || F.F4 || F.F5 || F.F6 || F.F7 || F.F8)
    {
        if(gSelfChkMask == 0)
            gLaBaFlg = 1;
    }

    // 强制激活：F5~F8 任意一个长按 6 秒(1ms*6000)绕过自检，强制置激活位
    // 用途：自检异常(如某开关卡死)时仍可通过长按 F5~F8 强制激活；松开即清零计时，须连续按住
    if(F.F5 || F.F6 || F.F7 || F.F8)
    {
        if(forceDly < 6000) forceDly++;
        if(forceDly >= 6000)
            gLaBaFlg = 1;           // 强制激活，不判 gSelfChkMask
    }
    else
    {
        forceDly = 0;
    }
}

// 自检异常描述（错误码表与 SelfChk_Proc 一致：10~20 开关/模式，30~34 自复位按钮，50~55 手柄）
const char* SelfChk_Str(uint32_t mask)
{
    switch(mask)
    {
        case 0:     return "              ";
        case 10:    return "水泵未关";
        case 11:    return "搅拌未关";
        case 12:    return "换钎未关";
        case 13:    return "空压机未关";
        case 14:    return "水阀未在中位";
        case 15:    return "注浆未在中位";
        case 16:    return "电机启停未复位";
        case 17:    return "钻臂模式未复位";
        case 18:    return "锚杆模式未复位";
        case 19:    return "吊篮模式未复位";
        case 20:    return "启动未复位";
        case 30:    return "F5未复位";
        case 31:    return "F6未复位";
        case 32:    return "F7未复位";
        case 33:    return "F8未复位";
        case 34:    return "手柄1顶钮未复位";
		case 35:    return "手柄2顶钮未复位";
		case 36:    return "手柄3顶钮未复位";
        case 50:    return "手柄1上下未回中";
        case 51:    return "手柄1左右未回中";
        case 52:    return "手柄2上下未回中";
        case 53:    return "手柄2左右未回中";
        case 54:    return "手柄3上下未回中";
        case 55:    return "手柄3左右未回中";
        default:    return "未知";
    }
}

// 逐项检查，首个异常写入 *err_mask（0=无异常）。位号以《213遥控器测试结果》实测 gDIBitV 为准
// 数据源用 gDIBitV0^0x8000（原始扫描值）：安全帧期间 gDIBitV 被清零，不能直接用
static void SelfChk_Proc(uint32_t* err_mask)
{
    unsigned long vDI = gDIBitV0 ^ 0x8000;

    if(err_mask == NULL)
        return;
    *err_mask = 0;

    // 2档自锁：安全态=关，极性别按测试结果单独判断
    if(_BitV(vDI, 0) == 0)                        *err_mask = 10;  // 水泵未关(bit0=1为关)
    else if(_BitV(vDI, 1) == 0)                  *err_mask = 11;  // 搅拌未关(bit1=1为关)
    else if(_BitV(vDI, 15))                      *err_mask = 12;  // 换钎未关(bit15=1为开)
    else if(_BitV(vDI, 28) || _BitV(vDI, 29))    *err_mask = 13;  // 空压机未关(bit28/29=1为开)
    // 3档：安全态=中位(两DI均为0)
    else if(_BitV(vDI, 4) || _BitV(vDI, 5))      *err_mask = 14;  // 水阀未中位(bit4=开,bit5=关)
    else if(_BitV(vDI, 2) || _BitV(vDI, 3))      *err_mask = 15;  // 注浆未中位(bit2=正,bit3=反)
    // 其它开关：安全态=复位(0)
    else if(_BitV(vDI, 16))                      *err_mask = 16;  // 电机启停未复位
    else if(_BitV(vDI, 17))                      *err_mask = 17;  // 钻臂模式未复位
    else if(_BitV(vDI, 18))                      *err_mask = 18;  // 锚杆模式未复位
    else if(_BitV(vDI, 19))                      *err_mask = 19;  // 吊篮模式未复位
    // 自复位按钮：安全态=复位(0)
    else if(_BitV(vDI, 20))                      *err_mask = 30;  // F5
    else if(_BitV(vDI, 21))                      *err_mask = 31;  // F6
    else if(_BitV(vDI, 22))                      *err_mask = 32;  // F7
    else if(_BitV(vDI, 23))                      *err_mask = 33;  // F8(打铃)
	else if(_BitV(vDI, 6))                       *err_mask = 34;  // 手柄1顶钮
    else if(_BitV(vDI, 30))                      *err_mask = 35;  // 手柄2顶钮(0x18A.3.6)
	else if(_BitV(vDI, 7))                       *err_mask = 36;  // 手柄3顶钮
    // 手柄必须回中位(0x7F)
    else if(VKC_Send[2] != 0x7F)                  *err_mask = 50;  // 手柄1上下
    else if(VKC_Send[3] != 0x7F)                  *err_mask = 51;  // 手柄1左右
    else if(VKC_Send[4] != 0x7F)                  *err_mask = 52;  // 手柄2上下
    else if(VKC_Send[5] != 0x7F)                  *err_mask = 53;  // 手柄2左右
    else if(VKC_Send[6] != 0x7F)                  *err_mask = 54;  // 手柄3上下
    else if(VKC_Send[7] != 0x7F)                  *err_mask = 55;  // 手柄3左右
}
// DI修正：当开关量对应模拟量时，在此修正
void DI_Correction()
{
	if(ADCRst[16] > 500) // 修正DI9 空压机
	{
		gDIBitV0 |= (1<<29);
		gDIBitV |= (1<<29);
	}
	else
	{
		gDIBitV0 &= ~(1<<29);
		gDIBitV &= ~(1<<29);
	}

	if(VKC_Send[1] == 0) // 手柄1顶部按钮修正
	{
		gDIBitV0 |= (1<<6);
		gDIBitV |= (1<<6);
	}
	else
	{
		gDIBitV0 &= ~(1<<6);
		gDIBitV &= ~(1<<6);
	}
	
	if(ADCRst[8] > 500) // 手柄3顶部按钮修正
	{
		gDIBitV0 |= (1<<7);
		gDIBitV |= (1<<7);
	}
	else
	{
		gDIBitV0 &= ~(1<<7);
		gDIBitV &= ~(1<<7);
	}
}
// ADC重映射
void ADC_Correction()
{
	if(ADCRst[9] > 0xA70)
		ADCRst[9] = 0xA70;
	else if(ADCRst[9] < 0x7)
		ADCRst[9] = 0;
	ADCRst[9] = ADCRst[9] * 0x7F / 0xA70;
}
// 自检状态机，10ms 节拍：0=上电延时300ms → 1=自检循环 → 2=正常运行
// 以屏上信号图标为判据（图标红=未激活gLaBaFlg==0 或 无线信号gWLSingV<=1）：
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
        case 1: // 自检循环，全部松开/回中才通过
            SelfChk_Proc(&gSelfChkMask);
            if(gSelfChkMask == 0)
            {
                gSelfChkOK = 1;
                status = 2;
            }
            break;
        case 2: // 正常运行
            if(gLaBaFlg != 0 && gWLSingV > 1)   // 已激活且在线：正常操控，不做自检
            {
                if(gSelfChkMask != 0)           // 带异常(掉线期间按过键)恢复：回自检循环，全部松开才恢复
                    status = 1;
                break;
            }
            SelfChk_Proc(&gSelfChkMask);        // 信号图标红：持续自检
            if(gSelfChkMask != 0)
                gSelfChkOK = 0;                 // 有异常禁止激活（见 Activar）
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
        return 1;
    return 0;
}

// 清理界面
void clear_lcd(void)
{
    for(uint8_t i = 0; i < 320/16 - 5; i++)
        LCD_Disp_Txt(FONT_16, i*16, 5, (uchar*)"  ");
    for(uint8_t y = 0; y < 240/16 - 2; y++)
        for(uint8_t x = 0; x < 320/16; x++)
            LCD_Disp_Txt(FONT_16, x*16, y*16+30, (uchar*)"  ");
}

static const uint8_t high = 22;        // 行高
static const uint8_t margR = 10;       // 左边距
static const uint8_t margL = 10;       // 左边距
static const uint8_t margT = 5;		   // 上边距
static const uint8_t Title = 30; // 第一行的高度

// 显示车状态反馈 0x220 / 0x221 / 0x222
void display()
{
    uint8_t mode = KX_119_recv_dis.data[0].u8Data[0];
    LCD_Set_Color(COLOR_WHITE_, -1);
	uint8_t temp= 20; // 补偿
    // 左列
    LCD_Disp_Txt(FONT_16, S_WIDTH/2-(16*4+8*1)/2-temp, margT, (uchar*)"模式:");
    if(_BitV(mode, 0))
        LCD_Disp_Txt(FONT_16, S_WIDTH/2-(16*4+8*1)/2-temp+(16*2+8), margT, (uchar*)"钻臂");
    else if(_BitV(mode, 1))
        LCD_Disp_Txt(FONT_16, S_WIDTH/2-(16*4+8*1)/2-temp+(16*2+8), margT, (uchar*)"锚杆");
    else if(_BitV(mode, 2))
        LCD_Disp_Txt(FONT_16, S_WIDTH/2-(16*4+8*1)/2-temp+(16*2+8), margT, (uchar*)"吊篮");
    else
        LCD_Disp_Txt(FONT_16, S_WIDTH/2-(16*4+8*1)/2-temp+(16*2+8), margT, (uchar*)"  --");

    LCD_Disp_Txt(FONT_16, margL, Title+high*0, (uchar*)"回转压力:");
	LCD_Disp_Txt(FONT_16, margL, Title+high*1, (uchar*)"冲击压力:");
	LCD_Disp_Txt(FONT_16, margL, Title+high*2, (uchar*)"推进压力:");
	LCD_Disp_Txt(FONT_16, margL, Title+high*3, (uchar*)"注浆压力:");
	LCD_Disp_Txt(FONT_16, margL, Title+high*4, (uchar*)"润滑气压:");
	LCD_Disp_Txt(FONT_16, margL, Title+high*5, (uchar*)"水压:");

	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*0, (uchar*)"bar");
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*1, (uchar*)"bar");
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*2, (uchar*)"bar");
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*3, (uchar*)"Mpa");
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*4, (uchar*)"bar");
	LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*5, (uchar*)"bar");

	LCD_Set_Color(COLOR_YELLW_, -1);
    LCD_Disp_Intx(FONT_16, margL+16*4+8, Title+high*0, KX_119_recv_dis.data[1].u16Data, 5); //回转压力
    LCD_Disp_Intx(FONT_16, margL+16*4+8, Title+high*1, KX_119_recv_dis.data[2].u16Data, 5); //冲击压力
    LCD_Disp_Intx(FONT_16, margL+16*4+8, Title+high*2, KX_119_recv_dis.data[3].u16Data, 5); //推进压力
	LCD_Disp_Intx(FONT_16, margL+16*4+8, Title+high*3, KX_119_recv_dis.data[9].u16Data, 5); // 注浆压力
	LCD_Disp_Intx(FONT_16, margL+16*4+8, Title+high*4, KX_119_recv_dis.data[4].u8Data[0], 5); // 润滑气压
	LCD_Disp_Intx(FONT_16, margL+16*4+8, Title+high*5, KX_119_recv_dis.data[0].u8Data[1], 5); // 水压

    // 右列
    LCD_Set_Color(COLOR_WHITE_, -1);
    LCD_Disp_Txt(FONT_16, S_WIDTH/2 + margR, Title+high*0, (uchar*)"预应力:");
	LCD_Disp_Txt(FONT_16, S_WIDTH/2 + margR, Title+high*1, (uchar*)"锁定扭矩:");
	LCD_Disp_Txt(FONT_16, S_WIDTH/2 + margR, Title+high*2, (uchar*)"孔数:");
	LCD_Disp_Txt(FONT_16, S_WIDTH/2 + margR, Title+high*3, (uchar*)"钻孔深度:");
	LCD_Disp_Txt(FONT_16, S_WIDTH/2 + margR, Title+high*4, (uchar*)"锚杆深度:");
	LCD_Disp_Txt(FONT_16, S_WIDTH/2 + margR, Title+high*5, (uchar*)"注浆量:");
	LCD_Disp_Txt(FONT_16, S_WIDTH/2 + margR, Title+high*6, (uchar*)"水流量开关:");
	
	LCD_Disp_Txt(FONT_16, S_WIDTH/2 + margR + 16*4+8*6, Title+high*0, (uchar*)"kN");
	LCD_Disp_Txt(FONT_16, S_WIDTH/2 + margR + 16*4+8*6, Title+high*1, (uchar*)"Nm");
	LCD_Disp_Txt(FONT_16, S_WIDTH/2 + margR + 16*4+8*6, Title+high*2, (uchar*)"个");
	LCD_Disp_Txt(FONT_16, S_WIDTH/2 + margR + 16*4+8*6, Title+high*3, (uchar*)"mm");
	LCD_Disp_Txt(FONT_16, S_WIDTH/2 + margR + 16*4+8*6, Title+high*4, (uchar*)"mm");
	LCD_Disp_Txt(FONT_16, S_WIDTH/2 + margR + 16*4+8*6, Title+high*5, (uchar*)"L");

	LCD_Set_Color(COLOR_YELLW_, -1);
    LCD_Disp_Intx(FONT_16, S_WIDTH/2 + margR+16*4+8, Title+high*0, KX_119_recv_dis.data[5].u16Data, 5);
    LCD_Disp_Intx(FONT_16, S_WIDTH/2 + margR+16*4+8, Title+high*1, KX_119_recv_dis.data[6].u16Data, 5);
    LCD_Disp_Intx(FONT_16, S_WIDTH/2 + margR+16*4+8, Title+high*2, KX_119_recv_dis.data[4].u8Data[1], 3);
    LCD_Disp_Intx(FONT_16, S_WIDTH/2 + margR+16*4+8, Title+high*3, KX_119_recv_dis.data[7].u16Data, 5);
    LCD_Disp_Intx(FONT_16, S_WIDTH/2 + margR+16*4+8, Title+high*4, KX_119_recv_dis.data[8].u16Data, 5);
    LCD_Disp_Intx(FONT_16, S_WIDTH/2 + margR+16*4+8, Title+high*5, KX_119_recv_dis.data[10].u16Data, 5);
	if(_BitV(mode, 3))
		LCD_Disp_Txt(FONT_16, S_WIDTH/2 + margR+16*5+8, Title+high*6, (uchar*)"打开");
	else
		LCD_Disp_Txt(FONT_16, S_WIDTH/2 + margR+16*5+8, Title+high*6, (uchar*)"关闭");

}
// 自检异常提示：gSelfChkMask!=0 时在状态页显示红字（与 GLZYTC 同款）
void selfDisplay(void)
{
    uint8_t strBuf[32];
    uint32_t mask = gSelfChkMask;
    if(showWarn(mask))
    {
        LCD_Set_Color(COLOR_RED_, -1);
        snprintf((char*)strBuf, sizeof(strBuf), "自检异常:%s", SelfChk_Str(mask));
        LCD_Disp_Txt(FONT_16, margL, 190, strBuf);
    }
    else
    {
        LCD_Disp_Txt(FONT_16, margL, 190, "                        ");
    }
}

// 主要逻辑
void MainLogic(void)
{
    display();
    selfDisplay();
}

#endif // (USER_TYPE == KX119_GLMZ213_L3X)
