/*
 * @Author: fmy 921736537@qq.com
 * @Date: 2023-06-04 19:59:42
 * @LastEditors: fmy 921736537@qq.com
 * @LastEditTime: 2023-07-01 12:49:59
 * @FilePath: \GD32F303\BSP\bsp_gpio.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "bsp_flash.h"

/* calculate the number of page to be programmed/erased */
uint32_t PageNum = (FMC_WRITE_END_ADDR - FMC_WRITE_START_ADDR + 1) / FMC_PAGE_SIZE;

void fmc_erase_pages(void)
{
    uint32_t EraseCounter;

    /* unlock the flash program/erase controller */
    fmc_unlock();

    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

    /* erase the flash pages */
    for (EraseCounter = 0; EraseCounter < PageNum; EraseCounter++)
    {
        fmc_page_erase(FMC_WRITE_START_ADDR + (FMC_PAGE_SIZE * EraseCounter));
        fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
    }

    /* lock the main FMC after the erase operation */
    fmc_lock();
}

void fmc_program(uint32_t data, uint32_t addressx)
{
    /* unlock the flash program/erase controller */
    fmc_unlock();

    /* program flash */
    fmc_word_program(addressx, data);
    fmc_flag_clear(FMC_FLAG_BANK1_END | FMC_FLAG_BANK1_WPERR | FMC_FLAG_BANK1_PGERR);
    /* lock the main FMC after the program operation */
    fmc_lock();
}
