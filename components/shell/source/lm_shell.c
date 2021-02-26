#include "lmiracle.h"
#include "lm_shell.h"
#include "lm_serial.h"
#include "mini-printf.h"
#include <stdarg.h>
#include <string.h>


/* 定义控制台设备指针 */
static lm_console_t *p_console = NULL;

/* 回调 */
static  iap_upgrade_t p_iap_upgrade = NULL;

/* 定义控制台实体 */
static Shell g_console = {0};

/**
 * 打印输出
 */
void lm_kprintf(const char *fmt, ...)
{
    va_list va;

    /* 1.检查输入参数是否有效 */
    if (unlikely(NULL == p_console)) {
        return ;
    }

    /* 2.格式化数据 */
    va_start(va, fmt);
    mini_vsnprintf((void *)p_console->cole_out_buf, p_console->out_s, fmt, va);

    /* 3.向串口输出数据 */
    lm_serial_write(p_console->com, \
                    p_console->cole_out_buf, \
                    strlen((void *)p_console->cole_out_buf));

    /* 5.结束 */
    va_end(va);
}

/**
 * 控制台写数据接口
 */
static void lm_console_data_write (const char data)
{
    lm_serial_write(p_console->com, (void *)&data, 1);
}

/**
 * shell任务
 */
static void lm_shell_run (void *p_arg)
{
    lm_kprintf("shell task start... \r\n");

    while (1) {
        /* 1.读取串口数据 */
        uint32_t len = lm_serial_read(p_console->com,
                                      p_console->recv_buf,
                                      p_console->recv_size);
        
        if (len > 0) {
            /* 2.处理上位机请求*/
            if (p_iap_upgrade) {
                p_iap_upgrade(p_console->recv_buf, len);
            }

            /* 3.处理接收的数据 */
            for (int i = 0; i < len; i++) {
                shellHandler(&g_console, (char)p_console->recv_buf[i]);
            }
        }
    }
}

/**
 * 控制台接口注册
 */
int lm_console_register(lm_console_t *p_cole)
{
    struct lm_serial_info serial_info;

    /* 1.检查输入参数是否有效 */
    if (unlikely(NULL == p_cole)) {
        return LM_ERROR;
    }

    /* 2.配置串口 */
    lm_serial_get_info(p_cole->com, &serial_info);
    serial_info.config.baud_rate = p_cole->baud_rate;
    serial_info.idle_timeout = 10;
    serial_info.read_timeout = 0xFFFFFFFF;                /* 读阻塞 */
    lm_serial_set_info(p_cole->com, (const struct lm_serial_info *)&serial_info);

    /* 3.注册 */
    p_console = p_cole;

    return LM_OK;
}

int lm_iap_register (iap_upgrade_t iap_cb)
{
    /* 1.注册回调*/
    p_iap_upgrade = iap_cb;

    return LM_OK;
}

/**
 * shell初始化
 */
int lm_shell_init (void)
{
    /* 1.检查输入参数是否有效 */
    if (unlikely(NULL == p_console)) {
        return LM_ERROR;
    }

    /* 2.挂载发送回调 */
    g_console.write = lm_console_data_write;

    /* 3.初始化shell */
    shellInit(&g_console, p_console->cole_buf, p_console->cole_s);

    /* 4.创建shell任务 */
    lm_task_create("lm_shell", lm_shell_run, NULL, \
                    p_console->stack_size, p_console->prio);

    return LM_OK;
}

/* end of file */
