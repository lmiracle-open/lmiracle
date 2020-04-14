#include "lmiracle.h"
#include "lm_shell.h"
#include "mini-printf.h"
#include <stdarg.h>

#include "user_mb_app.h"

#define LM_SHELL_OUT_SIZE               300             /* 标志输出缓冲区SIZE */

#define LM_SHELL_BUFF_SIZE              512

static lm_console_t __g_console;

static uint8_t recv_buf[200];

/* 定义shell实体 */
static Shell __g_shell;

/* 申请shell缓存 */
char shellBuffer[LM_SHELL_BUFF_SIZE];

/* shell写函数 */
static void lm_shell_write (const char data)
{
    if (__g_console.write) {
        __g_console.write(__g_console.com, (void *)&data, 1);
    }
}

/* shell读函数 */
static char lm_shell_read (char *data)
{
    int ret = LM_OK;

    uint16_t len;

    if (__g_console.read) {
        ret = __g_console.read(data, &len);
    }

    return (char)ret;
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
    __g_console.read = p_console->read;

    return LM_OK;
}

/* modbus 从机轮询任务 */
void lm_modbus_slave_poll (void *p_arg)
{
    lm_kprintf("modbus slave task start... \r\n");

    /* 1.初始化modbus */
    eMBInit(MB_RTU, 0x01, 1, 115200,  MB_PAR_EVEN);

    /* 2.使能modbus */
    eMBEnable();

    /* 3.轮询任务 */
    while (1) {
        eMBPoll();
    }
}

/**
 * shell初始化
 */
int lm_shell_init (void)
{
    lm_err_t ret = LM_OK;

    /* 挂载回调 */
    __g_shell.write = lm_shell_write;
    __g_shell.read = lm_shell_read;

    /* 初始化shell */
    shellInit(&__g_shell, shellBuffer, LM_SHELL_BUFF_SIZE);

    lm_task_create("lm_shell", lm_shell_run, NULL, 1024, 5);

    return ret;
}

/* end of file */
