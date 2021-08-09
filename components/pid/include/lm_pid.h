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
    #define P_ON_M      0                   /* 测量的比例模式 */
    #define P_ON_E      1                   /* 传统模式 */

    /*2. PID算法用到的一些参数 */
    double          set_point;              /* PID设定目标值 */
    double          input_value;            /* PID输入有效值 */
    double          output_value;           /* PID输出有效值 */
    double          output_min;             /* PID输出最小限值 */
    double          output_max;             /* PID输出最大限值 */
    unsigned long   sample_time;            /* 采样时间 */
    int             controller_dir;         /* 输出方向 */
    bool            p_on_e;                 /* 基于测量的比例标志：true->传统模式 false->测量的比例模式 */

    double          kp;                     /* (P)比例调节参数 */
    double          ki;                     /* (I)积分调节参数 */
    double          kd;                     /* (D)微分调节参数 */

    double          disp_kp;                /* (P)比例调节参数原始值-用户使用 */
    double          disp_ki;                /* (I)积分调节参数原始值-用户使用 */
    double          disp_kd;                /* (D)微分调节参数原始值-用户使用 */

    unsigned long   last_time;              /* 上一次时间 */
    bool            auto_flag;              /* 自动模式标志: AUTOMATIC->自动模式 MANUAL->手动模式  */
    double          output_sum;             /* 累计误差和 */
    double          last_input;             /* 上一次输入的有效值 */
    int             p_on;                   /* 模式选择: P_ON_E->传统模式 P_ON_M->测量的比例模式 */

    /* 3. 自己添加用户数据  by terryall */
    double          pid_out_origin;         /* pid输出原始值 */
    double          pid_sum_origin;         /* pid误差累计和原始值 */

} lm_pid_t;

/******************************************************************************/
/******************************* pid模块对外提供的一些接口 **************************/
/******************************************************************************/

/**
 * @brief 1. pid模块初始化
 *
 * @param[in]   pid_info    pid结构体指针
 *
 * @return  错误码
 */
extern int lm_pid_module_init (struct lm_pid *pid_info);

/**
 * @brief 2. 位置式pid计算函数
 *
 * @param[in]   in_value    被控量
 * @param[in]   set_point   目标值
 * @param[in]   *out_value  输出值
 *
 * @return  错误码
 */
extern int lm_positional_pid_compute (double in_value, double set_point, double *out_value);

/**
 * @brief 3. pid参数设置
 *
 * @param[in]   kp    比例系数
 * @param[in]   ki    积分时间常数
 * @param[in]   kd    微分时间常数
 * @param[in]   p_on  模式选择，P_ON_M->测量的比例模式  P_ON_E->传统模式
 *
 * @return  None
 */
extern void lm_pid_set_tunings (double kp, double ki, double kd, int p_on);

/**
 * @brief 4. pid采样周期设置
 *
 * @param[in]   new_sample_time    采样时间
 *
 * @return  None
 */
extern void lm_pid_set_sample_time (int new_sample_time);

/**
 * @brief 5. pid输出阈值设置
 *
 * @param[in]   min    下限
 * @param[in]   max    上限
 *
 * @return  None
 */
extern void lm_pid_set_output_limits (double min, double max);

/**
 * @brief 6. pid运行模式设置
 *
 * @param[in]   mode    运行模式，AUTOMATIC->自动模式 MANUAL->手动模式
 *
 * @return  None
 */
extern void lm_pid_set_mode (int mode);

/**
 * @brief 7. pid输出方向设置
 *
 * @param[in]   dir    控制方向设置，DIRECT->正向  REVERSE->反向
 *
 * @return  None
 */
extern void lm_pid_set_controller_direction (int dir);

/**
 * @brief 8. 获取 P参数
 *
 * @param[in]   None
 *
 * @return  kp值
 */
extern double lm_pid_get_kp (void);

/**
 * @brief 9. 获取 I参数
 *
 * @param[in]   None
 *
 * @return  ki值
 */
extern double lm_pid_get_ki (void);

/**
 * @brief 10. 获取 D参数
 *
 * @param[in]   None
 *
 * @return  kd值
 */
extern double lm_pid_get_kd (void);

/**
 * @brief 11. 获取 运行模式
 *
 * @param[in]   None
 *
 * @return  AUTOMATIC->自动模式 MANUAL->手动模式
 */
extern int lm_pid_get_mode (void);

/**
 * @brief 12. 获取 输出方向
 *
 * @param[in]   None
 *
 * @return  DIRECT->正向  REVERSE->反向
 */
extern int lm_pid_get_direction (void);

/**
 * @brief 13. 获取 误差累计和原始值
 *
 * @param[in]   None
 *
 * @return  原始累计误差
 */
extern double lm_pid_get_origin_error_sum (void);

/**
 * @brief 14. 获取 pid输出原始值
 *
 * @param[in]   None
 *
 * @return  原始输出值
 */
extern double lm_pid_get_origin_output (void);

LM_END_EXTERN_C

#endif /* __LM_PID_H */

/* end of file */
