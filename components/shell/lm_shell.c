#include "lmiracle.h"


void lm_shell_run (void *p_arg)
{

    while (1) {
        lm_task_delay(1000);
    }


}


/**
 * shell初始化
 */
int lm_shell_init (void)
{
    lm_err_t ret = LM_OK;
    lm_task_create("lm_shell", lm_shell_run, NULL, 1024, 5);

    return ret;
}
