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
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    uint8_t *p_src;                             /* 源指针 */
    uint8_t * volatile p_read;                  /* 读数据指针 */
    uint8_t * volatile p_write;                 /* 写数据指针 */
    volatile uint32_t fill_cnt;                 /* 计数器 */
    uint32_t size;                              /* 缓冲区大小 */
} lm_ringbuf_t;

/**
 * @brief 初始化环形缓冲区
 * @param p_rb,环形缓冲区指针
 *        buf,缓存数组
 *        size,缓存大小
 * @return 成功：环形缓冲区指针  失败：NULL
 */
static inline
int lm_ringbuf_create(lm_ringbuf_t *p_rb, uint8_t *buf, uint32_t size)
{
    if (NULL == p_rb || NULL == buf || size < 2) {
        return LM_ERROR;
    }

    p_rb->p_src = p_rb->p_read = p_rb->p_write = buf;
    p_rb->fill_cnt = 0;
    p_rb->size = size;

    return LM_OK;
}

/**
 * @brief 写数据到环形缓冲区
 * @param p_rb,环形缓冲区指针
 *        data,待写入的数据指针
 *        len,写入长度
 * @return 错误码
 */
static inline
int lm_ringbuf_push(lm_ringbuf_t *p_rb, uint8_t *data, uint16_t len)
{
    if (NULL == p_rb || p_rb->fill_cnt >= p_rb->size) {
        /* 缓冲区已满 */
        return LM_ENULL;
    }

    for (int i = 0; i < len; i ++) {
        p_rb->fill_cnt ++;

        *p_rb->p_write ++ = data[i];

        /* if写入指针超过缓冲区末尾 则把指针指向原点 */
        if (p_rb->p_write >= p_rb->p_src + p_rb->size) {
            p_rb->p_write = p_rb->p_src;
        }
    }

    return LM_OK;
}

/**
 * @brief 从缓冲区中读取数据
 * @param p_rb,环形缓冲区指针
 *        data,读出的数据
 *        len,读出的长度
 * @return 长度
 */
static inline
int lm_ringbuf_pop(lm_ringbuf_t *p_rb, uint8_t *data, uint16_t len)
{
    if (NULL == p_rb || NULL == data || p_rb->fill_cnt <= 0) {
        return LM_ERROR;
    }

    /* if读取的数据超出拥有数据的长度 则最大只能读取fill_cnt */
    if (len > p_rb->fill_cnt) {
        len = p_rb->fill_cnt;
    }

    uint32_t cnt = p_rb->fill_cnt;
    for (int i = 0; i < len; i++) {
        p_rb->fill_cnt --;
        *data = *p_rb->p_read ++;
        *data ++;
        /* if 读取指针超过缓冲区末尾 则把指针指向原点 */
        if (p_rb->p_read >= p_rb->p_src + p_rb->size) {
            p_rb->p_read = p_rb->p_src;
        }
    }

    return LM_OK;
}

/**
 * @brief 清空缓冲区
 * @param p_rb,环形缓冲区指针
 * @return 错误码
 */
static inline
int lm_ringbuf_clear(lm_ringbuf_t *p_rb)
{
    if (NULL == p_rb) {
        return LM_ERROR;
    }

    memset(p_rb->p_src, 0, p_rb->size);
    p_rb->p_read = p_rb->p_write = p_rb->p_src;
    p_rb->fill_cnt = 0;

    return LM_OK;
}

/**
 * @brief 读取指定位置数据
 * @param p_rb,环形缓冲区指针
 *        index,序号
 * @return 错误码
 */
static inline
uint8_t lm_ringbuf_idx_read(lm_ringbuf_t *p_rb, uint32_t index)
{
    uint8_t data = 0;

    if (NULL == p_rb) {
        return LM_ERROR;
    }

    if (p_rb->fill_cnt <= 0) {
        return LM_ERROR;
    }

    if (index > p_rb->fill_cnt) {
        index = p_rb->fill_cnt;
    }

    if (p_rb->p_read + index >= p_rb->p_src + p_rb->size) {
        index = index - (p_rb->p_src + p_rb->size - p_rb->p_read);
    }

    data = p_rb->p_read[index];

    return data;
}

#ifdef __cplusplus
}
#endif

#endif /* __LM_RINGBUF_H */

/* end of file */
