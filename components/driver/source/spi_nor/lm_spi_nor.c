#include "lm_spi_nor.h"
#include "lmiracle.h"
#include "lm_heap.h"

#define SPI_NOR_MAX_ID_LEN    6
#define SPI_NOR_MAX_ADDR_WIDTH    4

#define  MAX_READY_WAIT_TIME      3000
struct flash_info {
    uint8_t          id[SPI_NOR_MAX_ID_LEN];
    uint8_t          id_len;
    unsigned         sector_size;
    uint16_t         n_sectors;

    uint16_t         page_size;
    uint16_t         addr_width;
    uint16_t         flags;
#define SECT_4K                 BIT(0)    /* SPINOR_OP_BE_4K works uniformly */
#define SPI_NOR_NO_ERASE        BIT(1)    /* No erase command needed */
#define SST_WRITE               BIT(2)    /* use SST byte programming */
#define SPI_NOR_NO_FR           BIT(3)    /* Can't do fastread */
#define SECT_4K_PMC             BIT(4)    /* SPINOR_OP_BE_4K_PMC works uniformly */
#define SPI_NOR_DUAL_READ       BIT(5)    /* Flash supports Dual Read */
#define SPI_NOR_QUAD_READ       BIT(6)    /* Flash supports Quad Read */
#define USE_FSR                 BIT(7)    /* use flag status register */
#define SPI_NOR_HAS_LOCK        BIT(8)    /* use flag status register */
#define SPI_NOR_HAS_TB      BIT(9)  /*
                     * Flash SR has Top/Bottom (TB) protect
                     * bit. Must be used with
                     * SPI_NOR_HAS_LOCK.
                     */
#define SPI_S3AN        BIT(10) /*
                     * Xilinx Spartan 3AN In-System Flash
                     * (MFR cannot be used for probing
                     * because it has the same value as
                     * ATMEL flashes)
                     */
#define SPI_NOR_4B_OPCODES  BIT(11) /*
                     * Use dedicated 4byte address op codes
                     * to support memory size above 128Mib.
                     */
#define NO_CHIP_ERASE       BIT(12) /* Chip does not support chip erase */
#define SPI_NOR_SKIP_SFDP   BIT(13) /* Skip parsing of SFDP tables */
#define USE_CLSR        BIT(14) /* use CLSR command */

#define SPI_NOR_OCTAL_READ  BIT(15)  /* Flash supports DDR Octal Read */

};

#define JEDEC_MFR(info) ((info)->id[0])

static const struct lm_spi_dev_id *__spi_nor_match_id (const char *name);

/* 读状态 */
static int __spi_nor_read_sr (lm_spi_nor_dev_t *p_nor)
{
    int ret;
    uint8_t val;

    ret = p_nor->pfunc_read_reg(p_nor, LM_SPINOR_OP_RDSR, &val, 1);

    if (ret < 0) {
//        lm_pr_err("error %d reading SR\n", (int) ret);
        return ret;
    }

    return val;
}


/*
 * Read the flag status register, returning its value in the location
 */
static int __spi_nor_read_fsr (lm_spi_nor_dev_t *p_nor)
{
    int ret;
    uint8_t val;

    ret = p_nor->pfunc_read_reg(p_nor, LM_SPINOR_OP_RDFSR, &val, 1);
    if (ret < 0) {
//        lm_pr_err("error %d reading FSR\n", ret);
        return ret;
    }

    return val;
}

/*
 * Read configuration register, returning its value in the
 */
static int __spi_nor_read_cr (lm_spi_nor_dev_t *p_nor)
{
    int ret;
    uint8_t val;

    ret = p_nor->pfunc_read_reg(p_nor, LM_SPINOR_OP_RDCR, &val, 1);
    if (ret < 0) {
//        lm_pr_err("error %d reading CR\n", ret);
        return ret;
    }

    return val;
}

/*
 * 写状态寄存器
 */
static inline int __spi_nor_write_sr (lm_spi_nor_dev_t *p_nor, uint8_t val)
{
    p_nor->cmd_buf[0] = val;
    return p_nor->pfunc_write_reg(p_nor, LM_SPINOR_OP_WRSR, p_nor->cmd_buf, 1);
}
/*
 * 写使能
 */
static inline int __spi_nor_write_enable (lm_spi_nor_dev_t *p_nor)
{
    return p_nor->pfunc_write_reg(p_nor, LM_SPINOR_OP_WREN, NULL, 0);
}

/*
 * 写失能
 */
static inline int __spi_nor_write_disable (lm_spi_nor_dev_t *p_nor)
{
    return p_nor->pfunc_write_reg(p_nor, LM_SPINOR_OP_WRDI, NULL, 0);
}


static uint8_t spi_nor_convert_opcode(uint8_t opcode, const uint8_t table[][2], size_t size)
{
    size_t i;

    for (i = 0; i < size; i++)
        if (table[i][0] == opcode)
            return table[i][1];

    /* No conversion found, keep input op code. */
    return opcode;
}

static inline uint8_t spi_nor_convert_str_to_dtr_read(uint8_t opcode)
{
    static const uint8_t spi_nor_sdr_to_dtr_read[][2] = {
        { LM_SPINOR_OP_READ_FAST,  LM_SPINOR_OP_READ_1_1_1_DTR },
        { LM_SPINOR_OP_READ_1_2_2, LM_SPINOR_OP_READ_1_2_2_DTR },
        { LM_SPINOR_OP_READ_1_4_4, LM_SPINOR_OP_READ_1_4_4_DTR },
    };

    return spi_nor_convert_opcode(opcode, spi_nor_sdr_to_dtr_read,
                      ARRAY_LEN(spi_nor_sdr_to_dtr_read));
};

static inline uint8_t spi_nor_convert_3to4_read(uint8_t opcode)
{
    static const uint8_t spi_nor_3to4_read[][2] = {
        { LM_SPINOR_OP_READ,       LM_SPINOR_OP_READ_4B },
        { LM_SPINOR_OP_READ_FAST,  LM_SPINOR_OP_READ_FAST_4B },
        { LM_SPINOR_OP_READ_1_1_2, LM_SPINOR_OP_READ_1_1_2_4B },
        { LM_SPINOR_OP_READ_1_2_2, LM_SPINOR_OP_READ_1_2_2_4B },
        { LM_SPINOR_OP_READ_1_1_4, LM_SPINOR_OP_READ_1_1_4_4B },
        { LM_SPINOR_OP_READ_1_4_4, LM_SPINOR_OP_READ_1_4_4_4B },

        { LM_SPINOR_OP_READ_1_1_1_DTR,    LM_SPINOR_OP_READ_1_1_1_DTR_4B },
        { LM_SPINOR_OP_READ_1_2_2_DTR,    LM_SPINOR_OP_READ_1_2_2_DTR_4B },
        { LM_SPINOR_OP_READ_1_4_4_DTR,    LM_SPINOR_OP_READ_1_4_4_DTR_4B },
    };

    return spi_nor_convert_opcode(opcode, spi_nor_3to4_read,
                      ARRAY_LEN(spi_nor_3to4_read));
}

static inline uint8_t spi_nor_convert_3to4_program(uint8_t opcode)
{
    static const uint8_t spi_nor_3to4_program[][2] = {
        { LM_SPINOR_OP_PP,         LM_SPINOR_OP_PP_4B },
        { LM_SPINOR_OP_PP_1_1_4, LM_SPINOR_OP_PP_1_1_4_4B },
        { LM_SPINOR_OP_PP_1_4_4, LM_SPINOR_OP_PP_1_4_4_4B },
    };

    return spi_nor_convert_opcode(opcode, spi_nor_3to4_program,
                      ARRAY_LEN(spi_nor_3to4_program));
}

static inline uint8_t spi_nor_convert_3to4_erase(uint8_t opcode)
{
    static const uint8_t spi_nor_3to4_erase[][2] = {
        { LM_SPINOR_OP_BE_4K,    LM_SPINOR_OP_BE_4K_4B },
        { LM_SPINOR_OP_BE_32K,    LM_SPINOR_OP_BE_32K_4B },
        { LM_SPINOR_OP_SE,        LM_SPINOR_OP_SE_4B },
    };

    return spi_nor_convert_opcode(opcode, spi_nor_3to4_erase,
                      ARRAY_LEN(spi_nor_3to4_erase));
}

static void __spi_nor_set_4byte_opcodes(lm_spi_nor_dev_t *p_nor,
                      const struct flash_info *info)
{
    /* Do some manufacturer fixups first */
    switch (JEDEC_MFR(info)) {
    case SNOR_MFR_SPANSION:
        /* No small sector erase for 4-byte command set */
        p_nor->erase_opcode = LM_SPINOR_OP_SE;
        p_nor->nvram.erasesize = info->sector_size;
        break;

    default:
        break;
    }

    p_nor->read_opcode = spi_nor_convert_3to4_read(p_nor->read_opcode);
    p_nor->program_opcode = spi_nor_convert_3to4_program(p_nor->program_opcode);
    p_nor->erase_opcode = spi_nor_convert_3to4_erase(p_nor->erase_opcode);
}


/*
 * 4线模式设置
 */
static inline int __spi_nor_set_4byte (lm_spi_nor_dev_t      *p_nor,
                                       struct flash_info     *p_info,
                                       int                    enable)
{
    int status;
    uint8_t need_wren = LM_FALSE;
    uint8_t cmd;

    switch (JEDEC_MFR(p_info)) {

    case SNOR_MFR_MICRON:
    case SNOR_MFR_MICRONO:
        /* Micron 需要 WREN */
        need_wren = LM_TRUE;
    case SNOR_MFR_MACRONIX:
    case SNOR_MFR_WINBOND:
        if (need_wren)
            __spi_nor_write_enable(p_nor);

        cmd = enable ? LM_SPINOR_OP_EN4B : LM_SPINOR_OP_EX4B;
        status = p_nor->pfunc_write_reg(p_nor, cmd, NULL, 0);
        if (need_wren)
            __spi_nor_write_disable(p_nor);

        return status;
    default:
        /* Spansion style */
        p_nor->cmd_buf[0] = enable << 7;
        return p_nor->pfunc_write_reg(p_nor, LM_SPINOR_OP_BRWR, p_nor->cmd_buf, 1);
    }
}



static inline int __spi_nor_sr_ready (lm_spi_nor_dev_t *p_nor)
{
    int sr = __spi_nor_read_sr(p_nor);
    if (sr < 0)
        return sr;
    else
        return !(sr & SR_WIP);
}

static inline int __spi_nor_fsr_ready (lm_spi_nor_dev_t *p_nor)
{
    int fsr = __spi_nor_read_fsr(p_nor);
    if (fsr < 0)
        return fsr;
    else
        return fsr & FSR_READY;
}

