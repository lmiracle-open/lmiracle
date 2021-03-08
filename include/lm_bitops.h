/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_bitops.h
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    linxuew         V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 位操作模块
*******************************************************************************/

#ifndef __LM_BITOPS_H
#define __LM_BITOPS_H

#include "lm_asm.h"

LM_BEGIN_EXTERN_C

#define BIT(nr)         (1UL << (nr))
#define BIT_ULL(nr)     (1ULL << (nr))
#define BIT_MASK(nr)        (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)        ((nr) / BITS_PER_LONG)
#define BIT_ULL_MASK(nr)    (1ULL << ((nr) % BITS_PER_LONG_LONG))
#define BIT_ULL_WORD(nr)    ((nr) / BITS_PER_LONG_LONG)

/* 32位处理器 */
#define BITS_PER_LONG    32

#ifndef BITS_PER_LONG_LONG
#define BITS_PER_LONG_LONG    64
#endif

 #define GENMASK(h, l) \
     (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

 #define GENMASK_ULL(h, l) \
     (((~0ULL) - (1ULL << (l)) + 1) & \
      (~0ULL >> (BITS_PER_LONG_LONG - 1 - (h))))

/**
 * @brief 返回输入参数最好有效位,如:0x80000000:返回32;
 *                             0x00000000:返回0
 */
static inline int constant_fls(int x)
{
    int r = 32;

    if (!x)
        return 0;
    if (!(x & 0xffff0000u)) {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u)) {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u)) {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u)) {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u)) {
        x <<= 1;
        r -= 1;
    }
    return r;
}

/**
 * @brief 返回输入参数最好有效位,如:0x80000000:返回32;
 *                             0x00000000:返回0
 */
static inline int lm_fls (int x)
{
    if (__builtin_constant_p(x))
           return constant_fls(x);

    return 32 - __clz(x);
}

LM_END_EXTERN_C

#endif /* __LM_BITOPS_H */

/* end of file */
