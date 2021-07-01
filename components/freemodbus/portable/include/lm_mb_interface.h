/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_mb_interface.h
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : modbus接口移植模块
*******************************************************************************/

#ifndef __LM_MB_INTERFACE_H
#define __LM_MB_INTERFACE_H

#include "mb.h"
#include "mbconfig.h"
#include "mbframe.h"
#include "mbutils.h"
#include "lmiracle.h"

LM_BEGIN_EXTERN_C

/*******************************************************************************
* Description   : modbus寄存器定义
*******************************************************************************/
typedef enum {
    LM_MB_COIL_REG,                                            /* 线圈寄存器 */
    LM_MB_DISCRETE_REG,                                        /* 离散量寄存器 */
    LM_MB_INPUT_REG,                                           /* 输入寄存器 */
    LM_MB_HOLD_REG,                                            /* 保持寄存器 */
} lm_mb_reg_e;

/*************************** modbus从机移植接口数据结构定义  *************************/

/**
 * @brief modbus定时器注册结构体
 */
typedef struct lm_mb_timer {
    /* 定时器ID */
    uint32_t timer_id;
    /* 定时器开关 */
    uint32_t timer_switch;

    /**
     * @brief 定时器到期时间设置回调
     */
    int (*timer_expired_set) (uint32_t id, uint32_t timeout);

    /**
     * @brief 定时器睡眠回调
     */
    int (*timer_sleep) (uint32_t id, bool enable);

    /**
     * @brief 定时器超时时间计算回调
     */
    uint32_t (*timeout_calculation) (uint16_t time);

} lm_mb_timer_t;

/**
 * @brief modbus串口注册结构体
 */
typedef struct lm_mb_serial {
    uint32_t  com;                          /* 串口号 */
    bool      transmit_type;                /* 串口传输方式 */
} lm_mb_serial_t;

/**
 * @brief modbus寄存器注册结构体
 */
typedef struct lm_mb_modbus {

    eMBMode     mb_mode;                    /* 通信模式 */

    UCHAR       slave_addr;                 /* 从机地址 */

    eMBParity   mb_par;                     /* 校验模式 */

    ULONG       mb_baud;                    /* 波特率 */

    UCHAR       mb_com;                     /* 串口号 */

    USHORT      coil_start_addr;            /* 线圈寄存器起始地址 */
    USHORT      coil_num;                   /* 线圈寄存器个数 */
    UCHAR       *coil_buf;                  /* 线圈寄存器缓存  */

    USHORT      disc_start_addr;            /* 离散寄存器起始地址 */
    USHORT      disc_num;                   /* 离散寄存器个数 */
    UCHAR       *disc_buf;                  /* 离散寄存器缓存  */

    USHORT      in_start_addr;              /* 输入寄存器起始地址 */
    USHORT      in_num;                     /* 输入寄存器个数 */
    USHORT      *in_buf;                    /* 输入寄存器缓存 */

    USHORT      hold_start_addr;            /* 保持寄存器起始地址 */
    USHORT      hold_num;                   /* 保持寄存器个数 */
    USHORT      *hold_buf;                  /* 保持寄存器缓存 */

} lm_mb_modbus_t;

/***************************** modbus从机对外提供接口  *****************************/

/**
 * @brief mdobus寄存器接口注册
 *
 * @param[in]   p_mb_reg 寄存器指针
 *
 * @return  错误码
 */
extern int lm_modbus_reg_register (lm_mb_modbus_t *p_mb_reg);

/**
 * @brief 串口底层接口注册
 *
 * @param[in]   p_mb_serial 串口指针
 *
 * @return  错误码
 */
extern int lm_modbus_serial_register (const lm_mb_serial_t *p_mb_serial);

/**
 * @brief 定时器底层接口注册
 *
 * @param[in]   p_mb_timer 定时器指针
 *
 * @return  错误码
 */
extern int lm_modbus_timer_register (const lm_mb_timer_t *p_mb_timer);

/**
 * @brief 定时器超时处理回调
 *
 * @param[in]   None
 *
 * @return  错误码
 */
extern int lm_modbus_timer_expired_cb (void);

/**
 * @brief modbus寄存器写操作
 *
 * @param[in]   type,寄存器类型
 * @param[in]   pbuf,有效数据
 * @param[in]   addr,写寄存器地址
 * @param[in]   inum,写寄存器个数
 *
 * @return  错误码
 */
extern
int lm_modbus_reg_write (uint8_t type, const uint8_t *pbuf, uint16_t addr, uint16_t inum);

/**
 * @brief modbus寄存器读操作
 *
 * @param[in]   data,有效数据
 * @param[in]   size,长度
 *
 * @return  错误码
 */
extern int lm_modbus_reg_read (uint8_t *data, uint16_t size);

LM_END_EXTERN_C

#endif /* __LM_MB_INTERFACE_H */

/* end of file */
