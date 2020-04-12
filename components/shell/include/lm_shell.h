#ifndef __LM_SHELL_H
#define __LM_SHELL_H

#include "shell.h"
#include <stdbool.h>

/* 控制台输出回调函数类型 */
typedef int (*console_output_t)(uint32_t com, const void *data, uint16_t len);

typedef struct __console {
    uint32_t com;
    console_output_t write;
} lm_console_t;

/* 注册接口 */
extern int lm_console_register(lm_console_t *p_console);

/* 读接口 */
extern int lm_console_data_read (uint8_t *data, uint16_t len);

/* 初始化接口 */
extern int lm_shell_init (void);

#endif /* __LM_SHELL_H */

/* end of file */
