/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_ringbuf.h
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    linxuew         V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 环形缓冲区模块
*******************************************************************************/

#ifndef __LM_RINGBUF_H
#define __LM_RINGBUF_H

#include "lmiracle.h"

LM_BEGIN_EXTERN_C

/* ring buffer */
struct lm_ringbuf
{
    uint8_t *buffer_ptr;
    /*
     *          mirror = 0                    mirror = 1
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 ||| 0 | 1 | 2 | 3 | 4 | 5 | 6 | Full
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     *  read_idx-^                   write_idx-^
     *
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 ||| 0 | 1 | 2 | 3 | 4 | 5 | 6 | Empty
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * read_idx-^ ^-write_idx
     *
     */
    uint16_t read_mirror : 1;
    uint16_t read_index : 15;
    uint16_t write_mirror : 1;
    uint16_t write_index : 15;

    int16_t buffer_size;
};

enum lm_ringbuf_state
{
    LM_RINGBUF_EMPTY,
    LM_RINGBUF_FULL,
    /* half full is neither full nor empty */
    LM_RINGBUF_HALFFULL,
};

/**
 * @brief 初始化环形缓冲区(静态)
 *
 * @param[in] p_rb   环形缓冲区指针
 * @param[in] pool   环形缓冲区地址
 * @param[in] size   环形缓冲区大小
 *
 * @return LM_OK  成功
 *         其他    失败
 */
extern int lm_ringbuf_init (struct lm_ringbuf *p_rb, uint8_t *pool, size_t size);

/**
 * @brief 将数据写入环形缓存区
 *
 * @param[in] p_rb   环形缓冲区指针
 * @param[in] ptr    需要写入数据的地址
 * @param[in] length 需要写入数据的长度
 *
 * return 大于０ : 写入数据的长度
 *        小于０ : 错误码
 */
extern size_t lm_ringbuf_put (struct lm_ringbuf *p_rb,
                              const uint8_t     *ptr,
                              uint16_t           length);

/**
 * @brief 将数据写入环形缓存区,如果满了，覆盖以前数据
 *
 * @param[in] p_rb   环形缓冲区指针
 * @param[in] ptr    需要写入数据的地址
 * @param[in] length 需要写入数据的长度
 *
 * return 大于０ : 写入数据的长度
 *        小于０ : 错误码
 */
extern size_t lm_ringbuf_put_force (struct lm_ringbuf *p_rb,
                                    const uint8_t     *ptr,
                                    uint16_t           length);

/**
 * @brief 获取数据
 *
 * @param[in]  p_rb   环形缓冲区指针
 * @param[out] ptr    保存获取数据的地址
 * @param[in]  length 获取数据的长度
 *
 * return 大于０ : 获取数据的长度
 *        小于０ : 错误码
 */
extern size_t lm_ringbuf_get (struct lm_ringbuf *p_rb,
                              uint8_t           *ptr,
                              uint16_t           length);

/**
 * @brief 将一个字节写入到环形缓存区
 *
 * @param[in] p_rb 环形缓冲区指针
 * @param[in] ch   需要写入的字符
 *
 * return 大于０ : 写入数据的长度
 *        小于０ : 错误码
 */
extern size_t lm_ringbuf_putchar (struct lm_ringbuf *p_rb, const uint8_t ch);


/**
 * @brief 将一个字节强制写入到环形缓存区
 *
 * @param[in] p_rb 环形缓冲区指针
 * @param[in] ch   需要写入的字符
 *
 * return 大于０ : 写入数据的长度
 *        小于０ : 错误码
 */
extern size_t lm_ringbuf_putchar_force (struct lm_ringbuf *p_rb, const uint8_t ch);

/**
 * @brief 获取一个字节
 *
 * @param[in]  p_rb 环形缓冲区指针
 * @param[out] ch   保存获取数据的地址
 *
 * return 大于０ : 获取数据的长度
 *        小于０ : 错误码
 */
extern size_t lm_ringbuf_getchar (struct lm_ringbuf *p_rb, uint8_t *ch);

/**
 * @brief 获取环形缓存区数据长度
 *
 * @param[in]  p_rb   环形缓冲区指针
 *
 * return 大于０ : 环形缓存区中数据的长度
 *        小于０ : 错误码
 */
extern size_t lm_ringbuf_data_len (struct lm_ringbuf *p_rb);


/**
 * @brief 复位环形缓存区
 *
 * @param[in]  p_rb   环形缓冲区指针
 */
extern void lm_ringbuf_reset (struct lm_ringbuf *p_rb);


/**
 * @brief 获取环形缓存区的长度
 *
 * @param[in]  p_rb   环形缓冲区指针
 *
 * @return 返回环形缓存区长度
 */
static inline size_t lm_ringbuf_get_size(struct lm_ringbuf *p_rb)
{
    if (p_rb == NULL) {
        return 0;
    }

    return p_rb->buffer_size;
}

/**
 * @brief 返回环形缓存区的空间
 */
#define lm_ringbuf_space_len(p_rb) ((p_rb)->buffer_size - lm_ringbuf_data_len(p_rb))

LM_END_EXTERN_C

#endif /* __LM_RINGBUF_H */

/* end of file */
