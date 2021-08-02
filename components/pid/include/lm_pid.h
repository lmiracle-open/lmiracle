/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_pid.h
* Change Logs   :
* Date          Author          Notes
* 2021-07-31    terryall        V1.0    reference Arduino PID Library - Version 1.2.1
*******************************************************************************/

/*******************************************************************************
* Description   : PID算法库
*******************************************************************************/

#ifndef __LM_PID_H
#define __LM_PID_H

#include "lmiracle.h"                       /* 包含蓝梦内部头文件 */

LM_BEGIN_EXTERN_C

/******************************************************************************/

/* PID参数结构体 */
typedef struct lm_pid {

    /* 1. PID算法用到的一些宏定义 */
    #define AUTOMATIC   1                   /* 开启PID计算 */
    #define MANUAL      0                   /* 关闭PID计算 */
    #define DIRECT      0                   /* PID控制方向为正 */
    #define REVERSE     1                   /* PID控制方向为负 */
    #define P_ON_M      0                   /* 抑制超调模式1 */
    #define P_ON_E      1                   /* 抑制超调模式2 */

    /*2. PID算法用到的一些参数 */
    double          set_point;              /* PID设定目标值 */
    double          input_value;            /* PID输入有效值 */
    double          output_value;           /* PID输出有效值 */
    double          output_min;             /* PID输出最小限值 */
    double          output_max;             /* PID输出最大限值 */
    unsigned long   sample_time;            /* 采样时间 */
    int             controller_dir;         /* 输出方向 */
    bool            overshoot_flag;         /* 抑制超调标志 */

    double          kp;                     /* (P)比例调节参数 */
    double          ki;                     /* (I)积分调节参数 */
    double          kd;                     /* (D)微分调节参数 */

    double          disp_kp;                /* (P)比例调节参数原始值-用户使用 */
    double          disp_ki;                /* (I)积分调节参数原始值-用户使用 */
    double          disp_kd;                /* (D)微分调节参数原始值-用户使用 */

    unsigned long   last_time;              /* 上一次时间 */
    bool            auto_flag;              /* 自动模式标志 */
    double          output_sum;             /* 累计误差和 */
    double          last_input;             /* 上一次输入的有效值 */
    int             p_on_mode;              /* 抑制超调模式 */

} lm_pid_t;

/******************************************************************************/
/******************************* pid对外提供的一些接口 *****************************/
/******************************************************************************/

/**
 * @brief 位置式pid计算函数
 *
 * @param[in]   in_value    被控量
 * @param[in]   set_point   目标值
 * @param[in]   *out_value  输出值
 *
 * @return  错误码
 */
extern
int lm_positional_pid_compute (double in_value, double set_point, double *out_value);

/**
 * @brief pid运行模式设置
 *
 * @param[in]   mode    运行模式
 *
 * @return  None
 */
extern void lm_pid_run_mode_set (int mode);

/**
 * @brief pid输出阈值设置
 *
 * @param[in]   min    下限
 * @param[in]   max    上限
 *
 * @return  None
 */
extern void lm_pid_output_limits_set (double min, double max);

/**
 * @brief pid参数调节设置
 *
 * @param[in]   kp    比例系数
 * @param[in]   ki    积分时间常数
 * @param[in]   kd    微分时间常数
 * @param[in]   pOn   抑制超调模式
 *
 * @return  None
 */
extern void lm_pid_param_tunings_set (double kp, double ki, double kd, int pOn);

/**
 * @brief pid输出方向设置
 *
 * @param[in]   dir    控制方向设置
 *
 * @return  None
 */
extern void lm_pid_controller_dir_set (int dir);

/**
 * @brief pid采样周期设置
 *
 * @param[in]   sample_time    采样时间
 *
 * @return  None
 */
extern void lm_pid_sample_time_set (int sample_time);

/**
 * @brief 获取pid参数
 *
 * @param[in]   None
 *
 * @return  pid参数结构体指针
 */
extern struct lm_pid *lm_pid_param_get (void);

/**
 * @brief pid模块初始化
 *
 * @param[in]   pid_info    pid结构体指针
 *
 * @return  错误码
 */
extern int lm_pid_module_init (struct lm_pid *pid_info);

LM_END_EXTERN_C

#endif /* __LM_PID_H */

/* end of file */
