/*
 * FreeModbus Libary: FreeRTOS Port
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: porttimer.c,v 1.60 2013/08/13 15:07:05 Armink $
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

#include "lm_mb_interface.h"

/* ----------------------- static functions ---------------------------------*/

/* 定义mb定时器指针 */
const static lm_mb_timer_t *__gp_mb_timer = NULL;

/* ----------------------- Start implementation -----------------------------*/

/**
 * @brief 初始化定时器
 */
BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
    uint32_t timeout = 0;

    /* 1. 检查输入参数是否有效 */
    lm_assert(NULL != __gp_mb_timer);

    /* 2. 计算超时时间 具体算法由用户自己实现 */
    if (__gp_mb_timer->timeout_calculation) {
        timeout = __gp_mb_timer->timeout_calculation(usTim1Timerout50us);
    } else {
        /* todo: 如果用户没有实现计算函数 则使用默认公式计算 */
        timeout = ((usTim1Timerout50us + 10) / 20 + 3);
    }

    /* 3. 定时器到期时间设置 */
    if (__gp_mb_timer->timer_expired_set) {
        __gp_mb_timer->timer_expired_set(__gp_mb_timer->timer_id, timeout);
        return TRUE;
    }

    return FALSE;
}

/**
 * @brief 启动定时器
 */
void vMBPortTimersEnable (void)
{
    if(__gp_mb_timer->timer_switch) {
        if (__gp_mb_timer->timer_sleep) {
                __gp_mb_timer->timer_sleep(__gp_mb_timer->timer_id, true);
            }
    }
}

/**
 * @brief 停止定时器
 */
void vMBPortTimersDisable (void)
{
    if(__gp_mb_timer->timer_switch) {
       if (__gp_mb_timer->timer_sleep) {
            __gp_mb_timer->timer_sleep(__gp_mb_timer->timer_id, false);
        }
    }
}

/**
 * @brief 定时器超时处理函数 底层硬件定时器中断调用
 */
int lm_modbus_timer_expired_cb (void)
{
    (void) pxMBPortCBTimerExpired();

    return LM_OK;
}

/******************************************************************************/

/**
 * @brief 定时器底层接口注册
 */
int lm_modbus_timer_register (const lm_mb_timer_t *p_mb_timer)
{
    /* 1.检查输入参数是否有效 */
    lm_assert(NULL != p_mb_timer);

    /* 2.注册 */
    __gp_mb_timer = p_mb_timer;

    return LM_OK;
}

/* end of file */
