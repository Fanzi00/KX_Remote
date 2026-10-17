#ifndef KXCAN_GLZYTC_H_
#define KXCAN_GLZYTC_H_
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



typedef struct KX119_Recv{
	union_u16_data data[12];
}KX119_Recv_t;

extern KX119_Recv_t KX_119_recv_dis;

void WL_Recv(unsigned char vGrp, const unsigned char* buf);
uint8_t showWarn(uint32_t t);
void clear_lcd(void);
void MainLogic(void);
void SelfChk_main(void);	// 上电自检状态机：10ms 节拍(main.c)周期调用，实现见 KXCAN_GLZYTC.c
void Activar();

#endif // KXCAN_GLZYTC_H_
