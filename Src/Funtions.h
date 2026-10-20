#ifndef FUNTIONS_H
#define FUNTIONS_H

uint16_t Analog_Remap(const uint32_t *in,
                      uint16_t in_min, uint16_t in_mid, uint16_t in_max,
                      uint16_t in_dead,
                      uint16_t out_min, uint16_t out_mid, uint16_t out_max);
					  
#endif // FUNTIONS_H
