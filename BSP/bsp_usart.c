/*
 * @Author: fmy 921736537@qq.com
 * @Date: 2023-06-04 20:28:57
 * @LastEditors: fmy 921736537@qq.com
 * @LastEditTime: 2023-06-15 22:21:08
 * @FilePath: \GD32F303\BSP\bsp_usart.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "bsp_usart.h"
#include <stdio.h>
uint8_t rxbuffer[uart_rx_len];
uint16_t Rx_len;
uint8_t usart1_rx_ok;
void GD_USART_MspInit(void)
{
    /*enable the  clock*/

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
}
void GD_USART_DMA_Init(void)
{
    dma_parameter_struct dma_init_struct;
    dma_deinit(DMA0, DMA_CH5);
    usart_flag_clear(USART1, USART_FLAG_RBNE);
    usart_dma_receive_config(USART1, USART_RECEIVE_DMA_ENABLE);
    dma_struct_para_init(&dma_init_struct);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)rxbuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = uart_rx_len;
    dma_init_struct.periph_addr = USART1_DATA_ADDRESS;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH5, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH5);
    /* enable DMA channel4 */
    dma_channel_enable(DMA0, DMA_CH5);
}
void GD_USART_Init(void)
{
    /* USART configure */
    GD_USART_MspInit();

    usart_deinit(USART_COM);
    usart_baudrate_set(USART_COM, 576000U);
    usart_word_length_set(USART_COM, USART_WL_8BIT);
    usart_parity_config(USART_COM, USART_PM_NONE);
    usart_stop_bit_set(USART_COM, USART_STB_1BIT);
    usart_receive_config(USART_COM, USART_RECEIVE_ENABLE);

    usart_transmit_config(USART_COM, USART_TRANSMIT_ENABLE);
    usart_enable(USART_COM);
    /*开启USART1中断 */
    nvic_irq_enable(USART1_IRQn, 0, 0);
    /* 使能串口发送中断 */

    usart_interrupt_enable(USART_COM, USART_INT_RBNE);
    // usart_interrupt_enable(USART_COM, USART_INT_IDLE);
}

int fputc(int ch, FILE *f)
{
  usart_data_transmit(USART1, (uint8_t)ch);
  while (RESET == usart_flag_get(USART1, USART_FLAG_TBE))
    ;
  return ch;
}
