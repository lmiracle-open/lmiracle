/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_sys_start.c
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 系统启动模块(系统相关初始化)
*******************************************************************************/

#include "lmiracle.h"
#include "lm_time.h"

/**
 * @brief 初始化系统相关的
 */
int lm_system_init (void)
{
    int ret = LM_OK;

    /* 0.todo:预留 */

    /* 1. 时间初始化 */
    lm_time_init();

    return ret;
}

/* end of file */
