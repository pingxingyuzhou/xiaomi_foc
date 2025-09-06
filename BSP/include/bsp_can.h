/*
 * @Author: fmy 921736537@qq.com
 * @Date: 2023-06-16 22:10:25
 * @LastEditors: fmy 921736537@qq.com
 * @LastEditTime: 2023-06-17 11:58:01
 * @FilePath: \GD32F303\BSP\include\bsp_can.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include "gd32f30x_libopt.h"



#define Can_TxHand 0x300
#define Can_RxHand 0x400




void GD_CAN_Init(void);

void GD_CAN_Send_Message(uint32_t id, uint8_t *tx_buf, uint8_t len);

void GD_CAN0_RxFifo1PendingCallback(void);

#endif
