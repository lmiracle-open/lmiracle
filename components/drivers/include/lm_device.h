/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_device.h
* Change Logs   :
* Date         Author      Notes
* 2019-02-26   terryall     V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 设备框架层代码
*******************************************************************************/

#ifndef __LM_DEVICE_H
#define __LM_DEVICE_H

#include "lm_param_cfg.h"
#include "lmiracle.h"
#include "lm_list.h"

#define LM_DEVICE_FLAG_RDONLY           0x001       /**< read only */
#define LM_DEVICE_FLAG_WRONLY           0x002       /**< write only */
#define LM_DEVICE_FLAG_RDWR             0x003       /**< read and write */

#define LM_DEVICE_FLAG_ACTIVATED        0x010       /**< device is activated */
#define LM_DEVICE_FLAG_REMOVABLE        0x004       /**< removable device */
#define LM_DEVICE_FLAG_STANDALONE       0x008       /**< standalone device */
#define LM_DEVICE_FLAG_ACTIVATED        0x010       /**< device is activated */
#define LM_DEVICE_FLAG_SUSPENDED        0x020       /**< device is suspended */
#define LM_DEVICE_FLAG_STREAM           0x040       /**< stream mode */

#define LM_DEVICE_FLAG_INT_RX           0x100       /**< INT mode on Rx */
#define LM_DEVICE_FLAG_DMA_RX           0x200       /**< DMA mode on Rx */
#define LM_DEVICE_FLAG_INT_TX           0x400       /**< INT mode on Tx */
#define LM_DEVICE_FLAG_DMA_TX           0x800       /**< DMA mode on Tx */

#define LM_DEVICE_OFLAG_CLOSE           0x000       /**< device is closed */
#define LM_DEVICE_OFLAG_RDONLY          0x001       /**< read only access */
#define LM_DEVICE_OFLAG_WRONLY          0x002       /**< write only access */
#define LM_DEVICE_OFLAG_RDWR            0x003       /**< read and write */
#define LM_DEVICE_OFLAG_OPEN            0x008       /**< device is opened */
#define LM_DEVICE_OFLAG_MASK            0xf0f       /**< mask of open flag */

/* 设备管理链表 */
struct device_node {
    uint8_t depth;                          /* 链表深度 */
    struct lm_list_head list;               /* 设备链表 */
};

typedef struct lm_device *lm_device_t;

/* 设备回调 */
typedef int (*rx_indicate_t)(lm_device_t dev, uint32_t size);/* 接收回调挂载 */
typedef int (*tx_complete_t)(lm_device_t dev, void *buffer); /* 发送完成回调 */

/* 设备链表控制块 */
struct lm_device {
    char        name[LM_DEV_NAME_SIZE];         /* 设备名称 */
    uint16_t                flag;               /* 设备状态 */
    uint16_t               open_flag;           /* 设备打开状态 */

    /* 设备回调 */
    rx_indicate_t rx_indicate;                  /* 接收回调挂载 */
    tx_complete_t tx_complete;                  /* 发送完成回调 */

    /* 设备操作接口 */
    int  (*init)   (lm_device_t dev);
    int  (*open)   (lm_device_t dev, int oflag);
    int  (*close)  (lm_device_t dev);
    int  (*read)   (lm_device_t dev, int pos, void *buffer, uint32_t size);
    int  (*write)  (lm_device_t dev, int pos, const void *buffer, uint32_t size);
    int  (*control)(lm_device_t dev, int cmd, void *args);

    struct lm_list_head     list;               /* 设备链表 */
    void *userdata;                             /* 用户数据 */
};

/* 设备查找接口 */
extern
lm_device_t lm_device_find (const char *name);

/* 设备注册接口 */
extern
int lm_device_register (lm_device_t dev, const char *name, uint32_t flags);

/* 设备注销接口 */
extern
int lm_device_destroy (const char *name);

/* 设备初始化接口 */
extern
int lm_device_init (lm_device_t dev);

/* 设备打开接口 */
extern
int lm_device_open (lm_device_t dev, uint32_t oflag);

/* 设备关闭接口 */
extern
int lm_device_close (lm_device_t dev);

/* 设备读取数据接口 */
extern
uint32_t lm_device_read (lm_device_t dev, int pos, void *buffer, uint32_t size);

/* 设备写数据接口 */
extern
uint32_t lm_device_write (lm_device_t dev, int pos, void *buffer, uint32_t size);

/* 设备控制接口 */
extern
int lm_device_control (lm_device_t dev, int cmd, void *arg);

/* 设备接收回调挂载接口 */
extern
int lm_device_rx_rsp_cb_set (lm_device_t dev, rx_indicate_t rx_cb);

/* 设备发送完成回调挂载接口 */
extern
int lm_device_tx_done_cb_set (lm_device_t dev, tx_complete_t tx_cb);

#endif /* __LM_DEVICE_H */

/* end of file */