static int __spi_nor_ready (lm_spi_nor_dev_t *p_nor)
{
    int sr, fsr;
    sr = __spi_nor_sr_ready(p_nor);
    if (sr < 0)
        return sr;
    fsr = (p_nor->flags & LM_SNOR_F_USE_FSR) ? __spi_nor_fsr_ready(p_nor) : 1;
    if (fsr < 0)
        return fsr;
    return sr && fsr;
}

/*
 * 等待就绪或者超时
 */
static int __spi_nor_wait_till_ready_with_timeout (lm_spi_nor_dev_t *p_nor,
                                                 unsigned long timeout_jiffies)
{
    int         ret;
    unsigned long timeout = 0;

    timeout = MAX_READY_WAIT_TIME + timeout_jiffies;

    while (timeout) {

        ret = __spi_nor_ready(p_nor);
        if (ret < 0)
            return ret;
        if (ret)
            return 0;

        lm_task_delay(1);
        timeout --;
    }

    return -LM_ETIMEOUT;
}

static int __spi_nor_wait_till_ready (lm_spi_nor_dev_t *p_nor)
{
    return __spi_nor_wait_till_ready_with_timeout(p_nor,
                           100);  /* todo: 延时需要处理 */
}

/*
 * 擦除整片
 */
static int __spi_nor_erase_chip(lm_spi_nor_dev_t *p_nor)
{
    return p_nor->pfunc_write_reg(p_nor, LM_SPINOR_OP_CHIP_ERASE, NULL, 0);
}

static int __spi_nor_lock_and_prep (lm_spi_nor_dev_t *p_nor, enum lm_spi_nor_ops ops)
{
    int ret = 0;

    lm_mutex_lock(&p_nor->lock, LM_SEM_WAIT_FOREVER);

    if (p_nor->pfunc_prepare) {
        ret = p_nor->pfunc_prepare(p_nor, ops);
        if (ret) {
//            dev_err(p_nor->dev, "failed in the preparation.\n");
            lm_mutex_unlock(&p_nor->lock);
            return ret;
        }
    }
    return ret;
}

static void spi_nor_unlock_and_unprep(lm_spi_nor_dev_t *p_nor, enum lm_spi_nor_ops ops)
{
    if (p_nor->pfunc_unprepare)
        p_nor->pfunc_unprepare(p_nor, ops);
    lm_mutex_unlock(&p_nor->lock);
}

/*
 * 擦除一个扇区
 */
static int __spi_nor_erase_sector(lm_spi_nor_dev_t *p_nor, uint32_t addr)
{
    uint8_t buf[SPI_NOR_MAX_ADDR_WIDTH];
    int i;

    if (p_nor->pfunc_erase)
        return p_nor->pfunc_erase(p_nor, addr);

    /*
     * Default implementation, if driver doesn't have a specialized HW
     * control
     */
    for (i = p_nor->addr_width - 1; i >= 0; i--) {
        buf[i] = addr & 0xff;
        addr >>= 8;
    }

    return p_nor->pfunc_write_reg(p_nor, p_nor->erase_opcode, buf, p_nor->addr_width);
}

/*
 * 擦除一个或者多个区域
 */
static int __spi_nor_erase (lm_nvram_dev_t *p_nvram, struct erase_info *instr)
{
    lm_spi_nor_dev_t *p_nor = lm_nvram_to_spi_nor(p_nvram);
    uint32_t addr, len;
    uint32_t rem;
    int ret;

    rem = instr->len % p_nvram->erasesize;
    if (rem) {
        return -LM_EINVAL;
    }

    addr = instr->addr;
    len = instr->len;

    ret = __spi_nor_lock_and_prep(p_nor, SPI_NOR_OPS_ERASE);
    if (ret)
        return ret;

    /* 擦除整片 */
    if (len == p_nvram->size) {
        __spi_nor_write_enable(p_nor);

        if (__spi_nor_erase_chip(p_nor)) {
            ret = -LM_EIO;
            goto erase_err;
        }

        /*
         * todo 需要计算延时
         */
        ret = __spi_nor_wait_till_ready(p_nor);
        if (ret)
            goto erase_err;

    } else {
        while (len) {
            __spi_nor_write_enable(p_nor);

            ret = __spi_nor_erase_sector(p_nor, addr);
            if (ret) {
                goto erase_err;
            }

            addr += p_nvram->erasesize;
            len -= p_nvram->erasesize;

            ret = __spi_nor_wait_till_ready(p_nor);
            if (ret)
                goto erase_err;
        }
    }

    __spi_nor_write_disable(p_nor);

erase_err:
    spi_nor_unlock_and_unprep(p_nor, SPI_NOR_OPS_ERASE);

    instr->state = ret ? NVRAM_ERASE_FAILED : NVRAM_ERASE_DONE;
    /* 可以调用完成回调 */

    return ret;
}


#if 0
static int spi_nor_lock (lm_nvram_dev_t *p_nvram, long ofs, uint64_t len)
{
    lm_spi_nor_dev_t *p_nor = lm_nvram_to_spi_nor(p_nvram);
    uint32_t offset = ofs;
    uint8_t status_old, status_new;
    int ret = 0;

    ret = __spi_nor_lock_and_prep(p_nor, SPI_NOR_OPS_LOCK);
    if (ret)
        return ret;

    status_old = __spi_nor_read_sr(p_nor);

    if (offset < p_nvram->size - (p_nvram->size / 2))
        status_new = status_old | SR_BP2 | SR_BP1 | SR_BP0;
    else if (offset < p_nvram->size - (p_nvram->size / 4))
        status_new = (status_old & ~SR_BP0) | SR_BP2 | SR_BP1;
    else if (offset < p_nvram->size - (p_nvram->size / 8))
        status_new = (status_old & ~SR_BP1) | SR_BP2 | SR_BP0;
    else if (offset < p_nvram->size - (p_nvram->size / 16))
        status_new = (status_old & ~(SR_BP0 | SR_BP1)) | SR_BP2;
    else if (offset < p_nvram->size - (p_nvram->size / 32))
        status_new = (status_old & ~SR_BP2) | SR_BP1 | SR_BP0;
    else if (offset < p_nvram->size - (p_nvram->size / 64))
        status_new = (status_old & ~(SR_BP2 | SR_BP0)) | SR_BP1;
    else
        status_new = (status_old & ~(SR_BP2 | SR_BP1)) | SR_BP0;

    /* Only modify protection if it will not unlock other areas */
    if ((status_new & (SR_BP2 | SR_BP1 | SR_BP0)) >
                (status_old & (SR_BP2 | SR_BP1 | SR_BP0))) {
        __spi_nor_write_enable(p_nor);
        ret = __spi_nor_write_sr(p_nor, status_new);
        if (ret)
            goto err;
    }

err:
    spi_nor_unlock_and_unprep(p_nor, SPI_NOR_OPS_LOCK);
    return ret;
}

static int spi_nor_unlock(lm_nvram_dev_t *p_nvram, long ofs, uint64_t len)
{
    lm_spi_nor_dev_t *p_nor = lm_nvram_to_spi_nor(p_nvram);
    uint32_t offset = ofs;
    uint8_t status_old, status_new;
    int ret = 0;

    ret = __spi_nor_lock_and_prep(p_nor, SPI_NOR_OPS_UNLOCK);
    if (ret)
        return ret;

    status_old = __spi_nor_read_sr(p_nor);

    if (offset+len > p_nvram->size - (p_nvram->size / 64))
        status_new = status_old & ~(SR_BP2 | SR_BP1 | SR_BP0);
    else if (offset+len > p_nvram->size - (p_nvram->size / 32))
        status_new = (status_old & ~(SR_BP2 | SR_BP1)) | SR_BP0;
    else if (offset+len > p_nvram->size - (p_nvram->size / 16))
        status_new = (status_old & ~(SR_BP2 | SR_BP0)) | SR_BP1;
    else if (offset+len > p_nvram->size - (p_nvram->size / 8))
        status_new = (status_old & ~SR_BP2) | SR_BP1 | SR_BP0;
    else if (offset+len > p_nvram->size - (p_nvram->size / 4))
        status_new = (status_old & ~(SR_BP0 | SR_BP1)) | SR_BP2;
    else if (offset+len > p_nvram->size - (p_nvram->size / 2))
        status_new = (status_old & ~SR_BP1) | SR_BP2 | SR_BP0;
    else
        status_new = (status_old & ~SR_BP0) | SR_BP2 | SR_BP1;

    /* Only modify protection if it will not lock other areas */
    if ((status_new & (SR_BP2 | SR_BP1 | SR_BP0)) <
                (status_old & (SR_BP2 | SR_BP1 | SR_BP0))) {
        __spi_nor_write_enable(p_nor);
        ret = __spi_nor_write_sr(p_nor, status_new);
        if (ret)
            goto err;
    }

err:
    spi_nor_unlock_and_unprep(p_nor, SPI_NOR_OPS_UNLOCK);
    return ret;
}
#endif

/* Used when the "_ext_id" is two bytes at most */
#define INFO(_jedec_id, _ext_id, _sector_size, _n_sectors, _flags)  \
    ((unsigned long)&(struct flash_info) {             \
        .id = {                         \
            ((_jedec_id) >> 16) & 0xff,         \
            ((_jedec_id) >> 8) & 0xff,          \
            (_jedec_id) & 0xff,             \
            ((_ext_id) >> 8) & 0xff,            \
            (_ext_id) & 0xff,               \
            },                      \
        .id_len = (!(_jedec_id) ? 0 : (3 + ((_ext_id) ? 2 : 0))),   \
        .sector_size = (_sector_size),              \
        .n_sectors = (_n_sectors),              \
        .page_size = 256,                   \
        .flags = (_flags),                  \
    })

#define INFO6(_jedec_id, _ext_id, _sector_size, _n_sectors, _flags) \
    ((unsigned long)&(struct flash_info) {             \
        .id = {                         \
            ((_jedec_id) >> 16) & 0xff,         \
            ((_jedec_id) >> 8) & 0xff,          \
            (_jedec_id) & 0xff,             \
            ((_ext_id) >> 16) & 0xff,           \
            ((_ext_id) >> 8) & 0xff,            \
            (_ext_id) & 0xff,               \
            },                      \
        .id_len = 6,                        \
        .sector_size = (_sector_size),              \
        .n_sectors = (_n_sectors),              \
        .page_size = 256,                   \
        .flags = (_flags),                  \
    })

#define CAT25_INFO(_sector_size, _n_sectors, _page_size, _addr_width, _flags)   \
    ((unsigned long)&(struct flash_info) {             \
        .sector_size = (_sector_size),              \
        .n_sectors = (_n_sectors),              \
        .page_size = (_page_size),              \
        .addr_width = (_addr_width),                \
        .flags = (_flags),                  \
    })

