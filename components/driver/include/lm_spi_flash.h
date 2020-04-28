#ifndef __LM_SPI_FLASH_H
#define __LM_SPI_FLASH_H

#include "lmiracle.h"
#include "lm_nvram.h"
#include "lm_spi_nor.h"

/**
 * @brief flash配置
 */
typedef struct lm_spi_flash_cfg
{
    const char                    *name;           /* 设备名称名字 */

    uint8_t                       spi_id;          /* SPI 总线ID */

    uint8_t                       bits_per_word;
    uint16_t                      spi_mode;        /* SPI 模式 */
    uint32_t                      spi_speed;       /* SPI 速度 */

    uint32_t                      spi_flags;       /* SPI 传输标志 */
    const void                   *cs_gpio;         /* 片选 */

    const lm_nvram_info_t        *p_nvram_info;    /* NVRAM 配置 */

    void (*pfunc_platform_init) (void);            /* 硬件平台初始化 */
} lm_spi_flash_cfg_t;

#ifndef LM_SPI_FLASH_MAX_CMD_SIZE
#define    LM_SPI_FLASH_MAX_CMD_SIZE        6
#endif

typedef struct lm_spi_flash_dev
{
    lm_spi_dev_t                    spi;
    const lm_spi_flash_cfg_t       *p_cfg;

    lm_spi_nor_dev_t                spi_nor;
    uint8_t                         command[LM_SPI_FLASH_MAX_CMD_SIZE];
} lm_spi_flash_dev_t;

extern int lm_spi_flash_register(lm_spi_flash_dev_t       *p_flash,
                                 const lm_spi_flash_cfg_t *p_cfg);

#endif /* __LM_SPI_FLASH_H */

/* end of file */
