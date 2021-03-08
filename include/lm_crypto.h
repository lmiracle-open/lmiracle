/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_crypto.h
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 加密模块
*******************************************************************************/

#ifndef __LM_CRYPTO_H
#define __LM_CRYPTO_H

#include "lmiracle.h"

LM_BEGIN_EXTERN_C

/**
 * @brief 获取UID回调
 *
 * @param[out]    p_buf 保存ID的地址,p_buf空闲区域需要清0
 * @param[in]     len   p_buf的长度
 * @param[out]    rlen  ID实际长度
 *
 * @return  错误码
 */
typedef int (*crypto_uid_get_t) (uint8_t *p_buf, uint32_t len, uint32_t *rlen);

/**
 * @brief 写秘钥回调函数  写入到Flash中,一般放到内部Flash的最后一个扇区
 *
 * @param[in]      p_buf 需要写入的秘钥
 * @param[in]      len   秘钥的长度,最大256字节
 * @param[out]     wlen  实际写入秘钥的长度
 *
 * @return  错误码
 */
typedef int (*crypto_write_skey_t) (const uint8_t *p_buf, uint32_t len, uint32_t *wlen);

/**
 * @brief 读秘钥回调函数
 *
 * @param[in]      p_buf 读取秘钥的缓存区
 * @param[in]      len   秘钥的长度,最大256字节
 * @param[out]     rlen  实际读取秘钥的长度
 *
 * @return  错误码
 */
typedef int (*crypto_read_skey_t) (uint8_t *p_buf, uint32_t len, uint32_t *rlen);

/******************************************************************************/

/**
 * @brief 加密回调函数通知
 *
 * @param[out]    p_buf 保存ID的地址,p_buf空闲区域需要清0
 * @param[in]     len   p_buf的长度
 * @param[out]    rlen  ID实际长度
 *
 * @return  错误码
 */
extern int lm_crypto_cb_notice (    crypto_uid_get_t    p_uid_get, \
                                    crypto_write_skey_t p_skey_write, \
                                    crypto_read_skey_t  p_skey_read);
/**
 * @brief 获取UID
 *
 * @param[out]    p_buf 保存ID的地址,p_buf空闲区域需要清0
 * @param[in]     len   p_buf的长度
 * @param[out]    rlen  ID实际长度
 *
 * @return  错误码
 */
extern int lm_crypto_get_uid (uint8_t *p_buf, int len, int *rlen);


/**
 * @brief 写入秘钥,写入到Flash中,一般放到内部Flash的最后一个扇区
 *
 * @param[in]      p_buf 需要写入的秘钥
 * @param[in]      len   秘钥的长度,最大256字节
 * @param[out]     wlen  实际写入秘钥的长度
 *
 * @return  错误码
 */
extern int lm_crypto_write_skey (uint8_t *p_buf, int len, int *wlen);

/**
 * @brief 读取秘钥
 *
 * @param[in]      p_buf 读取秘钥的缓存区
 * @param[in]      len   秘钥的长度,最大256字节
 * @param[out]     rlen  实际读取秘钥的长度
 *
 * @return  错误码
 */
extern int lm_crypto_read_skey (uint8_t *p_buf, int len, int *rlen);

/**
 * @brief 加密,在程序的关键部位调用该函数,如果硬件没有烧写秘钥,则程序不能正常执行
 *        该函数不用去实现,以上其他函数必须实现
 */
extern void lm_crypto_encrypt (void);

/**
 * @brief 加密模块初始化
 *
 * @param[in]   None
 *
 * @return  错误码
 */
extern int lm_crypto_init (void);

LM_END_EXTERN_C

#endif /* __LM_CRYPTO_H */

/* end of file */
