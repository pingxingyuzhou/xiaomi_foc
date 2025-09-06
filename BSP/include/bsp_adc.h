#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "gd32f30x_libopt.h"
void GD_ADC_DMA_Init(void);
void GD_ADC_Init(void);

extern uint16_t adc_Power_value[7];

#endif
