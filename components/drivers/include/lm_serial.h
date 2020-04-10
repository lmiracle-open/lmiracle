/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_serial.h
* Change Logs   :
* Date         Author      Notes
* 2019-02-26   terryall     V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 串口框架层代码
*******************************************************************************/

#ifndef __LM_SERIAL_H
#define __LM_SERIAL_H

#include "lmiracle.h"
#include "lm_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LM_DEVICE_CTRL_CONFIG           0x03    /* configure device */
#define LM_DEVICE_CTRL_SET_INT          0x10    /* enable receive irq */
#define LM_DEVICE_CTRL_CLR_INT          0x11    /* disable receive irq */
#define LM_DEVICE_CTRL_GET_INT          0x12

#define LM_SERIAL_EVENT_RX_IND          0x01    /* Rx indication */
#define LM_SERIAL_EVENT_TX_DONE         0x02    /* Tx complete   */
#define LM_SERIAL_EVENT_RX_DMADONE      0x03    /* Rx DMA transfer done */
#define LM_SERIAL_EVENT_TX_DMADONE      0x04    /* Tx DMA transfer done */
#define LM_SERIAL_EVENT_RX_TIMEOUT      0x05    /* Rx timeout    */

#define LM_SERIAL_DMA_RX                0x01
#define LM_SERIAL_DMA_TX                0x02

#define LM_SERIAL_RX_INT                0x01
#define LM_SERIAL_TX_INT                0x02

#define LM_SERIAL_ERR_OVERRUN           0x01
#define LM_SERIAL_ERR_FRAMING           0x02
#define LM_SERIAL_ERR_PARITY            0x03

#define LM_SERIAL_TX_DATAQUEUE_SIZE     2048
#define LM_SERIAL_TX_DATAQUEUE_LWM      30




/* gpio操作接口 */
struct lm_serial_ops {
    int (*serial_ctrl) (lm_device_t dev, const char *name, uint32_t value);
    int (*serial_putc) (lm_device_t dev, const char *name, uint32_t mode);
    int (*serial_getc) (lm_device_t dev, const char *name, uint32_t value);
    int (*gpio_read) (lm_device_t dev, const char *name);
    int (*gpio_toggle) (lm_device_t dev, const char *name);
    int (*gpio_attach_irq) (lm_device_t dev, const char *name, uint32_t io_mode,
                            gpio_irqcb_t irq_cb, void *args);
    int (*gpio_detach_irq) (lm_device_t dev, const char *name);
    int (*gpio_irq_enable) (lm_device_t dev, const char *name, uint32_t enabled);
};

/* gpio设备管理结构体 */
struct lm_device_gpio {
    struct lm_device dev;
    const struct lm_gpio_ops *ops;
};

/* gpio设备注册接口 */
extern
int lm_device_gpio_register (const char *name, const struct lm_gpio_ops *ops,
                             void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* __LM_SERIAL_H */

/* end of file */
