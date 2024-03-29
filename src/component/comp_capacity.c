/*
  剩余电量算法。

  通过电压值计算剩余电量。
*/

#include "comp_capacity.h"

/**
 * @brief 通过电压计算电池剩余电量
 *
 * @param vbat 电池电压值
 * @return float 剩余电量比例
 */
float capacity_get_battery_remain(float vbat) {
  const float TB47S_CELL = 6.0f;
  if (vbat > (4.2f * TB47S_CELL)) {
    return 1.0f;
  } else {
    float hi, lo, base, step;
    if (vbat > (4.06f * TB47S_CELL)) {
      hi = 4.2f;
      lo = 4.06f;
      base = 0.9f;
      step = 0.1f;
    } else if (vbat > (4.0f * TB47S_CELL)) {
      hi = 4.06f;
      lo = 4.0f;
      base = 0.8f;
      step = 0.1f;
    } else if (vbat > (3.93f * TB47S_CELL)) {
      hi = 4.0f;
      lo = 3.93f;
      base = 0.7f;
      step = 0.1f;
    } else if (vbat > (3.87f * TB47S_CELL)) {
      hi = 3.93f;
      lo = 3.87f;
      base = 0.6f;
      step = 0.1f;
    } else if (vbat > (3.82f * TB47S_CELL)) {
      hi = 3.87f;
      lo = 3.82f;
      base = 0.5f;
      step = 0.1f;
    } else if (vbat > (3.79f * TB47S_CELL)) {
      hi = 3.82f;
      lo = 3.79f;
      base = 0.4f;
      step = 0.1f;
    } else if (vbat > (3.77f * TB47S_CELL)) {
      hi = 3.79f;
      lo = 3.77f;
      base = 0.3f;
      step = 0.1f;
    } else if (vbat > (3.73f * TB47S_CELL)) {
      hi = 3.77f;
      lo = 3.73f;
      base = 0.2f;
      step = 0.1f;
    } else if (vbat > (3.7f * TB47S_CELL)) {
      hi = 3.73f;
      lo = 3.7f;
      base = 0.15f;
      step = 0.05f;
    } else if (vbat > (3.68f * TB47S_CELL)) {
      hi = 3.7f;
      lo = 3.68f;
      base = 0.1f;
      step = 0.05f;
    } else if (vbat > (3.5f * TB47S_CELL)) {
      hi = 3.68f;
      lo = 3.5f;
      base = 0.05f;
      step = 0.05f;
    } else if (vbat > (2.5f * TB47S_CELL)) {
      hi = 3.5f;
      lo = 2.5f;
      base = 0.0f;
      step = 0.05f;
    } else {
      return 0.0f;
    }
    return base + step / (hi - lo) * (vbat / TB47S_CELL - lo);
  }
}

/**
 * @brief 通过电压计算容剩余电量
 *
 * @param vcap 电容电压
 * @param vbat 电池电压
 * @param v_cutoff 截止电压
 * @return float 电容剩余电量比例
 */
float capacity_get_capacitor_remain(float vcap, float vbat, float v_cutoff) {
  /* 根据公式E=1/2CU^2，省略常数项 */
  const float c_max = vbat * vbat;
  const float c_cap = vcap * vcap;
  const float c_min = v_cutoff * v_cutoff;
  float percentage = (c_cap - c_min) / (c_max - c_min);
  clampf(&percentage, 0.0f, 1.0f);
  return percentage;
}
