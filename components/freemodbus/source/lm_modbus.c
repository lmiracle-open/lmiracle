#include "lmiracle.h"
#include "lm_shell.h"
#include "mini-printf.h"
#include <stdarg.h>

#include "lm_kservice.h"
#include "user_mb_app.h"

#include "lm_modbus.h"

/* 定义mb参数指针 */
const static lm_mb_param_t *lm_mb_param = NULL;

/**
 * modbus参数注册接口
 */
int lm_modbus_param_register (const lm_mb_param_t *mb_param)
{
    /* 1.检查输入参数是否有效 */
    if (unlikely(NULL == mb_param)) {
        return LM_ERROR;
    }

    /* 2.注册 */
    lm_mb_param = mb_param;

    return LM_OK;
}

/**
 * modbus 从机发送任务 (测试用)
 */

extern USHORT usSRegHoldBuf[S_REG_HOLDING_NREGS]; /* 存储保持寄存器的数组 */

static void lm_modbus_slave_send (void *p_arg)
{
    lm_assert(NULL == lm_mb_param);

    lm_kprintf("modbus slave send task start... \r\n");

    USHORT  *usRegHoldingBuf = usSRegHoldBuf;
    int i = 0;

    /* 3.轮询任务 */
    while (1) {

        ENTER_CRITICAL_SECTION();

        usRegHoldingBuf[3] = (USHORT)(i++);/* 改变保持寄存器 3 的数据 */

        EXIT_CRITICAL_SECTION();

        if (i == 10000) {
            i = 0;
        }

        lm_task_delay(2000);
    }
}

/**
 * modbus 从机轮询任务
 */
static void lm_modbus_slave_poll (void *p_arg)
{
    lm_kprintf("modbus slave task start... \r\n");

    lm_assert(NULL == lm_mb_param);

    /* 1.初始化modbus */
    eMBInit(lm_mb_param->embmode, \
            lm_mb_param->slave_addr, \
            lm_mb_param->uport, \
            lm_mb_param->ubaud,  \
            lm_mb_param->embparity);

    /* 2.使能modbus */
    eMBEnable();

    /* 3.轮询任务 */
    while (1) {
        eMBPoll();
        lm_task_delay(200);
    }
}

/**
 * modbus初始化
 */
int lm_modbus_init (void)
{
    lm_err_t ret = LM_OK;

    lm_assert(NULL == lm_mb_param);

    lm_task_create("modbus_slave", \
                    lm_modbus_slave_poll, \
                    NULL, \
                    lm_mb_param->stack_size, \
                    lm_mb_param->prio);

    /* 测试用 */
    lm_task_create("slave_send", \
                    lm_modbus_slave_send, \
                    NULL, \
                    lm_mb_param->stack_size, \
                    lm_mb_param->prio);

    return ret;
}

/* end of file */
