#include "lmiracle.h"
#include "lm_serial.h"


/* 链表头 */
static LIST_HEAD(__g_spi_list);

static struct lm_serial_port *__gp_serial[COM_MUX] = {NULL};

#define __com2serial(com) (__gp_serial[com])

const static struct lm_serial_info __g_serial_info_default =
                    LM_SERIAL_INFO_DEFAULT;

int lm_serial_get_info (int com, struct lm_serial_info *p_info)
{
    int ret = LM_OK;

    struct lm_serial_port *p_serial;

    if (lm_is_int_context()) {
        return -LM_ENOTSUP;
    }

    if ((com >= COM_MUX) || (p_info == NULL)) {
        return -LM_EINVAL;
    }

    p_serial = __com2serial(com);
    if (NULL == p_serial) {
        return -LM_ENODEV;
    }

    /* 获取读写锁 */
    lm_mutex_lock(&p_serial->wr_mutex, LM_SEM_WAIT_FOREVER);
    lm_mutex_lock(&p_serial->ro_mutex, LM_SEM_WAIT_FOREVER);

    memcpy(p_info, &p_serial->serial_info, sizeof(*p_info));

    lm_mutex_unlock(&p_serial->ro_mutex);
    lm_mutex_unlock(&p_serial->wr_mutex);

    return ret;
}


/*
 * @brief 配置串口
 */
int lm_serial_set_info (int com, const struct lm_serial_info *p_info)
{
    int ret;
    struct lm_serial_port *p_serial;

    if (lm_is_int_context()) {
        return -LM_ENOTSUP;
    }

    if ((com >= COM_MUX) || (p_info == NULL)) {
        return -LM_EINVAL;
    }

    p_serial = __com2serial(com);
    if (NULL == p_serial) {
        return -LM_ENODEV;
    }

    /* 获取读写锁 */
    lm_mutex_lock(&p_serial->wr_mutex, LM_SEM_WAIT_FOREVER);
    lm_mutex_lock(&p_serial->ro_mutex, LM_SEM_WAIT_FOREVER);

    if (p_serial->p_ops->pfunc_set_config) {
        ret = p_serial->p_ops->pfunc_set_config(p_serial, &p_info->config);
        if (!ret) {
            memcpy(&p_serial->serial_info, p_info, sizeof(*p_info));
        }
    }


    lm_mutex_unlock(&p_serial->ro_mutex);
    lm_mutex_unlock(&p_serial->wr_mutex);

    return ret;
}


/*
 * 串口读
 */
int lm_serial_read (int com, void *p_buf, size_t size)
{
    struct lm_serial_port    *p_serial = NULL;

    uint32_t total_time  = 0, timeout = 0;
    uint32_t  start_tick = 0;
    size_t    idx        = 0;
    uint32_t  len        = 0;
    uint8_t *p_buffer;

    /* 剩下的全局超时 */
    uint32_t  remain_total_timeout = 0;

    /* 保存全局超时和码间超时中最小的一个 */
    uint32_t  least_timeout = 0;

    if (lm_is_int_context()) {
        return -LM_ENOTSUP;
    }

    if ((com >= COM_MUX) || (p_buf == NULL) || ((int)size <= 0)) {
        return -LM_EINVAL;
    }

    p_serial = __com2serial(com);
    if (NULL == p_serial) {
        return -LM_ENODEV;
    }

    p_buffer = (uint8_t *)p_buf;
    lm_mutex_lock(&p_serial->ro_mutex, LM_SEM_WAIT_FOREVER);

    total_time = p_serial->serial_info.read_timeout;
    timeout    = total_time;

    if (total_time > p_serial->serial_info.idle_timeout) {
        least_timeout = p_serial->serial_info.idle_timeout;
    } else {
        least_timeout = total_time;
    }

    /* 获取互斥量后才开始记录 超时时间 */
    start_tick = lm_sys_get_tick();

    while (size) {

        if (lm_semb_take(&p_serial->ro_sync_semb, timeout) != LM_OK) {

            /* 接收超时 */
            lm_mutex_unlock(&p_serial->ro_mutex);
            return idx;
        }

        len      =  lm_ringbuf_get(&p_serial->rbuf, &p_buffer[idx], size);
        idx      += len;
        size     -= len;

        /*
         * 如果没有读完,下次进来可以直接读取
         */
        if (lm_ringbuf_data_len(&p_serial->rbuf)){
            lm_semb_give(&p_serial->ro_sync_semb);
        }

        if (total_time != (uint32_t)-1) {

            uint32_t tmp_use_time = \
                    lm_tick_to_ms(lm_sys_get_tick() - start_tick);

            if (tmp_use_time > total_time) {
                /* 总超时已到 */
                lm_mutex_unlock(&p_serial->ro_mutex);
                return idx;
            } else {
                /* 剩下的全局超时时间  */
                remain_total_timeout = total_time - tmp_use_time;
            }

            /*
             * 如果剩下的超时时间小于最小的一个超时,
             * 则使用最小的,否则使用最小超时
             */
            if (remain_total_timeout > least_timeout) {
                timeout = least_timeout;
            } else {
                timeout = remain_total_timeout;
            }
        } else {
            /* 设置下一次的超时为码间超时 */
            timeout = least_timeout;
        }
    }

    lm_mutex_unlock(&p_serial->ro_mutex);

    return idx;
}


/*
 * 串口发送
 */
int lm_serial_write (int com, const void *p_buf, size_t size)
{
    struct lm_serial_port *p_serial;
    size_t           wlen      = 0;

    if (lm_is_int_context()) {
        return -LM_ENOTSUP;
    }

    if ((com >= COM_MUX ) || (p_buf == NULL) || ((int)size <= 0)) {
        return -LM_EINVAL;
    }

    p_serial = __com2serial(com);

    if (NULL == p_serial) {
        return -LM_ENODEV;
    }

    lm_mutex_lock(&p_serial->wr_mutex, LM_SEM_WAIT_FOREVER);

    if (p_serial->p_ops->pfunc_send) {
        wlen = p_serial->p_ops->pfunc_send(p_serial, p_buf, size);
    }

    lm_mutex_unlock(&p_serial->wr_mutex);

    return wlen;
}


/*
 * 注册串口驱动
 */
int lm_serial_register (struct lm_serial_port *p_serial)
{
    int ret = LM_OK;

    if (p_serial == NULL) {
        return -LM_EINVAL;
    }

    if ((p_serial->recv_buf  == NULL) || \
        ((p_serial->id) >= COM_MUX)||(p_serial->id < 0))  {
        return -LM_EINVAL;
    }

    /* 该设备是否已近创建 */
    if (__com2serial(p_serial->id) == NULL) {
        __com2serial(p_serial->id) = p_serial;
    } else {
        return -LM_EEXIST;
    }

    /* 初始化环形缓存区 */
    lm_ringbuf_init(&p_serial->rbuf, p_serial->recv_buf, p_serial->buf_size);

    /* 创建同步锁和信号量 */
    lm_mutex_create(&p_serial->wr_mutex);
    lm_mutex_create(&p_serial->ro_mutex);
    lm_semb_create(&p_serial->ro_sync_semb);

    /* 初始化默认配置 */
    memcpy(&p_serial->serial_info,
           &__g_serial_info_default,  sizeof(p_serial->serial_info));

    lm_list_add_tail(&p_serial->list , &__g_spi_list);

    return ret;
}


/* end of file */
