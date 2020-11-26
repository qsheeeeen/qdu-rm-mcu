/*
  混合器
*/

#include "mixer.h"

#include "math.h"

/**
 * @brief 初始化混合器
 * 
 * @param mixer 混合器
 * @param mode 混合器模式
 * @return int8_t 0对应没有错误
 */
int8_t Mixer_Init(Mixer_t *mixer, Mixer_Mode_t mode) {
  if (mixer == NULL) return -1;

  mixer->mode = mode;
  return 0;
}

/**
 * @brief 计算输出
 * 
 * @param mixer 混合器
 * @param vx x轴方向
 * @param vy y轴方向
 * @param wz z轴旋转
 * @param out 输出数组
 * @param len 输出数组长短
 * @param scale 输出放大因子
 * @return int8_t 0对应没有错误
 */
int8_t Mixer_Apply(Mixer_t *mixer, float vx, float vy, float wz, float *out,
                   int8_t len, float scale) {
  if (mixer == NULL) return -1;

  switch (mixer->mode) {
    case MIXER_MECANUM:
      if (len == 4) {
        out[0] = vx - vy + wz;
        out[1] = vx + vy + wz;
        out[2] = -vx + vy + wz;
        out[3] = -vx - vy + wz;
      } else {
        goto error;
      }
      break;

    case MIXER_PARLFIX4:
      if (len == 4) {
        out[0] = -vx;
        out[1] = vx;
        out[2] = vx;
        out[3] = -vx;
      } else {
        goto error;
      }
    case MIXER_PARLFIX2:
      if (len == 2) {
        out[0] = -vx;
        out[1] = vx;
      } else {
        goto error;
      }
    case MIXER_OMNICROSS:
    case MIXER_OMNIPLUS:
      goto error;
  }

  float max = 0.f;
  for (int8_t i = 0; i < len; i++) {
    if (fabsf(out[i]) > max) max = out[i];
  }
  if (fabsf(max) > 1.f) {
    for (int8_t i = 0; i < len; i++) {
      out[i] /= fabsf(max);
    }
  }
  for (int8_t i = 0; i < len; i++) {
    out[i] *= scale;
  }
  return 0;

error:
  for (uint8_t i = 0; i < len; i++) out[i] = 0;
  return -1;
}
