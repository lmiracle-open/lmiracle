/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_positional_pid.c
* Change Logs   :
* Date          Author          Notes
* 2021-07-31    terryall        V1.0    reference Arduino PID Library - Version 1.2.1
*******************************************************************************/

/*******************************************************************************
* Description   : 位置式PID算法库
*******************************************************************************/

#include "lm_pid.h"                                 /* 包含pid头文件 */
#include "lm_time.h"

static struct lm_pid *__gp_pid_info = NULL;      /* 定义PID信息结构体指针 */

/**
 * @brief 位置式pid计算函数
 */
int lm_positional_pid_compute (double in_value, double set_point, double *out_value)
{
    struct lm_pid *__pid = __gp_pid_info;

    /* 1. 判断当前pid运行模式是否为自动运行状态 */
    if ((__pid == NULL) || !__pid->auto_flag) {
        return false;   /* 手动模式就是不使用pid,退出pid计算流程 */
    }

    /* 2. 检查采样时间是否到期 */
    if (!lm_time_expire_check(&__pid->last_time, __pid->sample_time, true)) {
        return false;   /* pid计算时间未到,退出pid计算流程 */
    }

    __pid->input_value = in_value;
    __pid->set_point   = set_point;

    /* 3. 计算当前偏差 */
    double error = __pid->set_point - __pid->input_value;

    /* 4. 计算最近两次偏差 */
    double d_input = (__pid->input_value - __pid->last_input);

    /* 5. 计算偏差累计和(积分项使用) */
    __pid->output_sum += (__pid->ki * error);

    /* 6. 根据抑制超调标志重新计算偏差累计和  todo: if 抑制超调标志为假表示需要抑制超调 */
    if (!__pid->overshoot_flag) {
        __pid->output_sum -= __pid->kp * d_input;
    }

    /* 7. 偏差累计和边界处理 */
    if (__pid->output_sum > __pid->output_max) {
        __pid->output_sum= __pid->output_max;
    } else if (__pid->output_sum < __pid->output_min) {
        __pid->output_sum= __pid->output_min;
    }

    /* 8. 计算比例项: if 抑制超调标志为真表示不需要抑制超调,正常输出，反之比例项清零 */
    double output;
    if (__pid->overshoot_flag) {
        output = __pid->kp * error;
    } else {
        output = 0;
    }

    /* 9. 计算最终的pid输出结果 */
    output += __pid->output_sum - __pid->kd * d_input;  /* 比例+积分-微分 */

    /* 10. pid输出边界处理 */
    if (output > __pid->output_max) {
        output = __pid->output_max;
    } else if (output < __pid->output_min) {
        output = __pid->output_min;
    }
    __pid->output_value = output;

    /* 11. 更新pid输出值 */
    *out_value = __pid->output_value;

    /* 12. 更新上一次输入有效值 */
    __pid->last_input = __pid->input_value;

    return true;
}

/**
 * @brief pid运行模式设置
 */
void lm_pid_run_mode_set (int mode)
{
    /* 1. 检查pid模块是否已注册 */
    lm_assert(NULL != __gp_pid_info);

    /* 2. 获取pid运行模式: true->自动运行  false->手动运行(关闭pid) */
    bool new_auto = (mode == AUTOMATIC) ? true : false;

    /* 3. 切换运行模式 */
    if (new_auto && !__gp_pid_info->auto_flag) {
        /* todo: 从手动切换到自动需要重新初始化以下两个参数 */
        __gp_pid_info->output_sum = __gp_pid_info->output_value;
        __gp_pid_info->last_input = __gp_pid_info->input_value;
        /* 误差累计和范围限定 */
        if (__gp_pid_info->output_sum > __gp_pid_info->output_max) {
            __gp_pid_info->output_sum = __gp_pid_info->output_max;
        }
        if (__gp_pid_info->output_sum < __gp_pid_info->output_min) {
            __gp_pid_info->output_sum = __gp_pid_info->output_min;
        }
    }

    /* 4. 更新当前运行模式 */
    __gp_pid_info->auto_flag = new_auto;
}

/**
 * @brief pid输出阈值设置
 */
