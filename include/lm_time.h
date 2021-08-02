/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_time.h
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 系统时间模块
*******************************************************************************/

#ifndef __LM_TIME_H
#define __LM_TIME_H

#include "lm_types.h"

LM_BEGIN_EXTERN_C

/*******************************************************************************
* MacroName      : time_after()
* Description    : 时间a在时间b之后
* EntryParameter : type,时间值的有符号类型, a,b,时间值
* ReturnValue    : true,成立, false,不成立
*******************************************************************************/
#define time_after(type,a,b)        ((type)(b) - (type)(a) <= 0)

/**
 * @brief 时间结构体
 */
typedef struct lm_tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
} lm_tm_t;

/**
 * @brief 设置系统时间回调函数
 *
 * @param[in] p_tm  时间配置
 *
 * @return  错误码
 */
typedef int (*sys_time_set_t) (lm_tm_t *p_time);

/**
 * @brief 获取系统时间回调函数
 *
 * @param[in] p_tm  时间配置
 *
 * @return  错误码
 */
typedef int (*sys_time_get_t) (lm_tm_t *p_time);

/**
 * @brief 设置系统时间
 *
 * @param[in] p_tm  时间配置
 *
 * @return  错误码
 */
extern int lm_time_set (lm_tm_t *p_tm);

/**
 * @brief 获取系统时间
 *
 * @param[in] p_tm  时间配置
 *
 * @return  错误码
 */
extern int lm_time_get (lm_tm_t *p_tm);

/**
 * @brief 时间回调函数通知
 *
 * @param[in] p_set 设置时间函数
 * @param[in] p_get 获取时间函数
 *
 * @return  错误码
 */
extern int lm_time_cb_notice (sys_time_set_t p_set, sys_time_get_t p_get);

/**
 * @brief 检查时间到期
 *
 * @param[out]  oldtick,上次时间tick(如果超时,会被更新到当前tick值)
 * @param[in]   ms,间隔毫秒
 * @param[in]   update,是否更新时间
 *
 * @return      true,到期, false,未到期
 */
extern bool lm_time_expire_check (uint32_t *oldtick, uint32_t ms, bool update);

/**
 * @brief 时间初始化
 *
 * @param[in] None
 *
 * @return  错误码
 */
extern int lm_time_init (void);

LM_END_EXTERN_C

#endif /* __LM_TIME_H */

/* end of file */

