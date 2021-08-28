/*
 * 发射器模组
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ----------------------------------------------------------------- */
#include <cmsis_os2.h>

#include "component/cmd.h"
#include "component/filter.h"
#include "component/pid.h"
#include "device/can.h"
#include "device/referee.h"
/* Exported constants ------------------------------------------------------- */
#define SHOOT_OK (0)        /* 运行正常 */
#define SHOOT_ERR (-1)      /* 运行时发现了其他错误 */
#define SHOOT_ERR_NULL (-2) /* 运行时发现NULL指针 */
#define SHOOT_ERR_MODE (-3) /* 运行时配置了错误的CMD_ShootMode_t */

/* Exported macro ----------------------------------------------------------- */
/* Exported types ----------------------------------------------------------- */

/* 用enum组合所有PID，方便访问，配合数组使用 */
enum Shoot_Acuator_e {
  SHOOT_ACTR_FRIC1_IDX = 0, /* 1号摩擦轮相关的索引值 */
  SHOOT_ACTR_FRIC2_IDX,     /* 2号摩擦轮相关的索引值 */
  SHOOT_ACTR_TRIG_IDX,      /* 扳机电机相关的索引值 */
  SHOOT_ACTR_NUM,           /* 总共的动作器数量 */
};

/* 发射机构型号 */
typedef enum {
  SHOOT_MODEL_17MM = 0, /* 17mm发射机构 */
  SHOOT_MODEL_42MM,     /* 42mm发射机构 */
} Shoot_Model_t;

/* 发射器参数的结构体，包含所有初始化用的参数，通常是const，存好几组。*/
typedef struct {
  KPID_Params_t fric_pid_param; /* 摩擦轮电机控制PID的参数 */
  KPID_Params_t trig_pid_param; /* 扳机电机控制PID的参数 */
  /* 低通滤波器截止频率 */
  struct {
    /* 输入 */
    struct {
      float fric; /* 摩擦轮电机 */
      float trig; /* 扳机电机 */
    } in;

    /* 输出 */
    struct {
      float fric; /* 摩擦轮电机 */
      float trig; /* 扳机电机 */
    } out;
  } low_pass_cutoff_freq;

  float num_trig_tooth;   /* 拨弹盘中一圈能存储几颗弹丸 */
  float trig_gear_ratio;  /* 拨弹电机减速比 3508:19, 2006:36 */
  float fric_radius;      /* 摩擦轮半径，单位：米 */
  float cover_open_duty;  /* 弹舱盖打开时舵机PWM占空比 */
  float cover_close_duty; /* 弹舱盖关闭时舵机PWM占空比 */
  Shoot_Model_t model;    /* 发射机构型号 */
  float bullet_speed;     /* 弹丸初速度 */
  uint32_t min_shoot_delay; /* 通过设置最小发射间隔来设置最大射频 */
} Shoot_Params_t;

typedef struct {
  float heat;          /* 现在热量水平 */
  float last_heat;     /* 之前的热量水平 */
  float heat_limit;    /* 热量上限 */
  float speed_limit;   /* 弹丸初速是上限 */
  float cooling_rate;  /* 冷却速率 */
  float heat_increase; /* 每发热量增加值 */

  float last_bullet_speed; /* 之前的弹丸速度 */
  uint32_t available_shot; /* 热量范围内还可以发射的数量 */
} Shoot_HeatCtrl_t;

typedef struct {
  uint32_t last_shoot; /* 上次发射器时间 单位：ms */
  bool last_fire;      /* 上次开火状态 */
  bool first_fire;     /* 第一次收到开火指令 */
  uint32_t shooted;    /* 已经发射的弹丸 */
  uint32_t to_shoot;   /* 计划发射的弹丸 */
  float bullet_speed;  /* 弹丸初速度 */
  uint32_t period_ms;  /* 弹丸击发延迟 */
  CMD_FireMode_t fire_mode;
} Shoot_FireCtrl_t;

/*
 * 运行的主结构体，所有这个文件里的函数都在操作这个结构体。
 * 包含了初始化参数，中间变量，输出变量。
 */
typedef struct {
  uint32_t lask_wakeup;
  float dt;

  const Shoot_Params_t *param; /* 发射器的参数，用Shoot_Init设定 */

  /* 模块通用 */
  CMD_ShootMode_t mode; /* 发射器模式 */

  /* 反馈信息 */
  struct {
    float fric_rpm[2];      /* 摩擦轮电机转速，单位：RPM */
    float trig_motor_angle; /* 拨弹电机角度，单位：弧度 */
    float trig_angle;       /* 拨弹转盘角度，单位：弧度 */
  } feedback;

  /* PID计算的目标值 */
  struct {
    float fric_rpm[2]; /* 摩擦轮电机转速，单位：RPM */
    float trig_angle;  /* 拨弹电机角度，单位：弧度 */
  } setpoint;

  /* 反馈控制用的PID */
  struct {
    KPID_t fric[2]; /* 控制摩擦轮 */
    KPID_t trig;    /* 控制拨弹电机 */
  } pid;

  /* 过滤器 */
  struct {
    /* 反馈值滤波器 */
    struct {
      LowPassFilter2p_t fric[2]; /* 过滤摩擦轮 */
      LowPassFilter2p_t trig;    /* 过滤拨弹电机 */
    } in;

    /* 输出值滤波器 */
    struct {
      LowPassFilter2p_t fric[2]; /* 过滤摩擦轮 */
      LowPassFilter2p_t trig;    /* 过滤拨弹电机 */
    } out;
  } filter;

  Shoot_HeatCtrl_t heat_ctrl;
  Shoot_FireCtrl_t fire_ctrl;

  float out[SHOOT_ACTR_NUM]; /* 输出数组，通过Shoot_Acuator_e里的值访问 */

} Shoot_t;

/* Exported functions prototypes -------------------------------------------- */

/**
 * \brief 初始化发射器
 *
 * \param s 包含发射器数据的结构体
 * \param param 包含发射器参数的结构体指针
 * \param target_freq 任务预期的运行频率
 *
 * \return 函数运行结果
 */
int8_t Shoot_Init(Shoot_t *s, const Shoot_Params_t *param, float target_freq);

/**
 * \brief 更新发射器的反馈信息
 *
 * \param s 包含发射器数据的结构体
 * \param can CAN设备结构体
 *
 * \return 函数运行结果
 */
int8_t Shoot_UpdateFeedback(Shoot_t *s, const CAN_t *can);

/**
 * \brief 运行发射器控制逻辑
 *
 * \param s 包含发射器数据的结构体
 * \param s_cmd 发射器控制指令
 * \param s_ref 裁判系统数据
 * \param dt_sec 两次调用的时间间隔
 *
 * \return 函数运行结果
 */
int8_t Shoot_Control(Shoot_t *s, CMD_ShootCmd_t *s_cmd,
                     Referee_ForShoot_t *s_ref, uint32_t now);

/**
 * \brief 复制发射器输出值
 *
 * \param s 包含发射器数据的结构体
 * \param out CAN设备发射器输出结构体
 */
void Shoot_DumpOutput(Shoot_t *s, CAN_ShootOutput_t *out);

/**
 * \brief 清空输出值
 *
 * \param output 要清空的结构体
 */
void Shoot_ResetOutput(CAN_ShootOutput_t *output);

/**
 * @brief 导出发射器UI数据
 *
 * @param s 发射器结构体
 * @param ui UI结构体
 */
void Shoot_DumpUI(Shoot_t *s, Referee_ShootUI_t *ui);

#ifdef __cplusplus
}
#endif
