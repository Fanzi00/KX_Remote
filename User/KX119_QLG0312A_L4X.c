#include "_Device_config.h"
#include "App_usr.h"
#if (USER_TYPE == KX119_QLG0312A_L4X)

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
// 无流量旋钮（_XN_LiuL=0）
const KnobValCfg_t gKnobValTab[] = {
	{0, _XN1_AD_CHN,  54, 102+5, 4,  42, 102+12},		// 左旋钮 数值y=vDispLine1+5 圆弧y=vDispLine1+12
	{0, _XN2_AD_CHN, 257, 102+5, 4, 246, 102+12},		// 右旋钮
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
    F.F1 = _BitV(gDIBitV, 16);
    F.F2 = _BitV(gDIBitV, 17);
    F.F3 = _BitV(gDIBitV, 18);
    F.F4 = _BitV(gDIBitV, 19);
    F.F5 = _BitV(gDIBitV, 20);
    F.F6 = _BitV(gDIBitV, 21);
    F.F7 = _BitV(gDIBitV, 22);
    F.F8 = _BitV(gDIBitV, 23);
    // TODO: 上述 gDIBitV 位映射按实机面板按钮接线确认（协议 F 按钮位于 0x318.2.x）
    if(F.F3 || F.F4 || F.F5 || F.F6 || F.F7 || F.F8)
    {
        if(gSelfChkMask == 0)       // 自检异常不允许激活；本机型无自检，gSelfChkMask 恒为0
            gLaBaFlg = 1;
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

void display()
{
	uint8_t word_lenth = 32;
	// 臂选择（协议 0x318.0.2=左臂, 0x318.0.3=右臂, 中臂=!左&&右）
    // TODO: 本地 gDIBitV 的臂选择位按实机面板接线确认（此处先用 bit2/bit3 占位）
    static uint8_t flag = 0;
	static uint8_t flag_old = 0;
	flag = KX_119_recv_dis.data[0].u8Data[0];
	LCD_Set_Color(COLOR_WHITE_, -1);
    if(flag == 0)
    {
        LCD_Disp_Txt(FONT_32, Vermid-word_lenth*5/2, 240/2-word_lenth*2, (uchar*)"未连接臂架");
    }
    else if(flag == 1)
    {
        LCD_Disp_Txt(FONT_32, Vermid-word_lenth*4/2, 240/2-word_lenth*2, (uchar*)"连接左臂");
    }
    else if(flag == 2)
    {
        LCD_Disp_Txt(FONT_32, Vermid-word_lenth*4/2, 240/2-word_lenth*2, (uchar*)"连接中臂");
    }
	else if (flag == 3)
	{
        LCD_Disp_Txt(FONT_32, Vermid-word_lenth*4/2, 240/2-word_lenth*2, (uchar*)"连接右臂");
	}
	if(flag != flag_old)
		LCD_Disp_Txt(FONT_32, Vermid-word_lenth*5/2, 240/2-word_lenth*2, (uchar*)"                ");
	flag_old = flag;
	LCD_Disp_Txt(FONT_32, Vermid-(word_lenth*5+word_lenth/2*7)/2, 240/2, (uchar*)"吊篮倾角:");
	LCD_Set_Color(COLOR_YELLW_, -1);
	int16_t temp = KX_119_recv_dis.data[0].u8Data[1] | KX_119_recv_dis.data[1].u8Data[0] << 8;
	LCD_Disp_Intx(FONT_32, Vermid-(word_lenth*5+word_lenth/2*7)/2 + (word_lenth*4+word_lenth/2), 240/2, temp, 5);
	
	LCD_Set_Color(COLOR_WHITE_, -1);
	LCD_Disp_Txt(FONT_32, Vermid+(word_lenth*4+word_lenth/2*6)/2, 240/2, (uchar*)"度");
}

// 本机型无自检，自检异常区固定清屏即可
void selfDisplay(void)
{
    LCD_Disp_Txt(FONT_16, margL, Title+high*5, "                        ");
}

// 主要逻辑
void MainLogic(void)
{
    display();
    selfDisplay();
}

#endif // (USER_TYPE == KX119_QLG0312A_L4X)
