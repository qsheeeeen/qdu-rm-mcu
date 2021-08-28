/**
 * @file can.c
 * @author Qu Shen (503578404@qq.com)
 * @brief CAN总线数据处理
 * @version 1.0.0
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 * 从消息队列中得到CAN总线收到的原始数据
 * 解析后分组存放
 * 根据需要发给对应的任务
 *
 */

/* Includes ----------------------------------------------------------------- */
#include "device/can.h"

#include "device/referee.h"
#include "task/user_task.h"

/* Private typedef ---------------------------------------------------------- */
/* Private define ----------------------------------------------------------- */
/* Private macro ------------------------------------------------------------ */
/* Private variables -------------------------------------------------------- */

#ifdef DEBUG
CAN_t can;
CAN_Output_t can_out;
CAN_RawRx_t can_rx;
#else
static CAN_t can;
static CAN_Output_t can_out;
static CAN_RawRx_t can_rx;
#endif

/* Private function --------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
void Task_Can(void *argument) {
  UNUSED(argument);
  const uint32_t delay_tick = osKernelGetTickFreq() / TASK_FREQ_CAN;

  /* Device Setup */
  CAN_Init(&can, &runtime.cfg.robot_param->can);

  uint32_t tick = osKernelGetTickCount(); /* 控制任务运行频率的计时 */
  /* Task Setup */
  while (1) {
#ifdef DEBUG
    runtime.stack_water_mark.can = osThreadGetStackSpace(osThreadGetId());
#endif
    tick += delay_tick; /* 计算下一个唤醒时刻 */
    while (osMessageQueueGet(can.msgq_raw, &can_rx, 0, 0) == osOK) {
      CAN_StoreMsg(&can, &can_rx);
    }
    osMessageQueueReset(runtime.msgq.can.feedback.chassis);
    osMessageQueuePut(runtime.msgq.can.feedback.chassis, &can, 0, 0);

    osMessageQueueReset(runtime.msgq.can.feedback.gimbal);
    osMessageQueuePut(runtime.msgq.can.feedback.gimbal, &can, 0, 0);

    osMessageQueueReset(runtime.msgq.can.feedback.launcher);
    osMessageQueuePut(runtime.msgq.can.feedback.launcher, &can, 0, 0);

    if (CAN_CheckFlag(&can, CAN_REC_CAP_FINISHED, true)) {
      osMessageQueueReset(runtime.msgq.can.feedback.cap);
      osMessageQueuePut(runtime.msgq.can.feedback.cap, &can, 0, 0);
    } else {
      // Error Handle
    }

    if (CAN_CheckFlag(&can, CAN_REC_TOF_FINISHED, true)) {
      osMessageQueueReset(runtime.msgq.can.feedback.tof);
      osMessageQueuePut(runtime.msgq.can.feedback.tof, &can, 0, 0);
    } else {
      // Error Handle
    }

    if (osMessageQueueGet(runtime.msgq.can.output.chassis, &(can_out.chassis),
                          0, 0) == osOK) {
      CAN_Motor_Control(CAN_MOTOR_GROUT_CHASSIS, &can_out, &can);
    }

    if (osMessageQueueGet(runtime.msgq.can.output.gimbal, &(can_out.gimbal), 0,
                          0) == osOK) {
      CAN_Motor_Control(CAN_MOTOR_GROUT_GIMBAL1, &can_out, &can);
    }

    if (osMessageQueueGet(runtime.msgq.can.output.launcher, &(can_out.launcher),
                          0, 0) == osOK) {
      CAN_Motor_Control(CAN_MOTOR_GROUT_LAUNCHER1, &can_out, &can);
    }

    if (osMessageQueueGet(runtime.msgq.can.output.cap, &(can_out.cap), 0, 0) ==
        osOK) {
      CAN_Cap_Control(&(can_out.cap), &can);
    }
    osDelayUntil(tick); /* 运行结束，等待下一次唤醒 */
  }
}
