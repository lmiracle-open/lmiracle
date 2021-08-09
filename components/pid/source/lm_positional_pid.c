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

static struct lm_pid __g_pid_info = {0};            /* 定义pid结构体 */

/**
 * @brief 1. pid模块初始化
 */
int lm_pid_module_init (struct lm_pid *pid_info)
{
    /* 1. 检查输入参数是否有效 */
    lm_assert(NULL != pid_info);

    /* 2. pid参数初始化 */
    memcpy(&__g_pid_info, pid_info, sizeof(lm_pid_t));

    /* 3. 设置pid输出阈值 */
    lm_pid_set_output_limits(__g_pid_info.output_min, __g_pid_info.output_max);

    /* 4. 设置输出控制方向 */
    lm_pid_set_controller_direction(__g_pid_info.controller_dir);

    /* 5. 设置pid参数调节 */
    lm_pid_set_tunings(__g_pid_info.kp, __g_pid_info.ki, __g_pid_info.kd, __g_pid_info.p_on);

    /* 6. 设置pid计算起始时间 */
    __g_pid_info.last_time = lm_sys_get_tick() - __g_pid_info.sample_time;

    return LM_OK;
}

/**
 * @brief 2. 位置式pid计算函数
 */
int lm_positional_pid_compute (double in_value, double set_point, double *out_value)
{
    struct lm_pid *__pid = &__g_pid_info;

    /* 1. 判断当前pid运行模式是否为自动运行状态 */
    if ((__pid == NULL) || !__pid->auto_flag) {
        return false;   /* 手动模式就是不使用pid,退出pid计算流程 */
    }

    /* 2. 检查采样时间是否到期 */
    if (!lm_time_expire_check(&__pid->last_time, __pid->sample_time, true)) {
        return false;   /* pid计算时间未到,退出pid计算流程 */
    }

    /* 3. 获取输入有效值 */
    __pid->input_value = in_value;

    /* 4. 获取设定值 */
    __pid->set_point   = set_point;

    /* 5. 计算当前偏差 */
    double error = __pid->set_point - __pid->input_value;

    /* 6. 计算最近两次偏差 */
    double d_input = __pid->input_value - __pid->last_input;

    /* 7. 计算偏差累计和(积分项使用) */
    __pid->output_sum += (__pid->ki * error);

    /* 8. 判断是否为测量的比例模式(此模式可以有效抑制超调) */
    if (!__pid->p_on_e) {
        __pid->output_sum -= __pid->kp * d_input;
    }

    /* todo: 误差累计和原始数据输出(用户查看) by terryall add */
    __pid->pid_sum_origin = __pid->output_sum;

    /* 9. 偏差累计和边界处理 */
    if (__pid->output_sum > __pid->output_max) {
        __pid->output_sum= __pid->output_max;
    } else if (__pid->output_sum < __pid->output_min) {
        __pid->output_sum= __pid->output_min;
    }

    /* 10. 计算比例项: todo 在测量的比例模式下比例项起到阻碍作用，此处不参与运算输出为0, 只有在传统模式下 才起正向作用 */
    double output;
    if (__pid->p_on_e) {
        output = __pid->kp * error;
    } else {
        output = 0;
    }

    /* 11. 计算最终的pid输出结果 */
    output += __pid->output_sum - __pid->kd * d_input;  /* 比例+积分-微分 */

    /* todo: pid最终输出的原始值(用户查看) by terryall add */
    __pid->pid_out_origin = output;

    /* 12. pid输出边界处理 */
    if (output > __pid->output_max) {
        output = __pid->output_max;
    } else if (output < __pid->output_min) {
        output = __pid->output_min;
    }

    /* 13. 更新pid输出有效值 */
    __pid->output_value = output;
    *out_value = __pid->output_value;

    /* 14. 更新上一次输入有效值 */
    __pid->last_input = __pid->input_value;

    return true;
}

/**
 * @brief 3. pid参数调节设置
 */
void lm_pid_set_tunings (double kp, double ki, double kd, int p_on)
{
    /* 1. 参数范围检测 */
    if (kp < 0 || ki < 0 || kd < 0) return;

    /* 2. 更新pid控制模式 */
    __g_pid_info.p_on = p_on;

    /* 3. 更新测量的比例模式标志 */
    __g_pid_info.p_on_e = (p_on == P_ON_E) ? true : false;

    /* 4. 更新pid显示参数(用户使用的参数) */
    __g_pid_info.disp_kp = kp;
    __g_pid_info.disp_ki = ki;
    __g_pid_info.disp_kd = kd;

    /* 5. 临时变量，用于计算采样时间，此处将ms转换为s */
    double sample_time_in_sec = ((double)__g_pid_info.sample_time)/1000;

    /* 6. 更新pid本地参数(算法使用的参数) */
    __g_pid_info.kp = kp;
    __g_pid_info.ki = ki * sample_time_in_sec;
    __g_pid_info.kd = kd / sample_time_in_sec;

    /* 7. 通过判断pid输出的方向，计算出最终的pid参数 */
    if (__g_pid_info.controller_dir == REVERSE) {
        __g_pid_info.kp = (0 - __g_pid_info.kp);
        __g_pid_info.ki = (0 - __g_pid_info.ki);
        __g_pid_info.kd = (0 - __g_pid_info.kd);
    }
}

