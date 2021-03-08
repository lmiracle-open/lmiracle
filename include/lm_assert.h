/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_assert.h
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 断言模块
*******************************************************************************/

#ifndef __LM_ASSERT_H
#define __LM_ASSERT_H

LM_BEGIN_EXTERN_C

/* todo: 需要调试的时候 定义该参数 */
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

/**
 * @brief 断言执行函数
 *
 * @param[in]     msg   需要输出的消息内容
 *
 * @return  None
 */
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

LM_END_EXTERN_C

/** @}  lm_if_assert */

#endif /* __LM_ASSERT_H */

/* end of file */
