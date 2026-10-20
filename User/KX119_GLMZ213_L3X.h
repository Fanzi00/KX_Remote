#ifndef KX119_GLMZ213_L3X_H_
#define KX119_GLMZ213_L3X_H_
#include <stdint.h>

#pragma anon_unions // 匿名结构/联合，其成员可直接访问

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

// 接收机回传 3 帧（0x220 / 0x221 / 0x222），每帧 8 字节 = 4 个 uint16（小端）
// 共 12 个 uint16：data[0..3]=0x220, data[4..7]=0x221, data[8..11]=0x222
typedef struct KX119_Recv{
    union_u16_data data[12];
}KX119_Recv_t;

extern KX119_Recv_t KX_119_recv_dis;

#endif // KX119_GLMZ213_L3X_H_
