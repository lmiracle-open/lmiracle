/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_types.h
* Change Logs   :
* Date         Author      Notes
* 2019-06-07   terryall    V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 系统文件
*******************************************************************************/

#ifndef __LM_TYPES_H__
#define __LM_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#define LM_TRUE                 0
#define LM_FALSE                1

#define likely(exp)         __builtin_expect(!!(exp), 1)    /* 期望位真 */
#define unlikely(Exp)       __builtin_expect(!!(Exp), 0)    /* 期望为假 */

#define ARRAY_LEN(x)        (sizeof(x) / sizeof((x)[0]))    /* 获取一个数组元素个数 */

#endif

/* end of file */
