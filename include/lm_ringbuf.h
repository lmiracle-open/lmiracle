/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_ringbuf.h
* Change Logs   :
* Date         Author      Notes
* 2019-06-07   terryall     V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 环形缓冲区
*******************************************************************************/

#ifndef __LM_RINGBUF_H
#define __LM_RINGBUF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lmiracle.h"
#include "lm_error.h"
#include "lm_types.h"
#include "lm_heap.h"

#include <string.h>

#include <stdbool.h>

typedef struct
{
    uint16_t head;                  /* 头指针 */
    uint16_t tail;                  /* 尾指针 */
    uint16_t len;                   /* 有效数据长度 */
    uint16_t size;                  /* 缓冲区最大长度 */
    uint8_t  *pbuf;                 /* 缓冲区 */
} lm_ringbuf_t;

/**
 * @brief 初始化环形缓冲区(静态)
 * @param p_rb,环形缓冲区指针
 *        buff,缓冲区数组
 *        size,缓冲区大小
 * @return 错误码
 */
static inline
int lm_ringbuf_init (lm_ringbuf_t *p_rb, uint8_t *buff, uint32_t size)
{
    /* 1.参数检查 */
    if (unlikely(NULL == p_rb || NULL == buff || size <= 0)) {
        return LM_ERROR;
    }

    p_rb->head = p_rb->tail = p_rb->len = 0;
    p_rb->size = size;
    p_rb->pbuf = buff;
    memset(p_rb->pbuf, 0, sizeof(uint8_t)*size);

    return LM_OK;
}

/**
 * @brief 动态创建环形缓冲区
 * @param p_rb,环形缓冲区指针
 *        size,缓冲区长度
 * @return 错误码
 */
static inline
int lm_ringbuf_create (lm_ringbuf_t *p_rb, uint32_t size)
{
    /* 1.参数检查 */
    if (NULL != p_rb || size <= 0) {
        return LM_ERROR;
    }

    /* 2.动态申请缓存 */
    p_rb->pbuf = lm_mem_alloc(size);
    if (unlikely(NULL == p_rb->pbuf)) {
        return LM_ENOMEM;
    }

    /* 3.初始化环形缓冲区 */
    p_rb->head = p_rb->tail = p_rb->len = 0;
    p_rb->size = size;
    memset(p_rb->pbuf, 0, sizeof(uint8_t)*size);

    return LM_OK;
}

/**
 * @brief 销毁环形缓冲区
 * @param p_rb,环形缓冲区指针
 * @return 错误码
 */
static inline
int lm_ringbuf_destroy (lm_ringbuf_t *p_rb)
{
    /* 1.参数检查 */
    if (unlikely(NULL == p_rb)) {
        return LM_ERROR;
    }

    /* 2.销毁缓存 */
    lm_mem_free(p_rb->pbuf);

    return LM_OK;
}

/**
 * @brief 写数据到环形缓冲区
 * @param p_rb,环形缓冲区指针
 *        data,待写入的数据
 * @return 错误码
 */
static inline
int lm_ringbuf_push (lm_ringbuf_t *p_rb, uint8_t data)
{
    /* 1.参数检查 */
    if (unlikely(NULL == p_rb)) {
        return LM_ERROR;
    }

    /* 2.判断缓冲区是否已满 */
    if (p_rb->len >= p_rb->size) {
        return LM_ENULL;
    }

    /* 3.从缓冲区尾入 */
    p_rb->pbuf[p_rb->tail] = data;

    /* 4.防止越界非法访问 */
    p_rb->tail = (p_rb->tail + 1) % p_rb->size;

    /* 5.数据区长度递增 */
    p_rb->len ++;

    return LM_OK;
}

/**
 * @brief 从缓冲区中读取数据
 * @param p_rb,环形缓冲区指针
 *        data,读出的数据
 * @return 错误码
 */
static inline
int lm_ringbuf_pop (lm_ringbuf_t *p_rb, uint8_t *data)
{
    /* 1.参数检查 */
    if (unlikely(NULL == p_rb || NULL == data)) {
        return LM_ERROR;
    }

    /* 2.有效数据长度检查 */
    if (p_rb->len == 0) {
        return LM_ERROR;
    }

    /* 3.先进先出FIFO，从缓冲区头出 */
    *data = p_rb->pbuf[p_rb->head];

    /* 4.防止越界非法访问 */
    p_rb->head = (p_rb->head + 1) % p_rb->size;

    /* 5.数据区长度递减 */
    p_rb->len --;

    return LM_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* __LM_RINGBUF_H */

/* end of file */
