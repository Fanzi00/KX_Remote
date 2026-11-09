#include "_Device_config.h"
#include "App_usr.h"
#if (USER_TYPE == KX119_QLG0312A_L4X)

// 摇杆 AD 通道映射表（每个机型在此自定；当前全项目统一 KX 表 {10,6,15,7,12,13,0,5}）
const uint8_t gArmUpTab[_ARM_MAX]   = {10, 6, 15, 7, 12, 13, 0, 5 };
const uint8_t gArmDownTab[_ARM_MAX] = {10, 6, 15, 7, 12, 13, 0, 5 };

Activar_t F = {0};
// 扭子开关图标配置表（App_lcd.c 的 Disp_SwiIconRow 遍历绘制）
// 行格式: {是否显示, 屏幕X, 屏幕Y, gDIBitV移位数, 几档开关(2/3), 是否自锁(0/1), 显示映射(0/1)}
// 两档自锁: 图标仅上/下; 两档自复位: 触发/中; 三档: 上/中/下(自锁自复位同)
// map=1: 上/下反转(交换图标1/2, 中态0不变) —— 实际显示与逻辑相反时置1
// shift 对应 gDIBitV 位号(车->接收机->发射机 回传 0x18A 位号):
//   左臂/右臂 0x18A.0.2/0.3->2/3(三档同取2,3) 破碎0.4->4 自动调平0.5->5
//   调平仰0.6->6 调平俯0.7->7 抓手松开1.0->8 抓手锁紧1.1->9 抓臂缩1.2->10 抓臂伸1.3->11
// 每排4格位置: 1.1/1.2/1.3/1.4 与 2.1/2.2/2.3/2.4; md"扭子开关布局"仅 1.1/1.4/2.1/2.2/2.4 有开关,
//   其余(1.2/1.3/2.3)设 en=0 隐藏。增删图标: 增删一行；末行 {0xFF} 为表结束标志，勿删
const SwiIconCfg_t gSwiIconTab1[] = {	// 第一排纽子
	{1, 45, 132, 2,  3, 1, 1},   // 1.1 左/中/右臂 三档自锁(0x18A.0.2左,0.3右 -> 左=下 右=上 中=中)
	{0, 85, 132, 0,  2, 0, 0},   // 1.2 无扭子开关(隐藏)
	{0, 205,132, 0,  2, 0, 0},   // 1.3 无扭子开关(隐藏)
	{1, 245,132, 8,  3, 0, 0},   // 1.4 抓手松开/锁紧 三档自复位(0x18A.1.0松开,1.1锁紧 -> 松开=下 锁紧=上 中=中)
	{0xFF}
};
const SwiIconCfg_t gSwiIconTab2[] = {	// 第二排纽子
	{1, 45, 170, 4,  3, 1, 1},   // 2.1 破碎/自动调平 三档自锁(0x18A.0.4破碎,0.5自动调平 -> 破碎=下 自动调平=上)
	{1, 85, 170, 6,  3, 0, 1},   // 2.2 调平仰/俯 两档自复位(0x18A.0.6 调平仰; 触发=下 松开=中)
	{0, 205,170, 0,  2, 0, 0},   // 2.3 无扭子开关(隐藏)
	{1, 245,170, 10, 3, 0, 0},   // 2.4 抓手伸缩 两档自复位(0x18A.1.2 抓臂缩; 触发=下 松开=中)
	{0xFF}
};
// 无旋钮（_XN_LiuL=0）
const KnobValCfg_t gKnobValTab[] = {
	{0,  54, 102+5, 4,  42, 102+12},		// 左旋钮 数值y=vDispLine1+5 圆弧y=vDispLine1+12
	{0, 257, 102+5, 4, 246, 102+12},		// 右旋钮
	{0xFF}
};

