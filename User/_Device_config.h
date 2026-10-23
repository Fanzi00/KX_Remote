#ifndef __DEVICE_CONFIG_H
#define __DEVICE_CONFIG_H

#include <stdint.h>

#include "app_lcd.h"
#include "LPC17xx.h"
#include "config.h"
#include "Funtions.h"
// 发射机屏幕尺寸：320*240
#define S_HEIGHT 240
#define S_WIDTH 320

// 纽子开关图标配置表条目（数据表由各遥控器 .c 定义：gSwiIconTab1/第一排、gSwiIconTab2/第二排）
// 注意: 第一排/第二排由调用处分别传入 stDIBitV1/stDIBitV2 作比较基准，故表内不再存 grp
typedef struct {
    unsigned char  en;     // 1=显示 0=隐藏 0xFF=表结束标志
    unsigned short x;      // 屏幕X
    unsigned short y;      // 屏幕Y
    unsigned char  shift;  // gDIBitV 右移位数(该开关最低有效位)
    unsigned char  gears;  // 几档开关: 2=两档(单DI, 上/下)  3=三档(两DI, 上/中/下)
    unsigned char  lock;   // 是否自锁: 0=自复位 1=自锁(仅两档开关有意义, 三档忽略)
    unsigned char  map;    // 显示映射: 0=正常 1=上/下反转(交换图标1/2, 中态0不变)
} SwiIconCfg_t;

// 流量旋钮"图标+数值"显示表条目（数据表由各遥控器 .c 定义：gKnobValTab，#if _XN_LiuL 时使用）
// 圆弧与数值绑定显示，en 同时控制两者；显示值 = ADCRst[ch] 原始采样值(12位 0~0xFFF)
typedef struct {
    unsigned char  en;      // 1=显示(圆弧+数值) 0=隐藏 0xFF=表结束标志
    unsigned short x;       // 数值屏幕X
    unsigned short y;       // 数值屏幕Y
    unsigned char  digit;   // 数值显示位数
    unsigned short arcX;    // 圆弧X
    unsigned short arcY;    // 圆弧Y
} KnobValCfg_t;

extern const SwiIconCfg_t gSwiIconTab1[];
extern const SwiIconCfg_t gSwiIconTab2[];
extern const KnobValCfg_t gKnobValTab[];

void WL_Recv(unsigned char vGrp, const unsigned char* buf);
uint8_t showWarn(uint32_t t);
const char* SelfChk_Str(uint32_t mask);   // 自检异常描述（错误码见 SelfChk_Proc）
void clear_lcd(void);
void MainLogic(void);
void SelfChk_main(void);    // 上电自检状态机：10ms 节拍(main.c)周期调用；按《213测试结果》逐项检查开关/手柄
void Activar(void);
void selfDisplay(void);
void DI_Correction(void);
void ADC_Correction(void);
void Straight_handle(void); // 直线手柄
/*******************自检功能开关*******************/
// 遥控器上电自检：定义 _SELF_CHK_EN=开启，#undef(或不定义)=关闭
// 开启：开机检测按键/手柄是否回位，异常时屏幕显示"自检异常:xxx"并禁止激活
// 关闭：不做任何按键/手柄状态检测，不显示自检提示，按键可直接激活
// 自检功能开关：用 #ifdef 控制。需要开启的机型在下面 USER_TYPE 分支里 #define _SELF_CHK_EN；
// 需要关闭的机型用 #undef _SELF_CHK_EN（或不在该分支定义）。无需给值。
/*******************自检功能开关*******************/

#define Example 0
#define KXCAN_GLGJT312E_L3X 1
#define KXCAN_GLZYTC_L2X 2
#define KX119_QLG0312A_L4X 3
#define KX119_GLMZ_L2X3J 4
#define KX119_GLMZ213_L3X 5

/*******************USER_TYPE*******************/
/*******************USER_TYPE*******************/

#define USER_TYPE		Example

/*******************USER_TYPE*******************/
/*******************USER_TYPE*******************/

// 示例
#if (USER_TYPE == Example)
#include "Example.h"

