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
static lm_devent_t     __gp_mb_slave_event = NULL;

/**
 * @brief modbus事件初始化
 */
BOOL
xMBPortEventInit( void )
{
    /* 1. 创建modbus事件标志组 */
    __gp_mb_slave_event = lm_event_create();

    lm_assert(NULL != __gp_mb_slave_event);

    return TRUE;
}

/**
 * @brief modbus事件发送
 */
BOOL
xMBPortEventPost( eMBEventType eEvent )
{
    /* 1. 检查输入参数是否有效 */
    if (unlikely(NULL == __gp_mb_slave_event)) {
        return FALSE;
    }

    /* 2. 设置事件位 */
    lm_event_set(__gp_mb_slave_event, eEvent);

    return TRUE;
}

/**
 * @brief modbus事件获取
 */
BOOL
xMBPortEventGet( eMBEventType * eEvent )
{
    lm_bits_t recv_event;

    /* 1. 检查输入参数是否有效 */
    if (unlikely(NULL == __gp_mb_slave_event)) {
        return FALSE;
    }

    /* 2. 等待事件发生 */
    recv_event = lm_event_wait(  __gp_mb_slave_event,
                    EV_READY | EV_FRAME_RECEIVED | EV_EXECUTE | EV_FRAME_SENT,
                    LM_TYPE_TRUE,     /* 退出时清除事件位 */
                    LM_TYPE_FALSE,    /* 以上事件只要满足其一就可以 */
                    LM_SEM_WAIT_FOREVER);
    /* 3. 分发事件 */
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

/* end of file */
