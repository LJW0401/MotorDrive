/**
  ****************************(C) COPYRIGHT 2023 POLARBEAR****************************
  * @file       DJI_motor_drive.c
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

#include "DJI_motor_drive.h"

static CAN_TxHeaderTypeDef  gimbal_tx_message;
static uint8_t              gimbal_can_send_data[8];
static CAN_TxHeaderTypeDef  chassis_tx_message;
static uint8_t              chassis_can_send_data[8];
static CAN_TxHeaderTypeDef  shoot_tx_message;
static uint8_t              shoot_can_send_data[8];

/**
 * @brief 发送电机控制电流
 * @param CAN CAN总线 DJI_CAN_1(hcan1) 或 DJI_CAN_2(hcan2)
 * @param tx_message 数据发送邮箱
 * @param CAN_STD_ID 标识符
 * @param current1 第1个电机的电流值
 * @param current2 第2个电机的电流值
 * @param current3 第3个电机的电流值
 * @param current4 第4个电机的电流值
 */
void CANSendDJIMotorCurrent(CAN_HandleTypeDef *CAN, CAN_TxHeaderTypeDef *tx_message, CAN_TxSTD_ID_e CAN_TxSTD_ID, int16_t current1, int16_t current2, int16_t current3, int16_t current4)
{
    uint32_t send_mail_box;
    uint8_t can_send_data[8]
    tx_message->StdId = CAN_TxSTD_ID;
    tx_message->IDE = CAN_ID_STD;
    tx_message->RTR = CAN_RTR_DATA;
    tx_message->DLC = 0x08;
    can_send_data[0] = (current1 >> 8);
    can_send_data[1] = current1;
    can_send_data[2] = (current2 >> 8);
    can_send_data[3] = current2;
    can_send_data[4] = (current3 >> 8);
    can_send_data[5] = current3;
    can_send_data[6] = (current4 >> 8);
    can_send_data[7] = current4;
    /*检测可用的发送邮箱*/
    uint32_t free_TxMailbox = HAL_CAN_GetTxMailboxesFreeLevel(CAN);//检测是否有空闲邮箱
    while (free_TxMailbox<3){//等待空闲邮箱数达到3
        free_TxMailbox = HAL_CAN_GetTxMailboxesFreeLevel(CAN);
    }
    /* 将发送信息添加到发送邮箱中 */
    HAL_CAN_AddTxMessage(CAN, tx_message, can_send_data, &send_mail_box);
}


// void CANCmdMotor(int16_t yaw, int16_t pitch, int16_t shoot, int16_t rev)
// {
//     CAN_TxHeader_MI.StdId = 0x200;
//     CAN_TxHeader_MI.ExtId = 0x200 | (MI_MASTERID << 8);
//     CAN_TxHeader_MI.IDE = CAN_ID_EXT;
//     CAN_TxHeader_MI.RTR = CAN_RTR_DATA;
//     CAN_TxHeader_MI.DLC = 0x08;
//     CAN_TxHeader_MI.TransmitGlobalTime = DISABLE;
// }
