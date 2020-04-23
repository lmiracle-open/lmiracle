
#include <lm_ringbuf.h>
#include "lmiracle.h"
#include <string.h>

static inline enum lm_ringbuf_state lm_ringbuf_status(struct lm_ringbuf *p_rb)
{
    if (p_rb->read_index == p_rb->write_index)
    {
        if (p_rb->read_mirror == p_rb->write_mirror)
            return LM_RINGBUF_EMPTY;
        else
            return LM_RINGBUF_FULL;
    }
    return LM_RINGBUF_HALFFULL;
}

int lm_ringbuf_init (struct lm_ringbuf *p_rb, uint8_t *pool, size_t size)
{
    /* 1.参数检查 */
    if (unlikely(NULL == p_rb || NULL == pool || size <= 0)) {
        return -LM_EINVAL;
    }

    p_rb->read_mirror = p_rb->read_index = 0;
    p_rb->write_mirror = p_rb->write_index = 0;
    p_rb->buffer_ptr = pool;
    p_rb->buffer_size = size;

    return LM_OK;
}

/**
 * 将数据写入环形缓存区
 */
size_t lm_ringbuf_put (struct lm_ringbuf *p_rb,
                       const uint8_t     *ptr,
                       uint16_t           length)
{
    uint16_t size;

    if (p_rb == NULL) {
        return -LM_EINVAL;
    }

    /* whether has enough space */
    size = lm_ringbuf_space_len(p_rb);

    /* no space */
    if (size == 0)
        return 0;

    /* drop some data */
    if (size < length)
        length = size;

    if (p_rb->buffer_size - p_rb->write_index > length)
    {
        /* read_index - write_index = empty space */
        memcpy(&p_rb->buffer_ptr[p_rb->write_index], ptr, length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        p_rb->write_index += length;
        return length;
    }

    memcpy(&p_rb->buffer_ptr[p_rb->write_index],
           &ptr[0],
           p_rb->buffer_size - p_rb->write_index);
    memcpy(&p_rb->buffer_ptr[0],
           &ptr[p_rb->buffer_size - p_rb->write_index],
           length - (p_rb->buffer_size - p_rb->write_index));

    /* we are going into the other side of the mirror */
    p_rb->write_mirror = ~p_rb->write_mirror;
    p_rb->write_index = length - (p_rb->buffer_size - p_rb->write_index);

    return length;
}

/**
 *
 * 将数据写入环形缓存区,如果满了，覆盖以前数据
 *
 */
size_t lm_ringbuf_put_force (struct lm_ringbuf *p_rb,
                             const uint8_t     *ptr,
                             uint16_t           length)
{
    uint16_t space_length;

    if (p_rb == NULL) {
        return -LM_EINVAL;
    }

    space_length = lm_ringbuf_space_len(p_rb);

    if (length > p_rb->buffer_size)
    {
        ptr = &ptr[length - p_rb->buffer_size];
        length = p_rb->buffer_size;
    }

    if (p_rb->buffer_size - p_rb->write_index > length)
    {
        /* read_index - write_index = empty space */
        memcpy(&p_rb->buffer_ptr[p_rb->write_index], ptr, length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        p_rb->write_index += length;

        if (length > space_length)
            p_rb->read_index = p_rb->write_index;

        return length;
    }

    memcpy(&p_rb->buffer_ptr[p_rb->write_index],
           &ptr[0],
           p_rb->buffer_size - p_rb->write_index);
    memcpy(&p_rb->buffer_ptr[0],
           &ptr[p_rb->buffer_size - p_rb->write_index],
           length - (p_rb->buffer_size - p_rb->write_index));

    /* we are going into the other side of the mirror */
    p_rb->write_mirror = ~p_rb->write_mirror;
    p_rb->write_index = length - (p_rb->buffer_size - p_rb->write_index);

    if (length > space_length)
    {
        p_rb->read_mirror = ~p_rb->read_mirror;
        p_rb->read_index = p_rb->write_index;
    }

    return length;
}

/**
 * 获取数据
 */
size_t lm_ringbuf_get (struct lm_ringbuf *p_rb,
                       uint8_t           *ptr,
                       uint16_t           length)
{
    size_t size;

    if (p_rb == NULL) {
        return -LM_EINVAL;
    }

    /* whether has enough data  */
    size = lm_ringbuf_data_len(p_rb);

    /* no data */
    if (size == 0)
        return 0;

    /* less data */
    if (size < length)
        length = size;

    if (p_rb->buffer_size - p_rb->read_index > length)
    {
        /* copy all of data */
        memcpy(ptr, &p_rb->buffer_ptr[p_rb->read_index], length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        p_rb->read_index += length;
        return length;
    }

    memcpy(&ptr[0],
           &p_rb->buffer_ptr[p_rb->read_index],
           p_rb->buffer_size - p_rb->read_index);
    memcpy(&ptr[p_rb->buffer_size - p_rb->read_index],
           &p_rb->buffer_ptr[0],
           length - (p_rb->buffer_size - p_rb->read_index));

    /* we are going into the other side of the mirror */
    p_rb->read_mirror = ~p_rb->read_mirror;
    p_rb->read_index = length - (p_rb->buffer_size - p_rb->read_index);

    return length;
}

/**
 * 将一个字节写入到环形缓存区
 */
size_t lm_ringbuf_putchar (struct lm_ringbuf *p_rb, const uint8_t ch)
{
    if (p_rb == NULL) {
        return -LM_EINVAL;
    }

    /* whether has enough space */
    if (!lm_ringbuf_space_len(p_rb))
        return 0;

    p_rb->buffer_ptr[p_rb->write_index] = ch;

    /* flip mirror */
    if (p_rb->write_index == p_rb->buffer_size-1)
    {
        p_rb->write_mirror = ~p_rb->write_mirror;
        p_rb->write_index = 0;
    }
    else
    {
        p_rb->write_index++;
    }

    return 1;
}


/**
 * put a character into ring buffer
 *
 * When the buffer is full, it will discard one old data.
 */
size_t lm_ringbuf_putchar_force(struct lm_ringbuf *p_rb, const uint8_t ch)
{
    enum lm_ringbuf_state old_state;

    if (p_rb == NULL) {
        return -LM_EINVAL;
    }

    old_state = lm_ringbuf_status(p_rb);

    p_rb->buffer_ptr[p_rb->write_index] = ch;

    /* flip mirror */
    if (p_rb->write_index == p_rb->buffer_size-1)
    {
        p_rb->write_mirror = ~p_rb->write_mirror;
        p_rb->write_index = 0;
        if (old_state == LM_RINGBUF_FULL)
        {
            p_rb->read_mirror = ~p_rb->read_mirror;
            p_rb->read_index = p_rb->write_index;
        }
    }
    else
    {
        p_rb->write_index++;
        if (old_state == LM_RINGBUF_FULL)
            p_rb->read_index = p_rb->write_index;
    }

    return 1;
}

/**
 * get a character from a ringbuffer
 */
size_t lm_ringbuf_getchar(struct lm_ringbuf *p_rb, uint8_t *ch)
{
    if (p_rb == NULL) {
        return -LM_EINVAL;
    }


    /* ringbuffer is empty */
    if (!lm_ringbuf_data_len(p_rb))
        return 0;

    /* put character */
    *ch = p_rb->buffer_ptr[p_rb->read_index];

    if (p_rb->read_index == p_rb->buffer_size-1)
    {
        p_rb->read_mirror = ~p_rb->read_mirror;
        p_rb->read_index = 0;
    }
    else
    {
        p_rb->read_index++;
    }

    return 1;
}

/** 
 * 获取环形缓存区数据长度
 */
size_t lm_ringbuf_data_len (struct lm_ringbuf *p_rb)
{
    switch (lm_ringbuf_status(p_rb))
    {
    case LM_RINGBUF_EMPTY:
        return 0;
    case LM_RINGBUF_FULL:
        return p_rb->buffer_size;
    case LM_RINGBUF_HALFFULL:
    default:
        if (p_rb->write_index > p_rb->read_index)
            return p_rb->write_index - p_rb->read_index;
        else
            return p_rb->buffer_size - (p_rb->read_index - p_rb->write_index);
    };
}

/** 
 * 复位环形缓存区
 */
void lm_ringbuf_reset (struct lm_ringbuf *p_rb)
{
    if (p_rb == NULL) {
        return;
    }

    p_rb->read_mirror = 0;
    p_rb->read_index = 0;
    p_rb->write_mirror = 0;
    p_rb->write_index = 0;
}

/* end of file */
