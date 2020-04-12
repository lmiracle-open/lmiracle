#ifndef __LM_BARRIER_H
#define __LM_BARRIER_H

#define sev()   __asm__ __volatile__ ("sev" : : : "memory")
#define wfe()   __asm__ __volatile__ ("wfe" : : : "memory")
#define wfi()   __asm__ __volatile__ ("wfi" : : : "memory")


#define isb(option) __asm__ __volatile__ ("isb " #option : : : "memory")
#define dsb(option) __asm__ __volatile__ ("dsb " #option : : : "memory")
#define dmb(option) __asm__ __volatile__ ("dmb " #option : : : "memory")


/* The "volatile" is due to gcc bugs */
#define barrier() __asm__ __volatile__("": : :"memory")


#if defined(CONFIG_ARM_DMA_MEM_BUFFERABLE) || defined(CONFIG_SMP)
#define mb()        dsb()
#define rmb()       dsb()
#define wmb()       dsb()
#define dma_rmb()   dmb(osh)
#define dma_wmb()   dmb(oshst)
#else
#define mb()        barrier()
#define rmb()       barrier()
#define wmb()       barrier()
#define dma_rmb()   barrier()
#define dma_wmb()   barrier()
#endif


#endif /* __LM_BARRIER_H */

/* end of file */
