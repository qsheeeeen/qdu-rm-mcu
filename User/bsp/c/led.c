/* Includes ----------------------------------------------------------------- */
#include "bsp/led.h"

#include <tim.h>

/* Private define ----------------------------------------------------------- */
/* Private macro ------------------------------------------------------------ */
/* Private typedef ---------------------------------------------------------- */
/* Private variables -------------------------------------------------------- */
static uint32_t led_stats;

/* Private function  -------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

int8_t BSP_LED_Set(BSP_LED_Channel_t ch, BSP_LED_Status_t s, float duty_cycle) {
  if (duty_cycle > 1.0f) return BSP_ERR;

  uint32_t tim_ch;
  uint16_t pulse = (uint16_t)(duty_cycle * (float)UINT16_MAX);

  switch (ch) {
    case BSP_LED_RED:
      __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, pulse);
      tim_ch = TIM_CHANNEL_3;
      break;

    case BSP_LED_GRN:
      __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2, pulse);
      tim_ch = TIM_CHANNEL_2;
      break;

    case BSP_LED_BLU:
      __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, pulse);
      tim_ch = TIM_CHANNEL_1;
      break;
  }

  switch (s) {
    case BSP_LED_ON:
      HAL_TIM_PWM_Start(&htim5, tim_ch);
      led_stats |= tim_ch;
      break;

    case BSP_LED_OFF:
      HAL_TIM_PWM_Stop(&htim5, tim_ch);
      led_stats &= ~tim_ch;
      break;

    case BSP_LED_TAGGLE:
      if (led_stats & tim_ch) {
        HAL_TIM_PWM_Stop(&htim5, tim_ch);
        led_stats &= ~tim_ch;
      } else {
        HAL_TIM_PWM_Start(&htim5, tim_ch);
        led_stats |= tim_ch;
      }
      break;
  }

  return BSP_OK;
}
