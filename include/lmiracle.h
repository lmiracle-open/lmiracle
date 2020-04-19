#ifndef __LMIRACLE_H
#define __LMIRACLE_H

#include "FreeRTOS.h"
#include "task.h"
#include "lm_error.h"
#include "lm_io.h"
#include "lm_bitops.h"
#include "lm_types.h"
#include "osif.h"
#include "semphr.h"
#include "event_groups.h"
/**
 * @brief 创建动态任务
 *
 */
#define lm_task_create(pcName, pxTaskCode, pvParameters, usStackDepth,  uxPriority) \
                                      xTaskCreate(pxTaskCode, \
                                                  pcName, \
                                                  usStackDepth, \
                                                  pvParameters, \
                                                  uxPriority,    \
                                                  NULL )

/**
 * @brief 创建静态任务
 *
 */
#define lm_task_create_static(pcName, pxTaskCode, pvParameters, usStackDepth, uxPriority, puxStackBuffer, pxTaskBuffer) \
                                      xTaskCreateStatic(pxTaskCode, \
                                                  pcName, \
                                                  usStackDepth, \
                                                  pvParameters, \
                                                  uxPriority,    \
                                                  puxStackBuffer, \
                                                  pxTaskBuffer)

/**
 * @brief 启动调度器
 */
#define lm_scheduler_start()                vTaskStartScheduler()

/**
 * @brief 任务延时
 *
 * @param[tick] tick
 */
#define lm_task_delay(tick)                 vTaskDelay(tick);

typedef TickType_t lm_tick_t ;

/**
 * @brief 获取当前的tick值
 *
 * @return [lm_tick_t] 返回tick值
 */
#define lm_tick_get()                       xTaskGetTickCount()

/**
 * @brief 互斥锁类型
 */
typedef  mutex_t lm_mutex_t;

/**
 * @brief 创建互斥锁
 *
 * @return [lm_mutex_t]
 */
#define lm_mutex_create(p_mutex)            OSIF_MutexCreate(p_mutex)

/**
 * @brief 永久等待
 */
#define LM_SEM_WAIT_FOREVER                 OSIF_WAIT_FOREVER

/**
 * @brief 上锁
 */
#define lm_mutex_lock(p_mutex, timeout)     OSIF_MutexLock(p_mutex, timeout)

/**
 * @brief 解锁
 */
#define lm_mutex_unlock(p_mutex)            OSIF_MutexUnlock(p_mutex);

/**
 * @brief 信号量类型
 */
typedef semaphore_t lm_sem_t;


/**
 * @brief 创建计数型信号量
 *
 * @return [lm_sem_t]
 */
#define lm_sem_create(count, value)         OSIF_SemaCreate(count, value)


/**
 * @brief 释放信号量
 */
#define lm_sem_give(sem)                    OSIF_SemaPost(sem)

/**
 * @brief 获取信号量
 */
#define lm_sem_take(sem, timeout)           OSIF_SemaWait(sem, timeout)

/**
 * @brief 二值信号量类型
 */
typedef semaphore_t lm_semb_t;

/**
 * @brief 创建二值信号量
 */
#define lm_semb_create(semb)                OSIF_SembCreate(semb)

/**
 * @brief 释放二值信号量
 */
#define lm_semb_give(semb)                  OSIF_SembPost(semb)

/**
 * @brief 获取二值信号量
 */
#define lm_semb_take(semb,timeout)          OSIF_SembWait(semb, timeout)

/**
 * @brief 事件组类型
 */
typedef EventGroupHandle_t  lm_devent_t;
typedef StaticEventGroup_t  lm_sevent_t;
typedef EventBits_t         lm_bits_t;
/**
 * @brief 创建事件组
 */
#define lm_event_create()                   OSIF_EventCreate()

/**
 * @brief 创建事件组(静态)
 */
#define lm_event_create_static(event)       xEventGroupCreateStatic(event)

/**
 * @brief 设置事件组标志
 */
#define lm_event_set(event, bits)           OSIF_EventSet(event, bits)

/**
 * @brief 获取事件组标志
 */
#define lm_event_get(event)                 OSIF_EventGet(event)

/**
 * @brief 等待事件组标志
 */
#define lm_event_wait(event, bits, clearon, waitall, timeout) \
                        OSIF_EventWait(event,bits,clearon,waitall,timeout)

/**
 * @brief 清除事件组标志
 */
#define lm_event_clear(event, bits)         OSIF_EventClear(event, bits)
/**
 * @brief 进入临界区
 */
#define lm_critical_enter()                 vPortEnterCritical()


/**
 * @brief 退出临界区
 */
#define lm_critical_exit()                  vPortExitCritical()


#endif /* __LMIRACLE_H */

/* end of file */

