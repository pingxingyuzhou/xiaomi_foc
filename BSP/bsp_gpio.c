/*
 * @Author: fmy 921736537@qq.com
 * @Date: 2023-06-04 19:59:42
 * @LastEditors: fmy 921736537@qq.com
 * @LastEditTime: 2023-06-05 22:21:39
 * @FilePath: \GD32F303\BSP\bsp_gpio.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "bsp_gpio.h"

void GD_GPIO_Init(void)
{
    /* enable the gpio clock */

    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
    /*LED*/
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_bit_reset(GPIOC, GPIO_PIN_13);
    /*SPI2_NSS*/
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
    gpio_bit_set(GPIOA, GPIO_PIN_15);
    /*SPI1_NSS*/
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    gpio_bit_set(GPIOB, GPIO_PIN_12);
		/*EN*/
	  gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
    gpio_bit_reset(GPIOA, GPIO_PIN_11);
		/*nFAULT*/
		gpio_init(GPIOA,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,GPIO_PIN_12);

}
