#include "bsp_can.h"
#include "main.h"
#include "can.h"

can_receive_message_struct receive_message;
can_trasnmit_message_struct transmit_message;
uint8_t transmit_mailbox = 0;
uint16_t can_pass_cnt;
void GD_CAN_Mspinit(void)
{
    gpio_pin_remap_config(GPIO_CAN_PARTIAL_REMAP, ENABLE);
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
}
void GD_CAN_Filter_Init(void)
{
    can_filter_parameter_struct can_filter;
    /* initialize filter */

    /* CAN0 filter number */
    can_filter.filter_number = 0;

    /* initialize filter */
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
	 can_filter.filter_list_high = (((uint32_t)Can_RxHand << 21 | CAN_FT_DATA | CAN_FF_STANDARD) & 0xFFFF0000) >> 16; // 过滤器高字节;
	 can_filter.filter_list_low = (((uint32_t)Can_RxHand << 21 | CAN_FT_DATA | CAN_FF_STANDARD) & 0x0000FFFF);        // 过滤器低字节
	 can_filter.filter_mask_high = 0xE000;                                                                       // 过滤最高3bit
	 can_filter.filter_mask_low = 0x0006;
//    can_filter.filter_list_high = 0; // 过滤器高字节;
//    can_filter.filter_list_low = 0;  // 过滤器低字节
//    can_filter.filter_mask_high = 0; // 过滤最高3bit
//    can_filter.filter_mask_low = 0;
    can_filter.filter_fifo_number = CAN_FIFO1;
    can_filter.filter_enable = ENABLE;
    can_filter_init(&can_filter);
}
void GD_CAN_Init(void)
{
    can_parameter_struct can_parameter;
    GD_CAN_Mspinit();

    can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);

    /* initialize CAN register */
    can_deinit(CAN0);

    /* initialize CAN */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = ENABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.auto_retrans = ENABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = CAN_BT_BS1_7TQ;
    can_parameter.time_segment_2 = CAN_BT_BS2_2TQ;
    /* baudrate 1Mbps */
    can_parameter.prescaler = 6;
    can_init(CAN0, &can_parameter);
    GD_CAN_Filter_Init();
    /* enable CAN receive FIFO1 not empty interrupt */
    can_interrupt_enable(CAN0, CAN_INT_RFNE1);
    nvic_irq_enable(CAN0_RX1_IRQn, 2, 0);
}
void GD_CAN_Send_Message(uint32_t id, uint8_t *tx_buf, uint8_t len)
{
    uint16_t timeout = 0xFFFF;
    can_struct_para_init(CAN_TX_MESSAGE_STRUCT, &transmit_message);
    transmit_message.tx_sfid = id| Can_TxHand;
    transmit_message.tx_efid = 0;
    transmit_message.tx_ft = CAN_FT_DATA;
    transmit_message.tx_ff = CAN_FF_STANDARD;
    transmit_message.tx_dlen = len;
    uint8_t i;
    for (i = 0; i < len; i++)
    {
        transmit_message.tx_data[i] = *(tx_buf + i);
    }
    /* waiting for transmit completed */
    transmit_mailbox = can_message_transmit(CAN0, &transmit_message);
    while ((CAN_TRANSMIT_OK != can_transmit_states(CAN0, transmit_mailbox)) && (0 != timeout))
    {
        timeout--;
        can_fifo_release(CAN0, CAN_FIFO1);
    }
}

void GD_CAN0_RxFifo1PendingCallback(void)
{
    if (RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFL1))
    {
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFL1);

        can_struct_para_init(CAN_RX_MESSAGE_STRUCT, &receive_message);
        can_message_receive(CAN0, CAN_FIFO1, &receive_message);
        frame.can_id = receive_message.rx_sfid;
        frame.data[0] = receive_message.rx_data[0];
        frame.data[1] = receive_message.rx_data[1];
        frame.data[2] = receive_message.rx_data[2];
        frame.data[3] = receive_message.rx_data[3];
        frame.data[4] = receive_message.rx_data[4];
        frame.data[5] = receive_message.rx_data[5];
        frame.data[6] = receive_message.rx_data[6];
        frame.data[7] = receive_message.rx_data[7];
        CAN_rx_callback(&frame);
    }
}
