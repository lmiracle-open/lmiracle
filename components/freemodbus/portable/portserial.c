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
 * File: $Id: portserial.c,v 1.60 2013/08/13 15:07:05 Armink $
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

#include "lm_serial.h"          /* 串口框架层接口 */
#include "lm_mb_interface.h"

/* ----------------------- Defines ------------------------------------------*/
/* 定义modbus串口传输事件 */
#define __MB_SERIAL_TRANS_EVENT           (1<<0)

#define __MB_SERIAL_STASK_PRIO            6           /* 串口发送任务优先级 */
#define __MB_SERIAL_STASK_STACK           256         /* 串口发送任务栈深度 */

#define __MB_SERIAL_RTASK_PRIO            6           /* 串口接收任务优先级 */
#define __MB_SERIAL_RTASK_STACK           256         /* 串口接收任务栈深度 */

/* ----------------------- static functions ---------------------------------*/

/* 定义modbus串口指针 */
const static lm_mb_serial_t *__gp_mb_serial         = NULL;

/* 定义modbus串口事件实体 */
static lm_devent_t          __gp_mb_serial_event    = NULL;

/******************************************************************************/

/*
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR(void)
{
    pxMBFrameCBTransmitterEmpty();
}

/*
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR(void)
{
    pxMBFrameCBByteReceived();
}

/******************************************************************************/

/**
 * @brief modbus串口传输数据任务
 */
static void __mb_serial_trans_task (void* parameter)
{
    while (1) {
        /* 等待modbus串口传输开始事件 */
        lm_event_wait(  __gp_mb_serial_event, \
                        __MB_SERIAL_TRANS_EVENT, \
                        LM_TYPE_FALSE, \
                        LM_TYPE_FALSE, \
                        LM_SEM_WAIT_FOREVER);
        /* 执行modbus传输数据回调函数 */
        prvvUARTTxReadyISR();
    }
}

/**
 * @brief modbus串口读取数据任务
 */
static void __mb_serial_read_task (void* parameter)
{
    while (1) {
        /* 内部阻塞读取 */
        prvvUARTRxISR();
    }
}

/******************************************************************************/

/**
 * @brief 使能modbus串口
 */
void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    /* 1. 是否使能串口发送 */
    if (xTxEnable) {
        /* 发送串口开始传输事件 */
        lm_event_set(__gp_mb_serial_event, __MB_SERIAL_TRANS_EVENT);
    } else {
        /* 停止串口传输  */
        lm_event_wait(  __gp_mb_serial_event, \
                        __MB_SERIAL_TRANS_EVENT, \
                        LM_TYPE_TRUE, \
                        LM_TYPE_FALSE, \
                        0);
    }
}

/**
 * @brief 关闭modbus串口
 */
void vMBPortClose(void)
{
    /* empty */
    return ;
}
/**
 * @brief modbus串口发送数据 1byte
 */
BOOL xMBPortSerialPutByte(CHAR ucByte)
{
    /* 1. 发送数据 */
    return lm_serial_write(__gp_mb_serial->com, (const void *)&ucByte, 1);
}

/**
 * @brief modbus串口接收数据 1byte
 */
BOOL xMBPortSerialGetByte(CHAR * pucByte )
{
    /* 1. 接收数据 */
    return lm_serial_read(__gp_mb_serial->com, (uint8_t *)pucByte, 1);
}

/**
 * @brief modbus串口接收数据 nbyte
 */
uint32_t xMBPortSerialGetBuff(CHAR * ucBuff,uint32_t len)
{
    /* 1. 接收数据 */
    return lm_serial_read(__gp_mb_serial->com, ucBuff, len);
}

/**
 * @brief modbus串口发送数据 nbyte
 */
BOOL xMBPortSerialPutBuff (CHAR * ucBuff, uint32_t len)
{
    /* 1. 发送数据 */
    return lm_serial_write(__gp_mb_serial->com, ucBuff, len);
}

/**
 * @brief modbus获取串口传输类型
 */
void *get_serial_transmit_type (void)
{
    return (void *)&__gp_mb_serial->transmit_type;
}

/**
 * @brief modbus串口初始化
 */
BOOL xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
        eMBParity eParity)
{
    int ret = TRUE;

    struct lm_serial_info serial_info;

    /* 1.获取串口默认信息 */
    lm_serial_get_info(ucPORT, &serial_info);

    /* 2.设置modbus串口参数 */

    serial_info.config.transmit_type = __gp_mb_serial->transmit_type ;
    serial_info.config.baud_rate = ulBaudRate;
    serial_info.config.data_bits = ucDataBits;
    serial_info.config.parity = eParity;
    serial_info.config.fast_rect = 1;
    serial_info.idle_timeout = 0xFFFFFFFF;
    serial_info.read_timeout = 0xFFFFFFFF;                  /* 读阻塞 */
    lm_serial_set_info(ucPORT, (const struct lm_serial_info *)&serial_info);

    /* 3.创建modbus串口传输事件 */
    __gp_mb_serial_event = lm_event_create();
    lm_assert(NULL != __gp_mb_serial_event);

    /* 4.创建modbus从机串口发送数据任务 */
    ret = lm_task_create(   "serial_trans",                     \
                            __mb_serial_trans_task,             \
                            __MB_SERIAL_STASK_STACK,            \
                            __MB_SERIAL_STASK_PRIO,             \
                            NULL);
    lm_assert(LM_TYPE_FAIL != (lm_base_t)ret);

    /* 5.创建modbus从机串口读取数据任务 */
    ret = lm_task_create(   "serial_recv",                      \
                            __mb_serial_read_task,              \
                            __MB_SERIAL_RTASK_STACK,            \
                            __MB_SERIAL_RTASK_PRIO,             \
                            NULL);
    lm_assert(LM_TYPE_FAIL != (lm_base_t)ret);

    return ret;
}

/******************************************************************************/

/**
 * @brief modbus串口底层接口注册
 */
int lm_modbus_serial_register (const lm_mb_serial_t *p_mb_serial)
{
    /* 1.检查输入参数是否有效 */
    lm_assert(NULL != p_mb_serial);

    /* 2.注册 */
    __gp_mb_serial = p_mb_serial;

    return LM_OK;
}

/* end of file */
