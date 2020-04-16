/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*
* File Name     : lm_drv_led.h
* Change Logs   :
* Date         Author      Notes
* 2019-06-07   linxuew     V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : SPI FLash驱动文件
*******************************************************************************/

#ifndef __LM_SPI_NOR_H
#define __LM_SPI_NOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lmiracle.h"
#include "lm_nvram.h"
#include "lm_spi.h"

/**
 *
 */
#define SNOR_MFR_ATMEL      0x001F
#define SNOR_MFR_GIGADEVICE 0xc8
#define SNOR_MFR_INTEL      0x0089
#define SNOR_MFR_MICRON     0x0020 /* ST Micro <--> Micron */
#define SNOR_MFR_MICRONO    0x002C /* Original Micron */
#define SNOR_MFR_MACRONIX   0x00C2
#define SNOR_MFR_SPANSION   0x0001
#define SNOR_MFR_SST        0x00BF
#define SNOR_MFR_WINBOND    0xef /* Also used by some Spansion */



/* Flash opcodes. */
#define LM_SPINOR_OP_WREN          0x06    /* Write enable */
#define LM_SPINOR_OP_RDSR          0x05    /* Read status register */
#define LM_SPINOR_OP_WRSR          0x01    /* Write status register 1 byte */
#define LM_SPINOR_OP_RDSR2     0x3f    /* Read status register 2 */
#define LM_SPINOR_OP_WRSR2     0x3e    /* Write status register 2 */
#define LM_SPINOR_OP_READ          0x03    /* Read data bytes (low frequency) */
#define LM_SPINOR_OP_READ_FAST     0x0b    /* Read data bytes (high frequency) */
#define LM_SPINOR_OP_READ_1_1_2    0x3b    /* Read data bytes (Dual SPI) */
#define LM_SPINOR_OP_READ_1_2_2    0xbb    /* Read data bytes (Dual I/O SPI) */
#define LM_SPINOR_OP_READ_1_1_4    0x6b    /* Read data bytes (Quad SPI) */
#define LM_SPINOR_OP_READ_1_4_4    0xeb    /* Read data bytes (Quad I/O SPI) */
#define LM_SPINOR_OP_PP            0x02    /* Page program (up to 256 bytes) */
#define LM_SPINOR_OP_PP_1_1_4  0x32    /* Quad page program */
#define LM_SPINOR_OP_PP_1_4_4  0x38    /* Quad page program */
#define LM_SPINOR_OP_BE_4K         0x20    /* Erase 4KiB block */
#define LM_SPINOR_OP_BE_4K_PMC     0xd7    /* Erase 4KiB block on PMC chips */
#define LM_SPINOR_OP_BE_32K        0x52    /* Erase 32KiB block */
#define LM_SPINOR_OP_CHIP_ERASE    0xc7    /* Erase whole flash chip */
#define LM_SPINOR_OP_SE            0xd8    /* Sector erase (usually 64KiB) */
#define LM_SPINOR_OP_RDID          0x9f    /* Read JEDEC ID */
#define LM_SPINOR_OP_RDSFDP        0x5a    /* Read SFDP */
#define LM_SPINOR_OP_RDCR          0x35    /* Read configuration register */
#define LM_SPINOR_OP_RDFSR         0x70    /* Read flag status register */

