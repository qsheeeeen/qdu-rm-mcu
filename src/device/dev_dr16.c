/*
        DR16接收机

*/

#include "dev_dr16.h"

#include <string.h>

#include "FreeRTOS.h"
#include "bsp_uart.h"
#include "task.h"

#define DR16_CH_VALUE_MIN (364u)
#define DR16_CH_VALUE_MID (1024u)
#define DR16_CH_VALUE_MAX (1684u)

static TaskHandle_t thread_alert;
static bool inited = false;

static void DR16_RxCpltCallback(void) {
  BaseType_t switch_required;
  xTaskNotifyFromISR(thread_alert, SIGNAL_DR16_RAW_REDY, eSetValueWithOverwrite,
                     &switch_required);
  portYIELD_FROM_ISR(switch_required);
}

static bool DR16_DataCorrupted(const dr16_t *dr16) {
  ASSERT(dr16);

  if ((dr16->data.ch_r_x < DR16_CH_VALUE_MIN) ||
      (dr16->data.ch_r_x > DR16_CH_VALUE_MAX))
    return true;

  if ((dr16->data.ch_r_y < DR16_CH_VALUE_MIN) ||
      (dr16->data.ch_r_y > DR16_CH_VALUE_MAX))
    return true;

  if ((dr16->data.ch_l_x < DR16_CH_VALUE_MIN) ||
      (dr16->data.ch_l_x > DR16_CH_VALUE_MAX))
    return true;

  if ((dr16->data.ch_l_y < DR16_CH_VALUE_MIN) ||
      (dr16->data.ch_l_y > DR16_CH_VALUE_MAX))
    return true;

  if (dr16->data.sw_l == 0) return true;

  if (dr16->data.sw_r == 0) return true;

  return false;
}

err_t dr16_init(dr16_t *dr16) {
  ASSERT(dr16);
  if (inited) return ERR_INITED;
  VERIFY((thread_alert = xTaskGetCurrentTaskHandle()) != NULL);

  BSP_UART_RegisterCallback(BSP_UART_DR16, BSP_UART_RX_CPLT_CB,
                            DR16_RxCpltCallback);

  inited = true;
  return RM_OK;
}

err_t dr16_restart(void) {
  __HAL_UART_DISABLE(BSP_UART_GetHandle(BSP_UART_DR16));
  __HAL_UART_ENABLE(BSP_UART_GetHandle(BSP_UART_DR16));
  return RM_OK;
}

err_t dr16_start_dma_recv(dr16_t *dr16) {
  ASSERT(dr16);
  if (HAL_UART_Receive_DMA(BSP_UART_GetHandle(BSP_UART_DR16),
                           (uint8_t *)&(dr16->data),
                           sizeof(dr16->data)) == HAL_OK)
    return RM_OK;
  return ERR_FAIL;
}

bool dr16_wait_dma_cplt(uint32_t timeout) {
  return xTaskNotifyWait(0, 0, SIGNAL_DR16_RAW_REDY, pdMS_TO_TICKS(timeout));
}

err_t dr16_parse_rc(const dr16_t *dr16, cmd_rc_t *rc) {
  ASSERT(dr16);
  ASSERT(rc);

  if (DR16_DataCorrupted(dr16)) {
    return ERR_FAIL;
  } else {
    memset(rc, 0, sizeof(*rc));
  }

  float full_range = (float)(DR16_CH_VALUE_MAX - DR16_CH_VALUE_MIN);

  rc->ch.r.x = 2 * ((float)dr16->data.ch_r_x - DR16_CH_VALUE_MID) / full_range;
  rc->ch.r.y = 2 * ((float)dr16->data.ch_r_y - DR16_CH_VALUE_MID) / full_range;
  rc->ch.l.x = 2 * ((float)dr16->data.ch_l_x - DR16_CH_VALUE_MID) / full_range;
  rc->ch.l.y = 2 * ((float)dr16->data.ch_l_y - DR16_CH_VALUE_MID) / full_range;

  rc->sw_l = (cmd_switch_pos_t)dr16->data.sw_l;
  rc->sw_r = (cmd_switch_pos_t)dr16->data.sw_r;

  rc->mouse.x = dr16->data.x;
  rc->mouse.y = dr16->data.y;
  rc->mouse.z = dr16->data.z;

  rc->mouse.click.l = dr16->data.press_l;
  rc->mouse.click.r = dr16->data.press_r;

  rc->key = dr16->data.key;

  rc->ch_res = ((float)dr16->data.res - DR16_CH_VALUE_MID) / full_range;
  return RM_OK;
}

err_t dr16_handle_offline(const dr16_t *dr16, cmd_rc_t *rc) {
  ASSERT(dr16);
  ASSERT(rc);

  RM_UNUSED(dr16);
  memset(rc, 0, sizeof(*rc));
  return RM_OK;
}
