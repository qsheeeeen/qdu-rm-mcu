#pragma once

#include <stdbool.h>

#include "FreeRTOS.h"
#include "comp_type.h"
#include "comp_ui.h"
#include "queue.h"

typedef struct {
  float input_volt;
  float cap_volt;
  float input_curr;
  float target_power;
  float percentage;
} cap_feedback_t;

typedef struct {
  float power_limit;
} cap_control_t;

typedef struct {
  QueueHandle_t msgq_control;
  QueueHandle_t msgq_feedback;

  cap_feedback_t feedback;
} cap_t;

err_t cap_init(cap_t *cap);
err_t cap_update(cap_t *cap, uint32_t timeout);
err_t cap_control(cap_t *cap, cap_control_t *output);
err_t cap_handle_offline(cap_t *cap);
err_t cap_pack_ui(const cap_t *cap, ui_cap_t *ui);
