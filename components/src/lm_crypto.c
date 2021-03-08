/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_crypto.c
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 加密模块
*******************************************************************************/

#include "lm_crypto.h"
#include "lmiracle.h"

static crypto_uid_get_t     crypto_uid_get_cb       = NULL; /* 获取UID回调 */
static crypto_write_skey_t  crypto_write_skey_cb    = NULL; /* 写秘钥回调 */
static crypto_read_skey_t   crypto_read_skey_cb     = NULL; /* 读秘钥回调 */

/**
 * @brief 加密模块初始化
 */
int lm_crypto_init (void)
{
    int ret = LM_OK;

    return ret;
}

/**
 * @brief 加密回调设置函数
 */
int lm_crypto_cb_notice (   crypto_uid_get_t    p_uid_get, \
                            crypto_write_skey_t p_skey_write, \
                            crypto_read_skey_t  p_skey_read)
{
    crypto_uid_get_cb       = p_uid_get;
    crypto_write_skey_cb    = p_skey_write;
    crypto_read_skey_cb     = p_skey_read;

    return LM_OK;
}

/*
 * @brief 获取UID
 */
int lm_crypto_get_uid (uint8_t *p_buf, int len, int *rlen)
{
    int ret = LM_OK;

    if (crypto_uid_get_cb) {
        ret = crypto_uid_get_cb(p_buf, (uint32_t)len, (uint32_t *)rlen);
    }

    return ret;
}

/*
 * @brief 将秘钥写入flash中
 */
int lm_crypto_write_skey (uint8_t *p_buf, int len, int *wlen)
{
    int ret = LM_OK;

    if (crypto_write_skey_cb) {
        ret = crypto_write_skey_cb(p_buf, (uint32_t)len, (uint32_t *)wlen);
    }

    return ret;
}

/*
 * @brief 从flash中读取秘钥
 */
int lm_crypto_read_skey (uint8_t *p_buf, int len, int *rlen)
{
    int ret = LM_OK;

    if (crypto_read_skey_cb) {
        ret = crypto_read_skey_cb(p_buf, (uint32_t)len, (uint32_t *)rlen);
    }

    return ret;
}

/* end of file */
