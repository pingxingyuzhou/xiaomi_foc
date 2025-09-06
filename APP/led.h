#ifndef __LED_H__
#define __LED_H__

#include "main.h"

void LED_init(void);

static inline void LED_set(uint32_t state){
	if(state){
		GPIO_BOP(GPIOC) = (uint32_t)GPIO_PIN_13;
	}else{
		GPIO_BC(GPIOC) = (uint32_t)GPIO_PIN_13;
	}
}

static inline uint32_t LED_get(void){
	return (GPIO_OCTL(GPIOC)&(GPIO_PIN_13));
}

static inline void LED_toggle(void){
	if(LED_get()){
		LED_set(0);
	}else{
		LED_set(1);
	}
}

void LED_loop(void);

#endif
