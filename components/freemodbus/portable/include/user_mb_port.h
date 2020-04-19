#ifndef __USER_MB_PORT_H
#define __USER_MB_PORT_H

#include "mb.h"
#include "mbconfig.h"
#include "mbframe.h"
#include "mbutils.h"

#include <stdbool.h>

/* -----------------------Slave Defines -------------------------------------*/
#define S_DISCRETE_INPUT_START                    0
#define S_DISCRETE_INPUT_NDISCRETES               16
#define S_COIL_START                              0
#define S_COIL_NCOILS                             64
#define S_REG_INPUT_START                         0
#define S_REG_INPUT_NREGS                         100
#define S_REG_HOLDING_START                       0
#define S_REG_HOLDING_NREGS                       100

/* salve mode: holding register's all address */
#define          S_HD_RESERVE                     0
/* salve mode: input register's all address */
#define          S_IN_RESERVE                     0
/* salve mode: coil's all address */
#define          S_CO_RESERVE                     0
/* salve mode: discrete's all address */
#define          S_DI_RESERVE                     0

/* 用户定时器注册结构体 */
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

/* 用户串口注册结构体 */
typedef struct {
    uint32_t com;                           /* 串口ID */
    uint32_t stack_size;                    /* 串口发送事件任务栈深度 */
    uint32_t prio;                          /* 串口发送事件任务优先级 */

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

/* modbus参数注册结构体 */
typedef struct {
    uint32_t stack_size;                    /* modbus轮询任务栈深度 */
    uint32_t prio;                          /* modbus轮询任务优先级 */
    eMBMode  embmode;                       /* modbus模式  */
    UCHAR  slave_addr;                      /* 从机地址 */
    UCHAR  uport;                           /* 串口号 */
    eMBParity  embparity;                   /* 校验模式 */
    ULONG ubaud;                            /* 波特率 */
} lm_mb_param_t;

/**
 * @brief       modbus协议初始化
 * @param       None
 * @return      错误码
 */
extern int mb_protocol_init (void);

/**
 * @brief       modbus参数注册接口
 * @param       mb_param,参数指针
 * @return      错误码
 */
extern int mb_param_register (const lm_mb_param_t *mb_param);

/**
 * @brief       串口底层注册接口
 * @param       mb_serial,串口指针
 * @return      错误码
 */
extern int mb_hw_serial_register (const lm_mb_serial_t *p_mb);

/**
 * @brief       定时器底层注册接口
 * @param       mb_timer,定时器指针
 * @return      错误码
 */
extern int mb_hw_timer_register (const lm_mb_timer_t *mb_timer);

/**
 * @brief       定时器超时处理回调
 * @param       None
 * @return      错误码
 */
extern int mb_timer_expired_cb (void);

/**
 * @brief       串口接收通知回调
 * @param       None
 * @return      错误码
 */
extern int mb_serial_recv_notice_cb (void);

#endif /* __USER_MB_PORT_H */

/* end of file */