#define _ARM_STUDY_MASK 0xFF // 自学习通道选择：bit0~7=万向轴(=VKC_Send同下标)，1=学/0=跳(未接手柄置0)

#define SWI_ICON_EN 1 // 是否有扭子开关
#define _XN_LiuL    0 // 是否有流量旋钮
#define _SELF_CHK_EN  // 自检功能打开（定义即开启，由 #ifdef 控制）

#endif // #if (USER_TYPE == Example)

// 河南耿力 拱架台车 312E
#if (USER_TYPE == KXCAN_GLGJT312E_L3X)
#include "KXCAN_GLGJT312E.h"

#define _ARM_STUDY_MASK (1<<2|1<<3|1<<4|1<<5|1<<6|1<<7) // 自学习通道选择：bit0~7=万向轴(=VKC_Send同下标)，1=学/0=跳(未接手柄置0)

#define SWI_ICON_EN 1 // 是否有扭子开关
#define _XN_LiuL    0 // 是否有流量旋钮
#define _SELF_CHK_EN  // 自检功能打开（定义即开启，由 #ifdef 控制）

#endif // #if (USER_TYPE == KXCAN_GLGJT312E_L3X)

// 河南耿力 装药台车
#if (USER_TYPE == KXCAN_GLZYTC_L2X)
#include "KXCAN_GLZYTC.h"

#define _ARM_STUDY_MASK (1<<3|1<<4|1<<5|1<<6) // 自学习通道选择：bit0~7=万向轴(=VKC_Send同下标)，1=学/0=跳(未接手柄置0)

// 纽子开关图标：数据表在本遥控器 KXCAN_GLZYTC.c 中（gSwiIconTab1/2）
#define SWI_ICON_EN 1
#define _XN_LiuL    1 // 是否有流量旋钮
#define _SELF_CHK_EN  // 自检功能打开（定义即开启，由 #ifdef 控制）

#endif // #if (USER_TYPE == KXCAN_GLZYTC_L2X)

// 中铁强力 立拱架台车 QLG0312A
#if (USER_TYPE == KX119_QLG0312A_L4X)
#include "KX119_QLG0312A_L4X.h"

#define _ARM_STUDY_MASK 0xFF // 自学习通道选择：bit0~7=万向轴(=VKC_Send同下标)，1=学/0=跳(未接手柄置0)

#define SWI_ICON_EN 1
#define _XN_LiuL    0 // 是否有流量旋钮（QLG0312A 无流量旋钮，见协议文档）
// #define _SELF_CHK_EN  // 自检功能关闭（QLG0312A 无自检功能，见协议文档）

#endif // #if (USER_TYPE == KX119_QLG0312A_L4X)

// 耿力锚注（GMZ12I-X）
#if (USER_TYPE == KX119_GLMZ_L2X3J)
#include "KX119_GLMZ_L2X3J.h"

#define _ARM_STUDY_MASK ((1<<2)|(1<<3)|(1<<6)|(1<<7)) // 自学习通道选择：bit0~7=万向轴(=VKC_Send同下标)，1=学/0=跳；本机2万向=轴2/3/6/7 -> 0xCC

#define SWI_ICON_EN 1
#define _XN_LiuL    0 // 是否有流量旋钮（GLMZ 无流量旋钮，见协议文档）
#define _SELF_CHK_EN  // 自检功能关闭

#endif // #if (USER_TYPE == KX119_GLMZ_L2X3J)

// 耿力锚注一体机
#if (USER_TYPE == KX119_GLMZ213_L3X)
#include "KX119_GLMZ213_L3X.h"

#define _ARM_STUDY_MASK ((1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7)) // 自学习通道选择：bit0~7=万向轴(=VKC_Send同下标)，1=学/0=跳(未接手柄置0)

#define SWI_ICON_EN 1
#define _XN_LiuL    1 // 是否有旋钮
#define _SELF_CHK_EN  // 开启自检功能

#endif // #if (USER_TYPE == KX119_GLMZ213_L3X)

// 自学习通道选择默认值(上面各设备块未配置时)：全学
#ifndef _ARM_STUDY_MASK
#define _ARM_STUDY_MASK 0xFF
#endif

#endif // #ifndef __DEVICE_CONFIG_H
