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
 * File: $Id: portevent.c,v 1.60 2013/08/13 15:07:05 Armink $
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "lmiracle.h"

/* 定义事件实体 */
static lm_devent_t     slave_event = NULL;

BOOL
xMBPortEventInit( void )
{
    slave_event = lm_event_create();            /* 创建事件标志组 */

    /* 1.检查输入参数是否有效 */
    if (unlikely(NULL == slave_event)) {
        return FALSE;
    }

    return TRUE;
}

BOOL
xMBPortEventPost( eMBEventType eEvent )
{
    lm_assert(NULL == slave_event);

    lm_event_set(slave_event, eEvent);          /* 设置事件位 */

    return TRUE;
}

BOOL
xMBPortEventGet( eMBEventType * eEvent )
{
    lm_bits_t recv_event;

    lm_assert(NULL == slave_event);

    /* waiting forever OS event */
    recv_event = lm_event_wait(  slave_event,
                    EV_READY | EV_FRAME_RECEIVED | EV_EXECUTE | EV_FRAME_SENT,
                    pdTRUE,     /* 退出时清除事件位 */
                    pdFALSE,    /* 以上事件只要满足其一就可以 */
                    LM_SEM_WAIT_FOREVER);

    switch (recv_event) {
    case EV_READY:
        *eEvent = EV_READY;
        break;
    case EV_FRAME_RECEIVED:
        *eEvent = EV_FRAME_RECEIVED;
        break;
    case EV_EXECUTE:
        *eEvent = EV_EXECUTE;
        break;
    case EV_FRAME_SENT:
        *eEvent = EV_FRAME_SENT;
        break;
    }

    return TRUE;
}
