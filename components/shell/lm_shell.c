#include "lmiracle.h"
#include "lm_shell.h"
#include "mini-printf.h"
#include <stdarg.h>

#define LM_SHELL_OUT_SIZE               300             /* 标志输出缓冲区SIZE */

#define LM_SHELL_BUFF_SIZE              512

static lm_console_t __g_console;

static uint8_t recv_buf[200] = {0};

static uint16_t recv_len = 0;

/* 定义信号量 */
static SemaphoreHandle_t console_sem;

/* 定义shell实体 */
static Shell __g_shell;

/* 申请shell缓存 */
char shellBuffer[LM_SHELL_BUFF_SIZE];

/* shell写函数 */
static void lm_console_data_write (const char data)
{
    if (__g_console.write) {
        __g_console.write(__g_console.com, (void *)&data, 1);
    }
}

/* shell读函数 */
int lm_console_data_read (uint8_t *data, uint16_t len)
{
    if (NULL == data) {
        return LM_ERROR;
    }

    if (len > 0) {
        /* copy data */
        memcpy(recv_buf, data, len);
        recv_len = len;
        /* 释放信号量 */
        lm_sem_give(console_sem);
    }

    return LM_OK;
}

void lm_console_output(const char *data)
{
    /* 空函数 */
}

/* 打印输出 */
void lm_kprintf(const char *fmt, ...)
{
    va_list va;

    uint8_t buf[LM_SHELL_OUT_SIZE] = {0};

    /* 格式化数据 */
    va_start(va, fmt);
    mini_vsnprintf((void *)buf, LM_SHELL_OUT_SIZE, fmt, va);

    /* 输出数据 */
    if (__g_console.write) {
        __g_console.write(__g_console.com, buf, strlen((void *)buf));
    } else {
        lm_console_output((char *)buf);
    }

    va_end(va);
}

/* 控制台输出接口注册函数 */
int lm_console_register(lm_console_t *p_console)
{
    if (NULL == p_console) {
        return LM_ERROR;
    }

    __g_console.com = p_console->com;
    __g_console.write = p_console->write;

    return LM_OK;
}

/* todo 调整了堆大小 20*1024   configTOTAL_HEAP_SIZE  */

void lm_shell_run (void *p_arg)
{
    char data;
    uint16_t len;
    BaseType_t err = pdFALSE;

//    lm_kprintf("shell task start... \r\n");

    while (1) {
        if (console_sem) {
            err = lm_sem_take(console_sem, 1000);
            if (pdTRUE == err) {
                for (int i = 0; i < len; i++) {
                    shellHandler(&__g_shell, (char )recv_buf[i]);
                }
            }
        }
        lm_task_delay(10);
    }
}

/**
 * shell初始化
 */
int lm_shell_init (void)
{
    lm_err_t ret = LM_OK;

    /* 挂载回调 */
    __g_shell.write = lm_console_data_write;

    /* 初始化shell */
    shellInit(&__g_shell, shellBuffer, LM_SHELL_BUFF_SIZE);

    /* 初始化信号量 */
    console_sem = lm_sem_create_binary();

    lm_task_create("lm_shell", lm_shell_run, NULL, 1024, 5);

    return ret;
}

/* end of file */
