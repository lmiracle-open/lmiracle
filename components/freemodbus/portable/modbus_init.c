#include "lmiracle.h"
#include "lm_types.h"
#include "lm_kservice.h"

#include "lm_mb_interface.h"

/* 定义modbus参数指针 */
const static lm_mb_param_t *__g_modbus = NULL;

/**
 * @brief modbus 从机轮询任务
 */
static void __mb_slave_poll_task (void *p_arg)
{
    lm_kprintf("modbus slave poll task start... \r\n");

    /* 1.初始化modbus */
    eMBInit(__g_modbus->embmode, \
            __g_modbus->slave_addr, \
            __g_modbus->uport, \
            __g_modbus->ubaud, \
            __g_modbus->embparity);

    /* 2.使能modbus */
    eMBEnable();

    /* 3.轮询modbus事件 */
    while (1) {
        eMBPoll();
    }
}

/**
 * @brief modbus 协议初始化
 */
int lm_mb_protocol_init (void)
{
    lm_err_t ret = LM_OK;
    lm_base_t iReturn = LM_TYPE_PASS;

    (void)iReturn;

    /* 1.参数检查  */
    lm_assert(NULL == __g_modbus);

    /* 2.创建从机轮询任务 */
    iReturn = lm_task_create(   "slave_poll",                       \
                                __mb_slave_poll_task,               \
                                NULL,                               \
                                __g_modbus->ptask_stack_size,       \
                                __g_modbus->ptask_prio);
    lm_assert(LM_TYPE_FAIL == iReturn);

    return ret;
}

/**
 * @brief modbus 参数注册接口
 */
int lm_mb_param_register (const lm_mb_param_t *mb_param)
{
    /* 1.检查输入参数是否有效 */
    lm_assert(NULL == mb_param);

    /* 2.注册modbus参数 */
    __g_modbus = mb_param;

    return LM_OK;
}

/**
 * @brief modbus线圈寄存器回调函数(需要外部实现)
 */
eMBErrorCode __default eMBRegCoilsCB(       UCHAR * pucRegBuffer, \
                                            USHORT usAddress,\
                                            USHORT usNCoils, \
                                            eMBRegisterMode eMode)
{
    return MB_ENOERR;
}

/**
 * @brief modbus离散寄存器回调函数(需要外部实现)
 */
eMBErrorCode __default eMBRegDiscreteCB(    UCHAR * pucRegBuffer, \
                                            USHORT usAddress,\
                                            USHORT usNDiscrete, \
                                            eMBRegisterMode eMode)
{
    return MB_ENOERR;
}

/**
 * @brief modbus保持寄存器回调函数(需要外部实现)
 */
eMBErrorCode __default eMBRegHoldingCB(     UCHAR * pucRegBuffer, \
                                            USHORT usAddress,\
                                            USHORT usNRegs, \
                                            eMBRegisterMode eMode)
{
    return MB_ENOERR;
}

/**
 * @brief modbus输入寄存器回调函数(需要外部实现)
 */
eMBErrorCode __default eMBRegInputCB(       UCHAR * pucRegBuffer, \
                                            USHORT usAddress,\
                                            USHORT usNRegs, \
                                            eMBRegisterMode eMode)
{
    return MB_ENOERR;
}

/* end of file */
