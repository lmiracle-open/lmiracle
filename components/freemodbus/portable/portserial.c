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
 * File: $Id: portserial.c,v 1.60 2013/08/13 15:07:05 Armink $
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

#include "user_mb_port.h"
#include "lm_serial.h"

/* ----------------------- Defines ------------------------------------------*/
/* serial transmit event */
#define EVENT_SERIAL_TRANS_START                        (1<<0)

/* ----------------------- static functions ---------------------------------*/

/* 定义mb串口指针 */
const static lm_mb_serial_t *mb_serial = NULL;

/* 定义串口事件实体 */
static lm_devent_t event_serial = NULL;

/* ----------------------- static functions ---------------------------------*/

static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);

static void mb_slave_trans_task (void* parameter);
static void mb_slave_read_task (void* parameter);

/* ----------------------- Start implementation -----------------------------*/

/**
 * 串口底层注册接口
 */
int mb_hw_serial_register (const lm_mb_serial_t *p_mb)
{
    /* 1.检查输入参数是否有效 */
    lm_assert(NULL == p_mb);

    /* 2.注册 */
    mb_serial = p_mb;

    return LM_OK;
}

/**
 * 初始化串口
 */
BOOL xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
        eMBParity eParity)
{
    /* 1.初始化底层驱动 */
    struct lm_serial_info serial_info;
    lm_serial_get_info(ucPORT, &serial_info);
    serial_info.config.baud_rate = ulBaudRate;
    serial_info.config.data_bits = ucDataBits;
    serial_info.config.parity = eParity;
    serial_info.config.fast_rect = 1;
    serial_info.idle_timeout = 0xFFFFFFFF;
    serial_info.read_timeout = 0xFFFFFFFF;                /* 读阻塞 */
    lm_serial_set_info(ucPORT, (const struct lm_serial_info *)&serial_info);

    /* 2.初始化串口事件 */
    event_serial = lm_event_create();           /* 创建事件标志组 */
    if (unlikely(NULL == event_serial)) {
        return FALSE;
    }

    /* 3.创建从机事件任务 */
    lm_task_create( "slave_event", \
                    mb_slave_trans_task, \
                    NULL, \
                    mb_serial->stack_size, \
                    mb_serial->prio);

    lm_task_create( "read_task", \
                    mb_slave_read_task, \
                    NULL, \
                    256, \
                    (mb_serial->prio <=1 ) ?
                              mb_serial->prio :(mb_serial->prio -1));

    return TRUE;
}

/**
 * 使能串口
 */
void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    /* 1.检查输入参数是否有效 */
    if (unlikely(NULL == mb_serial)) {
        return ;
    }

    /* 3.使能接收 */
    if (xTxEnable) {
        /* start serial transmit */
        lm_event_set(event_serial, EVENT_SERIAL_TRANS_START);
    } else {
        /* stop serial transmit */
        lm_event_wait(  event_serial, \
                        EVENT_SERIAL_TRANS_START, \
                        pdTRUE, \
                        pdFALSE, \
                        0);
    }
}

/**
 * 关闭串口设备
 */
void vMBPortClose(void)
{
    /* empty */
    return ;
}

/**
 * 串口发送
 */
BOOL xMBPortSerialPutByte(CHAR ucByte)
{
    /* 1.发送数据 */
    lm_serial_write(mb_serial->com, (const void *)&ucByte, 1);

    return TRUE;
}

/**
 * 串口接收
 */
BOOL xMBPortSerialGetByte(CHAR * pucByte)
{
    /* 1.接收数据 */
    lm_serial_read(mb_serial->com, (uint8_t *)pucByte, 1);

    return TRUE;
}

/* 
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
void prvvUARTTxReadyISR(void)
{
    pxMBFrameCBTransmitterEmpty();
}

/* 
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR(void)
{
    pxMBFrameCBByteReceived();
}

/**
 * 串口接收通知回调  中断中调用
 */
int mb_serial_recv_notice_cb (void)
{
    prvvUARTRxISR();

    return LM_OK;
}

/**
 * Software simulation serial transmit IRQ handler.
 *
 * @param parameter parameter
 */
static void mb_slave_trans_task (void* parameter)
{
    while (1) {
        /* waiting for serial transmit start */
        lm_event_wait(  event_serial, \
                        EVENT_SERIAL_TRANS_START, \
                        pdFALSE, \
                        pdFALSE, \
                        LM_SEM_WAIT_FOREVER);
        /* execute modbus callback */
        prvvUARTTxReadyISR();
    }
}


static void mb_slave_read_task (void* parameter)
{
    while (1) {
        prvvUARTRxISR();
    }
}

/* end of file */