/* NOTE: double check command sets and memory organization when you add
 * more p_nor chips.  This current list focusses on newer chips, which
 * have been converging on command sets which including JEDEC ID.
 */
static const struct lm_spi_dev_id __spi_nor_ids[] = {
    /* Atmel -- some are (confusingly) marketed as "DataFlash" */
    { "at25fs010",  INFO(0x1f6601, 0, 32 * 1024,   4, SECT_4K) },
    { "at25fs040",  INFO(0x1f6604, 0, 64 * 1024,   8, SECT_4K) },

    { "at25df041a", INFO(0x1f4401, 0, 64 * 1024,   8, SECT_4K) },
    { "at25df321a", INFO(0x1f4701, 0, 64 * 1024,  64, SECT_4K) },
    { "at25df641",  INFO(0x1f4800, 0, 64 * 1024, 128, SECT_4K) },

    { "at26f004",   INFO(0x1f0400, 0, 64 * 1024,  8, SECT_4K) },
    { "at26df081a", INFO(0x1f4501, 0, 64 * 1024, 16, SECT_4K) },
    { "at26df161a", INFO(0x1f4601, 0, 64 * 1024, 32, SECT_4K) },
    { "at26df321",  INFO(0x1f4700, 0, 64 * 1024, 64, SECT_4K) },

    { "at45db081d", INFO(0x1f2500, 0, 64 * 1024, 16, SECT_4K) },

    /* EON -- en25xxx */
    { "en25f32",    INFO(0x1c3116, 0, 64 * 1024,   64, SECT_4K) },
    { "en25p32",    INFO(0x1c2016, 0, 64 * 1024,   64, 0) },
    { "en25q32b",   INFO(0x1c3016, 0, 64 * 1024,   64, 0) },
    { "en25p64",    INFO(0x1c2017, 0, 64 * 1024,  128, 0) },
    { "en25q64",    INFO(0x1c3017, 0, 64 * 1024,  128, SECT_4K) },
    { "en25qh128",  INFO(0x1c7018, 0, 64 * 1024,  256, 0) },
    { "en25qh256",  INFO(0x1c7019, 0, 64 * 1024,  512, 0) },

    /* ESMT */
    { "f25l32pa", INFO(0x8c2016, 0, 64 * 1024, 64, SECT_4K) },

    /* Everspin */
    { "mr25h256", CAT25_INFO( 32 * 1024, 1, 256, 2, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR) },
    { "mr25h10",  CAT25_INFO(128 * 1024, 1, 256, 3, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR) },

    /* Fujitsu */
    { "mb85rs1mt", INFO(0x047f27, 0, 128 * 1024, 1, SPI_NOR_NO_ERASE) },

    /* GigaDevice */
    { "gd25q32", INFO(0xc84016, 0, 64 * 1024,  64, SECT_4K) },
    { "gd25q64", INFO(0xc84017, 0, 64 * 1024, 128, SECT_4K) },
    { "gd25q128", INFO(0xc84018, 0, 64 * 1024, 256, SECT_4K) },

    /* Intel/Numonyx -- xxxs33b */
    { "160s33b",  INFO(0x898911, 0, 64 * 1024,  32, 0) },
    { "320s33b",  INFO(0x898912, 0, 64 * 1024,  64, 0) },
    { "640s33b",  INFO(0x898913, 0, 64 * 1024, 128, 0) },
    /* PMC */
    { "pm25lv512",   INFO(0,        0, 32 * 1024,    2, SECT_4K_PMC) },
    { "pm25lv010",   INFO(0,        0, 32 * 1024,    4, SECT_4K_PMC) },
    { "pm25lq032",   INFO(0x7f9d46, 0, 64 * 1024,   64, SECT_4K) },

    /* Spansion -- single (large) sector size only, at least
     * for the chips listed here (without boot sectors).
     */
    { "s25sl032p",  INFO(0x010215, 0x4d00,  64 * 1024,  64, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
    { "s25sl064p",  INFO(0x010216, 0x4d00,  64 * 1024, 128, 0) },
    { "s25fl256s0", INFO(0x010219, 0x4d00, 256 * 1024, 128, 0) },
    { "s25fl256s1", INFO(0x010219, 0x4d01,  64 * 1024, 512, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
    { "s25fl512s",  INFO(0x010220, 0x4d00, 256 * 1024, 256, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
    { "s70fl01gs",  INFO(0x010221, 0x4d00, 256 * 1024, 256, 0) },
    { "s25sl12800", INFO(0x012018, 0x0300, 256 * 1024,  64, 0) },
    { "s25sl12801", INFO(0x012018, 0x0301,  64 * 1024, 256, 0) },
    { "s25fl128s",  INFO6(0x012018, 0x4d0180, 64 * 1024, 256, SPI_NOR_QUAD_READ) },
    { "s25fl129p0", INFO(0x012018, 0x4d00, 256 * 1024,  64, 0) },
    { "s25fl129p1", INFO(0x012018, 0x4d01,  64 * 1024, 256, 0) },
    { "s25sl004a",  INFO(0x010212,      0,  64 * 1024,   8, 0) },
    { "s25sl008a",  INFO(0x010213,      0,  64 * 1024,  16, 0) },
    { "s25sl016a",  INFO(0x010214,      0,  64 * 1024,  32, 0) },
    { "s25sl032a",  INFO(0x010215,      0,  64 * 1024,  64, 0) },
    { "s25sl064a",  INFO(0x010216,      0,  64 * 1024, 128, 0) },
    { "s25fl008k",  INFO(0xef4014,      0,  64 * 1024,  16, SECT_4K) },
    { "s25fl016k",  INFO(0xef4015,      0,  64 * 1024,  32, SECT_4K) },
    { "s25fl064k",  INFO(0xef4017,      0,  64 * 1024, 128, SECT_4K) },
    { "s25fl132k",  INFO(0x014016,      0,  64 * 1024,  64, 0) },

    /* SST -- large erase sizes are "overlays", "sectors" are 4K */
    { "sst25vf040b", INFO(0xbf258d, 0, 64 * 1024,  8, SECT_4K | SST_WRITE) },
    { "sst25vf080b", INFO(0xbf258e, 0, 64 * 1024, 16, SECT_4K | SST_WRITE) },
    { "sst25vf016b", INFO(0xbf2541, 0, 64 * 1024, 32, SECT_4K | SST_WRITE) },
    { "sst25vf032b", INFO(0xbf254a, 0, 64 * 1024, 64, SECT_4K | SST_WRITE) },
    { "sst25vf064c", INFO(0xbf254b, 0, 64 * 1024, 128, SECT_4K) },
    { "sst25wf512",  INFO(0xbf2501, 0, 64 * 1024,  1, SECT_4K | SST_WRITE) },
    { "sst25wf010",  INFO(0xbf2502, 0, 64 * 1024,  2, SECT_4K | SST_WRITE) },
    { "sst25wf020",  INFO(0xbf2503, 0, 64 * 1024,  4, SECT_4K | SST_WRITE) },
    { "sst25wf040",  INFO(0xbf2504, 0, 64 * 1024,  8, SECT_4K | SST_WRITE) },
    { "sst25wf080",  INFO(0xbf2505, 0, 64 * 1024, 16, SECT_4K | SST_WRITE) },

    /* ST Microelectronics -- newer production may have feature updates */
    { "m25p05",  INFO(0x202010,  0,  32 * 1024,   2, 0) },
    { "m25p10",  INFO(0x202011,  0,  32 * 1024,   4, 0) },
    { "m25p20",  INFO(0x202012,  0,  64 * 1024,   4, 0) },
    { "m25p40",  INFO(0x202013,  0,  64 * 1024,   8, 0) },
    { "m25p80",  INFO(0x202014,  0,  64 * 1024,  16, 0) },
    { "m25p16",  INFO(0x202015,  0,  64 * 1024,  32, 0) },
    { "m25p32",  INFO(0x202016,  0,  64 * 1024,  64, 0) },
    { "m25p64",  INFO(0x202017,  0,  64 * 1024, 128, 0) },
    { "m25p128", INFO(0x202018,  0, 256 * 1024,  64, 0) },

    { "m25p05-nonjedec",  INFO(0, 0,  32 * 1024,   2, 0) },
    { "m25p10-nonjedec",  INFO(0, 0,  32 * 1024,   4, 0) },
    { "m25p20-nonjedec",  INFO(0, 0,  64 * 1024,   4, 0) },
    { "m25p40-nonjedec",  INFO(0, 0,  64 * 1024,   8, 0) },
    { "m25p80-nonjedec",  INFO(0, 0,  64 * 1024,  16, 0) },
    { "m25p16-nonjedec",  INFO(0, 0,  64 * 1024,  32, 0) },
    { "m25p32-nonjedec",  INFO(0, 0,  64 * 1024,  64, 0) },
    { "m25p64-nonjedec",  INFO(0, 0,  64 * 1024, 128, 0) },
    { "m25p128-nonjedec", INFO(0, 0, 256 * 1024,  64, 0) },

    { "m45pe10", INFO(0x204011,  0, 64 * 1024,    2, 0) },
    { "m45pe80", INFO(0x204014,  0, 64 * 1024,   16, 0) },
    { "m45pe16", INFO(0x204015,  0, 64 * 1024,   32, 0) },

    { "m25pe20", INFO(0x208012,  0, 64 * 1024,  4,       0) },
    { "m25pe80", INFO(0x208014,  0, 64 * 1024, 16,       0) },
    { "m25pe16", INFO(0x208015,  0, 64 * 1024, 32, SECT_4K) },

    { "m25px16",    INFO(0x207115,  0, 64 * 1024, 32, SECT_4K) },
    { "m25px32",    INFO(0x207116,  0, 64 * 1024, 64, SECT_4K) },
    { "m25px32-s0", INFO(0x207316,  0, 64 * 1024, 64, SECT_4K) },
    { "m25px32-s1", INFO(0x206316,  0, 64 * 1024, 64, SECT_4K) },
    { "m25px64",    INFO(0x207117,  0, 64 * 1024, 128, 0) },
    { "m25px80",    INFO(0x207114,  0, 64 * 1024, 16, 0) },

    /* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
    /* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
    { "w25x05", INFO(0xef3010, 0, 64 * 1024,  1,  SECT_4K) },
    { "w25x10", INFO(0xef3011, 0, 64 * 1024,  2,  SECT_4K) },
    { "w25x20", INFO(0xef3012, 0, 64 * 1024,  4,  SECT_4K) },
    { "w25x40", INFO(0xef3013, 0, 64 * 1024,  8,  SECT_4K) },
    { "w25x80", INFO(0xef3014, 0, 64 * 1024,  16, SECT_4K) },
    { "w25x16", INFO(0xef3015, 0, 64 * 1024,  32, SECT_4K) },
    { "w25x32", INFO(0xef3016, 0, 64 * 1024,  64, SECT_4K) },
    { "w25q20cl", INFO(0xef4012, 0, 64 * 1024,  4, SECT_4K) },
    { "w25q20bw", INFO(0xef5012, 0, 64 * 1024,  4, SECT_4K) },
    { "w25q20ew", INFO(0xef6012, 0, 64 * 1024,  4, SECT_4K) },
    { "w25q32", INFO(0xef4016, 0, 64 * 1024,  64, SECT_4K) },
    {
        "w25q32dw", INFO(0xef6016, 0, 64 * 1024,  64,
            SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
            SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
    },
    { "w25x64", INFO(0xef3017, 0, 64 * 1024, 128, SECT_4K) },
    { "w25q64", INFO(0xef4017, 0, 64 * 1024, 128, SECT_4K) },
    {
        "w25q64dw", INFO(0xef6017, 0, 64 * 1024, 128,
            SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
            SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
    },
    {
        "w25q128fw", INFO(0xef6018, 0, 64 * 1024, 256,
            SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
            SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
    },
    { "w25q80", INFO(0xef5014, 0, 64 * 1024,  16, SECT_4K) },
    { "w25q80bl", INFO(0xef4014, 0, 64 * 1024,  16, SECT_4K) },
    { "w25q128", INFO(0xef4018, 0, 64 * 1024, 256, SECT_4K) },
    { "w25q256", INFO(0xef4019, 0, 64 * 1024, 512, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
    { "w25m512jv", INFO(0xef7119, 0, 64 * 1024, 1024,
            SECT_4K | SPI_NOR_QUAD_READ | SPI_NOR_DUAL_READ) },


    /* Catalyst / On Semiconductor -- non-JEDEC */
    { "cat25c11", CAT25_INFO(  16, 8, 16, 1, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR) },
    { "cat25c03", CAT25_INFO(  32, 8, 16, 2, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR) },
    { "cat25c09", CAT25_INFO( 128, 8, 32, 2, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR) },
    { "cat25c17", CAT25_INFO( 256, 8, 32, 2, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR) },
    { "cat25128", CAT25_INFO(2048, 8, 64, 2, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR) },
    { },
};

static const struct lm_spi_dev_id *__spi_nor_read_id (lm_spi_nor_dev_t *p_nor)
{
    int                  tmp;
    uint8_t              id[SPI_NOR_MAX_ID_LEN];
    const struct flash_info   *info;

    tmp = p_nor->pfunc_read_reg(p_nor, LM_SPINOR_OP_RDID, id, SPI_NOR_MAX_ID_LEN);
    if (tmp < 0) {
//        dev_dbg(p_nor->dev, " error %d reading JEDEC ID\n", ret);
        return NULL;
    }

    for (tmp = 0; tmp < ARRAY_LEN(__spi_nor_ids) - 1; tmp++) {
        info = (void*)__spi_nor_ids[tmp].driver_data;
        if (info->id_len) {
            if (!memcmp(info->id, id, info->id_len))
                return &__spi_nor_ids[tmp];
        }
    }
//    dev_err(p_nor->dev, "unrecognized JEDEC id bytes: %02x, %2x, %2x\n",
//        id[0], id[1], id[2]);
    return NULL;
}

static int __spi_nor_read(lm_nvram_dev_t *p_nvram, uint32_t from,
                        uint8_t *buf, size_t len, size_t *retlen)
{
    lm_spi_nor_dev_t *p_nor = lm_nvram_to_spi_nor(p_nvram);
    int ret;
    size_t tmp_len;

//    dev_dbg(p_nor->dev, "from 0x%08x, len %zd\n", (uint32_t)from, len);

    ret = __spi_nor_lock_and_prep(p_nor, SPI_NOR_OPS_READ);
    if (ret) {
        return ret;
    }

    while (len) {
        uint32_t addr = from;

        ret = p_nor->pfunc_read(p_nor, addr, len, buf);
        if (ret == 0) {
            /* We shouldn't see 0-length reads */
            ret = -LM_EIO;
            goto read_err;
        }
        if (ret < 0)
            goto read_err;

        tmp_len += ret;
        buf += ret;
        from += ret;
        len -= ret;
    }
    ret = 0;

    if (retlen) {
        *retlen = tmp_len;
    }

read_err:
    spi_nor_unlock_and_unprep(p_nor, SPI_NOR_OPS_READ);
    return ret;

}


/*
 * 将地址范围内的数写入的spi p_nor(页编程)
 */
static int __spi_nor_write(lm_nvram_dev_t *p_nvram, uint32_t to,
    const uint8_t *buf, size_t len, size_t *retlen)
{
    lm_spi_nor_dev_t *p_nor = lm_nvram_to_spi_nor(p_nvram);
    uint32_t page_offset, page_size, i;
    int ret;

//    dev_dbg(p_nor->dev, "to 0x%08x, len %zd\n", (uint32_t)to, len);

    ret = __spi_nor_lock_and_prep(p_nor, SPI_NOR_OPS_WRITE);
    if (ret)
        return ret;

    __spi_nor_write_enable(p_nor);

    page_offset = to & (p_nor->page_size - 1);

    /* 可以写在一页上 */
    if (page_offset +len <= p_nor->page_size) {
        p_nor->pfunc_write(p_nor, to, len, buf);
    } else {
        page_size = p_nor->page_size - page_offset;
        p_nor->pfunc_write(p_nor, to, page_size, buf);

        for (i = page_size; i < len; i += page_size) {
            page_size = len -i;
            if (page_size > p_nor->page_size) {
                page_size = p_nor->page_size;
            }

            ret = __spi_nor_wait_till_ready(p_nor);
            if (ret) {
                goto write_err;
            }

            __spi_nor_write_enable(p_nor);

            p_nor->pfunc_write(p_nor, to + i, page_size, buf + i);
        }
    }

    ret = __spi_nor_wait_till_ready(p_nor);

write_err:
    spi_nor_unlock_and_unprep(p_nor, SPI_NOR_OPS_WRITE);
    return ret;
}

static int macronix_quad_enable (lm_spi_nor_dev_t *p_nor)
{
    int ret, val;

    val = __spi_nor_read_sr(p_nor);
    __spi_nor_write_enable(p_nor);

    p_nor->cmd_buf[0] = val | SR_QUAD_EN_MX;
    p_nor->pfunc_write_reg(p_nor, LM_SPINOR_OP_WRSR, p_nor->cmd_buf, 1);

    if (__spi_nor_wait_till_ready(p_nor))
        return 1;

    ret = __spi_nor_read_sr(p_nor);
    if (!(ret > 0 && (ret & SR_QUAD_EN_MX))) {
//        dev_err(p_nor->dev, "Macronix Quad bit not set\n");
        return -LM_EINVAL;
    }

    return 0;
}

/*
 * 具有2个字节的写状态寄存器和配置寄存器
 * 第一个字节将被写入状态寄存器
 * 第二个字节将被写入配置寄存器。
 * 如果发生错误，返回负数。
 */
static int write_sr_cr(lm_spi_nor_dev_t *p_nor, uint8_t *sr_cr)
{
    int ret;
    __spi_nor_write_enable(p_nor);

    ret = p_nor->pfunc_write_reg(p_nor, LM_SPINOR_OP_WRSR, sr_cr, 2);
    if (ret < 0) {
//        dev_err(p_nor->dev,
//            "error while writing configuration register\n");
        return -LM_EINVAL;
    }

    ret = __spi_nor_wait_till_ready(p_nor);
    if (ret) {
//        dev_err(p_nor->dev,
//            "timeout while writing configuration register\n");
        return ret;
    }

    return 0;
}


#if 0
static int micron_quad_enable(lm_spi_nor_dev_t *p_nor)
{
    int ret;
    uint8_t val;

    ret = p_nor->pfunc_read_reg(p_nor, LM_SPINOR_OP_RD_EVCR, &val, 1);
    if (ret < 0) {
//        dev_err(p_nor->dev, "error %d reading EVCR\n", ret);
        return ret;
    }

    __spi_nor_write_enable(p_nor);

    /* set EVCR, enable quad I/O */
    p_nor->cmd_buf[0] = val & ~EVCR_QUAD_EN_MICRON;
    ret = p_nor->pfunc_write_reg(p_nor, LM_SPINOR_OP_WD_EVCR, p_nor->cmd_buf, 1);
    if (ret < 0) {
//        dev_err(p_nor->dev, "error while writing EVCR register\n");
        return ret;
    }

    ret = __spi_nor_wait_till_ready(p_nor);
    if (ret)
        return ret;

    /* read EVCR and check it */
    ret = p_nor->pfunc_read_reg(p_nor, LM_SPINOR_OP_RD_EVCR, &val, 1);
    if (ret < 0) {
//        dev_err(p_nor->dev, "error %d reading EVCR\n", ret);
        return ret;
    }
    if (val & EVCR_QUAD_EN_MICRON) {
//        dev_err(p_nor->dev, "Micron EVCR Quad bit not clear\n");
        return -LM_EINVAL;
    }

    return 0;
}

#endif

static int spi_nor_check(lm_spi_nor_dev_t *p_nor)
{

    return 0;
}


struct spi_nor_read_command {
    uint8_t          num_mode_clocks;
    uint8_t          num_wait_states;
    uint8_t          opcode;
    enum lm_spi_nor_protocol   proto;
};

struct spi_nor_pp_command {
    uint8_t          opcode;
    enum lm_spi_nor_protocol   proto;
};


enum spi_nor_read_command_index {
    SNOR_CMD_READ,
    SNOR_CMD_READ_FAST,
    SNOR_CMD_READ_1_1_1_DTR,

    /* 2线 SPI */
    SNOR_CMD_READ_1_1_2,
    SNOR_CMD_READ_1_2_2,
    SNOR_CMD_READ_2_2_2,
    SNOR_CMD_READ_1_2_2_DTR,

    /* 4线 SPI */
    SNOR_CMD_READ_1_1_4,
    SNOR_CMD_READ_1_4_4,
    SNOR_CMD_READ_4_4_4,
    SNOR_CMD_READ_1_4_4_DTR,

    /* 8线 SPI */
    SNOR_CMD_READ_1_1_8,
    SNOR_CMD_READ_1_8_8,
    SNOR_CMD_READ_8_8_8,
    SNOR_CMD_READ_1_8_8_DTR,

    SNOR_CMD_READ_MAX
};


enum spi_nor_pp_command_index {
    SNOR_CMD_PP,

    /* 4线 SPI */
    SNOR_CMD_PP_1_1_4,
    SNOR_CMD_PP_1_4_4,
    SNOR_CMD_PP_4_4_4,

    /* 8线 SPI */
    SNOR_CMD_PP_1_1_8,
    SNOR_CMD_PP_1_8_8,
    SNOR_CMD_PP_8_8_8,

    SNOR_CMD_PP_MAX
};

struct spi_nor_flash_parameter {
    uint64_t             size;
    uint32_t             page_size;

    struct spi_nor_hwcaps       hwcaps;
    struct spi_nor_read_command reads[SNOR_CMD_READ_MAX];
    struct spi_nor_pp_command   page_programs[SNOR_CMD_PP_MAX];

    int (*quad_enable)(lm_spi_nor_dev_t *p_nor);
};

static void
__spi_nor_set_read_settings(struct spi_nor_read_command *read,
              uint8_t num_mode_clocks,
              uint8_t num_wait_states,
              uint8_t opcode,
              enum lm_spi_nor_protocol proto)
{
    read->num_mode_clocks = num_mode_clocks;
    read->num_wait_states = num_wait_states;
    read->opcode = opcode;
    read->proto = proto;
}


static void
__spi_nor_set_pp_settings(struct spi_nor_pp_command *pp,
            uint8_t opcode,
            enum lm_spi_nor_protocol proto)
{
    pp->opcode = opcode;
    pp->proto = proto;
}

/*
 * SFDP 解析
 */

/*
 * 读取SFDP参数,命令后跟3字节地址和8个虚拟时钟
 */
static int spi_nor_read_sfdp (lm_spi_nor_dev_t *p_nor, uint32_t addr,
                              size_t            len,   void    *buf)
{
    uint8_t addr_width, read_opcode, read_dummy;
    int ret;

    read_opcode = p_nor->read_opcode;
    addr_width = p_nor->addr_width;
    read_dummy = p_nor->read_dummy;

    p_nor->read_opcode = LM_SPINOR_OP_RDSFDP;
    p_nor->addr_width = 3;
    p_nor->read_dummy = 8;

    while (len) {
        ret = p_nor->pfunc_read(p_nor, addr, len, (uint8_t *)buf);
        if (!ret || ret > len) {
            ret = -LM_EIO;
            goto read_err;
        }
        if (ret < 0)
            goto read_err;

        buf += ret;
        addr += ret;
        len -= ret;
    }
    ret = 0;

read_err:
    p_nor->read_opcode = read_opcode;
    p_nor->addr_width = addr_width;
    p_nor->read_dummy = read_dummy;

    return ret;
}

struct sfdp_parameter_header {
    uint8_t        id_lsb;
    uint8_t        minor;
    uint8_t        major;
    uint8_t        length; /* in double words */
    uint8_t        parameter_table_pointer[3]; /* byte address */
    uint8_t        id_msb;
};

#define SFDP_PARAM_HEADER_ID(p)    (((p)->id_msb << 8) | (p)->id_lsb)
#define SFDP_PARAM_HEADER_PTP(p) \
    (((p)->parameter_table_pointer[2] << 16) | \
     ((p)->parameter_table_pointer[1] <<  8) | \
     ((p)->parameter_table_pointer[0] <<  0))

#define SFDP_BFPT_ID        0xff00    /* Basic Flash Parameter Table */
#define SFDP_SECTOR_MAP_ID    0xff81    /* Sector Map Table */

#define SFDP_SIGNATURE        0x50444653U
#define SFDP_JESD216_MAJOR     1
#define SFDP_JESD216_MINOR     0
#define SFDP_JESD216A_MINOR    5
#define SFDP_JESD216B_MINOR    6

struct sfdp_header {
    uint32_t       signature;    /* Ox50444653U <=> "SFDP" */
    uint8_t        minor;
    uint8_t        major;
    uint8_t        nph;      /* 0-base number of parameter headers */
    uint8_t        unused;

    /* Basic Flash Parameter Table. */
    struct sfdp_parameter_header    bfpt_header;
};

/* Basic Flash Parameter Table */

/*
 * JESD216 rev B defines a Basic Flash Parameter Table of 16 DWORDs.
 * They are indexed from 1 but C arrays are indexed from 0.
 */
#define BFPT_DWORD(i)        ((i) - 1)
#define BFPT_DWORD_MAX        16

/* The first version of JESB216 defined only 9 DWORDs. */
#define BFPT_DWORD_MAX_JESD216            9

/* 1st DWORD. */
#define BFPT_DWORD1_FAST_READ_1_1_2        BIT(16)
#define BFPT_DWORD1_ADDRESS_BYTES_MASK        GENMASK(18, 17)
#define BFPT_DWORD1_ADDRESS_BYTES_3_ONLY    (0x0UL << 17)
#define BFPT_DWORD1_ADDRESS_BYTES_3_OR_4    (0x1UL << 17)
#define BFPT_DWORD1_ADDRESS_BYTES_4_ONLY    (0x2UL << 17)
#define BFPT_DWORD1_DTR                BIT(19)
#define BFPT_DWORD1_FAST_READ_1_2_2        BIT(20)
#define BFPT_DWORD1_FAST_READ_1_4_4        BIT(21)
#define BFPT_DWORD1_FAST_READ_1_1_4        BIT(22)

/* 5th DWORD. */
#define BFPT_DWORD5_FAST_READ_2_2_2        BIT(0)
#define BFPT_DWORD5_FAST_READ_4_4_4        BIT(4)

/* 11th DWORD. */
#define BFPT_DWORD11_PAGE_SIZE_SHIFT        4
#define BFPT_DWORD11_PAGE_SIZE_MASK        GENMASK(7, 4)

/* 15th DWORD. */

/*
 * (from JESD216 rev B)
 * Quad Enable Requirements (QER):
 * - 000b: Device does not have a QE bit. Device detects 1-1-4 and 1-4-4
 *         reads based on instruction. DQ3/HOLD# functions are hold during
 *         instruction phase.
 * - 001b: QE is bit 1 of status register 2. It is set via Write Status with
 *         two data bytes where bit 1 of the second byte is one.
 *         [...]
 *         Writing only one byte to the status register has the side-effect of
 *         clearing status register 2, including the QE bit. The 100b code is
 *         used if writing one byte to the status register does not modify
 *         status register 2.
 * - 010b: QE is bit 6 of status register 1. It is set via Write Status with
 *         one data byte where bit 6 is one.
 *         [...]
 * - 011b: QE is bit 7 of status register 2. It is set via Write status
 *         register 2 instruction 3Eh with one data byte where bit 7 is one.
 *         [...]
 *         The status register 2 is read using instruction 3Fh.
 * - 100b: QE is bit 1 of status register 2. It is set via Write Status with
 *         two data bytes where bit 1 of the second byte is one.
 *         [...]
 *         In contrast to the 001b code, writing one byte to the status
 *         register does not modify status register 2.
 * - 101b: QE is bit 1 of status register 2. Status register 1 is read using
 *         Read Status instruction 05h. Status register2 is read using
 *         instruction 35h. QE is set via Writ Status instruction 01h with
 *         two data bytes where bit 1 of the second byte is one.
 *         [...]
 */
#define BFPT_DWORD15_QER_MASK            GENMASK(22, 20)
#define BFPT_DWORD15_QER_NONE            (0x0UL << 20) /* Micron */
#define BFPT_DWORD15_QER_SR2_BIT1_BUGGY        (0x1UL << 20)
#define BFPT_DWORD15_QER_SR1_BIT6        (0x2UL << 20) /* Macronix */
#define BFPT_DWORD15_QER_SR2_BIT7        (0x3UL << 20)
#define BFPT_DWORD15_QER_SR2_BIT1_NO_RD        (0x4UL << 20)
#define BFPT_DWORD15_QER_SR2_BIT1        (0x5UL << 20) /* Spansion */

struct sfdp_bfpt {
    uint32_t    dwords[BFPT_DWORD_MAX];
};


/* 快速读设置 */
static inline void
spi_nor_set_read_settings_from_bfpt(lm_spi_nor_dev_t *p_nor,
                    struct spi_nor_read_command *read,
                    uint16_t half,
                    enum lm_spi_nor_protocol proto)
{
    uint8_t opcode;

    read->num_mode_clocks = (half >> 5) & 0x07;
    read->num_wait_states = (half >> 0) & 0x1f;
    read->proto = proto;
    opcode = (half >> 8) & 0xff;

    if (lm_spi_nor_protocol_is_dtr(proto))
        opcode = spi_nor_convert_str_to_dtr_read(opcode);
    if (p_nor->addr_width == 4)
        opcode = spi_nor_convert_3to4_read(opcode);

    read->opcode = opcode;

}

struct sfdp_bfpt_read {
    /* The Fast Read x-y-z hardware capability in params->hwcaps.mask. */
    uint32_t            hwcaps;

    /*
     * The <supported_bit> bit in <supported_dword> BFPT DWORD tells us
     * whether the Fast Read x-y-z command is supported.
     */
    uint32_t            supported_dword;
    uint32_t            supported_bit;

    /*
     * The half-word at offset <setting_shift> in <setting_dword> BFPT DWORD
     * encodes the op code, the number of mode clocks and the number of wait
     * states to be used by Fast Read x-y-z command.
     */
    uint32_t            settings_dword;
    uint32_t            settings_shift;

    /* The SPI protocol for this Fast Read x-y-z command. */
    enum lm_spi_nor_protocol    proto;
};

static const struct sfdp_bfpt_read sfdp_bfpt_reads[] = {
    /* Fast Read 1-1-2 */
    {
        LM_SNOR_HWCAPS_READ_1_1_2,
        BFPT_DWORD(1), BIT(16),    /* Supported bit */
        BFPT_DWORD(4), 0,    /* Settings */
        SNOR_PROTO_1_1_2,
    },

    /* Fast Read 1-2-2 */
    {
        LM_SNOR_HWCAPS_READ_1_2_2,
        BFPT_DWORD(1), BIT(20),    /* Supported bit */
        BFPT_DWORD(4), 16,    /* Settings */
        SNOR_PROTO_1_2_2,
    },

    /* Fast Read 2-2-2 */
    {
        LM_SNOR_HWCAPS_READ_2_2_2,
        BFPT_DWORD(5),  BIT(0),    /* Supported bit */
        BFPT_DWORD(6), 16,    /* Settings */
        SNOR_PROTO_2_2_2,
    },

    /* Fast Read 1-1-4 */
    {
        LM_SNOR_HWCAPS_READ_1_1_4,
        BFPT_DWORD(1), BIT(22),    /* Supported bit */
        BFPT_DWORD(3), 16,    /* Settings */
        SNOR_PROTO_1_1_4,
    },

    /* Fast Read 1-4-4 */
    {
        LM_SNOR_HWCAPS_READ_1_4_4,
        BFPT_DWORD(1), BIT(21),    /* Supported bit */
        BFPT_DWORD(3), 0,    /* Settings */
        SNOR_PROTO_1_4_4,
    },

    /* Fast Read 1-4-4-DTR */
    {
        LM_SNOR_HWCAPS_READ_1_4_4_DTR,
        BFPT_DWORD(1), BIT(21),    /* Supported bit */
        BFPT_DWORD(3), 0,    /* Settings */
        SNOR_PROTO_1_4_4_DTR,
    },

    /* Fast Read 4-4-4 */
    {
        LM_SNOR_HWCAPS_READ_4_4_4,
        BFPT_DWORD(5), BIT(4),    /* Supported bit */
        BFPT_DWORD(7), 16,    /* Settings */
        SNOR_PROTO_4_4_4,
    },
};

struct sfdp_bfpt_erase {
    /*
     * The half-word at offset <shift> in DWORD <dwoard> encodes the
     * op code and erase sector size to be used by Sector Erase commands.
     */
    uint32_t            dword;
    uint32_t            shift;
};

static const struct sfdp_bfpt_erase sfdp_bfpt_erases[] = {
    /* Erase Type 1 in DWORD8 bits[15:0] */
    {BFPT_DWORD(8), 0},

    /* Erase Type 2 in DWORD8 bits[31:16] */
    {BFPT_DWORD(8), 16},

    /* Erase Type 3 in DWORD9 bits[15:0] */
    {BFPT_DWORD(9), 0},

    /* Erase Type 4 in DWORD9 bits[31:16] */
    {BFPT_DWORD(9), 16},
};

static int spi_nor_hwcaps2cmd (uint32_t hwcaps, const int table[][2], size_t size)
{
    size_t i;

    for (i = 0; i < size; i++)
        if (table[i][0] == (int)hwcaps)
            return table[i][1];

    return -LM_EINVAL;
}

static int spi_nor_hwcaps_pp2cmd(uint32_t hwcaps)
{
    static const int hwcaps_pp2cmd[][2] = {
        { LM_SNOR_HWCAPS_PP,       SNOR_CMD_PP },
        { LM_SNOR_HWCAPS_PP_1_1_4, SNOR_CMD_PP_1_1_4 },
        { LM_SNOR_HWCAPS_PP_1_4_4, SNOR_CMD_PP_1_4_4 },
        { LM_SNOR_HWCAPS_PP_4_4_4, SNOR_CMD_PP_4_4_4 },
        { LM_SNOR_HWCAPS_PP_1_1_8, SNOR_CMD_PP_1_1_8 },
        { LM_SNOR_HWCAPS_PP_1_8_8, SNOR_CMD_PP_1_8_8 },
        { LM_SNOR_HWCAPS_PP_8_8_8, SNOR_CMD_PP_8_8_8 },
    };

    return spi_nor_hwcaps2cmd(hwcaps, hwcaps_pp2cmd,
                  ARRAY_LEN(hwcaps_pp2cmd));
}

static int spi_nor_hwcaps_read2cmd(uint32_t hwcaps)
{
    static const int hwcaps_read2cmd[][2] = {
        { LM_SNOR_HWCAPS_READ,     SNOR_CMD_READ },
        { LM_SNOR_HWCAPS_READ_FAST,    SNOR_CMD_READ_FAST },
        { LM_SNOR_HWCAPS_READ_1_1_1_DTR,   SNOR_CMD_READ_1_1_1_DTR },
        { LM_SNOR_HWCAPS_READ_1_1_2,   SNOR_CMD_READ_1_1_2 },
        { LM_SNOR_HWCAPS_READ_1_2_2,   SNOR_CMD_READ_1_2_2 },
        { LM_SNOR_HWCAPS_READ_2_2_2,   SNOR_CMD_READ_2_2_2 },
        { LM_SNOR_HWCAPS_READ_1_2_2_DTR,   SNOR_CMD_READ_1_2_2_DTR },
        { LM_SNOR_HWCAPS_READ_1_1_4,   SNOR_CMD_READ_1_1_4 },
        { LM_SNOR_HWCAPS_READ_1_4_4,   SNOR_CMD_READ_1_4_4 },
        { LM_SNOR_HWCAPS_READ_4_4_4,   SNOR_CMD_READ_4_4_4 },
        { LM_SNOR_HWCAPS_READ_1_4_4_DTR,   SNOR_CMD_READ_1_4_4_DTR },
        { LM_SNOR_HWCAPS_READ_1_1_8,   SNOR_CMD_READ_1_1_8 },
        { LM_SNOR_HWCAPS_READ_1_8_8,   SNOR_CMD_READ_1_8_8 },
        { LM_SNOR_HWCAPS_READ_8_8_8,   SNOR_CMD_READ_8_8_8 },
        { LM_SNOR_HWCAPS_READ_1_8_8_DTR,   SNOR_CMD_READ_1_8_8_DTR },
    };

    return spi_nor_hwcaps2cmd(hwcaps, hwcaps_read2cmd,
                  ARRAY_LEN(hwcaps_read2cmd));
}



/*
 * 将配置寄存器中的Quad Enable（QE）位置1。
 */
static int spansion_quad_enable(lm_spi_nor_dev_t *p_nor)
{
    uint8_t sr_cr[2] = {0, CR_QUAD_EN_SPAN};
    int ret;

    ret = write_sr_cr(p_nor, sr_cr);
    if (ret)
        return ret;

    /* read back and check it */
    ret = __spi_nor_read_cr(p_nor);
    if (!(ret > 0 && (ret & CR_QUAD_EN_SPAN))) {
//        dev_err(p_nor->dev, "Spansion Quad bit not set\n");
        return -LM_EINVAL;
    }

    return 0;
}

/*
 * 将配置寄存器中的Quad Enable（QE）位置1。
 * 此功能应与不支持读取的QSPI存储器一起使用
 * 配置寄存器（35h）指令。
 * 配置寄存器的位1是Spansion的QE位，
 * 返回：成功则返回0，否则返回-errno。
 */
static int spansion_no_read_cr_quad_enable (lm_spi_nor_dev_t *p_nor)
{
    uint8_t sr_cr[2];
    int ret;

    /* 保留状态寄存器的当前值 */
    ret = __spi_nor_read_cr(p_nor);
    if (ret < 0) {
//        dev_err(p_nor->dev, "error while reading status register\n");
        return -LM_EINVAL;
    }
    sr_cr[0] = ret;
    sr_cr[1] = CR_QUAD_EN_SPAN;

    return write_sr_cr(p_nor, sr_cr);
}

/*
 *
 */
static int spansion_read_cr_quad_enable(lm_spi_nor_dev_t *p_nor)
{
    uint8_t sr_cr[2];
    int ret;

    /* Check current Quad Enable bit value. */
    ret = __spi_nor_read_cr(p_nor);
    if (ret < 0) {
//        dev_err(dev, "error while reading configuration register\n");
        return -LM_EINVAL;
    }

    if (ret & CR_QUAD_EN_SPAN)
        return 0;

    sr_cr[1] = ret | CR_QUAD_EN_SPAN;

    /* Keep the current value of the Status Register. */
    ret = __spi_nor_read_cr(p_nor);
    if (ret < 0) {
//        dev_err(dev, "error while reading status register\n");
        return -LM_EINVAL;
    }
    sr_cr[0] = ret;

    ret = write_sr_cr(p_nor, sr_cr);
    if (ret)
        return ret;

    /* Read back and check it. */
    ret = __spi_nor_read_cr(p_nor);
    if (!(ret > 0 && (ret & CR_QUAD_EN_SPAN))) {
//        dev_err(p_nor->dev, "Spansion Quad bit not set\n");
        return -LM_EINVAL;
    }

    return 0;
}

/**
 * *将状态寄存器2中的Quad Enable（QE）位置1。
 *
 * 这是设置SFDP中描述的QE位的过程之一
 * （JESD216 rev B）规范，但没有制造商使用此步骤
 * 已被识别，因此功能名称。
 */
static int sr2_bit7_quad_enable (lm_spi_nor_dev_t *p_nor)
{
    uint8_t sr2;
    int ret;

    /* Check current Quad Enable bit value. */
    ret = p_nor->pfunc_read_reg(p_nor, LM_SPINOR_OP_RDSR2, &sr2, 1);
    if (ret)
        return ret;
    if (sr2 & SR2_QUAD_EN_BIT7)
        return 0;

    /* Update the Quad Enable bit. */
    sr2 |= SR2_QUAD_EN_BIT7;

    __spi_nor_write_enable(p_nor);

    ret = p_nor->pfunc_write_reg(p_nor, LM_SPINOR_OP_WRSR2, &sr2, 1);
    if (ret < 0) {
//        dev_err(p_nor->dev, "error while writing status register 2\n");
        return -LM_EINVAL;
    }

    ret = __spi_nor_wait_till_ready(p_nor);
    if (ret < 0) {
//        dev_err(p_nor->dev, "timeout while writing status register 2\n");
        return ret;
    }

    /* Read back and check it. */
    ret = p_nor->pfunc_read_reg(p_nor, LM_SPINOR_OP_RDSR2, &sr2, 1);
    if (!(ret > 0 && (sr2 & SR2_QUAD_EN_BIT7))) {
//        dev_err(p_nor->dev, "SR2 Quad bit not set\n");
        return -LM_EINVAL;
    }

    return 0;
}


/**
 * Flash参数表。
 *
 * 基本Flash参数表是主要且唯一的必需表，如下所示：
 * 由SFDP（JESD216）规范定义。
 * 它为我们提供了数据数组的总大小（内存密度），并且
 * 快速读取，页面编程和扇区擦除的地址字节数命令。
 * 对于快速读取命令，它还给出了模式时钟周期数和每个的等待状
 * 态（重新组合为虚拟时钟周期数）支持的指令操作码。
 * 对于页面程序，自JESD216版本A起，页面大小现已可用，但是
 * 仍未提供受支持的指令操作码。
 * 对于扇区擦除命令，此表存储了受支持的指令op代码和相关的扇区大小。
 * 最后，自JESD216开始，四通道启用要求（QER）也可用。
 * 版本A。QER位将制造商相关程序编码为
 * 执行该操作以设置主机的某些内部寄存器中的Quad Enable（QE）位
 * 四通道SPI存储器。实际上，QE位（如果存在）必须先设置
 * 发送任何Quad SPI命令到存储器。实际上，将QE位置1
 * 告诉内存将其WP＃和HOLD＃/ RESET＃引脚重新分配给功能IO2
 * 和IO3，因此启用4（四）I / O线。

 */
static int __spi_nor_parse_bfpt(lm_spi_nor_dev_t *p_nor,
                  const struct sfdp_parameter_header *bfpt_header,
                  struct spi_nor_flash_parameter *params)
{
    lm_nvram_dev_t *p_nvram = &p_nor->nvram;
    struct sfdp_bfpt bfpt;
    size_t len;
    int i, cmd, err;
    uint32_t addr;
    uint16_t half;

    /* JESD216 Basic Flash Parameter Table length is at least 9 DWORDs. */
    if (bfpt_header->length < BFPT_DWORD_MAX_JESD216)
        return -LM_EINVAL;

    /* Read the Basic Flash Parameter Table. */
    len = sizeof(bfpt);
    addr = SFDP_PARAM_HEADER_PTP(bfpt_header);
    memset(&bfpt, 0, sizeof(bfpt));
    err = spi_nor_read_sfdp(p_nor, addr, len, &bfpt);
    if (err < 0)
        return err;

    /* Number of address bytes. */
    switch (bfpt.dwords[BFPT_DWORD(1)] & BFPT_DWORD1_ADDRESS_BYTES_MASK) {
    case BFPT_DWORD1_ADDRESS_BYTES_3_ONLY:
        p_nor->addr_width = 3;
        break;

    case BFPT_DWORD1_ADDRESS_BYTES_4_ONLY:
        p_nor->addr_width = 4;
        break;

    default:
        break;
    }

    /* Flash Memory Density (in bits). */
    params->size = bfpt.dwords[BFPT_DWORD(2)];
    if (params->size & BIT(31)) {
        params->size &= ~BIT(31);

        /*
         * Prevent overflows on params->size. Anyway, a NOR of 2^64
         * bits is unlikely to exist so this error probably means
         * the BFPT we are reading is corrupted/wrong.
         */
        if (params->size > 63)
            return -LM_EINVAL;

        params->size = 1ULL << params->size;
    } else {
        params->size++;
    }
    params->size >>= 3; /* Convert to bytes. */

    /* Fast Read settings. */
    for (i = 0; i < ARRAY_LEN(sfdp_bfpt_reads); i++) {
        const struct sfdp_bfpt_read *rd = &sfdp_bfpt_reads[i];
        struct spi_nor_read_command *read;

        if (!(bfpt.dwords[rd->supported_dword] & rd->supported_bit)) {
            params->hwcaps.mask &= ~rd->hwcaps;
            continue;
        }

        params->hwcaps.mask |= rd->hwcaps;
        cmd = spi_nor_hwcaps_read2cmd(rd->hwcaps);
        read = &params->reads[cmd];
        half = bfpt.dwords[rd->settings_dword] >> rd->settings_shift;
        spi_nor_set_read_settings_from_bfpt(p_nor, read, half, rd->proto);
    }

    /* Sector Erase settings. */
    for (i = 0; i < ARRAY_LEN(sfdp_bfpt_erases); i++) {
        const struct sfdp_bfpt_erase *er = &sfdp_bfpt_erases[i];
        uint32_t erasesize;
        uint8_t opcode;

        half = bfpt.dwords[er->dword] >> er->shift;
        erasesize = half & 0xff;

        /* erasesize == 0 means this Erase Type is not supported. */
        if (!erasesize)
            continue;

        erasesize = 1U << erasesize;
        opcode = (half >> 8) & 0xff;
#ifdef CONFIG_MTD_SPI_NOR_USE_4K_SECTORS
        if (erasesize == SZ_4K) {
            p_nor->erase_opcode = opcode;
            p_nvram->erasesize = erasesize;
            break;
        }
#endif
        if (!p_nvram->erasesize || p_nvram->erasesize < erasesize) {
            p_nor->erase_opcode = opcode;
            p_nvram->erasesize = erasesize;
        }
    }

    /* Stop here if not JESD216 rev A or later. */
    if (bfpt_header->length < BFPT_DWORD_MAX)
        return 0;

    /* Page size: this field specifies 'N' so the page size = 2^N bytes. */
    params->page_size = bfpt.dwords[BFPT_DWORD(11)];
    params->page_size &= BFPT_DWORD11_PAGE_SIZE_MASK;
    params->page_size >>= BFPT_DWORD11_PAGE_SIZE_SHIFT;
    params->page_size = 1U << params->page_size;

    /* Quad Enable Requirements. */
    switch (bfpt.dwords[BFPT_DWORD(15)] & BFPT_DWORD15_QER_MASK) {
    case BFPT_DWORD15_QER_NONE:
        params->quad_enable = NULL;
        break;

    case BFPT_DWORD15_QER_SR2_BIT1_BUGGY:
    case BFPT_DWORD15_QER_SR2_BIT1_NO_RD:
        params->quad_enable = spansion_no_read_cr_quad_enable;
        break;

    case BFPT_DWORD15_QER_SR1_BIT6:
        params->quad_enable = macronix_quad_enable;
        break;

    case BFPT_DWORD15_QER_SR2_BIT7:
        params->quad_enable = sr2_bit7_quad_enable;
        break;

    case BFPT_DWORD15_QER_SR2_BIT1:
        params->quad_enable = spansion_read_cr_quad_enable;
        break;

    default:
        return -LM_EINVAL;
    }

    return 0;
}


/*
 * 解析串行闪存可发现参数, JEDEC JESD216描述了串行闪存可发现参数标准, 串行闪存都需要遵守
 */
static int __spi_nor_parse_sfdp(lm_spi_nor_dev_t *p_nor,
                  struct spi_nor_flash_parameter *params)
{
    const struct sfdp_parameter_header *param_header, *bfpt_header;
    struct sfdp_parameter_header *param_headers = NULL;
    struct sfdp_header header;
    size_t psize;
    int i, err;


    /* 获取 SFDP 头 */
    err = spi_nor_read_sfdp(p_nor, 0, sizeof(header), &header);
    if (err < 0)
        return err;

    /* 检查SFDP版本 */
    if ((header.signature != SFDP_SIGNATURE) ||
        (header.major != SFDP_JESD216_MAJOR) ||
        (header.minor < SFDP_JESD216_MINOR))
        return -LM_EINVAL;

    /*
     * 确认第一个也是唯一的强制参数标头是JESD216中指定的基本闪存参数表ID。
     */
    bfpt_header = &header.bfpt_header;
    if (SFDP_PARAM_HEADER_ID(bfpt_header) != SFDP_BFPT_ID ||
        bfpt_header->major != SFDP_JESD216_MAJOR)
        return -LM_EINVAL;

    /**
     * 分配内存，然后一次读取所有参数头
     * 读取SFDP命令。 这些参数标题实际上将被解析
     * 两次：第一次获得基本Flash的最新版本
     * 参数表，然后第二次处理受支持的可选表。
     * 因此，我们一次读取了所有参数标题，以减少处理时间。
     * 因为我们不需要保留这些参数标题：
     */
    if (header.nph) {
        psize = header.nph * sizeof(*param_headers);

        param_headers = lm_mem_alloc(psize);
        if (!param_headers)
            return -LM_ENOMEM;

        err = spi_nor_read_sfdp(p_nor, sizeof(header),
                    psize, param_headers);
        if (err < 0) {
//            dev_err(dev, "failed to read SFDP parameter headers\n");
            goto exit;
        }
    }

    /*
     * 检查其他参数标题，以获取基本闪存参数表的最新版本。
     */
    for (i = 0; i < header.nph; i++) {
        param_header = &param_headers[i];

        if (SFDP_PARAM_HEADER_ID(param_header) == SFDP_BFPT_ID &&
            param_header->major == SFDP_JESD216_MAJOR &&
            (param_header->minor > bfpt_header->minor ||
             (param_header->minor == bfpt_header->minor &&
              param_header->length > bfpt_header->length)))
            bfpt_header = param_header;
    }

    err = __spi_nor_parse_bfpt(p_nor, bfpt_header, params);
    if (err)
        goto exit;

    /* Parse other parameter headers. */
    for (i = 0; i < header.nph; i++) {
        param_header = &param_headers[i];

        switch (SFDP_PARAM_HEADER_ID(param_header)) {
        case SFDP_SECTOR_MAP_ID:
//            dev_info(dev, "non-uniform erase sector maps are not supported yet.\n");
            break;

        default:
            break;
        }

        if (err)
            goto exit;
    }

exit:
    lm_mem_free(param_headers);
    return err;
}

static int __spi_nor_init_params(lm_spi_nor_dev_t                *p_nor,
                                 const struct flash_info         *info,
                                 struct spi_nor_flash_parameter  *params)
{
    /* 默认参数设置为0 */
    memset(params, 0, sizeof(*params));

    /* Set SPI NOR sizes. */
    params->size = info->sector_size * info->n_sectors;
    params->page_size = info->page_size;

    /* 快速读设置 */
    params->hwcaps.mask |= LM_SNOR_HWCAPS_READ;
    __spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ],
                  0, 0, LM_SPINOR_OP_READ,
                  SNOR_PROTO_1_1_1);

    if (!(info->flags & SPI_NOR_NO_FR)) {
        params->hwcaps.mask |= LM_SNOR_HWCAPS_READ_FAST;
        __spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_FAST],
                      0, 8, LM_SPINOR_OP_READ_FAST,
                      SNOR_PROTO_1_1_1);
    }

    if (info->flags & SPI_NOR_DUAL_READ) {
        params->hwcaps.mask |= LM_SNOR_HWCAPS_READ_1_1_2;
        __spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_1_1_2],
                      0, 8, LM_SPINOR_OP_READ_1_1_2,
                      SNOR_PROTO_1_1_2);
    }

    if (info->flags & SPI_NOR_QUAD_READ) {
        params->hwcaps.mask |= LM_SNOR_HWCAPS_READ_1_1_4;
        __spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_1_1_4],
                      0, 8, LM_SPINOR_OP_READ_1_1_4,
                      SNOR_PROTO_1_1_4);
    }

    if (info->flags & SPI_NOR_OCTAL_READ) {
        params->hwcaps.mask |= LM_SNOR_HWCAPS_READ_1_8_8_DTR;
        __spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_1_8_8_DTR],
                      0, 8, LM_SPINOR_OP_READ_1_8_8_DTR_4B,
                      SNOR_PROTO_1_8_8_DTR);
    }

    /* 页编程设置 */
    params->hwcaps.mask |= LM_SNOR_HWCAPS_PP;
    __spi_nor_set_pp_settings(&params->page_programs[SNOR_CMD_PP],
               LM_SPINOR_OP_PP, SNOR_PROTO_1_1_1);

    /* 进入四线模式 */
    if (params->hwcaps.mask & (LM_SNOR_HWCAPS_READ_QUAD |
                   LM_SNOR_HWCAPS_PP_QUAD)) {
        switch (JEDEC_MFR(info)) {
        case SNOR_MFR_MACRONIX:
            params->quad_enable = macronix_quad_enable;
            break;

        case SNOR_MFR_MICRON:
        case SNOR_MFR_MICRONO:
            break;

        default:
            /* Kept only for backward compatibility purpose. */
            params->quad_enable = spansion_quad_enable;
            break;
        }
    }

    /* 用从SFDP表读取的数据覆盖参数*/
    p_nor->addr_width = 0;
    p_nor->nvram.erasesize = 0;
    if ((info->flags & (SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ)) &&
        !(info->flags & SPI_NOR_SKIP_SFDP)) {
        struct spi_nor_flash_parameter sfdp_params;

        memcpy(&sfdp_params, params, sizeof(sfdp_params));
        if (__spi_nor_parse_sfdp(p_nor, &sfdp_params)) {
            p_nor->addr_width = 0;
            p_nor->nvram.erasesize = 0;
        } else {
            memcpy(params, &sfdp_params, sizeof(*params));
        }
    }

    return 0;
}

