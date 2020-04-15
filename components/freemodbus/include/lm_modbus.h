#ifndef __LM_MODBUS_H
#define __LM_MODBUS_H

#include <stdbool.h>
#include "lm_assert.h"

/* 定时器结构体 */
typedef struct {
    uint32_t timer_id;                      /* 定时器ID */

    /**
     * 定时器初始化回调
     */
    int (*timer_init) (uint32_t id, uint32_t timeout);

    /**
     * 定时器睡眠回调
     */
    int (*timer_sleep) (uint32_t id, bool enable);
} lm_mb_timer_t;

/* 串口结构体 */
typedef struct {
    uint32_t com;                           /* 串口ID */
    uint32_t stack_size;                    /* 任务栈深度 */
    uint32_t prio;                          /* 任务优先级 */

    /**
     * 串口初始化回调
     */
    int (*serial_init)(uint32_t com,uint32_t baud,uint8_t databit,uint8_t parity);

    /**
     * 串口中断使能回调
     */
    int (*serial_irq_enable)(uint32_t com, bool enabled);

    /**
     * 串口接收回调
     */
    int (*serial_read)(uint32_t com, uint8_t *data);

    /**
     * 串口发送回调
     */
    int (*serial_write)(uint32_t com, const void *data, uint16_t len);
} lm_mb_serial_t;

/* modbus参数结构体 */
typedef struct {
    uint32_t stack_size;                    /* 任务栈深度 */
    uint32_t prio;                          /* 任务优先级 */
    uint8_t  embmode;                       /* modbus模式  */
    uint8_t  slave_addr;                    /* 从机地址 */
    uint8_t  uport;                         /* 串口号 */
    uint8_t  embparity;                     /* 校验模式 */
    uint32_t ubaud;                         /* 波特率 */
} lm_mb_param_t;

/**
 * @brief       modbus初始化
 * @param       None
 * @return      错误码
 */
extern int lm_modbus_init (void);

/**
 * @brief       串口注册接口
 * @param       mb_serial,串口指针
 * @return      错误码
 */
extern int lm_modbus_serial_register (const lm_mb_serial_t *mb_serial);

/**
 * @brief       定时器注册接口
 * @param       mb_timer,定时器指针
 * @return      错误码
 */
extern int lm_modbus_timer_register (const lm_mb_timer_t *mb_timer);

/**
 * @brief       modbus参数注册接口
 * @param       mb_param,参数指针
 * @return      错误码
 */
extern int lm_modbus_param_register (const lm_mb_param_t *mb_param);

/**
 * @brief       定时器超时处理回调
 * @param       None
 * @return      错误码
 */
extern int lm_modbus_timer_cb (void);

/**
 * @brief       串口接收完成回调
 * @param       data,数据指针
 *              len,数据长度
 * @return      错误码
 */
extern int lm_serial_recv_cb (uint8_t *data, uint16_t len);

#endif /* __LM_MODBUS_H */

/* end of file */
