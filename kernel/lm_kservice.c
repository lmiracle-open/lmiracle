/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_kservice.c
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 内核相关服务模块
*******************************************************************************/

#include "lm_kservice.h"
#include "lm_serial.h"
#include "mini-printf.h"

/* todo:此处暂时先这样处理,后续再统一调整 */
static uint8_t *__gp_out_buf = NULL;            /* 格式化输出缓存地址 */
static uint16_t __g_out_len = 0;                /* 格式化输出缓存地址 */
static uint8_t __g_out_com;                     /* 格式化输出串口号 */

/**
 * @brief 格式化输出参数设置
 */
int lm_stdout_param_set (uint8_t com, uint8_t *buf, uint16_t size)
{
    __g_out_com = com;

    if (NULL == buf) {
        return -LM_ERROR;
    }

    __gp_out_buf = buf;
    __g_out_len = size;

    return LM_OK;
}

/**
 * @brief 格式化输出
 */
void lm_kprintf (const char *fmt, ...)
{
    va_list va;
    uint32_t length;

    /* 2. 开始解析参数 */
    va_start(va, fmt);
    length = mini_vsnprintf((char *)__gp_out_buf, \
            __g_out_len - 1, fmt, va);

    if (length > __g_out_len - 1) {
        length = __g_out_len - 1;
    }

    /* 2. 向串口输出数据 */
    lm_serial_write(__g_out_com, __gp_out_buf, length);

    /* 3. 结束参数解析 */
    va_end(va);
}

/* end of file */
