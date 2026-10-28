#include "_Device_config.h"
#include "App_usr.h"
#if (USER_TYPE == KX119_GLMZ_L2X3J)

// @ref ZZZ_project_data/KX119-GLMZ-L2X3J.md （协议 + 测试均已完成）
// gDIBitV 位号推导：0x18A 无线帧 = gDIBitV 的 4 字节(bit = B*8 + C，数字/模拟/手柄顶钮都在这 32 位里)
//   数字开关由 DI 扫描 gDIBitV0 |=(1<<i) 直接得到；模拟量/手柄顶钮开关经 App_IO.c 通用映射 或 本文件 DI_Correction 映射。
//   水泵=0(关=1/开=0)  空压机=25(关=1/开=0)  水阀开/关=2/3(三档自复位)  换钎=15(开=1/关=0)
//   注浆关/开(反/正)=8/9(三档自锁)  搅拌=1(开=1/关=0, 模拟量ADCRst[16]>500, 经DI_Correction)  F1~F4=16~19(侧边自复位)
//   电机启停=20  转臂模式=21  锚杆模式=22  吊篮模式=23
//   右手柄使能=28(模拟量ADCRst[1]>500, App_IO.c通用映射)  左手柄使能=6(VKC_Send[1]==0, 经DI_Correction)
//   启动/打铃=F4(=19)。急停走 vYKJiTing(负逻辑), 非FSJ_DI位
// 直线摇杆：1=ADCRst[3](0x189.6&7, 上0xE35/中0x7EE/下0x1A0)  2=VKC_Send[5](0x188.5, 摇杆轴)  3=ADCRst[9](0x18B.4&5, 上0xE36/中0x7F1/下0x1AB)
//   1/3 为模拟量, 经 ADC_Correction 线性映射(1->0~255走ad1槽, 3->0~126走ad2槽)后发往接收机; 2 为摇杆轴走VKCSend[5]
// 协议差异：0x1A0 的 B1.b2/b3(注浆位) 与 B2 为保留 -> 注浆/搅拌不向车转发（仅遥控端屏显）

Activar_t F = {0};
unsigned char gLinR1Send = 0;  // 直线摇杆1 映射值(0~255), ADC_Correction 写入, 走无线帧 ad1 槽(索引8)
unsigned char gLinR3Send = 0;  // 直线摇杆3 映射值(0~255), ADC_Correction 写入, 走无线帧 ad2 槽(索引9)
static DiffCtx gL2X3JDiff;   // 差分刷新上下文（调用方持有，每个屏一个）

// 扭子开关图标配置表（App_lcd.c 的 Disp_SwiIconRow 遍历绘制）
// 行格式: {是否显示, 屏幕X, 屏幕Y, gDIBitV移位数, 几档开关(2/3), 是否自锁(0/1), 显示映射(0/1)}
// 两档自锁: 图标仅上/下; 两档自复位: 触发/中; 三档: 上/中/下(自锁自复位同)
// map=1: 上/下反转(交换图标1/2, 中态0不变) —— 实际显示与逻辑相反时置1
// shift 对应 gDIBitV 位号（与接收机 FSJ_DI 位布局一致）
// 面板布局(测试"扭子开关布局")：
//   第一排: 1.1 水泵(2档自锁)  1.2 空压机(2档自锁)  1.3 换钎(2档自锁)  1.4 注浆(3档自锁)
//   第二排: 2.1 水阀(3档自复位)  2.4 搅拌(2档自锁)
// 增删图标: 增删一行；末行 {0xFF} 为表结束标志，勿删
const SwiIconCfg_t gSwiIconTab1[] = {	// 第一排纽子
	{1, 45, 132, 0,  2, 1, 0},   // 1.1 水泵 两档自锁 (bit0: 关=1, 开=0)
	{1, 85, 132, 25, 2, 1, 0},   // 1.2 空压机 两档自锁 (bit25: 关=1, 开=0)
	{1, 205,132, 15, 2, 1, 1},   // 1.3 换钎 两档自锁 (bit15: 开=1, 关=0)
	{1, 245,132, 8,  3, 1, 0},   // 1.4 注浆 三档自锁 (bit8=关/反, bit9=开/正, 中位两bit均0)
	{0xFF}
};
const SwiIconCfg_t gSwiIconTab2[] = {	// 第二排纽子
	{1, 45, 170, 2,  3, 0, 1},   // 2.1 水阀 三档自复位 (bit2=开, bit3=关, 中位两bit均0)
	{0, 85, 170, 0,  2, 0, 0},   // 2.2 无扭子开关(隐藏)
	{0, 205,170, 0,  2, 0, 0},   // 2.3 无扭子开关(隐藏)
	{1, 245,170, 1,  2, 1, 1},   // 2.4 搅拌 两档自锁 (bit1: 开=1/关=0; 模拟量 ADCRst[16]>500, 经 DI_Correction 映射)
	{0xFF}
};
// 无流量旋钮（_XN_LiuL=0）
const KnobValCfg_t gKnobValTab[] = {
	{0xFF}
};

