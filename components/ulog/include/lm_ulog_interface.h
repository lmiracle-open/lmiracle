/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_ulog_interface.h
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 日志移植接口模块
*******************************************************************************/

#ifndef __LM_ULOG_INTERFACE_H
#define __LM_ULOG_INTERFACE_H

#include "lmiracle.h"

LM_BEGIN_EXTERN_C

/******************************************************************************/

/**
 * @brief 日志参数配置结构体
 */
typedef struct lm_ulog {
    uint32_t com;                           /* 日志输出串口号 */
    uint8_t *ulog_out_buf;                  /* 日志输出缓存 */
    uint16_t ulog_s;                        /* 日志输出缓存大小 */

} lm_ulog_t;

/**
 * @brief 日志接口注册
 *
 * @param[in]   p_ulog 日志参数配置
 *
 * @return  错误码
 */
extern int lm_ulog_register (lm_ulog_t *p_ulog);

LM_END_EXTERN_C

#endif /* __LM_ULOG_INTERFACE_H */

/* end of file */
