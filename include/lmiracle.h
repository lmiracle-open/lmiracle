#ifndef __LMIRACLE_H
#define __LMIRACLE_H

#include "FreeRTOS.h"
#include "task.h"
#include "lm_error.h"

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
#define lm_scheduler_start()             vTaskStartScheduler()

/**
 * @brief 任务延时
 *
 * @param[tick] tick
 */
#define lm_task_delay(tick)     vTaskDelay(tick);

typedef TickType_t lm_tick_t ;

/**
 * @brief 获取当前的tick值
 *
 * @return [lm_tick_t] 返回tick值
 */
#define lm_tick_get()                xTaskGetTickCount()

/**
 * @brief 互斥锁类型
 */
typedef  void* lm_mutex_t;

/**
 * @brief 创建互斥锁
 *
 * @return [lm_mutex_t]
 */
#define lm_mutex_create()                xSemaphoreCreateMutex()

/**
 * @brief 上锁
 */
#define lm_mutex_lock(mutex, time)       xSemaphoreTake(mutex, time)

/**
 * @brief 解锁
 */
#define lm_mutex_unlock(mutex)           xSemaphoreGive(mutex)

/**
 * @brief 信号量类型
 */
typedef void* lm_sem_t;

/**
 * @brief 创建信号量
 *
 * @return [lm_sem_t]
 */
#define lm_sem_create(count, value)     xSemaphoreCreateCounting(count, value)


/**
 * @brief 释放信号量
 */
#define lm_sem_give(sem)                 xSemaphoreGive(sem)

/**
 * @brief 获取信号量
 */
#define lm_sem_take(sem, timeout)        xSemaphoreTake(sem, timeout)


/**
 * @brief 进入临界区
 */
#define lm_critical_enter()        vPortEnterCritical()

/**
 * @brief 退出临界区
 */
#define lm_critical_exit()        vPortExitCritical()



#endif /* __LMIRACLE_H */

/* end of file */