static int spi_nor_select_read(lm_spi_nor_dev_t *p_nor,
                   const struct spi_nor_flash_parameter *params,
                   uint32_t shared_hwcaps)
{
    int cmd;
    int best_match = lm_fls(shared_hwcaps & LM_SNOR_HWCAPS_READ_MASK) - 1;

    const struct spi_nor_read_command *read;

    if (best_match < 0)
        return -LM_EINVAL;

    cmd = spi_nor_hwcaps_read2cmd(BIT(best_match));
    if (cmd < 0)
        return -LM_EINVAL;

    read = &params->reads[cmd];
    p_nor->read_opcode = read->opcode;
    p_nor->read_proto = read->proto;

    /*
     * In the spi-p_nor framework, we don't need to make the difference
     * between mode clock cycles and wait state clock cycles.
     * Indeed, the value of the mode clock cycles is used by a QSPI
     * flash memory to know whether it should enter or leave its 0-4-4
     * (Continuous Read / XIP) mode.
     * eXecution In Place is out of the scope of the mtd sub-system.
     * Hence we choose to merge both mode and wait state clock cycles
     * into the so called dummy clock cycles.
     */
    p_nor->read_dummy = read->num_mode_clocks + read->num_wait_states;

    /*
     * STR mode may need 10 dummy cycles but the DDR mode should be
     * no more than 8 cycles
     */
    if (lm_spi_nor_protocol_is_dtr(read->proto))
        p_nor->read_dummy = p_nor->read_dummy > 8 ? 8 : p_nor->read_dummy;

    return 0;
}