KX119_Recv_t KX_119_recv_dis = {0};
// 无线数据解析(接收机回传 0x220 状态反馈): 仅 1 组(vGrp=0)，8 字节 = 模式 + 3×uint16压力 + 水压
//   字节0=工作模式, 1~2=回转压力, 3~4=冲击压力, 5~6=推进压力, 7=水压
void WL_Recv(unsigned char vGrp, const unsigned char* buf)
{
    if(vGrp != 0)   // GLMZ 仅 0x220 一帧反馈，其余组忽略
        return;

    KX_119_recv_dis.data[0].u8Data[0] = buf[0];                       // 工作模式
    KX_119_recv_dis.data[1].u16Data   = buf[1] | (buf[2] << 8);       // 回转压力
    KX_119_recv_dis.data[2].u16Data   = buf[3] | (buf[4] << 8);       // 冲击压力
    KX_119_recv_dis.data[3].u16Data   = buf[5] | (buf[6] << 8);       // 推进压力
    KX_119_recv_dis.data[4].u8Data[0] = buf[7];                       // 水压

    gDataFresh = 1;
    gLCDFlg = 1;
}

// 激活按钮设置
// 本机型仅 F1~F4 四个侧边自复位按钮（无 F5~F8）；bit20~23 为电机/转臂/锚杆/吊篮模式开关（非侧边按钮）
// gDIBitV 位映射已按《KX119-GLMZ-L2X3J.md 测试》确认：F1~F4 = bit16~19
void Activar()
{
    static unsigned int forceDly = 0;   // 强制激活长按计时(DI_Scan 1ms 节拍, 6000=6s)
    F.F1 = _BitV(gDIBitV, 16);
    F.F2 = _BitV(gDIBitV, 17);
    F.F3 = _BitV(gDIBitV, 18);
    F.F4 = _BitV(gDIBitV, 19);
    F.F5 = 0;
    F.F6 = 0;
    F.F7 = 0;
    F.F8 = 0;

    // 正常激活：任一 F1~F4 侧边按钮按下且自检通过(gSelfChkMask==0)
    if(F.F1 || F.F2 || F.F3 || F.F4)
    {
        if(gSelfChkMask == 0)       // 自检异常不允许激活
            gLaBaFlg = 1;
    }

    // 强制激活：本机型仅 F1~F4(无 F5~F8)，任一长按 6 秒(1ms*6000)绕过自检，强制置激活位；松开即清零计时，须连续按住
    if(F.F1 || F.F2 || F.F3 || F.F4)
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

// 自检异常描述（错误码表与 SelfChk_Proc 一致：10~20 开关/模式，30~34 自复位按钮，50~56 手柄/直线摇杆）
const char* SelfChk_Str(uint32_t mask)
{
    switch(mask)
    {
        case 0:     return "               ";
        case 10:    return "水泵未关        ";
        case 11:    return "空压机未关      ";
        case 12:    return "换钎未关        ";
        case 13:    return "搅拌未关        ";
        case 14:    return "水阀未在中位    ";
        case 15:    return "注浆未在中位    ";
        case 16:    return "电机未复位      ";
        case 17:    return "转臂模式未复位  ";
        case 18:    return "锚杆模式未复位  ";
        case 19:    return "吊篮模式未复位  ";
        case 30:    return "F1未复位       ";
        case 31:    return "F2未复位       ";
        case 32:    return "F3未复位       ";
        case 33:    return "F4未复位       ";
        case 50:    return "左手柄上下未回中";
        case 51:    return "左手柄左右未回中";
        case 52:    return "右手柄上下未回中";
        case 53:    return "右手柄左右未回中";
        case 54:    return "直线摇杆1未回中 ";
        case 55:    return "直线摇杆2未回中 ";
        case 56:    return "直线摇杆3未回中 ";
        default:    return "未知           ";
    }
}
// DI 修正：当开关量记录的是模拟量/手柄顶钮时，在此映射到 gDIBitV（参考 213 的 DI_Correction）
// App_IO.c 在 gDIBitV = gDIBitV0 ^ 0x8000 之前调用本函数
void DI_Correction(void)
{
    // 搅拌：模拟量 0x18D.4&5 = ADCRst[16](16bit)，>500 表示开
    if(ADCRst[16] > 500)
    {
        gDIBitV0 |= (1<<1);
        gDIBitV  |= (1<<1);
    }
    else
    {
        gDIBitV0 &= ~(1<<1);
        gDIBitV  &= ~(1<<1);
    }

    // 左手柄顶部按钮(使能)：0x188.1 = Y_Send[1] -> VKC_Send[1]，==0 表示按下(使能)
    if(VKC_Send[1] == 0)
    {
        gDIBitV0 |= (1<<6);
        gDIBitV  |= (1<<6);
    }
    else
    {
        gDIBitV0 &= ~(1<<6);
        gDIBitV  &= ~(1<<6);
    }
}

// ADC 修正：模拟量直线摇杆映射到 0~255 手柄量程，写入槽位变量（参考 213 的 ADC_Correction）
// 注：直线摇杆1/3 为模拟量(ADCRst)，不走 VKC_Send 摇杆轴，直接放入无线帧 ad1/ad2 槽发往接收机，
//     从而避开 KX_Send_joylogic 的(4,5)轴对主轴优先级逻辑（两路同时动时从轴被清零）。
//     直线摇杆2 是摇杆轴(VKC_Send[5])，由 KX_Send_joylogic 正常处理。
void ADC_Correction(void)
{
	ADC_value[0] = Analog_Remap(&ADCRst[3], 0x1A0, 0x7EE, 0xE35, 10, 0, 0x7F, 255); // 直线手柄1
	ADC_value[1] = Analog_Remap(&ADCRst[8], 0x1AB, 0x7EE, 0xE35, 10, 0, 0x7F, 255); // 首先手柄3
}
// 直线手柄值显示
void Straight_handle()
{
	char str[8] = {0};
	snprintf(str, sizeof(str), "%-3d", (ADC_value[0]+1) * 126 / 255);
	LCD_Disp_Txt(FONT_16, 120, 140, str);
	snprintf(str, sizeof(str), "%-3d", 126 - (VKC_Send[5]+1) * 126 / 255);
	LCD_Disp_Txt(FONT_16, 150, 140, str);
	snprintf(str, sizeof(str), "%-3d", (ADC_value[1]+1) * 126 / 255);
	LCD_Disp_Txt(FONT_16, 180, 140, str);
}
// 逐项检查，首个异常写入 *err_mask（0=无异常）。位号以《KX119-GLMZ-L2X3J.md 测试》实测 gDIBitV 为准
// 数据源用 gDIBitV0^0x8000（原始扫描值）：安全帧期间 gDIBitV 被清零，不能直接用
static void SelfChk_Proc(uint32_t* err_mask)
{
    unsigned long vDI = gDIBitV0 ^ 0x8000;

    if(err_mask == NULL)
        return;
    *err_mask = 0;

    // 2档自锁：安全态=关
    if(_BitV(vDI, 0) == 0)                        *err_mask = 10;  // 水泵未关(bit0=1为关)
    else if(_BitV(vDI, 25) == 0)                  *err_mask = 11;  // 空压机未关(bit25=1为关)
    else if(_BitV(vDI, 15))                       *err_mask = 12;  // 换钎未关(bit15=1为开)
    else if(_BitV(vDI, 1))                        *err_mask = 13;  // 搅拌未关(bit1=1为开, 需回到关=0; DI_Correction: ADCRst[16]>500)
    // 3档：安全态=中位(两DI均为0)
    else if(_BitV(vDI, 2) || _BitV(vDI, 3))       *err_mask = 14;  // 水阀未中位(bit2=开,bit3=关)
    else if(_BitV(vDI, 8) || _BitV(vDI, 9))       *err_mask = 15;  // 注浆未中位(bit8=关/反,bit9=开/正)
    // 其它开关（模式/使能）：安全态=复位(0)
    else if(_BitV(vDI, 20))                       *err_mask = 16;  // 电机启停未复位
    else if(_BitV(vDI, 21))                       *err_mask = 17;  // 转臂模式未复位
    else if(_BitV(vDI, 22))                       *err_mask = 18;  // 锚杆模式未复位
    else if(_BitV(vDI, 23))                       *err_mask = 19;  // 吊篮模式未复位
    // 自复位按钮：安全态=复位(0)
    // else if(_BitV(vDI, 16))                       *err_mask = 30;  // F1
    // else if(_BitV(vDI, 17))                       *err_mask = 31;  // F2
    // else if(_BitV(vDI, 18))                       *err_mask = 32;  // F3
    // else if(_BitV(vDI, 19))                       *err_mask = 33;  // F4(打铃)
    // 手柄/直线摇杆必须回中位(0x7F)：左手柄=VKC_Send[2,3]  右手柄=VKC_Send[6,7]  直线摇杆2=VKC_Send[5]
    // 直线摇杆1=ADCRst[3](0x189.6&7, 中位0x7EE)  直线摇杆3=ADCRst[9](0x18B.4&5, 中位0x7F1)（均为模拟量, 由 ADC_Correction 映射）
    else if(VKC_Send[2] != 0x7F)                  *err_mask = 50;  // 左手柄上下
    else if(VKC_Send[3] != 0x7F)                  *err_mask = 51;  // 左手柄左右
    else if(VKC_Send[6] != 0x7F)                  *err_mask = 52;  // 右手柄上下
    else if(VKC_Send[7] != 0x7F)                  *err_mask = 53;  // 右手柄左右
    else if(ADC_value[0] != 0x7F)  				  *err_mask = 54;  // 直线摇杆1未回中(ADCRst[3])
    else if(VKC_Send[5] != 0x7F)                  *err_mask = 55;  // 直线摇杆2未回中
    else if(ADC_value[1] != 0x7F)  				  *err_mask = 56;  // 直线摇杆3未回中(ADCRst[8])
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

static const uint8_t high = 30;
static const uint8_t margL = 10;
static const uint8_t margT = 5;
static const uint8_t Title = 30;

// 显示车状态反馈 0x220：工作模式 + 回转/冲击/推进压力 + 水压

// 静态文字：进页面时绘制（固定标签 + 单位），由 MainLogic_Enter 调用
void display_static(void)
{
    LCD_Set_Color(COLOR_WHITE_, -1);
    LCD_Disp_Txt(FONT_16, S_WIDTH/2 - (16*4+8)/2, margT+high*0, (uchar*)"模式:");
    LCD_Disp_Txt(FONT_16, margL, Title+high*0, (uchar*)"回转压力:");
    LCD_Disp_Txt(FONT_16, margL, Title+high*1, (uchar*)"冲击压力:");
    LCD_Disp_Txt(FONT_16, margL, Title+high*2, (uchar*)"推进压力:");
    LCD_Disp_Txt(FONT_16, margL, Title+high*3, (uchar*)"水压:");
    LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*0, (uchar*)"bar");
    LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*1, (uchar*)"bar");
    LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*2, (uchar*)"bar");
    LCD_Disp_Txt(FONT_16, margL+16*4+8*6, Title+high*3, (uchar*)"bar");
}

