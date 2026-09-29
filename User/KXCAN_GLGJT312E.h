#ifndef KXCAN_GLGJT312E_H_
#define KXCAN_GLGJT312E_H_
#include <stdint.h>

#pragma anon_unions // 启动匿名
typedef union{
	struct{
		uint8_t u8Data[2];
	};
	uint16_t u16Data;
}union_u16_data;

typedef struct KX119_Recv{
	uint32_t powerCheck; // 上电自检异常反馈
	union_u16_data data[12];
}KX119_Recv_t;

extern KX119_Recv_t KX_119_recv_dis;

void WL_Recv(void);
uint8_t showWarn(uint32_t t);
void clear_lcd(void);
void LiGong_mode(void);
void MainLogic(void);

#endif // KXCAN_GLGJT312E_H_
