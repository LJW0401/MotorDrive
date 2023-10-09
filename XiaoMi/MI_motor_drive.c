/**
 *
 * @File:        MI_motor_drive.c
 * @Author:      小企鹅
 *
 */
/* Includes -------------------------------------------------------------------*/
#include "MI_motor_drive.h"

CAN_TxHeaderTypeDef CAN_TxHeader_MI;

uint8_t MI_MASTERID = 1; //master id 发送指令时EXTID的bit8:15,反馈的bit0:7

/**
  * @brief          float转int，数据打包用
  * @param[in]      x float数值
  * @param[in]      x_min float数值的最小值
  * @param[in]      x_max float数值的最大值
  * @param[in]      bits  int的数据位数
  * @retval         none
  */
uint32_t FloatToUint(float x, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    if(x > x_max) x=x_max;
    else if(x < x_min) x=x_min;
    return (uint32_t) ((x-offset)*((float)((1<<bits)-1))/span);
}

/**
  * @brief          输入范围限制
  * @param[in]      x 输入数值
  * @param[in]      x_min 输入数值的最小值
  * @param[in]      x_max 输入数值的最大值
  * @retval         none
  */
float RangeRestrict(float x, float x_min, float x_max)
{
    float res;
    if(x > x_max) res=x_max;
    else if(x < x_min) res=x_min;
    else res = x;
    return res;
}

/**
  * @brief          小米电机初始化
  * @param[out]     hmotor 电机结构体
  * @param[in]      phcan can总线句柄
  * @retval         none
  */
void MI_motor_Init(MI_Motor_t* hmotor,CAN_HandleTypeDef *phcan, uint8_t motor_id)
{
    hmotor->phcan = phcan;
    hmotor->motor_id = motor_id;
}

/**
  * @brief          小米电机CAN通信发送
  * @param[in]      hmotor 电机结构体
  * @retval         none
  */
void MI_motor_CanTx(MI_Motor_t* hmotor)
{
    CAN_TxHeader_MI.DLC = 8;
    CAN_TxHeader_MI.IDE = CAN_ID_EXT;
    CAN_TxHeader_MI.RTR = CAN_RTR_DATA;
    CAN_TxHeader_MI.ExtId = *((uint32_t*)&(hmotor->EXT_ID));
    /*检测可用的发送邮箱*/
    uint32_t free_TxMailbox = HAL_CAN_GetTxMailboxesFreeLevel(hmotor->phcan);//检测是否有空闲邮箱
    while (free_TxMailbox<3){//等待空闲邮箱数达到3
        free_TxMailbox = HAL_CAN_GetTxMailboxesFreeLevel(hmotor->phcan);
    }
    /* 将发送信息添加到发送邮箱中 */
    uint32_t mailbox;
    HAL_CAN_AddTxMessage(hmotor->phcan, &CAN_TxHeader_MI, hmotor->txdata, &mailbox);//将发送的数据添加到发送邮箱中
}

/*-------------------- 按照小米电机文档写的各种通信类型 --------------------*/

/**
  * @brief          获取设备ID （通信类型0），需在电机使能前使用
  * @param[in]      hmotor 电机结构体
  * @retval         none
  */
void MI_motor_GetID(MI_Motor_t* hmotor)
{
    hmotor->EXT_ID.mode = 0;
    hmotor->EXT_ID.data = 0;
    hmotor->EXT_ID.motor_id = 0;
    hmotor->EXT_ID.res = 0;
 
    for(uint8_t i=0; i<8; i++)
    {
        hmotor->txdata[i]=0;
    }
    
    MI_motor_CanTx(hmotor);
}

/**
  * @brief          运控模式电机控制指令（通信类型1）
  * @param[in]      hmotor 电机结构体
  * @param[in]      torque 目标力矩
  * @param[in]      MechPosition 
  * @param[in]      speed 
  * @param[in]      kp 
  * @param[in]      kd 
  * @retval         none
  */
void MI_motor_Control(MI_Motor_t* hmotor, float torque, float MechPosition , float speed , float kp , float kd)
{
    hmotor->EXT_ID.mode = 1;
    hmotor->EXT_ID.motor_id = hmotor->motor_id;
    hmotor->EXT_ID.data = FloatToUint(torque,T_MIN,T_MAX,16);
    hmotor->EXT_ID.res = 0;
 
    hmotor->txdata[0]=FloatToUint(MechPosition,P_MIN,P_MAX,16)>>8;
    hmotor->txdata[1]=FloatToUint(MechPosition,P_MIN,P_MAX,16);
    hmotor->txdata[2]=FloatToUint(speed,V_MIN,V_MAX,16)>>8;
    hmotor->txdata[3]=FloatToUint(speed,V_MIN,V_MAX,16);
    hmotor->txdata[4]=FloatToUint(kp,KP_MIN,KP_MAX,16)>>8;
    hmotor->txdata[5]=FloatToUint(kp,KP_MIN,KP_MAX,16);
    hmotor->txdata[6]=FloatToUint(kd,KD_MIN,KD_MAX,16)>>8;
    hmotor->txdata[7]=FloatToUint(kd,KD_MIN,KD_MAX,16);

    MI_motor_CanTx(hmotor);
}