static int spi_nor_select_pp(lm_spi_nor_dev_t *p_nor,
                 const struct spi_nor_flash_parameter *params,
                 uint32_t shared_hwcaps)
{
    int cmd, best_match = lm_fls(shared_hwcaps & LM_SNOR_HWCAPS_PP_MASK) - 1;
    const struct spi_nor_pp_command *pp;

    if (best_match < 0)
        return -LM_EINVAL;

    cmd = spi_nor_hwcaps_pp2cmd(BIT(best_match));
    if (cmd < 0)
        return -LM_EINVAL;

    pp = &params->page_programs[cmd];
    p_nor->program_opcode = pp->opcode;
    p_nor->write_proto = pp->proto;
    return 0;
}

static int spi_nor_select_erase(lm_spi_nor_dev_t *p_nor,
                const struct flash_info *info)
{
    lm_nvram_dev_t *p_nvram = &p_nor->nvram;

    /* Do nothing if already configured from SFDP. */
    if (p_nvram->erasesize)
        return 0;

#ifdef CONFIG_NVRAM_SPI_NOR_USE_4K_SECTORS
    /* prefer "small sector" erase if possible */
    if (info->flags & SECT_4K) {
        p_nor->erase_opcode = LM_SPINOR_OP_BE_4K;
        p_nvram->erasesize = 4096;
    } else if (info->flags & SECT_4K_PMC) {
        p_nor->erase_opcode = LM_SPINOR_OP_BE_4K_PMC;
        p_nvram->erasesize = 4096;
    } else
#endif
    {
        p_nor->erase_opcode = LM_SPINOR_OP_SE;
        p_nvram->erasesize = info->sector_size;
    }
    return 0;
}


