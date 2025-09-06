#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "gd32f30x_libopt.h"

#define LED_ON gpio_bit_set(GPIOC, GPIO_PIN_13)
#define LED_OFF gpio_bit_reset(GPIOC, GPIO_PIN_13)

#define SPI1_NSS_H gpio_bit_set(GPIOB, GPIO_PIN_12)
#define SPI1_NSS_L gpio_bit_reset(GPIOB, GPIO_PIN_12)

#define SPI2_NSS_H gpio_bit_set(GPIOA, GPIO_PIN_15)
#define SPI2_NSS_L gpio_bit_reset(GPIOA, GPIO_PIN_15)

void GD_GPIO_Init(void);

#endif