/**
  * @brief          小米电机使能（通信类型 3）
  * @param[in]      hmotor 电机结构体
  * @param[in]      id 电机id
  * @retval         none
  */
void MI_motor_Enable(MI_Motor_t* hmotor)
{
    hmotor->EXT_ID.mode = 3;
    hmotor->EXT_ID.motor_id = hmotor->motor_id;
    hmotor->EXT_ID.data = MI_MASTERID;
    hmotor->EXT_ID.res = 0;
    for(uint8_t i=0; i<8; i++)
    {
        hmotor->txdata[i]=0;
    }

    MI_motor_CanTx(hmotor);
}

/**
  * @brief          电机停止运行帧（通信类型4）
  * @param[in]      hmotor 电机结构体
  * @retval         none
  */
void MI_motor_Stop(MI_Motor_t* hmotor)
{
    hmotor->EXT_ID.mode = 4;
    hmotor->EXT_ID.motor_id = hmotor->motor_id;
    hmotor->EXT_ID.data = MI_MASTERID;
    hmotor->EXT_ID.res = 0;
 
    for(uint8_t i=0; i<8; i++)
    {
        hmotor->txdata[i]=0;
    }

    MI_motor_CanTx(hmotor);
}

/**
  * @brief          设置电机机械零位（通信类型6）会把当前电机位置设为机械零位（掉电丢失）
  * @param[in]      hmotor 电机结构体
  * @retval         none
  */
void MI_motor_SetMechPositionToZero(MI_Motor_t* hmotor)
{
    hmotor->EXT_ID.mode = 6;
    hmotor->EXT_ID.motor_id = hmotor->motor_id;
    hmotor->EXT_ID.data = MI_MASTERID;
    hmotor->EXT_ID.res = 0;
    hmotor->txdata[0]=1;
 
    for(uint8_t i=1; i<8; i++)
    {
        hmotor->txdata[i]=0;
    }

    MI_motor_CanTx(hmotor);
}

/**
  * @brief          设置电机CAN_ID（通信类型7）更改当前电机CAN_ID , 立即生效，需在电机使能前使用
  * @param[in]      hmotor 电机结构体
  * @param[in]      Now_ID 电机现在的ID
  * @param[in]      Target_ID 想要改成的电机ID
  * @retval         none
  */
void MI_motor_ChangeID(MI_Motor_t* hmotor,uint8_t Now_ID,uint8_t Target_ID)
{
    hmotor->motor_id = Now_ID;

    hmotor->EXT_ID.mode = 7;	
    hmotor->EXT_ID.motor_id = Now_ID;
    hmotor->EXT_ID.data = Target_ID << 8 | MI_MASTERID;
    hmotor->EXT_ID.res = 0;
 
    for(uint8_t i=0; i<8; i++)
    {
        hmotor->txdata[i]=0;
    }

    MI_motor_CanTx(hmotor);
}

/**
  * @brief          单个参数读取（通信类型17）
  * @param[in]      hmotor 电机结构体
  * @param[in]      index 功能码
  * @retval         none
  */
void MI_motor_ReadParam(MI_Motor_t* hmotor,uint16_t index)
{
    hmotor->EXT_ID.mode = 17;
    hmotor->EXT_ID.motor_id = hmotor->motor_id;
    hmotor->EXT_ID.data = MI_MASTERID;
    hmotor->EXT_ID.res = 0;
    
    memcpy(&hmotor->txdata[0],&index,2);

    for(uint8_t i=2; i<8; i++)
    {
        hmotor->txdata[i]=0;
    }

    MI_motor_CanTx(hmotor);
}

/**
  * @brief          小米电机运行模式切换
  * @param[in]      hmotor 电机结构体
  * @param[in]      run_mode 更改的模式
  * @note           通信类型18 （掉电丢失）
  * @retval         none
  */
void MI_motor_ModeSwitch(MI_Motor_t* hmotor, uint8_t run_mode)
{
    uint16_t index = 0X7005;

    hmotor->EXT_ID.mode = 18;
    hmotor->EXT_ID.motor_id = hmotor->motor_id;
    hmotor->EXT_ID.data = MI_MASTERID;
    hmotor->EXT_ID.res = 0;

    for(uint8_t i=0;i<8;i++){
      hmotor->txdata[i]=0;
    }

    memcpy(&hmotor->txdata[0],&index,2);
    memcpy(&hmotor->txdata[4],&run_mode, 1);

    MI_motor_CanTx(hmotor);
}

