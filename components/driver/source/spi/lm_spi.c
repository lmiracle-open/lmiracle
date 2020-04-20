#include "lmiracle.h"
#include "lm_spi.h"
#include "string.h"


/* 链表头 */
LIST_HEAD(__g_spi_list);

/*
 * 片选
 */
static void __spi_set_cs (lm_spi_master_t *p_master, uint8_t enable)
{
    if (p_master->p_spi->mode & LM_SPI_CS_HIGH) {
        enable = !enable;
    }

    p_master->p_funcs->pfunc_set_cs(p_master, !enable);
}

static int __spi_validate_bits_per_word (lm_spi_master_t *p_master,
                                         uint8_t          bits_per_word)
{
    if (p_master->bits_per_word_mask) {

        /* Only 32 bits fit in the mask */
        if (bits_per_word > 32)
            return -LM_EINVAL;
        if (!(p_master->bits_per_word_mask & LM_SPI_BPW_MASK(bits_per_word)))
            return -LM_EINVAL;
    }

    return 0;
}

/*
 * 传输一个消息
 */
static int __spi_transfer_one_message (lm_spi_master_t  *p_master,
                                       lm_spi_message_t *p_msg)
{
    int                               ret;
    lm_spi_transfer_t                *xfer;
    uint8_t                          keep_cs = LM_FALSE;

    p_msg->status = -LM_EINPROGRESS;

    __spi_set_cs(p_master, 1);

    lm_list_for_each_entry(xfer, &p_msg->transfers, transfer_list) {

        if (xfer->p_txbuf || xfer->p_rxbuf) {
            if (p_master->p_funcs->pfunc_transfer) {
                if((ret = p_master->p_funcs->pfunc_transfer(p_master,
                                                            p_msg->p_spi,
                                                            xfer))) {
                    goto out;
                }
            }
        } else {
            if (xfer->len) {
                //todo错误
            }
        }

        /* 一次传输完成 */
        if (p_msg->status != -LM_EINPROGRESS) {
            goto out;
        }

        if (xfer->delay_usece) {
//            uint16_t us = xfer->delay_usece;
//            if (us < 10) {
//                lm_udelay(us);
//            } todo 没有us延时
        }

        if (xfer->cs_change) {
            if (lm_list_is_last(&xfer->transfer_list, &p_msg->transfers)) {
                keep_cs = LM_TRUE;
            } else {
                __spi_set_cs(p_master, 0);

                /* 延时10us */
                __spi_set_cs(p_master, 1);
            }
        }

        p_msg->actual_length += xfer->len;
    }

out:
    if ((ret != 0) || (!keep_cs)) {
        __spi_set_cs(p_master, 0);
    }

    return ret;
}


/*
 * 同步传输
 */
static int __spi_sync (lm_spi_master_t  *p_master,
                       lm_spi_dev_t     *p_spi,
                       lm_spi_message_t *p_msg)
{

    int                      ret = LM_OK;

    ret = __spi_transfer_one_message(p_master, p_msg);

    return ret;
}

/*
 * 查找 SPI 控制器
 */
static lm_spi_master_t *__find_spi_master (lm_spi_dev_t *p_spi)
{

    struct lm_list_head *p_node = __g_spi_list.next;
    lm_spi_master_t     *p_master = NULL;

    while(p_node != &__g_spi_list) {
        p_master = lm_list_entry(p_node, lm_spi_master_t, list);

        if (p_spi->bus_id == p_master->bus_id) {
            return p_master;
        }
        p_node = p_node->next;
    }

    return NULL;
}

/*
 * 设置 SPI 设备
 */
int lm_spi_setup (lm_spi_dev_t *p_spi)
{
    int                ret = LM_OK;
    uint32_t           bad_bits;

    lm_spi_master_t *p_master = NULL;

    p_master = __find_spi_master(p_spi);
    if (!p_master){
        return -LM_ENODEV;
    }

    /* 设置控制器中SPI设备指针 */
    p_master->p_spi = p_spi;

    if (((p_spi->mode & LM_SPI_TX_DUAL) && (p_spi->mode & LM_SPI_TX_QUAD)) ||
        ((p_spi->mode & LM_SPI_RX_DUAL) && (p_spi->mode & LM_SPI_RX_QUAD))) {
        return -LM_EINVAL;
    }

    if ((p_spi->mode & LM_SPI_3WIRE) && (p_spi->mode &
                (LM_SPI_TX_DUAL | LM_SPI_TX_QUAD | LM_SPI_RX_DUAL | LM_SPI_RX_QUAD))) {
        return -LM_EINVAL;
    }
    
    /* 获取不支持的模式 */
    bad_bits = p_spi->mode & ~p_master->mode_bits;
    if (bad_bits) {
        return -LM_EINVAL;
    }

    /* 默认8位 */
    if (!p_spi->bits_per_word) {
        p_spi->bits_per_word = 8;
    }

    if (!p_spi->max_speed_hz) {
        p_spi->max_speed_hz = p_master->max_speed_hz;
    }

    if ((ret = __spi_validate_bits_per_word(p_master, p_spi->bits_per_word))){
        return ret;
    }

    if (p_master->p_funcs->pfunc_setup) {
        ret  = p_master->p_funcs->pfunc_setup(p_master, p_spi);
    }

    /* 关闭片选 */
    __spi_set_cs(p_master, 0);

    /* todo: debug */

    return ret;
}

/*
 * SPI 同步传输
 */
int lm_spi_sync (lm_spi_dev_t     *p_spi,
                 lm_spi_message_t *p_msg)
{
    int ret;
    lm_spi_master_t *p_master = NULL;

    p_master = __find_spi_master(p_spi);
    if (!p_master){
        return -LM_ENODEV;
    }

    p_msg->p_spi = p_spi;

    lm_mutex_lock(&p_master->bus_lock_mutex, LM_SEM_WAIT_FOREVER);
    p_master->p_spi = p_spi;
    ret = __spi_sync(p_master, p_spi, p_msg);
    lm_mutex_unlock(&p_master->bus_lock_mutex);

    return ret;
}

/*
 * 先写后读
 */
int lm_spi_write_then_read (lm_spi_dev_t  *p_spi,
                            const uint8_t *txbuf,
                            size_t         n_tx,
                            uint8_t       *rxbuf,
                            size_t         n_rx)
{
    int ret = LM_OK;

    lm_spi_message_t           message;
    lm_spi_transfer_t          trans[2];

    lm_spi_message_init(&message);

    memset(trans, 0, sizeof(trans));

    if (n_tx) {
        trans[0].len = n_tx;
        trans[0].p_txbuf = txbuf;
        lm_spi_message_add_tail(&trans[0], &message);
    }

    if(n_rx) {
        trans[1].len = n_rx;
        trans[1].p_rxbuf = rxbuf;
        lm_spi_message_add_tail(&trans[1], &message);
    }

    /* 开启同步传输 */
    lm_spi_sync(p_spi, &message);

    return ret;
}

/*
 * 注册SPI驱动
 */
int lm_spi_register (lm_spi_master_t *p_master)
{
    int ret = LM_OK;

    /* 创建同步锁 */
    lm_mutex_create(&p_master->bus_lock_mutex);

    lm_list_add_tail(&p_master->list , &__g_spi_list);

    return ret;
}


/* end of file */
