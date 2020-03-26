/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_queue.h
* Change Logs   :
* Date         Author      Notes
* 2019-06-07   terryall     V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 队列
*******************************************************************************/

#ifndef __LM_QUEUE_H
#define __LM_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "queue.h"
#include "lmiracle.h"
#include "lm_error.h"
#include "lm_types.h"

typedef QueueHandle_t lm_queue_t;   /* 队列数据类型 */

/**
 * @brief 创建队列
 * @param   len,队列长度
 *          item_size,项目字节数
 * @return 成功：队列指针  失败：NULL
 */
static inline
lm_queue_t lm_queue_create(uint32_t len, uint32_t item_size)
{
    return xQueueCreate(len, item_size);
}

/**
 * @brief 入队
 * @param   queue,队列指针
 *          item,入队项目指针
 *          wait_time,等待时间 单位 ms
 * @return 错误码
 */
static inline
int lm_queue_push(lm_queue_t queue, void *item, uint32_t wait_time)
{
    if (NULL == queue || NULL == item) {
        return LM_ENULL;
    }

    if (pdPASS != xQueueSend(queue, item, wait_time)) {
        return LM_EQUEUE_FULL;
    }

    return LM_OK;
}

/**
 * @brief 出队
 * @param   queue,队列指针
 *          item,出队项目指针
 *          wait_time,等待时间 单位 ms
 *          del_flag,从队列中删除此项目标志  LM_TRUE,删除  LM_FALSE,不删除
 * @return 错误码
 */
static inline
int lm_queue_pop(lm_queue_t queue, void *item, uint32_t wait_time, bool del_flag)
{
    if (NULL == queue || NULL == item) {
        return LM_ENULL;
    }

    if (LM_TRUE == del_flag) {
        if (pdPASS != xQueueReceive(queue, item, wait_time)) {
            return LM_EQUEUE_EMPTY;
        }
    } else {
        if(pdPASS != xQueuePeek(queue, item, wait_time)) {
            return LM_EQUEUE_EMPTY;
        }
    }

    return LM_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* __LM_QUEUE_H */

/* end of file */
