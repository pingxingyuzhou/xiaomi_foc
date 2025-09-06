
#include "bsp_time.h"

void GD_TIM0_Mspinit(void)
{
  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
}
void GD_TIM0_Init(void)
{
  timer_parameter_struct timer_initpara;
  timer_oc_parameter_struct timer_ocintpara;
  GD_TIM0_Mspinit();
  timer_struct_para_init(&timer_initpara);
  timer_deinit(TIMER0);

  timer_initpara.alignedmode = TIMER_COUNTER_CENTER_UP; /* 对齐模式 */
  timer_initpara.prescaler = 0;                         /* 时钟预分频数   例如：时钟频率=120MHZ/(时钟预分频+1) */
  timer_initpara.period = PWM_ARR - 1;                  /* 自动重装载寄存器周期的值(计数值) */
  timer_initpara.clockdivision = TIMER_CKDIV_DIV1;      /* 采样分频 */
  timer_initpara.counterdirection = TIMER_COUNTER_UP;   /* 向上计数模式 */
  timer_initpara.repetitioncounter = 0;
  timer_init(TIMER0, &timer_initpara);

  timer_channel_output_struct_para_init(&timer_ocintpara);

  timer_ocintpara.outputstate = TIMER_CCX_ENABLE;
  timer_ocintpara.ocpolarity = TIMER_OC_POLARITY_HIGH;
  timer_ocintpara.ocidlestate = TIMER_OC_IDLE_STATE_LOW;

  timer_channel_output_config(TIMER0, TIMER_CH_0, &timer_ocintpara);
  timer_channel_output_config(TIMER0, TIMER_CH_1, &timer_ocintpara);
  timer_channel_output_config(TIMER0, TIMER_CH_2, &timer_ocintpara);
  timer_channel_output_config(TIMER0, TIMER_CH_3, &timer_ocintpara);

  /* CH0 configuration in PWM mode0,duty cycle 25% */
  timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0, (PWM_ARR >> 1));
  timer_channel_output_mode_config(TIMER0, TIMER_CH_0, TIMER_OC_MODE_PWM0);
  timer_channel_output_shadow_config(TIMER0, TIMER_CH_0, TIMER_OC_SHADOW_ENABLE);

  /* CH1 configuration in PWM mode0,duty cycle 50% */
  timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_1, (PWM_ARR >> 1));
  timer_channel_output_mode_config(TIMER0, TIMER_CH_1, TIMER_OC_MODE_PWM0);
  timer_channel_output_shadow_config(TIMER0, TIMER_CH_1, TIMER_OC_SHADOW_ENABLE);

  /* CH2 configuration in PWM mode0,duty cycle 75% */
  timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_2, (PWM_ARR >> 1));
  timer_channel_output_mode_config(TIMER0, TIMER_CH_2, TIMER_OC_MODE_PWM0);
  timer_channel_output_shadow_config(TIMER0, TIMER_CH_2, TIMER_OC_SHADOW_ENABLE);

  timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_3, (PWM_ARR - 20));
  timer_channel_output_mode_config(TIMER0, TIMER_CH_3, TIMER_OC_MODE_PWM0);
  timer_channel_output_shadow_config(TIMER0, TIMER_CH_3, TIMER_OC_SHADOW_DISABLE);

  /* auto-reload preload enable */
  timer_auto_reload_shadow_enable(TIMER0);
  /* auto-reload preload enable */
  nvic_irq_enable(TIMER0_UP_IRQn, 0, 0);        /* TIMER0中断设置，抢占优先级0，子优先级0 */
  timer_interrupt_enable(TIMER0, TIMER_INT_UP); /* 使能更新中断 */
}
void GD_Start_TIM0(void)
{
  timer_enable(TIMER0);
  /* TIMER0 primary output function enable */
  timer_primary_output_config(TIMER0, ENABLE);
}