static int spi_nor_setup (lm_spi_nor_dev_t *p_nor, const struct flash_info *info,
             const struct spi_nor_flash_parameter *params,
             const struct spi_nor_hwcaps *hwcaps)
{
    uint32_t ignored_mask, shared_mask;
    bool enable_quad_io;
    int err;

    /*
     * Keep only the hardware capabilities supported by both the SPI
     * controller and the SPI flash memory.
     */

    shared_mask = hwcaps->mask & params->hwcaps.mask;

    /* SPI n-n-n protocols are not supported yet. */
    ignored_mask = (LM_SNOR_HWCAPS_READ_2_2_2 |
                    LM_SNOR_HWCAPS_READ_4_4_4 |
                    LM_SNOR_HWCAPS_READ_8_8_8 |
                    LM_SNOR_HWCAPS_PP_4_4_4 |
                    LM_SNOR_HWCAPS_PP_8_8_8);
    if (shared_mask & ignored_mask) {
//        dev_dbg(p_nor->dev,
//            "SPI n-n-n protocols are not supported yet.\n");
        shared_mask &= ~ignored_mask;
    }

    /* Select the (Fast) Read command. */
    err = spi_nor_select_read(p_nor, params, shared_mask);
    if (err) {
//        dev_err(p_nor->dev,
//            "can't select read settings supported by both the SPI controller and memory.\n");
        return err;
    }

    /* Select the Page Program command. */
    err = spi_nor_select_pp(p_nor, params, shared_mask);
    if (err) {
//        dev_err(p_nor->dev,
//            "can't select write settings supported by both the SPI controller and memory.\n");
        return err;
    }

    /* Select the Sector Erase command. */
    err = spi_nor_select_erase(p_nor, info);
    if (err) {
//        dev_err(p_nor->dev,
//            "can't select erase settings supported by both the SPI controller and memory.\n");
        return err;
    }

    /* Enable Quad I/O if needed. */
    enable_quad_io = (lm_spi_nor_get_protocol_width(p_nor->read_proto) == 4 ||
                      lm_spi_nor_get_protocol_width(p_nor->write_proto) == 4);
    if (enable_quad_io && params->quad_enable) {
        err = params->quad_enable(p_nor);
        if (err) {
//            dev_err(p_nor->dev, "quad mode not supported\n");
            return err;
        }
    }

    return 0;
}


