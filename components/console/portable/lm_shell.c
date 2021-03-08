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

#include "lm_kservice.h"
#include "lm_ulog.h"

/* 控制台参数指针定义 */
static lm_console_t *__gp_console_info = NULL;

/* 控制台设备结构体定义 */
static lm_shell_t __g_shell_dev = {0};

/* 控制台应用层回调函数定义 */
static  shell_data_handle_t __g_shell_data_handle_cb = NULL;

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

    while (1) {
        /* 1. 读取串口数据 */
        len = lm_serial_read(   __gp_console_info->com,         \
                                __gp_console_info->recv_buf,    \
                                __gp_console_info->recv_size);
        /* 2. 处理数据 */
        if (len > 0) {
            /* 2.1 处理应用层业务 */
            if (__g_shell_data_handle_cb) {
                __g_shell_data_handle_cb(__gp_console_info->recv_buf, len);
            }

            /* 2.2 将接收的数据给shell后台处理 */
            for (int i = 0; i < len; i++) {
                        __shell_handler(&__g_shell_dev, \
                                        (char)__gp_console_info->recv_buf[i]);
            }
        }
    }
}

/******************************************************************************/

/**
 * @brief 控制台接口注册
 */
int lm_shell_register (lm_console_t *p_cole)
{
    struct lm_serial_info serial_info;

    /* 1. 检查输入参数是否有效 */
    lm_assert(NULL != p_cole);

    /* 2. 配置串口 */
    lm_serial_get_info(p_cole->com, &serial_info);
    serial_info.config.baud_rate = p_cole->baud_rate;
    serial_info.idle_timeout = 10;
    serial_info.read_timeout = 0xFFFFFFFF;                /* 读阻塞 */
    lm_serial_set_info(p_cole->com, (const struct lm_serial_info *)&serial_info);

    /* 3. 注册  */
    __gp_console_info = p_cole;

    return LM_OK;
}

/**
 * @brief shell 应用层数据通知(应用层注册)
 */
int lm_shell_data_notice (shell_data_handle_t shell_data_cb)
{
    /* 1. 注册回调应用层shell数据处理回调 */
    __g_shell_data_handle_cb = shell_data_cb;

    return LM_OK;
}

/**
 * @brief shell 控制台初始化
 */
int lm_shell_init (void)
{
    int ret = LM_OK;

    /* 1. 检查输入参数是否有效 */
    lm_assert(NULL != __gp_console_info);

    /* 2. 挂载发送回调 */
    __g_shell_dev.write = __shell_data_write;

    /* 3. 设置格式化输出参数 */
    lm_stdout_param_set(__gp_console_info->com, \
                        __gp_console_info->cole_out_buf, \
                        __gp_console_info->cole_s);

    /* 4. 设置日志输出参数 */
    lm_stdout_param_set(__gp_console_info->com, \
                        __gp_console_info->ulog_out_buf, \
                        __gp_console_info->ulog_s);

    /* 5. 初始化shell模块 */
    __shell_init(   &__g_shell_dev,                 \
                    __gp_console_info->cole_buf,    \
                    __gp_console_info->cole_s);

    /* 6. 创建shell任务 */
    ret = lm_task_create(   "shell",                            \
                            __shell_run_task,                   \
                            __gp_console_info->stack_size,      \
                            __gp_console_info->prio,            \
                            NULL);
    lm_assert(LM_TYPE_FAIL != (lm_base_t)ret);

    return ret;
}

/* end of file */
