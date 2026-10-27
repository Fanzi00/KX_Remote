#ifndef FUNTIONS_H
#define FUNTIONS_H

uint16_t Analog_Remap(const uint32_t *in,
                      uint16_t in_min, uint16_t in_mid, uint16_t in_max,
                      uint16_t in_dead,
                      uint16_t out_min, uint16_t out_mid, uint16_t out_max);
uint16_t adc_map_int(uint32_t adc, uint16_t adc_min, uint16_t adc_max, uint16_t out_min, uint16_t out_max);
void di_map_int(uint32_t in, unsigned long* v1, unsigned long* v2, uint8_t num);
void Disp_WLDiag(void);
#include "DiffRefresh.h"   // 差分刷新引擎（独立文件，实现见 DiffRefresh.c）

#endif // FUNTIONS_H