int lm_spi_nor_scan(lm_spi_nor_dev_t            *p_nor,
                    const char                  *name,
                    const struct spi_nor_hwcaps *hwcaps)
{

    struct spi_nor_flash_parameter params;

    const struct lm_spi_dev_id          *id = NULL;
    struct flash_info                 *info = NULL;
    lm_nvram_dev_t                    *p_nvram = &p_nor->nvram;
    int ret;

    ret = spi_nor_check(p_nor);
    if (ret)
        return ret;

    /* 初始化SPI控制命令 */
    p_nor->reg_proto   = SNOR_PROTO_1_1_1;
    p_nor->read_proto  = SNOR_PROTO_1_1_1;
    p_nor->write_proto = SNOR_PROTO_1_1_1;

    if (name) {
        id = __spi_nor_match_id(name);
    }

    /* 自动查找设备 */
    if (!id) {
        id = __spi_nor_read_id(p_nor);
    }

    if (!id) {
        return -LM_ENODEV;
    }

    info = (void *)id->driver_data;

    /*
     * 检测名字和设备是否一致
     */
    if (name && info->id_len) {
        const struct lm_spi_dev_id *jid;

        jid = __spi_nor_read_id(p_nor);
        if (!jid) {
            return -LM_ENODEV;
        } else if (jid != id) {
            id = jid;

            info = (void *)jid->driver_data;
        }
    }

    if ((ret = lm_mutex_create(&p_nor->lock))) {
        return ret;
    }

    /* 解析串行闪存配置表 */
    ret = __spi_nor_init_params(p_nor, info, &params);
    if (ret)
        return ret;

    /*
     * Atmel, SST 等芯片需要设置
     */
    if ((JEDEC_MFR(info) == SNOR_MFR_ATMEL) || \
        (JEDEC_MFR(info) == SNOR_MFR_INTEL) || \
        (JEDEC_MFR(info) == SNOR_MFR_SST)   || \
        (info->flags     &  SPI_NOR_HAS_LOCK)) {

        __spi_nor_write_enable(p_nor);
        __spi_nor_write_sr(p_nor, 0);
        __spi_nor_wait_till_ready(p_nor);
    }


    /* 初始化nvram,给函数指针赋值 */
    p_nvram->size = info->sector_size *info->n_sectors;
    p_nvram->pfunc_read  = __spi_nor_read;
    p_nvram->pfunc_write = __spi_nor_write;
    p_nvram->pfunc_erase = __spi_nor_erase;

    p_nor->page_size      = params.page_size;
    p_nvram->writebufsize = p_nor->page_size;

    if (info->flags & USE_FSR)
        p_nor->flags |= LM_SNOR_F_USE_FSR;


    p_nor->erase_opcode = LM_SPINOR_OP_SE;

    p_nor->page_size = info->page_size;

    ret = spi_nor_setup(p_nor, info, &params, hwcaps);

    if (p_nor->addr_width) {
        /* 已经配置SFDP */
    } else if (info->addr_width) {
        p_nor->addr_width = info->addr_width;
    } else if (p_nvram->size > 0x1000000) {
        /* 如果这个设备大于16M, 地址线配置为４ */
        p_nor->addr_width = 4;
        if (JEDEC_MFR(info) == SNOR_MFR_SPANSION ||
            (info->flags & SPI_NOR_4B_OPCODES)) {
            __spi_nor_set_4byte_opcodes(p_nor, info);
        } else {
            __spi_nor_set_4byte(p_nor, info, 1);
        }
    } else {
        p_nor->addr_width = 3;
    }

    /* 设置 NOR */
    p_nvram->priv = p_nor;

    /* 打印nvram分区 */

    return 0;
}

static const struct lm_spi_dev_id *__spi_nor_match_id(const char *name)
{
    const struct lm_spi_dev_id *id = __spi_nor_ids;
    while (id->name[0]) {
        if (!strcmp(name, id->name))
            return id;
        id++;
    }
    return NULL;
}


/* end of file */