/* 4-byte address opcodes - used on Spansion and some Macronix flashes. */
#define LM_SPINOR_OP_READ_4B        0x13    /* Read data bytes (low frequency) */
#define LM_SPINOR_OP_READ_FAST_4B   0x0c    /* Read data bytes (high frequency) */
#define LM_SPINOR_OP_READ_1_1_2_4B  0x3c    /* Read data bytes (Dual Output SPI) */
#define LM_SPINOR_OP_READ_1_2_2_4B  0xbc    /* Read data bytes (Dual I/O SPI) */
#define LM_SPINOR_OP_READ_1_1_4_4B  0x6c    /* Read data bytes (Quad Output SPI) */
#define LM_SPINOR_OP_READ_1_4_4_4B  0xec    /* Read data bytes (Quad I/O SPI) */
#define LM_SPINOR_OP_READ_1_4_4_D_4B 0xee    /* Read data bytes (DDR Quad SPI) */
#define LM_SPINOR_OP_PP_4B     0x12    /* Page program (up to 256 bytes) */
#define LM_SPINOR_OP_PP_1_1_4_4B   0x34    /* Quad page program */
#define LM_SPINOR_OP_PP_1_4_4_4B   0x3e    /* Quad page program */
#define LM_SPINOR_OP_BE_4K_4B  0x21    /* Erase 4KiB block */
#define LM_SPINOR_OP_BE_32K_4B 0x5c    /* Erase 32KiB block */
#define LM_SPINOR_OP_SE_4B     0xdc    /* Sector erase (usually 64KiB) */

/* Double Transfer Rate opcodes - defined in JEDEC JESD216B. */
#define LM_SPINOR_OP_READ_1_1_1_DTR    0x0d
#define LM_SPINOR_OP_READ_1_2_2_DTR    0xbd
#define LM_SPINOR_OP_READ_1_1_4_DTR    0x6d
#define LM_SPINOR_OP_READ_1_4_4_DTR    0xed

#define LM_SPINOR_OP_READ_1_1_1_DTR_4B 0x0e
#define LM_SPINOR_OP_READ_1_2_2_DTR_4B 0xbe
#define LM_SPINOR_OP_READ_1_4_4_DTR_4B 0xee
#define LM_SPINOR_OP_READ_1_8_8_DTR_4B 0x9d

/* Used for SST flashes only. */
#define LM_SPINOR_OP_BP            0x02    /* Byte program */
#define LM_SPINOR_OP_WRDI          0x04    /* Write disable */
#define LM_SPINOR_OP_AAI_WP        0xad    /* Auto address increment word program */

/* Used for Macronix and Winbond flashes. */
#define LM_SPINOR_OP_EN4B          0xb7    /* Enter 4-byte mode */
#define LM_SPINOR_OP_EX4B          0xe9    /* Exit 4-byte mode */

/* Used for Spansion flashes only. */
#define LM_SPINOR_OP_BRWR          0x17    /* Bank register write */


/* Used for Micron flashes only. */
#define LM_SPINOR_OP_RD_EVCR      0x65    /* Read EVCR register */
#define LM_SPINOR_OP_WD_EVCR      0x61    /* Write EVCR register */

/* Status Register bits. */
#define SR_WIP          BIT(0)  /* Write in progress */
#define SR_WEL          BIT(1)  /* Write enable latch */
/* meaning of other SR_* bits may differ between vendors */
#define SR_BP0          BIT(2)  /* Block protect 0 */
#define SR_BP1          BIT(3)  /* Block protect 1 */
#define SR_BP2          BIT(4)  /* Block protect 2 */
#define SR_TB           BIT(5)  /* Top/Bottom protect */
#define SR_SRWD         BIT(7)  /* SR write protect */
/* Spansion/Cypress specific status bits */
#define SR_E_ERR        BIT(5)
#define SR_P_ERR        BIT(6)

#define SR_QUAD_EN_MX       BIT(6)  /* Macronix Quad I/O */

/* Enhanced Volatile Configuration Register bits */
#define EVCR_QUAD_EN_MICRON BIT(7)  /* Micron Quad I/O */

/* Flag Status Register bits */
#define FSR_READY       BIT(7)

/* Configuration Register bits. */
#define CR_QUAD_EN_SPAN     BIT(1)  /* Spansion Quad I/O */

/* Status Register 2 bits. */
#define SR2_QUAD_EN_BIT7    BIT(7)



