/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_drv_led.h
* Change Logs   :
* Date         Author      Notes
* 2019-06-07   linxuew     V1.0    first version
*******************************************************************************/

#ifndef __LM_NVRAM_H
#define __LM_NVRAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lmiracle.h"
#include "lm_spi.h"


#define NVRAM_ERASE_PENDING   0x01
#define NVRAM_ERASING         0x02
#define NVRAM_ERASE_SUSPEND   0x04
#define NVRAM_ERASE_DONE      0x08
#define NVRAM_ERASE_FAILED    0x10

/* 4k */
#define CONFIG_NVRAM_SPI_NOR_USE_4K_SECTORS

/**
 * @brief NVRAM
 */
typedef struct lm_nvram_segment {
    char              *name;          /* 名字 */
    unsigned long      addr;          /* 起始地址 */
    unsigned long      size;          /* 大小 */
} lm_nvram_segment_t;



/**
 * @brief flash配置
 */
typedef struct lm_nvram_info
{
    const lm_nvram_segment_t             *p_zone;         /* 区域划分 */
    unsigned char                         zone_num;       /* 区域个数 */
    unsigned char                        *buf;            /* 缓存区  */
    uint32_t                              buf_size;       /* 缓存大小 */


} lm_nvram_info_t;

typedef struct lm_nvram_dev lm_nvram_dev_t;


struct erase_info {
    uint64_t addr;
    uint64_t len;
    uint64_t fail_addr;
    uint8_t state;
};

/**
 * @brief NVRAM设备
 */
struct lm_nvram_dev
{
    const lm_nvram_info_t       *p_info;

    uint32_t                     size;

    uint32_t                     erasesize;
    uint16_t                     writebufsize;

    int (*pfunc_read)(lm_nvram_dev_t *p_nvram,
                      uint32_t        addr,
                      uint8_t         *p_buf,
                      size_t          len,
                      size_t         *rlen);

    int (*pfunc_write)(lm_nvram_dev_t *p_nvram,
                       uint32_t        addr,
                       const uint8_t  *p_buf,
                       size_t          len,
                       size_t         *wlen);

    int (*pfunc_erase)(lm_nvram_dev_t    *p_nvram,
                       struct erase_info *instr);

    void           *priv;                /* 私有数据 */

};



/**
 * @brief 写NVRAM
 *
 * @param[in] p_name        存储器名字
 * @param[in] p_buf         写入缓存区数据
 * @param[in] offset        偏移位置
 * @param[in] len           长度
 *
 * @return  LM_OK           : 成功
 *         -LM_EXIO         : 存储器不存在
 *         -LM_ERROR        : 其他错误
 */
extern int
lm_nvram_write (char *p_name, uint8_t *p_buf, uint32_t offset, size_t len);

/**
 * @brief 读NVRAM
 *
 * @param[in]  p_name        存储器名字
 * @param[out] p_buf         读取数据的缓存区
 * @param[in]  offset        偏移位置
 * @param[in]  len           长度
 *
 * @return  LM_OK           : 成功
 *         -LM_EXIO         : 存储器不存在
 *         -LM_ERROR        : 其他错误
 */
extern int
lm_nvram_read (char *p_name, uint8_t *p_buf, uint32_t offset, size_t len);

/**
 * @brief 注册NVRAM设备
 */
extern int
lm_nvram_register (lm_nvram_dev_t *p_dev);

#ifdef __cplusplus
}
#endif

#endif /* __LM_NVRAM_H */

/* end of file */
