/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_io.h
* Change Logs   :
* Date          Author          Notes
* 2019-11-06    linxuew         V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : IO操作相关接口
*******************************************************************************/

#ifndef __LM_IO_H
#define __LM_IO_H

#include "lm_types.h"

LM_BEGIN_EXTERN_C

#define CONFIG_ARM_DMA_MEM_BUFFERABLE

/* IO barriers */
#ifdef CONFIG_ARM_DMA_MEM_BUFFERABLE
#include "lm_barrier.h"
#define __iormb()       rmb()
#define __iowmb()       wmb()
#else
#define __iormb()       do { } while (0)
#define __iowmb()       do { } while (0)
#endif

#ifndef __iomem
#define __iomem
#endif

#ifndef __force
#define __force
#endif

static inline void __raw_writew(uint16_t val, volatile void __iomem *addr)
{
    asm volatile("strh %1, %0"
             : : "Q" (*(volatile uint16_t __force *)addr), "r" (val));
}

static inline uint16_t __raw_readw(const volatile void __iomem *addr)
{
    uint16_t val;
    asm volatile("ldrh %0, %1"
             : "=r" (val)
             : "Q" (*(volatile uint16_t __force *)addr));
    return val;
}

static inline void __raw_writeb(uint8_t val, volatile void __iomem *addr)
{
    asm volatile("strb %1, %0"
              : :    "Qo" (*(volatile uint8_t __force *)addr), "r" (val));
}

static inline void __raw_writel(uint32_t val, volatile void __iomem *addr)
{
    asm volatile("str %1, %0"
             : : "Qo" (*(volatile uint32_t __force *)addr), "r" (val));
}

static inline uint8_t __raw_readb(const volatile void __iomem *addr)
{
    uint8_t val;
    asm volatile("ldrb %0, %1"
             : "=r" (val)
             : "Qo" (*(volatile uint8_t __force *)addr));
    return val;
}

static inline uint32_t __raw_readl(const volatile void __iomem *addr)
{
    uint32_t val;
    asm volatile("ldr %0, %1"
             : "=r" (val)
             : "Qo" (*(volatile uint32_t __force *)addr));
    return val;
}

#define readb_relaxed(c) ({ uint8_t  __r = __raw_readb(c); __r; })
#define readw_relaxed(c) ({ uint16_t __r = (__force uint16_t) \
                    __raw_readw(c); __r; })
#define readl_relaxed(c) ({ uint32_t __r = (__force uint32_t) \
                    __raw_readl(c); __r; })

#define writeb_relaxed(v,c) __raw_writeb(v,c)
#define writew_relaxed(v,c) __raw_writew((__force uint16_t)v, c)
#define writel_relaxed(v,c) __raw_writel((__force uint32_t)v, c)

#define readb(c)        ({ uint8_t  __v = readb_relaxed(c); __iormb(); __v; })
#define readw(c)        ({ uint16_t __v = readw_relaxed(c); __iormb(); __v; })
#define readl(c)        ({ uint32_t __v = readl_relaxed(c); __iormb(); __v; })

#define writeb(v,c)     ({ __iowmb(); writeb_relaxed(v,c); })
#define writew(v,c)     ({ __iowmb(); writew_relaxed(v,c); })
#define writel(v,c)     ({ __iowmb(); writel_relaxed(v,c); })

#define readsb(p,d,l)       __raw_readsb(p,d,l)
#define readsw(p,d,l)       __raw_readsw(p,d,l)
#define readsl(p,d,l)       __raw_readsl(p,d,l)

#define writesb(p,d,l)      __raw_writesb(p,d,l)
#define writesw(p,d,l)      __raw_writesw(p,d,l)
#define writesl(p,d,l)      __raw_writesl(p,d,l)

LM_END_EXTERN_C

#endif /* __LM_IO_H */

/* end of file */
