#ifndef __LM_SHELL_H
#define __LM_SHELL_H

#include "shell.h"

/* 控制台输出回调函数类型 */
typedef int (*console_output_t)(uint32_t com, const void *data, uint16_t len);

/* 控制台输入回调函数类型 */
typedef int (*console_input_t)(uint8_t *data, uint16_t *len);

typedef struct __console {
    uint32_t com;
    console_output_t write;
    console_input_t read;
} lm_console_t;

extern int lm_console_register(lm_console_t *p_console);

extern int lm_shell_init (void);

#endif /* __LM_SHELL_H */

/* end of file */
