/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_inc_pid.c
* Change Logs   :
* Date         Author      Notes
* 2020-8-1      wyf     V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : PID文件
*******************************************************************************/
#include "lm_inc_pid.h"
#include "lm_assert.h"

static lm_inc_pid_t *__pid_info = NULL;

/**
 * @brief:   增量式PID  参数初始化
 * @param:   *sptr    PID参数
 * @return:  None
 */
void incpid_init_register (lm_inc_pid_t *p_pid)
{
    /* 0.参数检查 */
    lm_assert(NULL == p_pid);

    __pid_info = p_pid;
}

/**
 * @brief:   增量式 PID控制算法
 * @param:   set_point   期望温度
 * @param:   next_point  实际温度
 * @return:  PID增量值
 */
float lm_inc_pid_ctrl (float set_point, float next_point)
{
    static float iError, iIncpid;

    /* 1. 当前误差  */
    iError = set_point - next_point;

    /* 2. 增量计算  */
    iIncpid = __pid_info->proportion * iError                        /* E[k]项 */
              - __pid_info->integral * __pid_info->last_error        /* E[k－1]项 */
              + __pid_info->derivative * __pid_info->prev_error;     /* E[k－2]项 */

    /* 3. 存储误差，用于下次计算 */
    __pid_info->prev_error = __pid_info->last_error;
    __pid_info->last_error = iError;

    /* 4. 返回增量值 */
    return iIncpid;
}


/* end of file */

