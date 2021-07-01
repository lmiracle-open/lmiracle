/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_shell.c
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : shell控制台模块
*******************************************************************************/

#include "lmiracle.h"
#include "lm_serial.h"
#include "lm_shell_interface.h"
#include "mini-printf.h"

#include "lm_kservice.h"

#define __CONSOLE_TASK_PRIO            10           /* 控制台任务优先级 */
#define __CONSOLE_TASK_STACK           256          /* 控制台任务栈深度 */

/* 控制台参数指针定义 */
static lm_console_t *__gp_console_info = NULL;

/* 控制台设备结构体定义 */
static lm_shell_t __g_shell_dev = {0};

/******************************************************************************/
/**
 * @brief 控制台写数据接口(写一个字节, shell模块内部使用)
 */
static void __shell_data_write (const char data)
{
    lm_serial_write(__gp_console_info->com, (void *)&data, 1);
}

/**
 * @brief shell运行任务
 */
static void __shell_run_task (void *p_arg)
{
    uint32_t len = 0;

    /* 1. 检查输入参数是否有效 */
    lm_assert(NULL != __gp_console_info);

    while (1) {
        /* 1. 读取串口数据 */
        len = lm_serial_read(   __gp_console_info->com,      \
                                __gp_console_info->recv_buf,      \
                                __gp_console_info->recv_size);
        /* 2. 处理数据 */
        if (len > 0) {
            /* 2.1 处理应用层业务 */
            if (__gp_console_info->shell_data_cb) {
                __gp_console_info->shell_data_cb(__gp_console_info->recv_buf, len);
            }

            /* 2.2 将接收的数据给shell后台处理 */
            for (int i = 0; i < len; i++) {
                __shell_handler(&__g_shell_dev, (char)__gp_console_info->recv_buf[i]);
            }
        }
    }
}

/******************************************************************************/

/**
 * @brief shell 控制台注册
 */
int lm_shell_register (lm_console_t *p_cole)
{
    int ret = LM_OK;

    struct lm_serial_info serial_info;

    /* 1. 检查输入参数是否有效 */
    lm_assert(NULL != p_cole);

    /* 2. 注册数据结构  */
    __gp_console_info = p_cole;

    /* 3. 配置串口 */
    lm_serial_get_info(__gp_console_info->com, &serial_info);
    serial_info.config.baud_rate = __gp_console_info->baud_rate;
    serial_info.idle_timeout = 10;
    serial_info.read_timeout = 0xFFFFFFFF;                /* 读阻塞 */
    lm_serial_set_info(__gp_console_info->com, \
                                    (const struct lm_serial_info *)&serial_info);

    /* 4. 挂载发送回调 */
    __g_shell_dev.write = __shell_data_write;

    /* 5. 初始化shell模块 */
    __shell_init(   &__g_shell_dev,                 \
                    __gp_console_info->cole_buf,    \
                    __gp_console_info->cole_s);

    /* 6. 创建shell任务 */
    ret = lm_task_create(   "shell",                            \
                            __shell_run_task,                   \
                            __CONSOLE_TASK_STACK,               \
                            __CONSOLE_TASK_PRIO,                \
                            NULL);
    lm_assert(LM_TYPE_FAIL != (lm_base_t)ret);

    return ret;
}

/******************************************************************************/
/********************************** 格式化输出 ***********************************/
/******************************************************************************/

/**
 * @brief lmiracle格式化输出
 */
void lm_kprintf (const char *fmt, ...)
{
    va_list va;
    uint32_t length;

    /* 1. 检查输入参数是否有效 */
    lm_assert(NULL != __gp_console_info);

    /* 2. 开始解析参数 */
    va_start(va, fmt);
    length = mini_vsnprintf((char *)__gp_console_info->out_buf, \
            __gp_console_info->out_s - 1, fmt, va);

    if (length > __gp_console_info->out_s - 1) {
        length = __gp_console_info->out_s - 1;
    }

    /* 2. 向串口输出数据 */
    lm_serial_write(__gp_console_info->com, __gp_console_info->out_buf, length);

    /* 3. 结束参数解析 */
    va_end(va);
}

/******************************************************************************/

/* end of file */
