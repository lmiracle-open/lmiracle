/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_assert.c
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 断言模块
*******************************************************************************/

#include "lm_kservice.h"

/**
 * @brief 断言执行函数
 */
void lm_assert_msg (const char *msg)
{
    volatile int exit = 0;

    (void)lm_kprintf(msg);

    while (0 == exit) {

    }
}

/* end of file */
