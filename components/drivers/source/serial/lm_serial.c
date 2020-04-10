/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_gpio.c
* Change Logs   :
* Date         Author      Notes
* 2020-02-26   terryall    V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : GPIO框架层代码
*******************************************************************************/

#include "lmiracle.h"
#include "lm_gpio.h"
#include "lm_device.h"

/* 定义gpio设备实体 */
static struct lm_device_gpio __gpio_dev;

/* gpio设备读数据接口 */
static
int __gpio_read (lm_device_t dev, int pos, void *buffer, uint32_t size)
{
    struct lm_device_gpio_status *status;
    struct lm_device_gpio *gpio = (struct lm_device_gpio *)dev;

    lm_assert(NULL != gpio);

    status = (struct lm_device_gpio_status *)buffer;

    if (NULL == status || sizeof(*status) != size) {
        return LM_OK;
    }

    status->status = gpio->ops->gpio_read(dev, status->name);

    return size;
}

/* gpio设备写数据接口 */
static
int __gpio_write (lm_device_t dev, int pos, const void *buffer, uint32_t size)
{
    struct lm_device_gpio_status *status;
    struct lm_device_gpio *gpio = (struct lm_device_gpio *)dev;

    lm_assert(NULL != gpio);

    status = (struct lm_device_gpio_status *)buffer;

    if (NULL == status || sizeof(*status) != size) {
        return LM_OK;
    }

    gpio->ops->gpio_write(dev, status->name, status->status);

    return size;
}

/* gpio设备控制接口 */
static
int __gpio_control (lm_device_t dev, int cmd, void *args)
{
    int err = LM_OK;

    struct lm_device_gpio_mode *mode;
    struct lm_device_gpio *gpio = (struct lm_device_gpio *)dev;

    lm_assert(NULL != gpio);

    mode = (struct lm_device_gpio_mode *)args;

    if (NULL == mode) {
        return -LM_ERROR;
    }

    gpio->ops->gpio_mode_set(dev, mode->name, mode->mode);

    return err;
}

/* gpio设备注册接口 */
int lm_device_gpio_register (const char *name, const struct lm_gpio_ops *ops,
                             void *userdata)
{
    int err = LM_OK;

    lm_assert(NULL != name);

    __gpio_dev.dev.init      = NULL;
    __gpio_dev.dev.open      = NULL;
    __gpio_dev.dev.close     = NULL;
    __gpio_dev.dev.read      = __gpio_read;
    __gpio_dev.dev.write     = __gpio_write;
    __gpio_dev.dev.control   = __gpio_control;

    __gpio_dev.ops = ops;
    __gpio_dev.dev.userdata = userdata;

    lm_device_register(&__gpio_dev.dev, name, LM_DEVICE_FLAG_RDWR);

    return err;
}

/******************************************************************************/

/* gpio时钟控制接口 */
int lm_gpio_rcc_ctrl (const char *name, uint32_t value)
{
    int err = LM_ERROR;

    lm_assert(NULL != __gpio_dev.ops);

    if (__gpio_dev.ops->gpio_rcc_ctrl) {
        return __gpio_dev.ops->gpio_rcc_ctrl(&__gpio_dev.dev, name, value);
    }

    return err;
}

/* gpio模式设置接口 */
int lm_gpio_mode_set (const char *name, uint32_t io_mode)
{
    int err = LM_ERROR;

    lm_assert(NULL != __gpio_dev.ops);

    if (__gpio_dev.ops->gpio_mode_set) {
        return __gpio_dev.ops->gpio_mode_set(&__gpio_dev.dev, name, io_mode);
    }

    return err;
}

/* gpio状态设置接口 */
int lm_gpio_write (const char *name, uint32_t io_value)
{
    int err = LM_ERROR;

    lm_assert(NULL != __gpio_dev.ops);

    if (__gpio_dev.ops->gpio_write) {
        return __gpio_dev.ops->gpio_write(&__gpio_dev.dev, name, io_value);
    }

    return err;
}

/* gpio状态读取接口 */
int lm_gpio_read (const char *name)
{
    int err = LM_ERROR;

    lm_assert(NULL != __gpio_dev.ops);

    if (__gpio_dev.ops->gpio_read) {
        return __gpio_dev.ops->gpio_read(&__gpio_dev.dev, name);
    }

    return err;
}

/* gpio状态翻转接口 */
int lm_gpio_toggle (const char *name)
{
    int err = LM_ERROR;

    lm_assert(NULL != __gpio_dev.ops);

    if (__gpio_dev.ops->gpio_read) {
        return __gpio_dev.ops->gpio_toggle(&__gpio_dev.dev, name);
    }

    return err;
}

/* gpio中断回调绑定接口 */
int lm_gpio_attach_irq (const char *name, uint32_t io_mode,
                                gpio_irqcb_t irq_cb, void *args)
{
    int err = LM_ERROR;

    lm_assert(NULL != __gpio_dev.ops);

    if (__gpio_dev.ops->gpio_attach_irq) {
        return __gpio_dev.ops->gpio_attach_irq(&__gpio_dev.dev, name, io_mode,
                                                irq_cb, args);
    }

    return err;
}

/* gpio中断回调解除接口 */
int lm_gpio_detach_irq (const char *name)
{
    int err = LM_ERROR;

    lm_assert(NULL != __gpio_dev.ops);

    if (__gpio_dev.ops->gpio_detach_irq) {
        return __gpio_dev.ops->gpio_detach_irq(&__gpio_dev.dev, name);
    }

    return err;
}

/* gpio中断开关接口 */
int lm_gpio_irq_enable (const char *name, uint32_t enabled)
{
    int err = LM_ERROR;

    lm_assert(NULL != __gpio_dev.ops);

    if (__gpio_dev.ops->gpio_irq_enable) {
        return __gpio_dev.ops->gpio_irq_enable(&__gpio_dev.dev, name, enabled);
    }

    return err;
}

/* end of file */

