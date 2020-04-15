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
#include "lm_modbus.h"
#include "lmiracle.h"

/* ----------------------- Defines ------------------------------------------*/
/* serial transmit event */
#define EVENT_SERIAL_TRANS_START                        (1<<0)

/* ----------------------- static functions ---------------------------------*/

/* 定义mb串口指针 */
const static lm_mb_serial_t *lm_mb_serial = NULL;

/* 定义串口事件实体 */
static lm_event_t event_serial = NULL;

/* ----------------------- static functions ---------------------------------*/

static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);

static void lm_modbus_slave_trans_task (void* parameter);

/* ----------------------- Start implementation -----------------------------*/

/**
 * 串口注册接口
 */
int lm_modbus_serial_register (const lm_mb_serial_t *mb_serial)
{
    /* 1.检查输入参数是否有效 */
    lm_assert(NULL == mb_serial);

    /* 2.注册 */
    lm_mb_serial = mb_serial;

    return LM_OK;
}

/**
 * 初始化串口
 */
BOOL xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
        eMBParity eParity)
{
    /* 1.初始化底层驱动 */
    if (lm_mb_serial->serial_init) {
        lm_mb_serial->serial_init(ucPORT, ulBaudRate, ucDataBits, eParity);
    }

    /* 2.初始化串口事件 */
    event_serial = lm_event_create();           /* 创建事件标志组 */
    if (unlikely(NULL == event_serial)) {
        return FALSE;
    }

    /* 3.创建从机事件任务 */
    lm_task_create("slave_event", lm_modbus_slave_trans_task, NULL, \
                                  lm_mb_serial->stack_size, lm_mb_serial->prio);

    return TRUE;
}

/**
 * 使能串口
 */
void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    /* 1.检查输入参数是否有效 */
    if (unlikely(NULL == lm_mb_serial)) {
        return ;
    }

//    /* 2.使能或失能中断 */
//    lm_mb_serial->serial_irq_enable(lm_mb_serial->com, xRxEnable);

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
    if (lm_mb_serial->serial_write) {
        lm_mb_serial->serial_write(lm_mb_serial->com, &ucByte, 1);
    }

    return TRUE;
}

/**
 * 串口接收
 */
BOOL xMBPortSerialGetByte(CHAR * pucByte)
{
    /* 1.接收数据 */
    if (lm_mb_serial->serial_read) {
        lm_mb_serial->serial_read(lm_mb_serial->com, (uint8_t *)pucByte);
    }

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
 * 串口接收完成回调
 */
int lm_serial_recv_cb (uint8_t *data, uint16_t len)
{
    (void)data;
    (void)len;

    prvvUARTRxISR();

    return LM_OK;
}

/**
 * Software simulation serial transmit IRQ handler.
 *
 * @param parameter parameter
 */
static void lm_modbus_slave_trans_task (void* parameter)
{
    while (1) {
        /* waiting for serial transmit start */
        lm_event_wait(  event_serial, \
                        EVENT_SERIAL_TRANS_START, \
                        pdTRUE, \
                        pdFALSE, \
                        LM_SEM_WAIT_FOREVER);
        /* execute modbus callback */
        prvvUARTTxReadyISR();

//        lm_task_delay(1);
    }
}
