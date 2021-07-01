/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_heap.h
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 内存管理模块
*******************************************************************************/

#ifndef __LM_HEAP_H
#define __LM_HEAP_H

#include "FreeRTOS.h"
#include "lm_types.h"

LM_BEGIN_EXTERN_C

/**
 * @brief 申请动态内存
 *
 * @param[in]   size    申请字节数
 *
 * @return  内存指针
 */
#define lm_mem_alloc(size)      pvPortMalloc(size)

/**
 * @brief 释放动态内存
 *
 * @param[in]   pv    内存指针
 *
 * @return  None
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
 *
 * @param[in]   None
 *
 * @return  剩余空间值
 */
#define lm_mem_freesize_get()   xPortGetFreeHeapSize()

LM_END_EXTERN_C

#endif  /* __LM_HEAP_H */

/* end of file */
