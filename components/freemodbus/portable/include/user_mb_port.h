#ifndef __USER_MB_PORT_H
#define __USER_MB_PORT_H

#include "mb.h"
#include "mbconfig.h"
#include "mbframe.h"
#include "mbutils.h"

#include <stdbool.h>

/***************************** modbus从机起始地址定义 *****************************/

/*
 * 寄存器数量可根据项目需求进行调整
 */
#define S_DISCRETE_INPUT_START              0       /* 离散输入寄存器起始地址 */
#define S_DISCRETE_INPUT_NDISCRETES         200     /* 离散输入寄存器数量 */
#define S_COIL_START                        0       /* 线圈寄存器起始地址 */
#define S_COIL_NCOILS                       200     /* 线圈寄存器数量 */
#define S_REG_INPUT_START                   0       /* 输入寄存器起始地址 */
#define S_REG_INPUT_NREGS                   100     /* 输入寄存器数量 */
#define S_REG_HOLDING_START                 0       /* 保持寄存器起始地址 */
#define S_REG_HOLDING_NREGS                 100     /* 保持寄存器数量 */

#define READ_COIL_REG                       0x01    /* 01. 读线圈寄存器 */
#define READ_DISCRETE_INPUT_REG             0x02    /* 02. 读离散输入寄存器 */
#define READ_HOLDING_REG                    0x03    /* 03. 读保持寄存器 */
#define READ_INPUT_REG                      0x04    /* 04. 读输入寄存器 */
#define WRITE_COIL_REG                      0x05    /* 05. 写单个线圈寄存器 */
#define WRITE_HOLDING_REG                   0x06    /* 06. 写单个保持寄存器 */
#define WRITE_MUL_COIL_REG                  0x0F    /* 15. 写多个线圈寄存器 */
#define WRITE_MUL_HOLDING_REG               0x10    /* 16. 写多个保持寄存器 */

/*********************************** end **************************************/

/**************************** modbus从机寄存器操作类型定义 **************************/

typedef enum {
    WRITE_COIL_REG_EVENT                    = 1<<0, /* 写线圈寄存器事件 */
    WRITE_HOLDING_REG_EVENT                 = 1<<1, /* 写保持寄存器事件 */
} mbRegWriteEvent;

typedef enum {
    LM_WRITE_COIL_REG,                              /* 写线圈寄存器 */
    LM_WRITE_DISCRETE_INPUT_REG,                    /* 写离散输入寄存器 */
    LM_WRITE_HOLDING_REG,                           /* 写保持寄存器 */
    LM_WRITE_INPUT_REG,                             /* 写输入寄存器 */
} mbRegOpsType;

/*********************************** end **************************************/

/*************************** modbus从机移植接口数据结构定义  *************************/

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

    /**
     * 写线圈寄存器回调  应用程序注册此函数
     */
    int (*execu_coil_reg_cb) (  uint8_t *pucRegBuffer, \
                                uint16_t usAddress, \
                                uint16_t usBitOffset, \
                                uint16_t usNCoils);

    /**
     * 写保持寄存器回调  应用程序注册此函数
     */
    int (*execu_hold_reg_cb) (  uint16_t *pucRegBuffer, \
                                uint16_t usAddress, \
                                uint16_t usNRegs);
} lm_mb_param_t;

/*********************************** end **************************************/

/***************************** modbus从机对外提供接口  *****************************/

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

/**
 * @brief       用户操作modbus寄存器接口
 * @param       type,寄存器类型
 * @param       pBuf,数据指针
 * @param       addr,寄存器地址
 * @param       num,寄存器数量
 * @return      错误码
 */
extern int lm_modbus_reg_ops (  mbRegOpsType type, \
                                uint8_t *pBuf, \
                                uint16_t addr, \
                                uint16_t num);

/*********************************** end **************************************/

#endif /* __USER_MB_PORT_H */

/* end of file */
