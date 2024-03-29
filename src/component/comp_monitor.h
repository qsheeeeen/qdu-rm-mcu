/**
 * @file comp_monitor.h
 * @author Qu Shen
 * @brief 监视器，用于监视任务运行状态
 * @version 0.1
 * @date 2021-08-31
 *
 * @copyright Copyright (c) 2021
 *
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_MONITOR_NAME_LEN (20)

typedef struct {
  char name[MAX_MONITOR_NAME_LEN];
  uint8_t priority;
  uint32_t patient_lost;
  uint32_t patient_work;

  bool enable;
  uint32_t showup_last;
  uint32_t cycle_time;
  uint32_t duration_lost;
  uint32_t duration_work;
  uint32_t found_lost;
  bool lost;

} monitor_t;

monitor_t *monitor_create(const char *name, uint8_t priority,
                          uint32_t patient_lost, uint32_t patient_work);

void monitor_report(monitor_t *monitor, uint32_t sys_time);
void monitor_examine(uint32_t sys_time);
void monitor_get_detail(char *detail_string, size_t len);
