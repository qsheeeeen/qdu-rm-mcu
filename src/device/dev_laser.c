#include "dev_laser.h"

#include "hal_tim.h"

int8_t Laser_Start(void) {
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  return 0;
}

int8_t Laser_Set(float duty_cycle) {
  if (duty_cycle > 1.0f) return -1;

  uint16_t pulse = (uint16_t)(duty_cycle * (float)UINT16_MAX);

  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pulse);

  return 0;
}

int8_t Laser_Stop(void) {
  HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
  return 0;
}