void lm_pid_output_limits_set (double min, double max)
{
    /* 1. 检查pid模块是否已注册 */
    lm_assert(NULL != __gp_pid_info);

    /* 2. 检查输入范围是否满足要求 */
    if (min >= max) return;

    /* 3. 更新本地参数 */
    __gp_pid_info->output_min = min;
    __gp_pid_info->output_max = max;

    /* 4. pid输出范围限定 */
    if (__gp_pid_info->auto_flag) {
        if (__gp_pid_info->output_value > __gp_pid_info->output_max) {
            __gp_pid_info->output_value = __gp_pid_info->output_max;
        } else if (__gp_pid_info->output_value < __gp_pid_info->output_min) {
            __gp_pid_info->output_value = __gp_pid_info->output_min;
        }
        if (__gp_pid_info->output_sum > __gp_pid_info->output_max) {
            __gp_pid_info->output_sum = __gp_pid_info->output_max;
        } else if (__gp_pid_info->output_sum < __gp_pid_info->output_min) {
            __gp_pid_info->output_sum = __gp_pid_info->output_min;
        }
    }
}

/**
 * @brief pid参数调节设置
 */
void lm_pid_param_tunings_set (double kp, double ki, double kd, int pOn)
{
    /* 1. 检查pid模块是否已注册 */
    lm_assert(NULL != __gp_pid_info);

    /* 2. 参数范围检测 */
    if (kp < 0 || ki < 0 || kd < 0) return;

    /* 3. 更新抑制超调模式 */
    __gp_pid_info->p_on_mode = pOn;

    /* 4. 更新抑制超调标志 */
    __gp_pid_info->overshoot_flag = (pOn == P_ON_E) ? true : false;

    /* 5. 更新pid显示参数(用户使用的参数) */
    __gp_pid_info->disp_kp = kp;
    __gp_pid_info->disp_ki = ki;
    __gp_pid_info->disp_kd = kd;

    /* 6. 临时变量，用于计算采样时间，此处将ms转换为s */
    double sample_time_in_sec = ((double)__gp_pid_info->sample_time)/1000;

    /* 5. 更新pid本地参数(算法使用的参数) */
    __gp_pid_info->kp = kp;
    __gp_pid_info->ki = ki * sample_time_in_sec;
    __gp_pid_info->kd = kd / sample_time_in_sec;

    /* 7. 通过判断挡墙pid输出的方向，计算出最终的pid参数 */
    if (__gp_pid_info->controller_dir == REVERSE) {
        __gp_pid_info->kp = (0 - __gp_pid_info->kp);
        __gp_pid_info->ki = (0 - __gp_pid_info->ki);
        __gp_pid_info->kd = (0 - __gp_pid_info->kd);
    }
}

/**
 * @brief pid输出方向设置
 */
void lm_pid_controller_dir_set (int dir)
{
    /* 1. 检查pid模块是否已注册 */
    lm_assert(NULL != __gp_pid_info);

    /* 2. 根据方向重新设定pid调节参数(反方向为负数) */
    if (__gp_pid_info->auto_flag && dir != __gp_pid_info->controller_dir) {
        __gp_pid_info->kp = (0 - __gp_pid_info->kp);
        __gp_pid_info->ki = (0 - __gp_pid_info->ki);
        __gp_pid_info->kd = (0 - __gp_pid_info->kd);
    }

    /* 3. 更新本地参数(输出方向) */
    __gp_pid_info->controller_dir = dir;
}

/**
 * @brief pid采样周期设置
 */
void lm_pid_sample_time_set (int sample_time)
{
    /* 1. 检查pid模块是否已注册 */
    lm_assert(NULL != __gp_pid_info);

    /* 2. 根据新的采样周期重新设定pid调节参数，以及更新旧的采样周期 */
    if (sample_time > 0) {
        double ratio  = (double)sample_time/(double)__gp_pid_info->sample_time;
        __gp_pid_info->ki *= ratio;
        __gp_pid_info->kd /= ratio;
        __gp_pid_info->sample_time = (unsigned long)sample_time;
    }
}

/**
 * @brief 获取pid参数
 */
struct lm_pid *lm_pid_param_get (void)
{
    return __gp_pid_info;
}

/**
 * @brief pid模块初始化
 */
int lm_pid_module_init (struct lm_pid *pid_info)
{
    /* 1. 检查输入参数是否有效 */
    lm_assert(NULL != pid_info);

    /* 2. 参数初始化 */
    __gp_pid_info = pid_info;

    /* 3. 设置pid输出阈值 */
    lm_pid_output_limits_set(pid_info->output_min, pid_info->output_max);

    /* 4. 设置输出控制方向 */
    lm_pid_controller_dir_set(pid_info->controller_dir);

    /* 5. 设置pid参数调节 */
    lm_pid_param_tunings_set(pid_info->kp, pid_info->ki, pid_info->kd, pid_info->p_on_mode);

    /* 6. 设置pid运行模式 */
    lm_pid_run_mode_set(AUTOMATIC);

    /* 6. 设置pid计算起始时间 */
    __gp_pid_info->last_time = lm_sys_get_tick() - __gp_pid_info->sample_time;

    return LM_OK;
}

/* end of file */
