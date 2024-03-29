#pragma once

#include <stdint.h>

#include "bsp.h"
/* PWM通道 */
typedef enum {
  BSP_PWM_IMU_HEAT,
  BSP_PWM_LAUNCHER_SERVO,
} BSP_PWM_Channel_t;

int8_t BSP_PWM_Start(BSP_PWM_Channel_t ch);
int8_t BSP_PWM_Set(BSP_PWM_Channel_t ch, float duty_cycle);
int8_t BSP_PWM_Stop(BSP_PWM_Channel_t ch);
