#ifndef __LM_SHELL_H
#define __LM_SHELL_H

#include "shell.h"

/* 控制台输出回调函数类型 */
typedef int (*console_output_t)(uint32_t com, const void *data, uint16_t len);

/* 控制台输入回调函数类型 */
typedef int (*console_input_t)(uint32_t com, uint8_t *data);

typedef struct {
    uint32_t stack_size;                    /* 控制台任务栈深度 */
    uint32_t prio;                          /* 控制台任务优先级 */
    uint32_t com;                           /* 控制台串口号 */
    uint32_t baud_rate;                     /* 波特率 */
    char *cole_buf;                         /* 控制台所用缓存 shell内部使用 */
    uint16_t cole_s;                        /* 控制台所用缓存大小 */
    uint8_t *recv_buf;                      /* 串口任务接收缓存 */
    uint32_t recv_size;                     /* 串口任务接收缓存大小 */
    uint8_t *cole_out_buf;                  /* 标准输出缓存 */
    uint16_t out_s;                         /* 标准输出缓存大小 */

//    console_output_t write;                 /* 写数据接口 */
//    console_input_t read;                   /* 读数据接口 */
} lm_console_t;

/**
 * @brief       控制台接口注册
 * @param       p_console,控制台指针
 * @return      错误码
 */
extern int lm_console_register(const lm_console_t *p_console);

/**
 * @brief       shell初始化
 * @param       None
 * @return      错误码
 */
extern int lm_shell_init (void);

#endif /* __LM_SHELL_H */

/* end of file */
