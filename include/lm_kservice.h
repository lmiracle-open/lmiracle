/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_kservice.h
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 内核相关服务模块
*******************************************************************************/

#ifndef __LM_KSERVICE_H
#define __LM_KSERVICE_H

#include "lmiracle.h"

LM_BEGIN_EXTERN_C

/**
 * @brief 格式化输出
 *
 * @param[in]   fmt     格式化字符串
 * @param[in]   ...     可变参数
 *
 * @return  None
 */
extern void lm_kprintf (const char *fmt, ...);

LM_END_EXTERN_C

#endif  /* __LM_KSERVICE_H */

/* end of file */