// 动态数据：实时走差分刷新（只发变化字段），由 MainLogic 每 N 帧调用
void display_dynamic(void)
{
    uint8_t mode = KX_119_recv_dis.data[0].u8Data[0];
    const char* m;
    if(_BitV(mode, 0))      m = "钻臂";
    else if(_BitV(mode, 1)) m = "锚杆";
    else if(_BitV(mode, 2)) m = "吊篮";
    else                    m = "  --";   // 4 半角字符宽 = 2 中文字宽，避免切回时残留

    LCD_Set_Color(COLOR_WHITE_, -1);
    Diff_Txt(&gL2X3JDiff, 0, S_WIDTH/2 - (16*4+8)/2 + 16*2+8, margT+high*0, m, FONT_16);

    LCD_Set_Color(COLOR_YELLW_, -1);
    Diff_Int(&gL2X3JDiff, 1, margL+16*4+8, Title+high*0, KX_119_recv_dis.data[1].u16Data, 5, FONT_16, 0);
    Diff_Int(&gL2X3JDiff, 2, margL+16*4+8, Title+high*1, KX_119_recv_dis.data[2].u16Data, 5, FONT_16, 0);
    Diff_Int(&gL2X3JDiff, 3, margL+16*4+8, Title+high*2, KX_119_recv_dis.data[3].u16Data, 5, FONT_16, 0);
    Diff_Int(&gL2X3JDiff, 4, margL+16*4+8, Title+high*3, KX_119_recv_dis.data[4].u8Data[0], 3, FONT_16, 0);

    Diff_EndFrame(&gL2X3JDiff);
}

// 自检异常提示：gSelfChkMask!=0 时在状态页显示红字（与 GLZYTC/213 同款）
void selfDisplay(void)
{
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
        LCD_Disp_Txt(FONT_16, margL, Title+high*5, "                            ");
    }
}

// 主要逻辑：每 N 帧刷新动态数据
void MainLogic(void)
{
    display_dynamic();
    selfDisplay();
}

// 进页面：绘制静态文字并请求整页全量（前 2 帧强制全发，之后仅发变化字段）
void MainLogic_Enter(void)
{
    display_static();
    Diff_PageEnter(&gL2X3JDiff);
}

#endif // (USER_TYPE == KX119_GLMZ_L2X3J)
