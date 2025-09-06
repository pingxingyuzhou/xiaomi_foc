

#include "gd32f30x_it.h"
#include "main.h"
#include "systick.h"

#include "foc_encoder.h"
#include "foc_handle.h"
#include "fsm.h"

/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
	Error_Handler();
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
		Error_Handler();
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
		Error_Handler();
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
		Error_Handler();
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
		Error_Handler();
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SVC_Handler(void)
{
		Error_Handler();
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
		Error_Handler();
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PendSV_Handler(void)
{
		Error_Handler();
}

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/

void SysTick_Handler(void)
{
	SysTickCnt++;
}

void USART1_IRQHandler(void)
{
    if (RESET != usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE))
    {
        usart_interrupt_flag_clear(USART1, USART_INT_FLAG_RBNE);
		FSM_input(usart_data_receive(USART1));
    }
    //Usart1_RX_IDLE_CallBack();

		
}

void TIMER0_UP_IRQHandler(void)
{
    static uint8_t cnt, cnt2;
    timer_interrupt_flag_clear(TIMER0, TIMER_INT_FLAG_UP); // 清中断

    if (cnt++ > 0) // 20k
    {
        cnt = 0;

    }
    if (cnt2++ > 39) // 1k
    {
        cnt2 = 0;

    }
}


void ADC0_1_IRQHandler(void)
{
    if (adc_interrupt_flag_get(ADC0, ADC_INT_FLAG_EOC) != RESET)
    {
        adc_interrupt_flag_clear(ADC0, ADC_INT_FLAG_EOC);
    }
	ADC_STAT(ADC0) &= ~((uint32_t)ADC_INT_FLAG_EOIC);
	ENCODER_sample(DT);
	Foc.adc_vbus = adc_Power_value[0];
	Foc.adc_phase_a = adc_inserted_data_read(ADC0, ADC_INSERTED_CHANNEL_0);
	Foc.adc_phase_b = adc_inserted_data_read(ADC1, ADC_INSERTED_CHANNEL_0);
	Foc.i_a = I_SCALE * (float)(Foc.adc_phase_a_offset - Foc.adc_phase_a);
	Foc.i_b = I_SCALE * (float)(Foc.adc_phase_b_offset - Foc.adc_phase_b);
	Foc.i_c = -Foc.i_a - Foc.i_b;
	Foc.v_bus = 0.98f*Foc.v_bus + 0.02f*Foc.adc_vbus*V_SCALE;
	FSM_loop();
}
void CAN0_RX1_IRQHandler(void)
{
    GD_CAN0_RxFifo1PendingCallback();
}
