/*
 * @Author: fmy 921736537@qq.com
 * @Date: 2023-06-04 20:28:57
 * @LastEditors: fmy 921736537@qq.com
 * @LastEditTime: 2023-06-15 21:39:40
 * @FilePath: \GD32F303\BSP\include\bsp_usart.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __BSP_USART_H
#define __BSP_USART_H

#include "gd32f30x_libopt.h"

#define USART1_DATA_ADDRESS ((uint32_t)&USART_DATA(USART1))
#define USART_COM USART1
#define uart_rx_len 500

void GD_USART_DMA_Init(void);

void GD_USART_Init(void);
void Usart1_RX_IDLE_CallBack(void);

#endif
