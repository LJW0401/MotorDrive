# 介绍
在使用小米电机的时候我们通常会选择一种模式进行控制，可以参考示例代码中的各种模式控制代码。

除了发送控制指令对电机进行控制外，我们通常还需要获取电机的状态，这时候就需要用到CAN中断回调函数来接收电机返回的信息，同样可以参考示例代码中的中断回调函数代码。

# 示例代码
## 运动控制模式
```C
#include "MI_motor_drive.h"
extern CAN_HandleTypeDef hcan1;

MI_Motor_t MI_motor;

void control_task(void const *pvParameters)
{
    MI_motor_Init(&MI_Motor,&MI_CAN_1,1);//小米电机结构体初始化
    while (1)
    {
        float torque = 1.0f;//设定输出力矩为1N*m
        MI_motor_ControlMode(&MI_Motor, torque, 0, 0, 0, 0);//发送控制信号

        vTaskDelay(4);//系统延时(os delay)
    }
}
```

## 位置模式
```C
#include "MI_motor_drive.h"
extern CAN_HandleTypeDef hcan1;

MI_Motor_t MI_motor;

void control_task(void const *pvParameters)
{
    MI_motor_Init(&MI_Motor,&MI_CAN_1,1);//小米电机结构体初始化
    while (1)
    {
        float loc_ref = 1;//设定停止在 1rad 的位置
        float limit_spd = 1;//设定运行时的最大速度为 1rad/s
        MI_motor_LocationMode(&MI_Motor,loc_ref,limit_spd);

        vTaskDelay(4);//系统延时(os delay)
    }
}
```

## 速度模式
```C
#include "MI_motor_drive.h"
extern CAN_HandleTypeDef hcan1;

MI_Motor_t MI_motor;

void control_task(void const *pvParameters)
{
    MI_motor_Init(&MI_Motor,&MI_CAN_1,1);//小米电机结构体初始化
    while (1)
    {
        float spd_ref = 2;//设定运行速度为 1rad/s
        MI_motor_SpeedMode(&MI_Motor,spd_ref);

        vTaskDelay(4);//系统延时(os delay)
    }
}
```

## 电流模式
```C
#include "MI_motor_drive.h"
extern CAN_HandleTypeDef hcan1;

MI_Motor_t MI_motor;

void control_task(void const *pvParameters)
{
    MI_motor_Init(&MI_Motor,&MI_CAN_1,1);//小米电机结构体初始化
    while (1)
    {
        float iq_ref = 1;//设定电流为 1A
        MI_motor_CurrentMode(&MI_Motor,iq_ref);

        vTaskDelay(4);//系统延时(os delay)
    }
}
```

## CAN中断回调函数
```C
/**
  * @brief          hal库CAN回调函数,接收电机数据
  * @param[in]      hcan:CAN句柄指针
  * @retval         none
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rx_header;
    RxCAN_info_s RxCAN_info;//用于存储小米电机反馈的数据
    uint8_t rx_data[8];

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data);//获取CAN数据
    if (rx_header.IDE == CAN_ID_EXT) {//小米电机解码
        memcpy(&RxCAN_info,&rx_header.ExtId,29);//将扩展标识符的内容解码成对应内容

        if (RxCAN_info.communication_type == 2){//若为通信类型2的反馈帧就对应解码
          MI_motor_RxDecode(&RxCAN_info,rx_data);
        }

        if (RxCAN_info.motor_id == 1){//根据ID更新对应电机的返回值
            MI_Motor.RxCAN_info = RxCAN_info;
        }
    }
}
```