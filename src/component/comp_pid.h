/*
  Modified from
  https://github.com/PX4/Firmware/blob/master/src/lib/pid/pid.h
*/

#pragma once

#include "comp_filter.h"
#include "comp_utils.h"

/* PID模式 */
typedef enum {
  KPID_MODE_NO_D = 0, /* 不使用微分项，PI控制器 */
  KPID_MODE_CALC_D, /* 根据反馈的值计算离散微分，忽略PID_Calc中的fb_dot */
  KPID_MODE_SET_D /* 直接提供微分值，PID_Calc中的fb_dot将被使用，(Gyros) */
} kpid_mode_t;

/* PID参数 */
typedef struct {
  float k;             /* 控制器增益，设置为1用于并行模式 */
  float p;             /* 比例项增益，设置为1用于标准形式 */
  float i;             /* 积分项增益 */
  float d;             /* 微分项增益 */
  float i_limit;       /* 积分项上限 */
  float out_limit;     /* 输出绝对值限制 */
  float d_cutoff_freq; /* D项低通截止频率 */
  float range;         /* 计算循环误差时使用，大于0时启用 */
} kpid_params_t;

/* PID主结构体 */
typedef struct {
  kpid_mode_t mode;
  const kpid_params_t *param;

  float dt_min; /* 最小PID_Calc调用间隔 */
  float i;      /* 积分 */

  struct {
    float err;  /* 上次误差 */
    float k_fb; /* 上次反馈值 */
    float out;  /* 上次输出 */
  } last;

  low_pass_filter_2p_t dfilter; /* D项低通滤波器 */
} kpid_t;

/**
 * @brief 初始化PID
 *
 * @param pid PID结构体
 * @param mode PID模式
 * @param sample_freq 采样频率
 * @param param PID参数
 */
void kpid_init(kpid_t *pid, kpid_mode_t mode, float sample_freq,
               const kpid_params_t *param);

/**
 * @brief PID计算
 *
 * @param pid PID结构体
 * @param sp 设定值
 * @param fb 反馈值
 * @param fb_dot 反馈值微分
 * @param dt 间隔时间
 * @return float 计算的输出
 */
float kpid_calc(kpid_t *pid, float sp, float fb, float fb_dot, float dt);

/**
 * @brief 重置微分项
 *
 * @param pid PID结构体
 */
void kpid_reset_i(kpid_t *pid);

/**
 * @brief 重置PID
 *
 * @param pid PID结构体
 */
void kpid_reset(kpid_t *pid);
