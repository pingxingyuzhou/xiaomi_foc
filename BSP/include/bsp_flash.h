/*
 * @Author: fmy 921736537@qq.com
 * @Date: 2023-07-01 12:34:53
 * @LastEditors: fmy 921736537@qq.com
 * @LastEditTime: 2023-07-01 12:51:22
 * @FilePath: \GD32F303\BSP\include\bsp_gpio copy.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __BSP_FLASH_H
#define __BSP_FLASH_H

#include "gd32f30x_libopt.h"

/*
 * bank0--2kB一页  bank1--4KB一页
 * 前512K在Bank0中,后面在Bank1中
 * 前256K CPU执行指令零等待
 */
/* 定义地址(32位机总线一次访问4个字节地址) */

#define FMC_PAGE_SIZE ((uint16_t)0x800U)
#define FMC_WRITE_START_ADDR ((uint32_t)0x0807F800U)
#define FMC_WRITE_END_ADDR ((uint32_t)0x0807FFFFU)

#define MOTOR_ID_ADDR ((uint32_t)(FMC_WRITE_START_ADDR + 0x000U))

/* 读FLASH */
#define FMC_READ(addrx) (*(volatile uint32_t *)(uint32_t)(addrx))

// /* 1MB FLASH */
// #define FMC_PAGE_SIZE ((uint16_t)0x1000U) /* bank1 4kB each page */
// #define FMC_WRITE_START_ADDR ((uint32_t)0x0808000U)
// #define FMC_WRITE_END_ADDR ((uint32_t)0x080FFFFFU)
void fmc_erase_pages(void);
void fmc_program(uint32_t data, uint32_t addressx);
#endif