/**
 * @brief 4. pid采样周期设置
 */
void lm_pid_set_sample_time (int new_sample_time)
{
    /* 1. 根据新的采样周期重新设定pid调节参数，以及更新旧的采样周期 */
    if (new_sample_time > 0) {
        double ratio  = (double)new_sample_time/(double)__g_pid_info.sample_time;
        __g_pid_info.ki *= ratio;
        __g_pid_info.kd /= ratio;
        __g_pid_info.sample_time = (unsigned long)new_sample_time;
    }
}

/**
 * @brief 5. pid输出阈值设置
 */
void lm_pid_set_output_limits (double min, double max)
{
    /* 1. 检查输入范围是否满足要求 */
    if (min >= max) return;

    /* 2. 更新本地参数 */
    __g_pid_info.output_min = min;
    __g_pid_info.output_max = max;

    /* 3. pid输出范围限定 */
    if (__g_pid_info.auto_flag) {
        if (__g_pid_info.output_value > __g_pid_info.output_max) {
            __g_pid_info.output_value = __g_pid_info.output_max;
        } else if (__g_pid_info.output_value < __g_pid_info.output_min) {
            __g_pid_info.output_value = __g_pid_info.output_min;
        }
        if (__g_pid_info.output_sum > __g_pid_info.output_max) {
            __g_pid_info.output_sum = __g_pid_info.output_max;
        } else if (__g_pid_info.output_sum < __g_pid_info.output_min) {
            __g_pid_info.output_sum = __g_pid_info.output_min;
        }
    }
}

/**
 * @brief pid初始化，todo: 此函数只有在切换运行模式的时候调用
 */
static void __pid_initialize (void)
{
    /* todo: 从手动切换到自动需要重新初始化以下两个参数 */
    __g_pid_info.output_sum = __g_pid_info.output_value;
    __g_pid_info.last_input = __g_pid_info.input_value;

    /* 误差累计和范围限定 */
    if (__g_pid_info.output_sum > __g_pid_info.output_max) {
        __g_pid_info.output_sum = __g_pid_info.output_max;
    }
    if (__g_pid_info.output_sum < __g_pid_info.output_min) {
        __g_pid_info.output_sum = __g_pid_info.output_min;
    }
}

/**
 * @brief 6. pid运行模式设置
 */
void lm_pid_set_mode (int mode)
{
    /* 1. 获取pid运行模式: true->自动运行  false->手动运行(关闭pid) */
    bool new_auto = (mode == AUTOMATIC) ? true : false;

    /* 2. 切换运行模式 */
    if (new_auto && !__g_pid_info.auto_flag) {
        __pid_initialize();
    }

    /* 3. 更新当前运行模式 */
    __g_pid_info.auto_flag = new_auto;
}

/**
 * @brief 7. pid输出方向设置
 */
void lm_pid_set_controller_direction (int dir)
{
    /* 1. 根据方向重新设定pid调节参数(反方向为负数) */
    if (__g_pid_info.auto_flag && dir != __g_pid_info.controller_dir) {
        __g_pid_info.kp = (0 - __g_pid_info.kp);
        __g_pid_info.ki = (0 - __g_pid_info.ki);
        __g_pid_info.kd = (0 - __g_pid_info.kd);
    }

    /* 2. 更新本地参数(输出方向) */
    __g_pid_info.controller_dir = dir;
}

/******************************************************************************/
/******************************* 获取pid相关参数 *********************************/
/******************************************************************************/

/**
 * @brief 8. 获取 P参数
 */
double lm_pid_get_kp (void)
{
    return __g_pid_info.disp_kp;
}

/**
 * @brief 9. 获取 I参数
 */
double lm_pid_get_ki (void)
{
    return __g_pid_info.disp_ki;
}

/**
 * @brief 10. 获取 D参数
 */
double lm_pid_get_kd (void)
{
    return __g_pid_info.disp_kd;
}

/**
 * @brief 11. 获取 运行模式
 */
int lm_pid_get_mode (void)
{
    return (__g_pid_info.auto_flag ? AUTOMATIC : MANUAL);
}

/**
 * @brief 12. 获取 输出方向
 */
int lm_pid_get_direction (void)
{
    return __g_pid_info.controller_dir;
}

/**
 * @brief 13. 获取 误差累计和原始值
 */
double lm_pid_get_origin_error_sum (void)
{
    return __g_pid_info.pid_sum_origin;
}

/**
 * @brief 14. 获取 pid输出原始值
 */
double lm_pid_get_origin_output (void)
{
    return __g_pid_info.pid_out_origin;
}

/* end of file */