enum lm_spi_nor_read_mode {
    LM_SPI_NOR_NORMAL = 0,
    LM_SPI_NOR_FAST,
    LM_SPI_NOR_DUAL,
    LM_SPI_NOR_QUAD,
};

enum lm_spi_nor_ops {
        SPI_NOR_OPS_READ = 0,
        SPI_NOR_OPS_WRITE,
        SPI_NOR_OPS_ERASE,
        SPI_NOR_OPS_LOCK,
        SPI_NOR_OPS_UNLOCK,
};

enum lm_spi_nor_option_flags {
    LM_SNOR_F_USE_FSR          = 0x01U,
};

/**
 * @brief flash区域配置
 */
typedef struct lm_spi_nor_zone_cfg {
    char              *name;          /* 名字 */
    unsigned long      addr;          /* 起始地址 */
    unsigned long      size;          /* 大小 */
} lm_spi_nor_zone_cfg_t;

struct spi_nor_hwcaps {
    uint32_t mask;
};

/**
 * 读控制
 */
#define LM_SNOR_HWCAPS_READ_MASK       GENMASK(14, 0)
#define LM_SNOR_HWCAPS_READ        BIT(0)
#define LM_SNOR_HWCAPS_READ_FAST       BIT(1)
#define LM_SNOR_HWCAPS_READ_1_1_1_DTR  BIT(2)

#define LM_SNOR_HWCAPS_READ_DUAL       GENMASK(6, 3)
#define LM_SNOR_HWCAPS_READ_1_1_2      BIT(3)
#define LM_SNOR_HWCAPS_READ_1_2_2      BIT(4)
#define LM_SNOR_HWCAPS_READ_2_2_2      BIT(5)
#define LM_SNOR_HWCAPS_READ_1_2_2_DTR  BIT(6)

#define LM_SNOR_HWCAPS_READ_QUAD       GENMASK(10, 7)
#define LM_SNOR_HWCAPS_READ_1_1_4      BIT(7)
#define LM_SNOR_HWCAPS_READ_1_4_4      BIT(8)
#define LM_SNOR_HWCAPS_READ_4_4_4      BIT(9)
#define LM_SNOR_HWCAPS_READ_1_4_4_DTR  BIT(10)

#define LM_SNOR_HWCPAS_READ_OCTO       GENMASK(14, 11)
#define LM_SNOR_HWCAPS_READ_1_1_8      BIT(11)
#define LM_SNOR_HWCAPS_READ_1_8_8      BIT(12)
#define LM_SNOR_HWCAPS_READ_8_8_8      BIT(13)
#define LM_SNOR_HWCAPS_READ_1_8_8_DTR  BIT(14)

#define LM_SNOR_HWCAPS_PP_MASK GENMASK(22, 16)
#define LM_SNOR_HWCAPS_PP      BIT(16)

#define LM_SNOR_HWCAPS_PP_QUAD GENMASK(19, 17)
#define LM_SNOR_HWCAPS_PP_1_1_4    BIT(17)
#define LM_SNOR_HWCAPS_PP_1_4_4    BIT(18)
#define LM_SNOR_HWCAPS_PP_4_4_4    BIT(19)

#define LM_SNOR_HWCAPS_PP_OCTO GENMASK(22, 20)
#define LM_SNOR_HWCAPS_PP_1_1_8    BIT(20)
#define LM_SNOR_HWCAPS_PP_1_8_8    BIT(21)
#define LM_SNOR_HWCAPS_PP_8_8_8    BIT(22)


/* Supported SPI protocols */
#define SNOR_PROTO_INST_MASK    GENMASK(23, 16)
#define SNOR_PROTO_INST_SHIFT   16
#define SNOR_PROTO_INST(_nbits) \
    ((((unsigned long)(_nbits)) << SNOR_PROTO_INST_SHIFT) & \
     SNOR_PROTO_INST_MASK)

