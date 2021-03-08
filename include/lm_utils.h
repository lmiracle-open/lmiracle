/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_utils.c
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 单元模块(包含一些常用算法 字符串操作 类型转换等)
*******************************************************************************/

#ifndef __LM_UTILS_H
#define __LM_UTILS_H

#include "lmiracle.h"

LM_BEGIN_EXTERN_C

/**
 * @brief float转字符串
 *
 * @param[out] str      字符串
 * @param[in]  number   浮点数
 * @param[in]  i_n      整数位数
 * @param[in]  d_n      小数位数
 * @param[in]  flag     是否加正负号标志      true: 加     false: 不加
 *
 * @return  错误码
 */
extern int lm_utils_float_to_str (  char        *str,       \
                                    float       number,     \
                                    uint8_t     i_n,        \
                                    uint8_t     d_n,        \
                                    bool        flag);

LM_END_EXTERN_C

#endif /* __LM_UTILS_H */

/* end of file */
