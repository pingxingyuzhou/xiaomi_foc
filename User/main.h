

#ifndef MAIN_H
#define MAIN_H

#include "systick.h"
#include "arm_math.h"
#include <stdio.h>
#include <stdlib.h>
#include "bsp_gpio.h"
#include "bsp_usart.h"
#include "bsp_time.h"
#include "bsp_spi.h"
#include "bsp_adc.h"
#include "bsp_can.h"


#define PWM_ARR 3000 // 20KHz
#define DT (1.0f / 20000.0f)
#define CURRENT_MEASURE_HZ 20000

#define SHUNT_RESISTENCE	0.001f								// Resistance of phase current sampling resistor
#define V_SCALE (16.3f * 3.3f / 4095.0f)     					// Bus volts per A/D Count (0.015311 V)
#define I_SCALE (3.3f / 4095.0f) / SHUNT_RESISTENCE / 40.0f		// Amps per A/D Count (0.020147 A)



#define __DEBUG__

#ifdef __DEBUG__
	#define DEBUG(...)		printf(__VA_ARGS__);
#else
	#define DEBUG(...)
#endif

void Error_Handler(void);

#endif /* MAIN_H */
