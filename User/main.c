/*
 * @Author: fmy 921736537@qq.com
 * @Date: 2023-06-04 18:50:48
 * @LastEditors: fmy 921736537@qq.com
 * @LastEditTime: 2023-08-03 21:36:10
 * @FilePath: \GD32F303\User\main.c
 */
/*****************************/
#include "gd32f30x.h"
#include "main.h"
#include "controller.h"
#include "led.h"
#include "drv8323.h"
#include "fsm.h"
#include "usr_config.h"
#include "foc_handle.h"
#include "foc_encoder.h"
#include "can.h"

/*****************************/
float set;
float cal_angle;

int main(void)
{

  nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
  /* configure systick */
  systick_config();
  rcu_config();
  GD_GPIO_Init();
  GD_ADC_DMA_Init();
  GD_ADC_Init();

  GD_USART_Init(); // usart init befor dma init

  GD_CAN_Init();
  GD_TIM0_Init();

  GD_SPI1_Init();
  GD_SPI2_Init();

  SYSTICK_delay_ms(10);
  GD_Start_TIM0();

  if (0 != DRV8323_reset())
  {
    DEBUG("\n\rDRV8323 init fail!\n\r");
    while (1)
      ;
  }

  if (0 == USR_CONFIG_read_config())
  {
    DEBUG("\n\rConfig loaded ok\n\r");
  }
  else
  {
    USR_CONFIG_set_default_config();
    DEBUG("\n\rConfig loaded faile set to default\n\r");
  }

  UsrConfig.control_mode = CONTROL_MODE_CURRENT;
	UsrConfig.can_timeout_ms=1000;
	SYSTICK_delay_ms(100);
  FOC_zero_current(&Foc);
  FSM_input(CMD_MENU);
	
	
  while (1)
  {
		
    LED_loop();
		if(SYSTICK_get_tick()%5==0)
		{
//		DEBUG("%.3f,%d,%.5f,%.5f\n",Encoder.velocity,Encoder.raw,Foc.i_q_filt,Foc.i_d_filt);

		}

    #ifdef __DEBUG__

		#else

    #endif
  }
}
void Error_Handler(void)
{
  __disable_irq();

  PWMC_switch_off_pwm();

  while (1)
  {
  }
}
