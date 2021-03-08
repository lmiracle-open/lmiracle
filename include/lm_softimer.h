/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_softimer.h
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 软件定时器模块
*******************************************************************************/

#ifndef __LM_SOFTIMER_H
#define __LM_SOFTIMER_H

#include "timers.h"
#include "lmiracle.h"

LM_BEGIN_EXTERN_C

#define SOFTIMER_RELOAD_ENABLE          pdTRUE      /* 软定时器重复加载使能 */
#define SOFTIMER_RELOAD_DISABLE         pdFAIL      /* 软定时器重复加载关闭 */

/* 定时器数据类型 */
typedef TimerHandle_t lm_timer_t;

/* 软定时器回调函数类型 */
typedef void (*softimer_handle_t)(lm_timer_t timer);

/**
 * @brief 创建定时器
 *
 * @param[in] handle    定时器回调函数
 * @param[in] reload    重复加载标记
 * @param[in] ms        定时ms数
 *
 * @return  定时器句柄
 */
static inline
lm_timer_t lm_softimer_create(softimer_handle_t handle, uint8_t reload, uint16_t ms)
{
    TimerHandle_t timer = NULL;

    if (0 != reload) {
        reload = pdTRUE;
    }

    timer = xTimerCreate(NULL, pdMS_TO_TICKS(ms), reload, NULL, handle);

    if (NULL == timer) {
        return NULL;
    }

    return timer;
}

/**
 * @brief 启动定时器
 *
 * @param[in] timer 定时器指针
 * @param[in] isr   中断调用使能   (1,中断, 0,非中断)
 *
 * @return  错误码
 */
static inline
int lm_softimer_start(lm_timer_t timer, bool isr)
{
    if (LM_TRUE != isr) {
        if (pdPASS != xTimerStartFromISR(timer, pdFALSE)) {
            return LM_ERROR;
        }
    } else {
        if (pdPASS != xTimerStart(timer, 0)) {
            return LM_ERROR;
        }
    }

    return LM_OK;
}

/**
 * @brief 停止定时器
 *
 * @param[in] timer 定时器指针
 * @param[in] isr   中断调用使能   (1,中断, 0,非中断)
 *
 * @return  错误码
 */
static inline
int lm_softimer_stop(lm_timer_t timer, uint8_t isr)
{
    if (LM_TRUE != isr) {
        if (pdTRUE != xTimerStopFromISR(timer, pdFALSE)) {
            return LM_ERROR;
        }
    } else {
        if (pdTRUE != xTimerStop(timer, 0)) {
            return LM_ERROR;
        }
    }

    return LM_OK;
}

/**
 * @brief 复位定时器
 *
 * @param[in] timer 定时器指针
 * @param[in] isr   中断调用使能   (1,中断, 0,非中断)
 *
 * @return  错误码
 */
static inline
int lm_softimer_reset(lm_timer_t timer, uint8_t isr)
{
    if (LM_TRUE != isr) {
        if (pdTRUE != xTimerResetFromISR(timer, pdFALSE)) {
            return LM_ERROR;
        }
    } else {
        if (pdTRUE != xTimerReset(timer, 0)) {
            return LM_ERROR;
        }
    }

    return LM_OK;
}

/**
 * @brief 删除定时器
 *
 * @param[in] timer  定时器指针
 *
 * @return  None
 */
static inline
void lm_sorftimer_delete(lm_timer_t timer)
{
    xTimerDelete(timer, 0);
}

LM_END_EXTERN_C

#endif /* __LM_SOFTIMER_H */

/* end of file */
