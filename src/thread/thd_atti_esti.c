/**
 * @file thd_atti_esti.c
 * @author Qu Shen
 * @brief 姿态解算线程
 * @version 0.1
 * @date 2021-09-05
 *
 * @copyright Copyright (c) 2021
 *
 * 收集传感器数据，解算后得到四元数，转换为欧拉角之后发布。
 */

#include <string.h>

#include "comp_ahrs.h"
#include "mid_msg_dist.h"
#include "thd.h"

#define THD_PERIOD_MS (2)
#define THD_DELAY_TICK (pdMS_TO_TICKS(THD_PERIOD_MS))

void thd_atti_esti(void* arg) {
  RM_UNUSED(arg);

  ahrs_t gimbal_ahrs;
  eulr_t gimbal_eulr;
  vector3_t accl, gyro;

  publisher_t* gimbal_eulr_pub =
      msg_dist_create_topic("gimbal_eulr", sizeof(eulr_t));

  subscriber_t* accl_sub = msg_dist_subscribe("gimbal_accl", true);
  subscriber_t* gyro_sub = msg_dist_subscribe("gimbal_gyro", true);

  /* 初始化姿态解算算法 */
  ahrs_init(&gimbal_ahrs, NULL);

  float now;
  uint32_t previous_wake_time = xTaskGetTickCount();
  while (1) {
    msg_dist_poll(accl_sub, &accl, 0);
    msg_dist_poll(gyro_sub, &gyro, 0);

    /* 锁住RTOS内核防止数据解析过程中断，造成错误 */
    vTaskSuspendAll();

    /* 根据设备接收到的数据进行姿态解析 */
    now = (float)xTaskGetTickCount() / configTICK_RATE_HZ;
    ahrs_update(&gimbal_ahrs, &accl, &gyro, NULL, now);

    /* 根据解析出来的四元数计算欧拉角 */
    ahrs_get_eulr(&gimbal_eulr, &gimbal_ahrs);
    xTaskResumeAll();

    /* 发布数据 */
    msg_dist_publish(gimbal_eulr_pub, &gimbal_eulr);

    /* 运行结束，等待下一次唤醒 */
    xTaskDelayUntil(&previous_wake_time, THD_DELAY_TICK);
  }
}
THREAD_DECLEAR(thd_atti_esti, 256, 3);
