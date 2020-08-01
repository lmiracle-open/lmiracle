/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_utils.h
* Change Logs   :
* Date         Author      Notes
* 2019-06-07   terryall     V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   :
*******************************************************************************/

#ifndef __LM_UTILS_H
#define __LM_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lmiracle.h"
#include <string.h>
#include <stdbool.h>

/* 浮点数转换字符串 */
extern int float_to_str(char *str, float number, uint8_t g, uint8_t l, bool flag);

#ifdef __cplusplus
}
#endif

#endif /* __LM_UTILS_H */

/* end of file */
