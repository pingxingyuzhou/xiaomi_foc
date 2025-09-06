/*
 * @Author: fmy 921736537@qq.com
 * @Date: 2023-06-04 19:59:42
 * @LastEditors: fmy 921736537@qq.com
 * @LastEditTime: 2023-07-17 21:00:39
 * @FilePath: \GD32F303\BSP\bsp_gpio.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "bsp_adc.h"
#include "systick.h"
uint16_t adc_Power_value[7];

void GD_ADC_Mspinit(void)
{
  gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_0);
  gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_1);
  gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_3);
  /*HALL gpio*/
	/*ADC_CHANNEL_4/5/6/7/14/15*/
  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_4);
  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_5);
  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_6);
  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_7);
  gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_4);
  gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_5);
}
void GD_ADC_DMA_Init(void)
{
  /* ADC_DMA_channel configuration */
  dma_parameter_struct dma_data_parameter;

  /* ADC DMA_channel configuration */
  dma_deinit(DMA0, DMA_CH0);

  /* initialize DMA single data mode */
  dma_data_parameter.periph_addr = (uint32_t)(&ADC_RDATA(ADC0));
  dma_data_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
  dma_data_parameter.memory_addr = (uint32_t)(&adc_Power_value);
  dma_data_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
  dma_data_parameter.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
  dma_data_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;
  dma_data_parameter.direction = DMA_PERIPHERAL_TO_MEMORY;
  dma_data_parameter.number = 7;
  dma_data_parameter.priority = DMA_PRIORITY_HIGH;
  dma_init(DMA0, DMA_CH0, &dma_data_parameter);

  dma_circulation_enable(DMA0, DMA_CH0);

  /* enable DMA channel */
  dma_channel_enable(DMA0, DMA_CH0);
}
void GD_ADC_Init(void)
{
  GD_ADC_Mspinit();
  adc_deinit(ADC0);
  adc_deinit(ADC1);
  /* ADC mode config */
  adc_mode_config(ADC_DAUL_INSERTED_PARALLEL);
  /* ADC continuous function enable */
  adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, ENABLE);
  adc_special_function_config(ADC1, ADC_CONTINUOUS_MODE, DISABLE);
  /* ADC scan function enable */
  adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
  adc_special_function_config(ADC1, ADC_SCAN_MODE, DISABLE);
  /* ADC data alignment config */
  adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
  adc_data_alignment_config(ADC1, ADC_DATAALIGN_RIGHT);

  /* ADC regular channel config */
  adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 7);
  adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_13, ADC_SAMPLETIME_7POINT5);
	//HALL
	adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_7, ADC_SAMPLETIME_7POINT5);
	adc_regular_channel_config(ADC0, 2, ADC_CHANNEL_14, ADC_SAMPLETIME_7POINT5);
	adc_regular_channel_config(ADC0, 3, ADC_CHANNEL_6, ADC_SAMPLETIME_7POINT5);
  adc_regular_channel_config(ADC0, 4, ADC_CHANNEL_4, ADC_SAMPLETIME_7POINT5);
	adc_regular_channel_config(ADC0, 5, ADC_CHANNEL_15, ADC_SAMPLETIME_7POINT5);
  adc_regular_channel_config(ADC0, 6, ADC_CHANNEL_5, ADC_SAMPLETIME_7POINT5);
  
  
  
  
  /* ADC channel length config */
  adc_channel_length_config(ADC0, ADC_INSERTED_CHANNEL, 1);
  adc_channel_length_config(ADC1, ADC_INSERTED_CHANNEL, 1);
  /* ADC regular channel config */
  adc_inserted_channel_config(ADC0, 0, ADC_CHANNEL_10, ADC_SAMPLETIME_7POINT5);
  adc_inserted_channel_config(ADC1, 0, ADC_CHANNEL_11, ADC_SAMPLETIME_7POINT5);

  /* ADC trigger config */
  adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);
  adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
  adc_external_trigger_source_config(ADC0, ADC_INSERTED_CHANNEL, ADC0_1_EXTTRIG_INSERTED_T0_CH3);
  adc_external_trigger_source_config(ADC1, ADC_INSERTED_CHANNEL, ADC0_1_2_EXTTRIG_INSERTED_NONE);
  adc_external_trigger_config(ADC0, ADC_INSERTED_CHANNEL, ENABLE);
  adc_external_trigger_config(ADC1, ADC_INSERTED_CHANNEL, ENABLE);

  /* ADC DMA function enable */
  adc_dma_mode_enable(ADC0);

  /* enable ADC interface */
  adc_enable(ADC0);
  SYSTICK_delay_ms(1);
  adc_calibration_enable(ADC0);

  adc_enable(ADC1);
  SYSTICK_delay_ms(1);
  adc_calibration_enable(ADC1);

  nvic_irq_enable(ADC0_1_IRQn, 0, 0);
  //	adc_interrupt_flag_clear(ADC0, ADC_INT_FLAG_EOC);
  adc_interrupt_flag_clear(ADC0, ADC_INT_FLAG_EOIC);

  adc_interrupt_enable(ADC0, ADC_INT_FLAG_EOIC);
  adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
  /* ADC software trigger enable */
}
