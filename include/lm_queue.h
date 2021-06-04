/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_queue.h
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 队列模块
*******************************************************************************/

#ifndef __LM_QUEUE_H
#define __LM_QUEUE_H

#include "lmiracle.h"

LM_BEGIN_EXTERN_C

/* 队列数据类型 */
typedef QueueHandle_t lm_queue_t;

/**
 * @brief 创建队列
 *
 * @param[in] len       队列长度
 * @param[in] item_size 项目字节数
 *
 * @return  队列句柄
 */
static inline
lm_queue_t lm_queue_create(uint32_t len, uint32_t item_size)
{
    return xQueueCreate(len, item_size);
}

/**
 * @brief 入队
 *
 * @param[in] queue     队列指针
 * @param[in] item      入队项目指针
 * @param[in] wait_time 等待时间 单位 ms
 *
 * @return  错误码
 */
static inline
int lm_queue_push(lm_queue_t queue, void *item, uint32_t wait_time, bool isr)
{
    if (NULL == queue || NULL == item) {
        return LM_ENULL;
    }

    if (LM_FALSE != isr) {
        /* todo: 此处有问题??? */
//        if (pdPASS != xQueueSendFromISR(queue, item, wait_time)) {
//            return LM_EQUEUE_FULL;
//        }
    } else {
        if (pdPASS != xQueueSend(queue, item, wait_time)) {
            return LM_EQUEUE_FULL;
        }
    }

    return pdPASS;
}

/**
 * @brief 出队
 *
 * @param[in]   queue       队列指针
 * @param[out]  item        出队项目指针
 * @param[in]   wait_time   等待时间 单位 ms
 * @param[in]   del_flag    从队列中删除此项目标志  LM_TRUE,删除  LM_FALSE,不删除
 *
 * @return  错误码
 */
static inline
int lm_queue_pop(lm_queue_t queue, void *item, uint32_t wait_time, bool del_flag, bool isr)
{
    if (NULL == queue || NULL == item) {
        return LM_ENULL;
    }

    if (LM_TRUE == del_flag) {
        if (LM_FALSE != isr) {
            /* todo: 此处有问题??? */
//            if (pdPASS != xQueueReceiveFromISR(queue, item, wait_time)) {
//                return LM_EQUEUE_EMPTY;
//            }
        } else {
            if (pdPASS != xQueueReceive(queue, item, wait_time)) {
                return LM_EQUEUE_EMPTY;
            }
        }
    } else {
        if (LM_FALSE != isr) {
            if (pdPASS != xQueuePeekFromISR(queue, item)) {
                return LM_EQUEUE_EMPTY;
            }
        } else {
            if (pdPASS != xQueuePeek(queue, item, wait_time)) {
                return LM_EQUEUE_EMPTY;
            }
        }
    }

    return pdPASS;
}

LM_END_EXTERN_C

#endif /* __LM_QUEUE_H */

/* end of file */
