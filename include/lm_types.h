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


#ifndef __barrier
#define __barrier()   __asm__ __volatile__("":::"memory")
#endif

#ifndef __unused
#define __unused    __attribute__((unused))
#endif

#ifndef __default
#define __default   __attribute__((weak))
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define LM_TRUE                 1
#define LM_FALSE                0

/* 字节交换函数 */
#define BYTE_SWAP16(x)      __builtin_bswap16(x)
#define BYTE_SWAP32(x)      __builtin_bswap32(x)
#define BYTE_SWAP64(x)      __builtin_bswap64(x)

#define likely(exp)         __builtin_expect(!!(exp), 1)    /* 期望位真 */
#define unlikely(Exp)       __builtin_expect(!!(Exp), 0)    /* 期望为假 */

#define SETBIT(val,bit)     (val |= (1 << bit))             /* 设置BIT位 */
#define CLRBIT(val,bit)     (val &= ~(1 << bit))            /* 清除BIT位 */
#define ISSETBIT(val,bit)   (val & (1 << bit))              /* 判断是否置位 */

#define ARRAY_LEN(x)        (sizeof(x)/sizeof((x)[0]))    /* 获取一个数组元素个数 */

#ifdef __cplusplus
}
#endif

#endif

/* end of file */
