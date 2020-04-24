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

#include "user_mb_port.h"

/* ----------------------- static functions ---------------------------------*/

/* 定义mb定时器指针 */
const static lm_mb_timer_t *mb_timer = NULL;

/* ----------------------- Start implementation -----------------------------*/

/**
 * 定时器底层注册接口
 */
int mb_hw_timer_register (const lm_mb_timer_t *p_mb)
{
    /* 1.检查输入参数是否有效 */
    lm_assert(NULL == p_mb);

    /* 2.注册 */
    mb_timer = p_mb;

    return LM_OK;
}

/**
 * 初始化定时器
 */
BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
    /**
     * 1.计算定时中断时间
     * [1] 定时器以50us基数
     * [2] modbus协议规定报文帧时长至少3.5个字符间隔
     * [3] 串口发送1个字符需要传输 1(起始位)+8(数据位)+1(校验位)+1(停止位)=11位
     * [4] 发送3.5个字符需要 3.5*11=38.5位
     * [5] if 波特率=9600 则1s传输9600位  传输1位需要大约 1/9600=0.104ms
     * [6] 传输38.5位大约需要 38.5*0.104=4.01ms 这个时间就是定时器中断时间 要比这个稍微大一点
     * [7] 以50us为计数周期 则4ms/50us=80就会溢出中断
     * [8] 在9600的波特率下，usTim1Timerout50us = 80
     * [9] 通用公式 timeout = (usTim1Timerout50us + 10) / 20 = 4.5ms 刚好比4ms大一点
     *
     */
    uint32_t timeout = (usTim1Timerout50us + 10) / 20 + 3;

    /* 2.初始化定时器 */
    if (mb_timer->timer_init) {
        mb_timer->timer_init(mb_timer->timer_id, timeout);
        return TRUE;
    }

    return FALSE;
}

/**
 * 启动定时器
 */
void vMBPortTimersEnable()
{
    if (mb_timer->timer_sleep) {
        mb_timer->timer_sleep(mb_timer->timer_id, true);
    }
}

/**
 * 停止定时器
 */
void vMBPortTimersDisable()
{
    if (mb_timer->timer_sleep) {
        mb_timer->timer_sleep(mb_timer->timer_id, false);
    }
}

/**
 * 定时器超时处理 中断中调用
 */
int mb_timer_expired_cb (void)
{
    (void) pxMBPortCBTimerExpired();

    return LM_OK;
}
