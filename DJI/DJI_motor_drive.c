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
  为了兼容官代，这里封装的函数命名都有2份，一份是按照Polarbear命名规范的，一份继承自老官代（后期也有可能被遗弃）
  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2023 POLARBEAR****************************
  */

#include "DJI_motor_drive.h"

static CAN_TxHeaderTypeDef gimbal_tx_message;
static uint8_t gimbal_can_send_data[8];
static CAN_TxHeaderTypeDef chassis_tx_message;
static uint8_t chassis_can_send_data[8];
static CAN_TxHeaderTypeDef shoot_tx_message;
static uint8_t shoot_can_send_data[8];

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
    uint8_t can_send_data[8] tx_message->StdId = CAN_TxSTD_ID;
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
    uint32_t free_TxMailbox = HAL_CAN_GetTxMailboxesFreeLevel(CAN); // 检测是否有空闲邮箱
    while (free_TxMailbox < 3)
    { // 等待空闲邮箱数达到3
        free_TxMailbox = HAL_CAN_GetTxMailboxesFreeLevel(CAN);
    }
    /* 将发送信息添加到发送邮箱中 */
    HAL_CAN_AddTxMessage(CAN, tx_message, can_send_data, &send_mail_box);
}

/*-------------------- 封装的一些控制函数 --------------------*/

/**
 * @brief          控制云台电机(0x209,0x20A,0x20B)
 * @param[in]      yaw: (0x209) 6020电机控制电流, 范围 [-30000,30000]
 * @param[in]      pitch: (0x20A) 6020电机控制电流, 范围 [-30000,30000]
 * @param[in]      rev: (0x20B) 保留，电机控制电流
 * @retval         none
 */
void CANCmdGimbal(int16_t yaw, int16_t pitch, int16_t rev)
{
    CANSendDJIMotorCurrent(&DJI_CAN_2, &gimbal_tx_message, TxSTD_ID_2FF,
                           yaw, pitch, rev, 0)
}

/**
 * @brief          控制云台电机(0x209,0x20A,0x20B)
 * @param[in]      yaw: (0x209) 6020电机控制电流, 范围 [-30000,30000]
 * @param[in]      pitch: (0x20A) 6020电机控制电流, 范围 [-30000,30000]
 * @param[in]      rev: (0x20B) 保留，电机控制电流
 * @retval         none
 */
void CAN_cmd_gimbal(int16_t yaw, int16_t pitch, int16_t rev)
{
    CANCmdGimbal(yaw, pitch, rev);
}

/**
 * @brief          控制底盘电机(0x201,0x202,0x203,0x204)
 * @param[in]      motor1: (0x201) 3508电机控制电流, 范围 [-16384,16384]
 * @param[in]      motor2: (0x202) 3508电机控制电流, 范围 [-16384,16384]
 * @param[in]      motor3: (0x203) 3508电机控制电流, 范围 [-16384,16384]
 * @param[in]      motor4: (0x204) 3508电机控制电流, 范围 [-16384,16384]
 * @retval         none
 */
void CANCmdChassis(int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4)
{
    CANSendDJIMotorCurrent(&DJI_CAN_1, &chassis_tx_message, TxSTD_ID_200,
                           motor1, motor2, motor3, motor4)
}

/**
 * @brief          控制底盘电机(0x201,0x202,0x203,0x204)
 * @param[in]      motor1: (0x201) 3508电机控制电流, 范围 [-16384,16384]
 * @param[in]      motor2: (0x202) 3508电机控制电流, 范围 [-16384,16384]
 * @param[in]      motor3: (0x203) 3508电机控制电流, 范围 [-16384,16384]
 * @param[in]      motor4: (0x204) 3508电机控制电流, 范围 [-16384,16384]
 * @retval         none
 */
void CAN_cmd_chassis(int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4)
{
    CANCmdChassis(motor1, motor2, motor3, motor4);
}


/**
 * @brief          控制射击部分的电机(0x205,0x206,0x207,0x208)
 * @param[in]      friction_wheel_left : (0x201) 左摩擦轮电机控制电流
 * @param[in]      friction_wheel_right : (0x201) 右摩擦轮电机控制电流
 * @param[in]      trigger : (0x201) 拨弹盘电机控制电流
 * @param[in]      rev : (0x201) 保留，电机控制电流
 * @retval         none
 */
void CANCmdShoot(int16_t friction_wheel_left, int16_t friction_wheel_right, int16_t trigger, int16_t rev)
{
    CANSendDJIMotorCurrent(&DJI_CAN_1, &shoot_tx_message, TxSTD_ID_1FF,
                           friction_wheel_left, friction_wheel_right, trigger, rev)
}