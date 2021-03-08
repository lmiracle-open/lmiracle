/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_types.h
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : lmiracle系统宏定义文件
*******************************************************************************/

#ifndef __LM_TYPES_H
#define __LM_TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>

/* C++扩展定义 */
#ifdef __cplusplus
    #define     LM_BEGIN_EXTERN_C           extern "C" {
    #define     LM_END_EXTERN_C             }
#else
    #define     LM_BEGIN_EXTERN_C
    #define     LM_END_EXTERN_C
#endif

LM_BEGIN_EXTERN_C

/******************************************************************************/

/* 内存屏障宏定义 */
#ifndef __barrier
#define __barrier()   __asm__ __volatile__("":::"memory")
#endif

/* 优化等级宏定义 */
#ifndef __optimize
#define __optimize(level) __attribute__((optimize(level)))
#endif

/* 取消字节对齐宏定义 */
#ifndef __packed
#define __packed    __attribute__((packed))
#endif

/* 不使用宏定义 */
#ifndef __unused
#define __unused    __attribute__((unused))
#endif

/* 弱函数宏定义 */
#ifndef __default
#define __default   __attribute__((weak))
#endif

/* NULL宏定义 */
#ifndef NULL
#define NULL ((void *)0)
#endif

/* true宏定义 */
#ifndef true
#define true 1
#endif


/* false宏定义 */
#ifndef false
#define false 0
#endif

/* 自定义 */
#define LM_TRUE     1
#define LM_FALSE    0

/******************************************************************************/

/* gcc内建函数之字节交换宏定义 */
#define BYTE_SWAP16(x)  __builtin_bswap16(x)        /* 16位字节交换 */
#define BYTE_SWAP32(x)  __builtin_bswap32(x)        /* 32位字节交换 */
#define BYTE_SWAP64(x)  __builtin_bswap64(x)        /* 64位字节交换 */

/* gcc内建函数之期望值优化宏定义 */
#define likely(exp)     __builtin_expect(!!(exp), 1)/* 期望位真 条件多数情况会发生 */
#define unlikely(Exp)   __builtin_expect(!!(Exp), 0)/* 期望为假 条件多数情况不会发生 */

/* 常用设置某个位或者清除某个位宏定义 */
#define SETBIT(val,bit)     (val |= (1 << bit))     /* 设置BIT位 */
#define CLRBIT(val,bit)     (val &= ~(1 << bit))    /* 清除BIT位 */
#define ISSETBIT(val,bit)   (val & (1 << bit))      /* 判断是否置位 */

/* 计算数组大小宏定义 */
#define ARRAY_LEN(x)        (sizeof(x)/sizeof((x)[0]))  /* 获取一个数组元素个数 */

/******************************************************************************/

/* 组件使能开关定义 */
#define LM_COMPONENTS_ENABLED(module) \
    ((defined(module ## _ENABLED) && (module ## _ENABLED)) ? 1 : 0)

/* 驱动使能开关定义 */
#define LM_DRIVER_ENABLED(module) \
    ((defined(module ## _ENABLED) && (module ## _ENABLED)) ? 1 : 0)

/* 应用程序使能开关定义 */
#define LM_APP_ENABLED(module) \
    ((defined(module ## _ENABLED) && (module ## _ENABLED)) ? 1 : 0)

LM_END_EXTERN_C

#endif /* __LM_TYPES_H */

/* end of file */
