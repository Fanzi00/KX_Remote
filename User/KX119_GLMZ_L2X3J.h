#ifndef KX119_GLMZ_L2X3J_H_
#define KX119_GLMZ_L2X3J_H_
#include <stdint.h>

#pragma anon_unions // 匿名结构/联合，其成员可直接访问

// 直线摇杆1/3 纳入摇杆自学习（复用 vRockV[16..19]，不改 Flash 结构）
#define _STR_RK_EN      1
#define _STR_RK1_ADCH   3   // 直线摇杆1 = ADCRst[3]
#define _STR_RK3_ADCH   8   // 直线摇杆3 = ADCRst[8]
#define _STR_RK1_RVIDX  8   // 直线摇杆1 在 vRockV 的通道索引(复用槽 16/17)
#define _STR_RK3_RVIDX  9   // 直线摇杆3 在 vRockV 的通道索引(复用槽 18/19)

typedef struct Activar
{
    uint8_t F1 : 1;
    uint8_t F2 : 1;
    uint8_t F3 : 1;
    uint8_t F4 : 1;
    uint8_t F5 : 1;
    uint8_t F6 : 1;
    uint8_t F7 : 1;
    uint8_t F8 : 1;
}Activar_t;

typedef union{
    struct{
        uint8_t u8Data[2];
    };
    uint16_t u16Data;
}union_u16_data;

typedef struct KX119_Recv{
    union_u16_data data[12];
}KX119_Recv_t;

extern KX119_Recv_t KX_119_recv_dis;

// 直线摇杆1/3 为模拟量(ADCRst)，经 ADC_Correction 线性映射到 0~255 后写入以下变量；
// 由 YKQ_Data_WL 放入无线帧 ad1(索引8)/ad2(索引9) 槽位发往接收机（避开 KX_Send_joylogic 主轴优先级）
extern unsigned char gLinR1Send;  // 直线摇杆1 = ADCRst[3](0x189.6&7)
extern unsigned char gLinR3Send;  // 直线摇杆3 = ADCRst[9](0x18B.4&5)
#endif // KX119_GLMZ_L2X3J_H_
