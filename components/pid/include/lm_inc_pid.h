/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_inc_pid.h
* Change Logs   :
* Date           Author      Notes
* 2020—08—01-07   wyf        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : INCREASE PID
*******************************************************************************/
#ifndef __LM_INC_PID__H
#define __LM_INC_PID__H

#include "lmiracle.h"
#include "string.h"

/**
 * @brief:   增量式PID结构体
 */
typedef struct {
    float   set_point;                 /* 设定目标 */
    float   last_error;                /* Error[-1]:last  */
    float   prev_error;                /* Error[-2]:previous  */
    double  proportion;                /* 比例常数 */
    double  integral;                  /* 积分常数 */
    double  derivative;                /* 微分常数 */
} lm_inc_pid_t;

/**
 * @brief:   增量式PID 参数初始化
 * @param:   *sptr   PID变量结构体
 * @return:  None
 */
extern void  incpid_init_register (lm_inc_pid_t *p_pid);

/**
 * @brief:   增量式 PID控制算法
 * @param:   *sptr       PID变量结构体
 * @param:   NextPoint   实际温度值
 * @return   None
 */
extern float  lm_inc_pid_ctrl (float set_point, float next_point);


#endif /* __LM_INC_PID__H */

/* end of file */


