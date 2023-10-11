# 介绍
在使用小米电机的时候我们通常会选择一种模式进行控制，可以参考示例代码中的各种模式控制代码。

除了发送控制指令对电机进行控制外，我们通常还需要获取电机的状态，这时候就需要用到CAN中断回调函数来接收电机返回的信息，同样可以参考示例代码中的中断回调函数代码。

## 完整功能体验
- [Gitee](https://gitee.com/Ljw0401/let-xiaomi-motor-move)
- 

## 驱动介绍
小米电机结构体
- 我们定义了小米电机结构体来标识电机的数据。
    ```C
    MI_Motor_t
    ```
- 使用对应的初始化函数进行初始化。
    ```C
    MI_motor_Init()
    ```

依据[CyberGear官方文档](https://gitee.com/SMBU-POLARBEAR/technical-documentation/blob/master/%E7%94%B5%E6%9C%BA/%E5%B0%8F%E7%B1%B3/CyberGear%E5%BE%AE%E7%94%B5%E6%9C%BA%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8E%E4%B9%A6.pdf)我们写了对应的几种通信类型<br>

- 通信类型0：获取设备 ID，获取设备的ID和64位MCU唯一标识符。
    ```C
    MI_motor_GetID()
    ```
- 通信类型1： 运控模式电机控制指令，用来向电机发送控制指令 。
    ```C
    MI_motor_Control()
    ```
- 通信类型2：电机反馈数据，在CAN中断回调函数中使用，用来向主机反馈电机运行状态。
    ```C
    MI_motor_RxDecode()
    ```
- 通信类型3：电机使能运行，启动电机。
    ```C
    MI_motor_Enable()
    ```
- 通信类型4：电机停止运行。
    ```C
    MI_motor_Stop()
    ```
- 通信类型6：设置电机机械零位，会把当前电机位置设为机械零位（掉电丢失） 。
    ```C
    MI_motor_SetMechPositionToZero()
    ```
- 通信类型7： 设置电机 CAN_ID，更改当前电机 CAN_ID , 立即生效。
    ```C
    MI_motor_ChangeID()
    ```
- 通信类型17：单个参数读取。
    ```C
    MI_motor_ReadParam()
    ```
- 通信类型18：单个参数写入。
    ```C
    MI_motor_ModeSwitch()
    MI_motor_WritePram()
    ```
<!-- - 通信类型21： -->

同时我们也封装了不同控制模式的函数，直接使用即可。
>因为在2023.10.11的测试中发现之前按照小米官方文档写的函数驱动多台电机时出现问题，参考[小米电机CyberGear STM32HAL 使用指南](https://blog.csdn.net/zdYukino/article/details/133505453)后修改了位置模式和速度模式的控制方式，至于依据小米官方文档的控制方式继续研究中。
- 力矩模式
    ```C
    MI_motor_TorqueControl()
    ```
- 位置模式
    ```C
    MI_motor_LocationControl()
    ```
- 速度模式
    ```C
    MI_motor_SpeedControl()
    ```

为了接收电机的数据，我们还提供了[CAN中断回调函数的示例代码](#can中断回调函数)

## 示例代码
### 力矩模式
```C
#include "MI_motor_drive.h"
extern CAN_HandleTypeDef hcan1;

MI_Motor_t MI_motor;

void control_task(void const *pvParameters)
{
    MI_motor_Init(&MI_Motor,&MI_CAN_1,1);//小米电机结构体初始化
    MI_motor_Enable(&MI_Motor);//电机使能
    while (1)
    {
        float torque = 1.0f;//设定输出力矩为1N*m
        MI_motor_TorqueControl(&MI_Motor, torque);//发送控制信号

        vTaskDelay(4);//系统延时(os delay)
    }
}
```

### 位置模式
```C
#include "MI_motor_drive.h"
extern CAN_HandleTypeDef hcan1;

MI_Motor_t MI_motor;

void control_task(void const *pvParameters)
{
    MI_motor_Init(&MI_Motor,&MI_CAN_1,1);//小米电机结构体初始化
    MI_motor_Enable(&MI_Motor);//电机使能
    while (1)
    {
        float location = 1;//设定停止在 1rad 的位置
        MI_motor_LocationControl(&MI_Motor,location,5,0.5);

        vTaskDelay(4);//系统延时(os delay)
    }
}
```

### 速度模式
```C
#include "MI_motor_drive.h"
extern CAN_HandleTypeDef hcan1;

MI_Motor_t MI_motor;

void control_task(void const *pvParameters)
{
    MI_motor_Init(&MI_Motor,&MI_CAN_1,1);//小米电机结构体初始化
    MI_motor_Enable(&MI_Motor);//电机使能
    while (1)
    {
        float speed = 2;//设定运行速度为 2rad/s
        MI_motor_SpeedControl(&MI_Motor,spd_ref,1);

        vTaskDelay(4);//系统延时(os delay)
    }
}
```

<!-- ### 电流模式
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
``` -->

### CAN中断回调函数
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

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data);

    memcpy(&RxCAN_info,&rx_header.ExtId,4);//将扩展标识符的内容解码到缓存区，获取通信类型

    if(RxCAN_info.communication_type == 0){//通信类型0的反馈帧解码
        RxCAN_info_type_0_s RxCAN_info_type_0;
        memcpy(&RxCAN_info_type_0,&rx_header.ExtId,4);//将扩展标识符的内容解码成通信类型0的对应内容
        memcpy(&RxCAN_info_type_0.MCU_id,rx_data,8);//获取MCU标识符
        OutputData.data_3 = RxCAN_info_type_0.motor_id;
    }else if(RxCAN_info.communication_type == 2){//通信类型2的反馈帧解码
        RxCAN_info_type_2_s RxCAN_info_type_2;
        memcpy(&RxCAN_info_type_2,&rx_header.ExtId,4);//将扩展标识符的内容解码成通信类型2的对应内容
        MI_motor_RxDecode(&RxCAN_info_type_2,rx_data);//通信类型2的数据解码
        if (RxCAN_info_type_2.motor_id == 1){
            MI_Motor.RxCAN_info = RxCAN_info_type_2;
            MI_Motor.motor_mode_state = RxCAN_info_type_2.mode_state;
        }
    }else if(RxCAN_info.communication_type == 17){//通信类型17的反馈帧解码
        RxCAN_info_type_17_s RxCAN_info_type_17;
        memcpy(&RxCAN_info_type_17,&rx_header.ExtId,4);//将扩展标识符的内容解码成通信类型17的对应内容
        memcpy(&RxCAN_info_type_17.index,&rx_data[0],2);//获取查找的参数索引码
        memcpy(&RxCAN_info_type_17.param,&rx_data[4],4);//获取查找的参数信息
    }
}
```

## 参考资料
[CyberGear微电机使用说明书.pdf](https://gitee.com/SMBU-POLARBEAR/technical-documentation/blob/master/%E7%94%B5%E6%9C%BA/%E5%B0%8F%E7%B1%B3/CyberGear%E5%BE%AE%E7%94%B5%E6%9C%BA%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8E%E4%B9%A6.pdf)<br>
[小米微电机STM32 HAL库驱动教程](https://blog.csdn.net/m0_53802226/article/details/132941275)<br>
[小米电机CyberGear STM32HAL 使用指南](https://blog.csdn.net/zdYukino/article/details/133505453)<br>