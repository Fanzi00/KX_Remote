#ifndef __DEVICE_CONFIG_H
#define __DEVICE_CONFIG_H

#include <stdint.h>

#include "app_lcd.h"
#include "LPC17xx.h"
#include "config.h"

// 纽子开关图标配置表条目（数据表由各遥控器 .c 定义：gSwiIconTab1/第一排、gSwiIconTab2/第二排）
typedef struct {
    unsigned char  en;     // 1=显示 0=隐藏 0xFF=表结束标志
    unsigned short x;      // 屏幕X
    unsigned short y;      // 屏幕Y
    unsigned char  grp;    // 状态缓存组: 1=与stDIBitV1比较 2=与stDIBitV2比较
    unsigned char  shift;  // gDIBitV 右移位数
    unsigned char  mask;   // 状态掩码
    unsigned char  map;    // 图标映射: 0=直接索引 1=1/2互换(其余保持) 2=+1(两档) 3=>2归0 4=1/2互换(其余归0)
} SwiIconCfg_t;

// 流量旋钮"图标+数值"显示表条目（数据表由各遥控器 .c 定义：gKnobValTab，#if _XN_LiuL 时使用）
// 圆弧与数值绑定显示，en 同时控制两者；显示值 = ADCRst[ch] 原始采样值(12位 0~0xFFF)
typedef struct {
    unsigned char  en;      // 1=显示(圆弧+数值) 0=隐藏 0xFF=表结束标志
    unsigned char  ch;      // ADCRst AD通道索引(如 _XN1_AD_CHN/_XN2_AD_CHN)
    unsigned short x;       // 数值屏幕X
    unsigned short y;       // 数值屏幕Y
    unsigned char  digit;   // 数值显示位数
    unsigned short arcX;    // 圆弧X
    unsigned short arcY;    // 圆弧Y
} KnobValCfg_t;

extern const SwiIconCfg_t gSwiIconTab1[];
extern const SwiIconCfg_t gSwiIconTab2[];
extern const KnobValCfg_t gKnobValTab[];

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

/*******************USER_TYPE*******************/
/*******************USER_TYPE*******************/

#define USER_TYPE 			KX119_QLG0312A_L4X

/*******************USER_TYPE*******************/
/*******************USER_TYPE*******************/

// 示例
#if (USER_TYPE == Example)

#define SWI_ICON_EN 1 // 是否有扭子开关
#define _XN_LiuL    0 // 是否有流量旋钮
#define _SELF_CHK_EN  // 自检功能打开（定义即开启，由 #ifdef 控制）

#endif // #if (USER_TYPE == Example)

// 河南耿力 拱架台车 312E
#if (USER_TYPE == KXCAN_GLGJT312E_L3X)

#define SWI_ICON_EN 1 // 是否有扭子开关
#define _XN_LiuL    0 // 是否有流量旋钮
#define _SELF_CHK_EN  // 自检功能打开（定义即开启，由 #ifdef 控制）

#endif // #if (USER_TYPE == KXCAN_GLGJT312E_L3X)

// 河南耿力 装药台车
#if (USER_TYPE == KXCAN_GLZYTC_L2X)

// 纽子开关图标：数据表在本遥控器 KXCAN_GLZYTC.c 中（gSwiIconTab1/2）
#define SWI_ICON_EN 1
#define _XN_LiuL    1 // 是否有流量旋钮
#define _SELF_CHK_EN  // 自检功能打开（定义即开启，由 #ifdef 控制）

#endif // #if (USER_TYPE == KXCAN_GLZYTC_L2X)

// 中铁强力 立拱架台车 QLG0312A
#if (USER_TYPE == KX119_QLG0312A_L4X)

#define SWI_ICON_EN 1
#define _XN_LiuL    1 // 是否有流量旋钮
// #define _SELF_CHK_EN  // 自检功能打开（定义即开启，由 #ifdef 控制）

#endif // #if (USER_TYPE == KX119_QLG0312A_L4X)

#endif // #ifndef __DEVICE_CONFIG_H
