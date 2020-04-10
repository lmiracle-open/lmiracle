/*
 * FreeModbus Libary: RT-Thread Port
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

/* ----------------------- static functions ---------------------------------*/

lm_modbus_timer_t *__lm_timer = NULL;

/* 定时器注册接口 */
int lm_modbus_timer_register (lm_modbus_timer_t *modbus_timer)
{
    int err = LM_OK;

    if (NULL == modbus_timer) {
        return LM_ERROR;
    }

    if (NULL == __lm_timer) {
        __lm_timer = modbus_timer;
    }

    return err;
}

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
    if (__lm_timer->timer_init) {
        __lm_timer->timer_init();
        return TRUE;
    }

    return FALSE;
}

void vMBPortTimersEnable()
{
    if (__lm_timer->timer_sleep) {
        __lm_timer->timer_sleep(true);
    }
}

void vMBPortTimersDisable()
{
    if (__lm_timer->timer_sleep) {
        __lm_timer->timer_sleep(false);
    }
}

/* 定时器超时处理 */
void lm_modbus_timer_timeout (void)
{
    (void) pxMBPortCBTimerExpired();
}