KX119_Recv_t KX_119_recv_dis = {0};
// 无线数据解析(接收机回传): 帧0/5/6 = 臂距数据，3 帧共 12 个 uint16
// 帧内不再放分组号，8 字节 = 4 个 uint16（小端），由帧类型决定是第几组
void WL_Recv(unsigned char vGrp, const unsigned char* buf)
{
    static uint8_t stGrpGot = 0;
    uint8_t i;

    if(vGrp >= 3)
        return;

    for(i=0; i<4; i++)
        KX_119_recv_dis.data[vGrp*4 + i].u16Data = buf[i*2] | (buf[i*2+1] << 8);

    stGrpGot |= (uint8_t)(1u << vGrp);
    if(stGrpGot == 0x07)           // 3 组都到齐
    {
        stGrpGot = 0;
        gDataFresh = 1;
        gLCDFlg = 1;
    }
}

// 激活按钮设置
void Activar()
{
    static unsigned int forceDly = 0;   // 强制激活长按计时(DI_Scan 1ms 节拍, 6000=6s)
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
        if(gSelfChkMask == 0)       // 自检异常不允许激活；本机型无自检，gSelfChkMask 恒为0
            gLaBaFlg = 1;
    }

    // 强制激活：F5~F8 任一长按 6 秒(1ms*6000)绕过自检，强制置激活位；松开即清零计时，须连续按住
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

// 自检状态机，10ms 节拍：KX119_QLG0312A_L4X 无自检功能，仅标记通过
void SelfChk_main(void)
{
    gSelfChkOK = 1;     // 无自检：始终认为通过，正常操控
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
static const uint8_t Vermid = 320 / 2;
static const uint8_t Title = 30;
static const uint8_t LwordLenth = 8 * 3 + 16 * 2;

static DiffCtx gQLGDiff;   // 差分刷新上下文（调用方持有，每个屏一个）

// 静态文字：进页面时绘制（固定标签 + 单位 + 自检区清屏），由 MainLogic_Enter 调用
void display_static(void)
{
    uint8_t word_lenth = 32;
    LCD_Set_Color(COLOR_WHITE_, -1);
    LCD_Disp_Txt(FONT_32, Vermid-(word_lenth*5+word_lenth/2*7)/2, 240/2, (uchar*)"吊篮倾角:");
    LCD_Disp_Txt(FONT_32, Vermid+(word_lenth*4+word_lenth/2*6)/2, 240/2, (uchar*)"度");
    // 自检异常区固定清屏（本机型无自检）
    // LCD_Disp_Txt(FONT_16, margL, Title+high*5, "                        ");
}

// 动态数据：实时走差分刷新（只发变化字段），由 MainLogic 每 N 帧调用
void display_dynamic(void)
{
    uint8_t word_lenth = 32;
    uint8_t flag = KX_119_recv_dis.data[0].u8Data[0];
    const char* m;
    if(flag == 0)      m = "未连接臂架";
    else if(flag == 1) m = "连接左臂　";   // 全角空格补齐到 5 字宽，避免切回短串时屏幕残留
    else if(flag == 2) m = "连接中臂　";
    else               m = "连接右臂　";

    LCD_Set_Color(COLOR_WHITE_, -1);
    Diff_Txt(&gQLGDiff, 0, Vermid-word_lenth*5/2, 240/2-word_lenth*2, m, FONT_32);

    LCD_Set_Color(COLOR_YELLW_, -1);
    int16_t temp = KX_119_recv_dis.data[0].u8Data[1] | KX_119_recv_dis.data[1].u8Data[0] << 8;
    Diff_Int(&gQLGDiff, 1, Vermid-(word_lenth*5+word_lenth/2*7)/2 + (word_lenth*4+word_lenth/2), 240/2, temp, 5, FONT_32, 0);

    Diff_EndFrame(&gQLGDiff);
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
    Diff_PageEnter(&gQLGDiff);
}

#endif // (USER_TYPE == KX119_QLG0312A_L4X)
