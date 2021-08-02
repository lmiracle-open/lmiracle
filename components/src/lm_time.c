/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_time.c
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 系统时间模块
*******************************************************************************/

#include "lmiracle.h"
#include "lm_time.h"

static sys_time_set_t sys_time_set_cb = NULL;       /* 设置系统时间回调 */
static sys_time_get_t sys_time_get_cb = NULL;       /* 获取系统时间回调 */

/**
 * @brief 时间初始化
 */
int lm_time_init (void)
{
    int ret = LM_OK;

    return ret;
}

/**
 * @brief 时间回调函数通知
 */
int lm_time_cb_notice (sys_time_set_t p_set, sys_time_get_t p_get)
{
    sys_time_set_cb = p_set;
    sys_time_get_cb = p_get;

    return LM_OK;
}

/**
 * @brief 设置系统时间
 */
int lm_time_set (lm_tm_t *p_tm)
{
    int ret = LM_OK;

    /* 1. 输入参数检查 */
    lm_assert(NULL != p_tm);

    if (sys_time_set_cb) {
        ret = sys_time_set_cb(p_tm);
    }

    return ret;
}

/**
 * @brief 获取系统时间
 */
int lm_time_get (lm_tm_t *p_tm)
{
    int ret = LM_OK;

    /* 1. 输入参数检查 */
    lm_assert(NULL != p_tm);

    if (sys_time_get_cb) {
        ret = sys_time_get_cb(p_tm);
    }

    return ret;
}

/**
 * @brief 检查时间到期
 */
bool lm_time_expire_check (uint32_t *oldtick, uint32_t ms, bool update)
{
    uint32_t tmpoint = 0;

    /* 1. 参数有效性检查 */
    if (NULL == oldtick) {
        return false;
    }

    /* 2.计算检测时间点 */
    tmpoint = *oldtick + lm_ms_to_tick(ms);

    /* 3. 检查到期 */
    if (time_after(int32_t, lm_sys_get_tick(), tmpoint)) {
        if (true == update) {
            *oldtick = tmpoint;
        }
        return true;
    }

    return false;
}

/* end of file */
