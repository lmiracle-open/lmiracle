#ifndef __LM_MB_INTERFACE_H
#define __LM_MB_INTERFACE_H

#include "mb.h"
#include "mbconfig.h"
#include "mbframe.h"
#include "mbutils.h"

#include <stdbool.h>

/*************************** modbus从机移植接口数据结构定义  *************************/

/* modbus定时器注册结构体 */
typedef struct {
    uint32_t timer_id;                                  /* 定时器ID */
    /* 定时器到期时间设置回调 */
    int (*timer_expired_set) (uint32_t id, uint32_t timeout);
    int (*timer_sleep) (uint32_t id, bool enable);      /* 定时器睡眠回调 */
} lm_mb_timer_t;

/* modbus串口注册结构体 */
typedef struct {
    uint32_t com;                                       /* 串口号 */
    uint32_t stask_stack_size;                          /* 串口发送任务栈深度 */
    uint32_t stask_prio;                                /* 串口发送任务优先级 */
    uint32_t rtask_stack_size;                          /* 串口接收任务栈深度 */
    uint32_t rtask_prio;                                /* 串口接收任务优先级 */
} lm_mb_serial_t;

/* modbus参数注册结构体 */
typedef struct {
    uint32_t ptask_stack_size;                          /* modbus轮询任务栈深度 */
    uint32_t ptask_prio;                                /* modbus轮询任务优先级 */
    eMBMode  embmode;                                   /* modbus模式  */
    UCHAR  slave_addr;                                  /* 从机地址 */
    UCHAR  uport;                                       /* 串口号 */
    eMBParity  embparity;                               /* 校验模式 */
    ULONG ubaud;                                        /* 波特率 */
} lm_mb_param_t;

/***************************** modbus从机对外提供接口  *****************************/

/**
 * @brief       modbus协议初始化
 * @param       None
 * @return      错误码
 */
extern int lm_mb_protocol_init (void);

/**
 * @brief       modbus参数注册接口
 * @param       mb_param,参数指针
 * @return      错误码
 */
extern int lm_mb_param_register (const lm_mb_param_t *mb_param);

/**
 * @brief       串口底层接口注册
 * @param       p_mb_serial,串口指针
 * @return      错误码
 */
extern int lm_mb_serial_register (const lm_mb_serial_t *p_mb_serial);

/**
 * @brief       定时器底层接口注册
 * @param       p_mb_timer,定时器指针
 * @return      错误码
 */
extern int lm_mb_timer_register (const lm_mb_timer_t *p_mb_timer);

/**
 * @brief       定时器超时处理回调
 * @param       None
 * @return      错误码
 */
extern int lm_mb_timer_expired_cb (void);

#endif /* __LM_MB_INTERFACE_H */

/* end of file */