/**
  * @brief          小米电机控制参数写入
  * @param[in]      hmotor 电机结构体
  * @param[in]      index 功能码
  * @param[in]      param 写入的参数
  * @note           通信类型18 （掉电丢失）
  * @retval         none
  */
 void MI_motor_WritePram(MI_Motor_t* hmotor, uint16_t index, float param)
 {
    hmotor->EXT_ID.mode = 18;
    hmotor->EXT_ID.motor_id = hmotor->motor_id;
    hmotor->EXT_ID.data = MI_MASTERID;
    hmotor->EXT_ID.res = 0;

    memcpy(&hmotor->txdata[0],&index,2);
    hmotor->txdata[2]=0;
    hmotor->txdata[3]=0;
    memcpy(&hmotor->txdata[4],&param, 4);

    MI_motor_CanTx(hmotor);
 }

/**
  * @brief          小米电机反馈帧解码（通信类型2）
  * @param[in]      Rx_can_info 接受到的电机数据结构体
  * @param[in]      rx_data[8] CAN线接收到的数据
  * @note           将接收到的CAN线数据解码到电机数据结构体中
  * @retval         none
  */
void MI_motor_RxDecode(RxCAN_info_s* RxCAN_info,uint8_t rx_data[8])
{
    uint16_t decode_temp_mi;//小米电机反馈数据解码缓冲
    decode_temp_mi = (rx_data[0] << 8 | rx_data[1]);
    RxCAN_info->angle = ((float)decode_temp_mi-32767.5)/32767.5*4*3.1415926f;;

    decode_temp_mi = (rx_data[2] << 8 | rx_data[3]);
    RxCAN_info->speed = ((float)decode_temp_mi-32767.5)/32767.5*30.0f;

    decode_temp_mi = (rx_data[4] << 8 | rx_data[5]);
    RxCAN_info->torque = ((float)decode_temp_mi-32767.5)/32767.5*12.0f;

    decode_temp_mi = (rx_data[6] << 8 | rx_data[7]);
    RxCAN_info->temperature = (float)decode_temp_mi/10.0f;
 }

/*-------------------- 封装的一些控制函数 --------------------*/

/**
  * @brief          小米电机运控模式控制指令
  * @param[in]      hmotor 电机结构体
  * @param[in]      torque 目标力矩
  * @param[in]      MechPosition 
  * @param[in]      speed 
  * @param[in]      kp 
  * @param[in]      kd 
  * @retval         none
  */
void MI_motor_ControlMode(MI_Motor_t* hmotor, float torque, float MechPosition , float speed , float kp , float kd)
{
    MI_motor_ModeSwitch(hmotor, CONTROL_MODE);
    MI_motor_Enable(hmotor);
    MI_motor_Control(hmotor, torque, MechPosition, speed, kp, kd);
}

/**
  * @brief          小米电机位置模式控制指令
  * @param[in]      hmotor 电机结构体
  * @param[in]      loc_ref 控制位置 rad
  * @param[in]      limit_spd 预设最大速度(0~30rad/s)
  * @retval         none
  */
void MI_motor_LocationMode(MI_Motor_t* hmotor, float loc_ref, float limit_spd)
{
    MI_motor_ModeSwitch(hmotor, LOCATION_MODE);
    MI_motor_Enable(hmotor);
    MI_motor_WritePram(hmotor, LIMIT_SPD, RangeRestrict(limit_spd, LIMIT_SPD_MIN, LIMIT_SPD_MAX));
    MI_motor_WritePram(hmotor, LOC_REF, loc_ref);
}

/**
  * @brief          小米电机速度模式控制指令
  * @param[in]      hmotor 电机结构体
  * @param[in]      spd_ref 控制速度
  * @retval         none
  */
void MI_motor_SpeedMode(MI_Motor_t* hmotor, float spd_ref)
{
    MI_motor_ModeSwitch(hmotor, SPEED_MODE);
    MI_motor_Enable(hmotor);
    MI_motor_WritePram(hmotor, SPD_REF, RangeRestrict(spd_ref, SPD_REF_MIN, SPD_REF_MAX));
}

/**
  * @brief          小米电机电流模式控制指令
  * @param[in]      hmotor 电机结构体
  * @param[in]      iq_ref 控制电流
  * @retval         none
  */
void MI_motor_CurrentMode(MI_Motor_t* hmotor, float iq_ref)
{
    MI_motor_ModeSwitch(hmotor, CURRENT_MODE);
    MI_motor_Enable(hmotor);
    MI_motor_WritePram(hmotor, IQ_REF, RangeRestrict(iq_ref, IQ_REF_MIN, IQ_REF_MAX));
}