#ifndef __LM_ASM_H
#define __LM_ASM_H


static inline unsigned int __clz(unsigned int x)
{
    unsigned int ret;

    __asm("clz\t%0, %1" : "=r"(ret) : "r"(x));

    return ret;
}








#endif /* __LM_ASM_H */
