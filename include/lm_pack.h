/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_pack.h
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 数据打包模块
*******************************************************************************/

#ifndef __LM_PACK_H
#define __LM_PACK_H

#include "lmiracle.h"

LM_BEGIN_EXTERN_C

#define __BYTE_ORDER__  __ORDER_BIG_ENDIAN__        /* todo: 大端格式 */

#if !defined(__BYTE_ORDER__) || !defined(__ORDER_BIG_ENDIAN__)
#error "The compiler does not support the libpack library."
#endif

/*******************************************************************************
* Description    : 字节交换函数
*******************************************************************************/
#define BYTE_SWAP16(x)      __builtin_bswap16(x)
#define BYTE_SWAP32(x)      __builtin_bswap32(x)
#define BYTE_SWAP64(x)      __builtin_bswap64(x)

/**
 * @brief 8位大端格式打包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回打包后数据长度
 */
static inline int pack_be8 (const uint8_t src, uint8_t *dst)
{
    *dst = src;
    return sizeof(uint8_t);
}

/**
 * @brief 8位小端格式打包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回打包后数据长度
 */
static inline int pack_le8 (const uint8_t src, uint8_t *dst)
{
    *dst = src;
    return sizeof(uint8_t);
}

/**
 * @brief 16位大端格式打包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回打包后数据长度
 */
static inline int pack_be16 (const uint16_t *src, uint16_t *dst, uint16_t size)
{
    for (int i = 0; i < size; i ++) {
        *dst++ = BYTE_SWAP16(src[i]);
    }
    return sizeof(uint16_t);
}

/**
 * @brief 16位小端格式打包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回打包后数据长度
 */
static inline int pack_le16 (const uint16_t src, uint16_t *dst)
{
    *dst =  src;
    return sizeof(uint16_t);
}

/**
 * @brief 32位大端格式打包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回打包后数据长度
 */
static inline int pack_be32 (const uint32_t src, uint32_t *dst)
{
    *dst = BYTE_SWAP32(src);
    return sizeof(uint32_t);
}

/**
 * @brief 32位小端格式打包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回打包后数据长度
 */
static inline int pack_le32 (const uint32_t src, uint32_t *dst)
{
    *dst = src;
    return sizeof(uint32_t);
}

/**
 * @brief 64位大端格式打包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回打包后数据长度
 */
static inline int pack_be64 (const uint64_t src, uint64_t *dst)
{
    *dst = BYTE_SWAP64(src);
    return sizeof(uint64_t);
}

/**
 * @brief 64位小端格式打包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回打包后数据长度
 */
static inline int pack_le64 (const uint64_t src, uint64_t *dst)
{
    *dst = src;
    return sizeof(uint64_t);
}

/**
 * @brief 8位大端格式解包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回解包后数据长度
 */
static inline int unpack_be8 (const uint8_t src, uint8_t *dst)
{
    *dst = src;
    return sizeof(uint8_t);
}

/**
 * @brief 8位小端格式解包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回解包后数据长度
 */
static inline int unpack_le8 (const uint8_t src, uint8_t *dst)
{
    *dst = src;
    return sizeof(uint8_t);
}

/**
 * @brief 16位大端格式解包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回解包后数据长度
 */
static inline int unpack_be16 (const uint16_t src, uint16_t *dst)
{
    *dst = BYTE_SWAP16(src);
    return sizeof(uint16_t);
}

/**
 * @brief 16位小端格式解包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回解包后数据长度
 */
static inline int unpack_le16 (const uint16_t src, uint16_t *dst)
{
    *dst = src;
    return sizeof(uint16_t);
}

/**
 * @brief 32位大端格式解包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回解包后数据长度
 */
static inline int unpack_be32 (const uint32_t src, uint32_t *dst)
{
    *dst = BYTE_SWAP32(src);
    return sizeof(uint32_t);
}

/**
 * @brief 32位小端格式解包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回解包后数据长度
 */
static inline int unpack_le32 (const uint32_t src, uint32_t *dst)
{
    *dst = src;
    return sizeof(uint32_t);
}

/**
 * @brief 64位大端格式解包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回解包后数据长度
 */
static inline int unpack_be64 (const uint64_t src, uint64_t *dst)
{
    *dst = BYTE_SWAP64(src);
    return sizeof(uint64_t);
}

/**
 * @brief 64位小端格式解包
 *
 * @param[in] src   原始数据
 * @param[in] dst   目标数据指针
 *
 * @return  返回解包后数据长度
 */
static inline int unpack_le64 (const uint64_t src, uint64_t *dst)
{
    *dst = src;
    return sizeof(uint64_t);
}

LM_END_EXTERN_C

#endif  /* __LM_PACK_H */

/* end of file */
