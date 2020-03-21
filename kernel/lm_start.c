#include "lmiracle.h"

extern void app_entry (void * p_arg);
extern void lm_system_init (void);
extern void lm_board_init (void);

void lm_system_init (void)
{

}


/**
 * @brief C程序入口
 */
int main (void)
{
    /* 1.系统相关初始化 */
    lm_system_init();

    /* 2.板卡相关初始化 */
    lm_board_init();

    /* 3.创建应用进程 */
    lm_task_create(app_entry, "app_entry", 1024, NULL, 5);

    /* 4.启动调度器 */
    lm_scheduler_start();

    /* 永远不会运行到此　*/

}

/* end of file */
