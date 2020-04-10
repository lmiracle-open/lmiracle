/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_event.h
* Change Logs   :
* Date         Author      Notes
* 2019-06-07   terryall     V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 队列
*******************************************************************************/

#ifndef __LM_EVENT_H
#define __LM_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "event_groups.h"
#include "lmiracle.h"

typedef EventGroupHandle_t lm_event_t;      /* 事件数据类型 */
typedef EventBits_t lm_bits_t;              /* 事件位数据类型 */

/**
 * @brief 创建事件组
 * @param None
 * @return 成功：事件指针  失败：NULL
 */
static inline
lm_event_t lm_event_create(void)
{
    return xEventGroupCreate();
}

/**
 * @brief 设置事件位
 * @param   xEventGroup,事件指针
 *          uxBitsToSet,需要设置的位
 * @return 错误码
 */
static inline
lm_bits_t lm_event_set(lm_event_t xEventGroup, const lm_bits_t uxBitsToSet)
{
    return xEventGroupSetBits(xEventGroup, uxBitsToSet);
}

/**
 * @brief 清除事件位
 * @param   xEventGroup,事件指针
 *          uxBitsToClear,需要清除的位
 * @return 错误码
 */
static inline
lm_bits_t lm_event_clear(lm_event_t xEventGroup, const lm_bits_t uxBitsToClear)
{
    return xEventGroupClearBits(xEventGroup, uxBitsToClear);
}

/**
 * @brief 获取事件
 * @param   xEventGroup,事件指针
 * @return 错误码
 */
static inline
lm_bits_t lm_event_get(lm_event_t xEventGroup)
{
    return xEventGroupGetBits(xEventGroup);
}

/**
 * @brief 等待事件
 * @param   xEventGroup,事件指针
 * @return 错误码
 */
static inline
lm_bits_t lm_event_wait(lm_event_t xEventGroup, \
                        const lm_bits_t uxBitsToWaitFor,\
                        const BaseType_t xClearOnExit,\
                        const BaseType_t xWaitForAllBits,\
                        TickType_t xTicksToWait)
{
    return xEventGroupWaitBits( xEventGroup, uxBitsToWaitFor, xClearOnExit,\
                                xWaitForAllBits, xTicksToWait);
}

#ifdef __cplusplus
}
#endif

#endif /* __LM_EVENT_H */

/* end of file */
