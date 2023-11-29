/**
  ****************************(C) COPYRIGHT 2023 POLARBEAR****************************
  * @file       DJI_motor_drive.h
  * @brief      DJI GM系列电机CAN发送
  * @note       适用于GM6020、GM3508、GM2006型号的电机
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     2023-11-29      Penguin         1. done
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2023 POLARBEAR****************************
  */

#ifndef __DJI_MOTOR_DRIVE_H
#define __DJI_MOTOR_DRIVE_H
#include "struct_typedef.h"
#include "stm32f4xx_hal.h"

#define DJI_CAN_1 hcan1
#define DJI_CAN_2 hcan2

typedef enum _CAN_TxSTD_ID_e
{
    STD_ID_1FF = 0x1FF,
    STD_ID_200 = 0x200,
    STD_ID_2FF = 0x2FF,
} CAN_TxSTD_ID_e;

void CANSendDJIMotorCurrent(CAN_HandleTypeDef *CAN, CAN_TxHeaderTypeDef *tx_message, CAN_STD_ID_e CAN_STD_ID, int16_t current1, int16_t current2, int16_t current3, int16_t current4)


#endif  // __DJI_MOTOR_DRIVE_H