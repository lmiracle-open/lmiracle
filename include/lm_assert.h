/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_assert.h
* Change Logs   :
* Date         Author      Notes
* 2020-02-25   terryall    V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 系统文件
*******************************************************************************/

#ifndef __LM_ASSERT_H__
#define __LM_ASSERT_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LM_DEBUG

/* help macro */
#define _LM_VTOS(n)     #n          /* Convert n to string */
#define _LM_SVAL(m)     _LM_VTOS(m) /* Get string value of macro m */

/**
 * \brief 断言一个表达式
 *
 * 当表达式为真时这个宏不做任何事情，为假时会调用lm_assert_msg()输出断言信息，
 * 断言信息格式为：
 * 文件名：行号：表达式
 */
#define lm_assert(e) \
    ((e) ? (void)0 : lm_assert_msg(__FILE__":"_LM_SVAL(__LINE__)":("#e")\n"))

extern void lm_assert_msg (const char *msg);

#else

/**
 * \brief 断言一个表达式
 *
 * 当表达式为真时这个宏不做任何事情，为假时会调用lm_assert_msg()输出断言信息，
 * 断言信息格式为：
 * 文件名：行号：表达式
 */
#define lm_assert(e)    ((void)0)
#endif

#ifdef __cplusplus
}
#endif

/** @}  lm_if_assert */

#endif /* __LM_ASSERT_H__ */

/* end of file */
