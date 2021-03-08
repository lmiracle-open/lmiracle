/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_error.h
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 错误码模块
*******************************************************************************/

#ifndef __LM_ERROR_H
#define __LM_ERROR_H

#include "lm_types.h"

LM_BEGIN_EXTERN_C

/* 错误码数据类型 */
typedef  unsigned long lm_err_t;

#define LM_OK                0               /**< There is no error */
#define LM_ERROR             1               /**< A generic error happens */
#define LM_ETIMEOUT          2               /**< Timed out */
#define LM_EFULL             3               /**< The resource is full */
#define LM_EEMPTY            4               /**< The resource is empty */
#define LM_ENOMEM            5               /**< No memory */
#define LM_ENOSYS            6               /**< No system */
#define LM_EBUSY             7               /**< Busy */
#define LM_EIO               8               /**< IO error */
#define LM_EINTR             9               /**< Interrupted system call */
#define LM_EINVAL            10              /**< Invalid argument */
#define LM_ENODEV            11              /**< 设备不存在 */
#define LM_ENULL             12              /* 空指针 */
#define LM_EQUEUE_FULL       13              /* 队列满 */
#define LM_EQUEUE_EMPTY      14              /* 队列空 */
#define LM_EINPROGRESS       15              /* 操作正在进行中 */
#define LM_EFAULT            16              /* 地址错误 */
#define LM_ENOENT            17
#define LM_EEXIST            18              /* 文件存在 */
#define LM_ENOTSUP           19              /* 不支持 */

LM_END_EXTERN_C

#endif /* __LM_ERROR_H */

/* end of file */
