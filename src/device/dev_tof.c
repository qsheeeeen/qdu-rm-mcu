#include "dev_tof.h"

#include <stdbool.h>
#include <string.h>

#include "bsp_can.h"
#include "comp_utils.h"
#include "dev_referee.h"

#define TOF_ID_BASE (0x280)

#define TOF_RES (1000) /* TOF数据分辨率 */

void TOF_Decode(tof_feedback_t *fb, const uint8_t *raw) {
  fb->dist = (float)((raw[2] << 16) | (raw[1] << 8) | raw[0]) / (float)TOF_RES;
  fb->status = raw[3];
  fb->signal_strength = (uint16_t)((raw[5] << 8) | raw[4]);
}

err_t tof_init(tof_t *tof) {
  tof->msgq_feedback = xQueueCreate(1, sizeof(can_rx_item_t));

  if (tof->msgq_feedback)
    return RM_OK;
  else
    return ERR_FAIL;
}

err_t tof_update(tof_t *tof, uint32_t timeout) {
  ASSERT(tof);
  can_rx_item_t pack;
  while (pdPASS ==
         xQueueReceive(tof->msgq_feedback, &pack, pdMS_TO_TICKS(timeout))) {
    if (pack.index == 0) {
      TOF_Decode(&(tof->feedback), pack.data);
    }
  }
  return RM_OK;
}

err_t tof_handle_offline(tof_t *tof) {
  ASSERT(tof);
  tof->feedback.dist = 0;
  tof->feedback.signal_strength = 0;
  return RM_OK;
}
