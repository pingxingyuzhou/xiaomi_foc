/*!
    \file    systick.c
    \brief   the systick configuration file

    \version 2017-02-10, V1.0.0, firmware for GD32F30x
    \version 2018-10-10, V1.1.0, firmware for GD32F30x
    \version 2018-12-25, V2.0.0, firmware for GD32F30x
    \version 2020-09-30, V2.1.0, firmware for GD32F30x
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "gd32f30x.h"
#include "systick.h"

#define SYSTICK_US_TICK (SystemCoreClock / 1000000U)
#define SYSTICK_MS_TICK (SystemCoreClock / 1000U)

volatile uint32_t SysTickCnt = 0;

/*!
    \brief      configure systick
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    /* enable ADC clock */
    rcu_periph_clock_enable(RCU_ADC0);
    rcu_periph_clock_enable(RCU_ADC1);
    /* enable DMA0 clock */
    rcu_periph_clock_enable(RCU_DMA0);
    /* config ADC clock */
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV4);
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_SPI1);
    rcu_periph_clock_enable(RCU_SPI2);
    rcu_periph_clock_enable(RCU_USART1);
    rcu_periph_clock_enable(RCU_TIMER0);
    rcu_periph_clock_enable(RCU_TIMER6);

    rcu_periph_clock_enable(RCU_AF);
}
void systick_config(void)
{
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SYSTICK_MS_TICK))
    {
        /* capture error */
        while (1)
        {
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x02U);
}

uint32_t SYSTICK_get_tick(void)
{
    return SysTickCnt;
}

uint32_t SYSTICK_get_ms_since(uint32_t tick)
{
    return (uint32_t)(SysTickCnt - tick);
}

void SYSTICK_delay_us(uint32_t uS)
{
    uint32_t elapsed = 0;
    uint32_t last_count = SysTick->VAL;

    for (;;)
    {
        uint32_t current_count = SysTick->VAL;
        uint32_t elapsed_uS;

        /* measure the time elapsed since the last time we checked */
        if (last_count > current_count)
        {
            elapsed += last_count - current_count;
        }
        else if (last_count < current_count)
        {
            elapsed += last_count + (SYSTICK_MS_TICK - current_count);
        }

        last_count = current_count;

        /* convert to microseconds */
        elapsed_uS = elapsed / SYSTICK_US_TICK;
        if (elapsed_uS >= uS)
        {
            break;
        }

        /* reduce the delay by the elapsed time */
        uS -= elapsed_uS;

        /* keep fractional microseconds for the next iteration */
        elapsed %= SYSTICK_US_TICK;
    }
}

void SYSTICK_delay_ms(uint32_t mS)
{
    while (mS--)
    {
        SYSTICK_delay_us(1000);
    }
}
