/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_shell_interface.h
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : shell 控制台移植接口模块
*******************************************************************************/

#ifndef __LM_SHELL_INTERFACE_H
#define __LM_SHELL_INTERFACE_H

#include "shell.h"
#include "lmiracle.h"

LM_BEGIN_EXTERN_C

/******************************************************************************/
/* shell 控制台数据类型定义 */
typedef Shell lm_shell_t;

/* shell 处理函数接口定义 */
#define __shell_handler(shell, data)        shellHandler(shell, data)

/* shell 初始化接口定义 */
#define __shell_init(shell, buffer, size)   shellInit(shell, buffer, size)

/* shell 命令导出接口定义 */
#define lm_shell_cmd_export(_name, _func, _desc) \
                    SHELL_EXPORT_CMD(   SHELL_CMD_PERMISSION(0) | \
                                        SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), \
                                        _name, _func, _desc)
/* shell 用户名定义 */
#define LM_SHELL_USER_NAME                  "lmiracle"

/* shell 获取系统时间接口定义 */
#define LM_SYS_TIME_GET()                   lm_sys_get_tick()

/******************************************************************************/
/**
 * @brief shell 控制台参数配置结构体
 */
typedef struct lm_console {
    uint32_t com;                           /* 控制台串口号 */
    uint32_t baud_rate;                     /* 波特率 */
    char *cole_buf;                         /* 控制台所用缓存 shell内部使用 */
    uint16_t cole_s;                        /* 控制台所用缓存大小 */
    uint8_t *recv_buf;                      /* 串口任务接收缓存 */
    uint32_t recv_size;                     /* 串口任务接收缓存大小 */
    uint8_t *out_buf;                       /* 标准输出缓存 */
    uint16_t out_s;                         /* 标准输出缓存大小 */

    /* 应用层回调注册 */
    int (*shell_data_cb) (const uint8_t *data, uint16_t len);

} lm_console_t;

/**
 * @brief shell 控制台接口注册
 *
 * @param[in]   p_console 控制台参数配置
 *
 * @return  错误码
 */
extern int lm_shell_register (lm_console_t *p_console);

LM_END_EXTERN_C

#endif /* __LM_SHELL_INTERFACE_H */

/* end of file */
