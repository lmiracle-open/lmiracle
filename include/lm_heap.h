/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_heap.h
* Change Logs   :
* Date         Author      Notes
* 2019-06-07   terryall    V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 内存管理
*******************************************************************************/

#ifndef __LM_HEAP_H
#define __LM_HEAP_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus  */

#include "FreeRTOS.h"

/**
 * @brief 申请动态内存
 * @param 申请内存字节数
 * @return 成功：内存指针  失败：NULL
 */
#define lm_mem_alloc(size)      pvPortMalloc(size)

/**
 * @brief 释放动态内存
 * @param 内存指针
 */
#define lm_mem_free(pv)                                                        \
    do {                                                                       \
        if (NULL != pv) {                                                      \
            vPortFree(pv);                                                     \
            pv = NULL;                                                         \
        }                                                                      \
    } while(0)

/**
 * @brief 获取当前空闲的堆空间
 * @return 剩余空间值
 */
#define lm_mem_freesize_get()   xPortGetFreeHeapSize()

#ifdef __cplusplus
}
#endif  /* __cplusplus  */

#endif  /* __LM_HEAP_H */

/* end of file */