#define SNOR_PROTO_ADDR_MASK    GENMASK(15, 8)
#define SNOR_PROTO_ADDR_SHIFT   8
#define SNOR_PROTO_ADDR(_nbits) \
    ((((unsigned long)(_nbits)) << SNOR_PROTO_ADDR_SHIFT) & \
     SNOR_PROTO_ADDR_MASK)

#define SNOR_PROTO_DATA_MASK    GENMASK(7, 0)
#define SNOR_PROTO_DATA_SHIFT   0
#define SNOR_PROTO_DATA(_nbits) \
    ((((unsigned long)(_nbits)) << SNOR_PROTO_DATA_SHIFT) & \
     SNOR_PROTO_DATA_MASK)

#define SNOR_PROTO_IS_DTR   BIT(24) /* Double Transfer Rate */

#define SNOR_PROTO_STR(_inst_nbits, _addr_nbits, _data_nbits)   \
    (SNOR_PROTO_INST(_inst_nbits) |             \
     SNOR_PROTO_ADDR(_addr_nbits) |             \
     SNOR_PROTO_DATA(_data_nbits))
#define SNOR_PROTO_DTR(_inst_nbits, _addr_nbits, _data_nbits)   \
    (SNOR_PROTO_IS_DTR |                    \
     SNOR_PROTO_STR(_inst_nbits, _addr_nbits, _data_nbits))

enum lm_spi_nor_protocol {
    SNOR_PROTO_1_1_1 = SNOR_PROTO_STR(1, 1, 1),
    SNOR_PROTO_1_1_2 = SNOR_PROTO_STR(1, 1, 2),
    SNOR_PROTO_1_1_4 = SNOR_PROTO_STR(1, 1, 4),
    SNOR_PROTO_1_1_8 = SNOR_PROTO_STR(1, 1, 8),
    SNOR_PROTO_1_2_2 = SNOR_PROTO_STR(1, 2, 2),
    SNOR_PROTO_1_4_4 = SNOR_PROTO_STR(1, 4, 4),
    SNOR_PROTO_1_8_8 = SNOR_PROTO_STR(1, 8, 8),
    SNOR_PROTO_2_2_2 = SNOR_PROTO_STR(2, 2, 2),
    SNOR_PROTO_4_4_4 = SNOR_PROTO_STR(4, 4, 4),
    SNOR_PROTO_8_8_8 = SNOR_PROTO_STR(8, 8, 8),

    SNOR_PROTO_1_1_1_DTR = SNOR_PROTO_DTR(1, 1, 1),
    SNOR_PROTO_1_2_2_DTR = SNOR_PROTO_DTR(1, 2, 2),
    SNOR_PROTO_1_4_4_DTR = SNOR_PROTO_DTR(1, 4, 4),
    SNOR_PROTO_1_8_8_DTR = SNOR_PROTO_DTR(1, 8, 8),
};


static inline bool lm_spi_nor_protocol_is_dtr(enum lm_spi_nor_protocol proto)
{
    return !!(proto & SNOR_PROTO_IS_DTR);
}

static inline uint8_t lm_spi_nor_get_protocol_inst_nbits(enum lm_spi_nor_protocol proto)
{
    return ((unsigned long)(proto & SNOR_PROTO_INST_MASK)) >>
        SNOR_PROTO_INST_SHIFT;
}

static inline uint8_t lm_spi_nor_get_protocol_addr_nbits(enum lm_spi_nor_protocol proto)
{
    return ((unsigned long)(proto & SNOR_PROTO_ADDR_MASK)) >>
        SNOR_PROTO_ADDR_SHIFT;
}

static inline uint8_t lm_spi_nor_get_protocol_data_nbits(enum lm_spi_nor_protocol proto)
{
    return ((unsigned long)(proto & SNOR_PROTO_DATA_MASK)) >>
        SNOR_PROTO_DATA_SHIFT;
}

static inline uint8_t lm_spi_nor_get_protocol_width(enum lm_spi_nor_protocol proto)
{
    return lm_spi_nor_get_protocol_data_nbits(proto);
}

