/*
 * @Author: fmy 921736537@qq.com
 * @Date: 2023-06-04 23:03:34
 * @LastEditors: fmy 921736537@qq.com
 * @LastEditTime: 2023-06-05 22:47:20
 * @FilePath: \GD32F303\BSP\include\bsp_spi.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include "gd32f30x_libopt.h"

void GD_SPI2_Init(void);
void GD_SPI1_Init(void);
uint16_t SPI_RW_byte(uint8_t spi_id, uint16_t byte);

#endif
