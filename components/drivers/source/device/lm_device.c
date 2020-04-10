/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_device.c
* Change Logs   :
* Date         Author      Notes
* 2020-02-26   terryall    V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 设备框架层代码
*******************************************************************************/

#include "lm_device.h"
#include "lm_heap.h"

/* 定义一个设备链表实体 并初始化 */
static struct device_node lm_device_node = {
    .depth = 0,
    .list = LIST_HEAD_INIT(lm_device_node.list)
};

/* 设备查找接口 */
lm_device_t lm_device_find (const char *name)
{
    struct lm_device *dev;
    struct lm_list_head *node;

    for (   node = lm_device_node.list.next;
            node != &(lm_device_node.list);
            node = node->next) {
        dev = lm_list_first_entry(node, struct lm_device, list);
        if (strncmp(dev->name, name, LM_DEV_NAME_SIZE) == 0) {
            return dev;
        }
    }

    return NULL;
}

/* 设备注册接口 */
int lm_device_register (lm_device_t dev, const char *name, uint32_t flags)
{
    int err = LM_OK;

    /* 1. 参数检查 */
    if (NULL == dev || NULL == name) {
        return LM_ERROR;
    }

    /* 2. 查找设备 if已经注册过 直接返回 */
    if (NULL != lm_device_find(name)) {
        return LM_ERROR;
    }

    /* 3. 注册设备 */
    strncpy(dev->name, name, LM_DEV_NAME_SIZE);

    lm_list_add(&dev->list, &lm_device_node.list);
    lm_device_node.depth ++;

    dev->flag = flags;
    dev->open_flag = 0;

    return err;
}

/* 设备注销接口 */
int lm_device_destroy (const char *name)
{
    int err = LM_OK;
    struct lm_device *dev = NULL, *tmp = NULL;

    /* 1. 检查链表是否为空 */
    if (lm_list_empty(&lm_device_node.list)) {
        return LM_ERROR;
    }

    lm_list_for_each_entry_safe (dev, tmp, &lm_device_node.list, list) {
        if (strncmp(dev->name, name, LM_DEV_NAME_SIZE) != 0) {
            continue;
        }

        /* TODO */
        lm_list_del(&dev->list);
        lm_mem_free(dev);
        lm_device_node.depth --;
    }

    return err;
}

/* 设备初始化接口 */
int lm_device_init (lm_device_t dev)
{
    int err = LM_OK;

    lm_assert(NULL != dev);

    if (dev->init) {
        if (!(dev->flag & LM_DEVICE_FLAG_ACTIVATED)) {
            err = dev->init(dev);
            if (LM_OK != err) {
                // TODO: 初始化失败
            } else {
                dev->flag |= LM_DEVICE_FLAG_ACTIVATED;
            }
        }
    }

    return err;
}

/* 设备打开接口 */
int lm_device_open (lm_device_t dev, uint32_t oflag)
{
    int err = LM_OK;

    lm_assert(NULL != dev);

    if (!(dev->flag & LM_DEVICE_FLAG_ACTIVATED)) {
        if (dev->init) {
            err = dev->init(dev);
            if (LM_OK != err) {
                // TODO: 初始化失败
                return LM_ERROR;
            }
        }
        dev->flag |= LM_DEVICE_FLAG_ACTIVATED;
    }

    /* device is a stand alone device and opened */
    if ((dev->flag & LM_DEVICE_FLAG_STANDALONE) &&
        (dev->open_flag & LM_DEVICE_OFLAG_OPEN)) {
        return -LM_ERROR;
    }

    /* call device open interface */
    if (dev->open) {
        err = dev->open(dev, oflag);
    } else {
        /* set open flag */
        dev->open_flag = (oflag & LM_DEVICE_OFLAG_MASK);
    }

    /* set open flag */
    if (err == LM_OK || err == -LM_ERROR) {
        dev->open_flag |= LM_DEVICE_OFLAG_OPEN;
        // TODO:
    }

    return err;
}

/* 设备关闭接口 */
int lm_device_close (lm_device_t dev)
{
    int err = LM_OK;

    lm_assert(NULL != dev);

    if (dev->close) {
        err = dev->close(dev);
    }

    return err;
}

/* 设备读取数据接口 */
uint32_t lm_device_read (lm_device_t dev, int pos, void *buffer, uint32_t size)
{
    int err = LM_ERROR;

    lm_assert(NULL != dev);

    if (dev->read) {
        return dev->read(dev, pos, buffer, size);
    }

    return err;
}

/* 设备写数据接口 */
uint32_t lm_device_write (lm_device_t dev, int pos, void *buffer, uint32_t size)
{
    int err = LM_ERROR;

    lm_assert(NULL != dev);

    if (dev->write) {
        return dev->write(dev, pos, buffer, size);
    }

    return err;
}

/* 设备控制接口 */
int lm_device_control (lm_device_t dev, int cmd, void *arg)
{
    int err = LM_ERROR;

    lm_assert(NULL != dev);

    if (dev->control) {
        return dev->control(dev, cmd, arg);
    }

    return err;
}

/* 设备接收回调挂载接口 */
int lm_device_rx_rsp_cb_set (lm_device_t dev, rx_indicate_t rx_cb)
{
    int err = LM_OK;

    lm_assert(NULL != dev);

    dev->rx_indicate = rx_cb;

    return err;
}

/* 设备发送完成回调挂载接口 */
int lm_device_tx_done_cb_set (lm_device_t dev, tx_complete_t tx_cb)
{
    int err = LM_OK;

    lm_assert(NULL != dev);

    dev->tx_complete = tx_cb;

    return err;
}

/* end of file */