typedef struct lm_spi_nor_dev lm_spi_nor_dev_t;

#define LM_SPI_NOR_MAX_CMD_SIZE   8

/**
 * @brief flash设备
 */
struct lm_spi_nor_dev
{
    lm_nvram_dev_t             nvram;

    lm_mutex_t                 lock;

    lm_spi_dev_t              *p_spi;

    uint32_t                    page_size;
    uint8_t                     addr_width;
    uint8_t                     erase_opcode;
    uint8_t                     read_opcode;
    uint8_t                     read_dummy;
    uint8_t                     program_opcode;
    enum  lm_spi_nor_read_mode  flash_read;
    uint32_t                    flags;
//    struct lm_spi_nor_xfer_cfg  cfg;


     enum lm_spi_nor_protocol   read_proto;
     enum lm_spi_nor_protocol   write_proto;
     enum lm_spi_nor_protocol   reg_proto;
     uint8_t                    sst_write_second;   /* sst 写操作 */


    uint8_t                    cmd_buf[LM_SPI_NOR_MAX_CMD_SIZE];

    int (*pfunc_prepare)(lm_spi_nor_dev_t *nor, enum lm_spi_nor_ops ops);
    void (*pfunc_unprepare)(lm_spi_nor_dev_t *nor, enum lm_spi_nor_ops ops);

    int (*pfunc_read_reg)(lm_spi_nor_dev_t *nor,
                          uint8_t opcode, uint8_t *buf,
                          int len);
    int (*pfunc_write_reg)(lm_spi_nor_dev_t *nor, uint8_t opcode, uint8_t *buf,
                           int len);

     size_t (*pfunc_read)(lm_spi_nor_dev_t *p_nor, uint32_t from,
             size_t len, uint8_t *read_buf);
     size_t (*pfunc_write)(lm_spi_nor_dev_t *p_nor, uint32_t to,
             size_t len, const uint8_t *write_buf);

    int (*pfunc_erase)(lm_spi_nor_dev_t *nor, uint32_t offs);

    void *priv;

    lm_mutex_t            *mutex;

} ;

static inline lm_spi_nor_dev_t *lm_nvram_to_spi_nor(lm_nvram_dev_t *p_nvram)
{
    return p_nvram->priv;
}

/**
 * @brief SPI FLASH 写
 *
 * @param[in]  p_dev     FLASH设备
 * @param[in]  addr      地址
 * @param[in]  p_buf     写入的数据
 * @param[in]  len       写入的长度
 * @param[out] wlen      实际写入的长度
 *
 * @return   LM_OK      成功
 *          -LM_EBUSY   设备忙
 *          -LM_EIO     设备信息错误
 */
extern int lm_spi_nor_write (lm_spi_nor_dev_t *p_dev,
                               uint32_t            addr,
                               uint8_t            *p_buf,
                               size_t              len,
                               size_t             *wlen);

/**
 * @brief SPI FLASH 读
 *
 * @param[in]  p_dev     FLASH设备
 * @param[in]  addr      地址
 * @param[out] p_buf     读取数据缓存区
 * @param[in]  len       读取的长度
 * @param[out] rlen      实际读取的长度
 *
 * @return   LM_OK      成功
 *          -LM_EBUSY   设备忙
 *          -LM_EIO     设备信息错误
 */
extern int lm_spi_nor_read (lm_spi_nor_dev_t *p_dev,
                              uint32_t            addr,
                              uint8_t            *p_buf,
                              size_t              len,
                              size_t             *rlen);


/**
 *
 */
extern int lm_spi_nor_scan(lm_spi_nor_dev_t            *p_nor,
                           const char                  *name,
                           const struct spi_nor_hwcaps *hwcaps);


#ifdef __cplusplus
}
#endif

#endif /* __LM_SPI_NOR_H */


/* end of file */

