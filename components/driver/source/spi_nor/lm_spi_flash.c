#include "lmiracle.h"
#include "lm_spi.h"
#include "lm_spi_flash.h"
#include "lm_spi_nor.h"
#include "lm_nvram.h"



static int __spi_flash_read_reg(lm_spi_nor_dev_t *p_nor, uint8_t code, uint8_t *val, int len)
{
    lm_spi_flash_dev_t *p_flash = p_nor->priv;
    lm_spi_dev_t       *p_spi = &p_flash->spi;
    int ret ;

    ret = lm_spi_write_then_read(p_spi, &code, 1, val, len);
    if (ret < 0)
        return ret;
//        dev_err(&spi->dev, "error %d reading %x\n", ret, code);

    return ret;
}

static void __spi_flash_addr2cmd (lm_spi_nor_dev_t *p_nor, unsigned int addr, uint8_t *cmd)
{
    /* opcode is in cmd[0] */
    cmd[1] = addr >> (p_nor->addr_width * 8 -  8);
    cmd[2] = addr >> (p_nor->addr_width * 8 - 16);
    cmd[3] = addr >> (p_nor->addr_width * 8 - 24);
    cmd[4] = addr >> (p_nor->addr_width * 8 - 32);
}

static int __spi_flash_cmdsz (lm_spi_nor_dev_t *p_nor)
{
    return 1 + p_nor->addr_width;
}

static int __spi_flash_write_reg (lm_spi_nor_dev_t *p_nor, uint8_t opcode, uint8_t *buf, int len)
{
    lm_spi_flash_dev_t *p_flash = p_nor->priv;
    lm_spi_dev_t       *p_spi   = &p_flash->spi;

    p_flash->command[0] = opcode;
    if (buf)
        memcpy(&p_flash->command[1], buf, len);

    return lm_spi_write_then_read(p_spi, p_flash->command, len + 1, NULL, 0);
}

static size_t __spi_flash_write(lm_spi_nor_dev_t *p_nor, uint32_t to, size_t len,
                const uint8_t *buf)
{
    lm_spi_flash_dev_t *p_flash = p_nor->priv;
    lm_spi_dev_t *p_spi = &p_flash->spi;
    unsigned int inst_nbits, addr_nbits, data_nbits, data_idx;

    lm_spi_transfer_t trans[3] = {};
    lm_spi_message_t message;
    int cmd_sz = __spi_flash_cmdsz(p_nor);
    size_t ret;

    /* get transfer protocols. */
    inst_nbits = lm_spi_nor_get_protocol_inst_nbits(p_nor->write_proto);
    addr_nbits = lm_spi_nor_get_protocol_addr_nbits(p_nor->write_proto);
    data_nbits = lm_spi_nor_get_protocol_data_nbits(p_nor->write_proto);

    lm_spi_message_init(&message);

    if (p_nor->program_opcode == LM_SPINOR_OP_AAI_WP && p_nor->sst_write_second)
        cmd_sz = 1;

    p_flash->command[0] = p_nor->program_opcode;
    __spi_flash_addr2cmd(p_nor, to, p_flash->command);

    trans[0].p_txbuf = p_flash->command;
    trans[0].tx_nbits = inst_nbits;
    trans[0].len = cmd_sz;
    lm_spi_message_add_tail(&trans[0], &message);

    /* split the op code and address bytes into two transfers if needed. */
    data_idx = 1;
    if (addr_nbits != inst_nbits) {
        trans[0].len = 1;

        trans[1].p_txbuf = &p_flash->command[1];
        trans[1].tx_nbits = addr_nbits;
        trans[1].len = cmd_sz - 1;
        lm_spi_message_add_tail(&trans[1], &message);

        data_idx = 2;
    }

    trans[data_idx].p_txbuf = buf;
    trans[data_idx].tx_nbits = data_nbits;
    trans[data_idx].len = len;
    lm_spi_message_add_tail(&trans[data_idx], &message);

    ret = lm_spi_sync(p_spi, &message);
    if (ret)
        return ret;

    ret = message.actual_length - cmd_sz;
    if (ret < 0)
        return -LM_EIO;
    return ret;
}

/*
 * 读
 */
static size_t __spi_flash_read(lm_spi_nor_dev_t *p_nor, uint32_t from, size_t len,
               uint8_t *buf)
{
    lm_spi_flash_dev_t *p_flash = p_nor->priv;
    lm_spi_dev_t       *p_spi   = &p_flash->spi;
    unsigned int inst_nbits, addr_nbits, data_nbits, data_idx;
    lm_spi_transfer_t trans[3];
    lm_spi_message_t message;
    unsigned int dummy = p_nor->read_dummy;
    size_t ret;
    int cmd_sz;

    /* 获取传输控制 */
    inst_nbits = lm_spi_nor_get_protocol_inst_nbits(p_nor->read_proto);
    addr_nbits = lm_spi_nor_get_protocol_addr_nbits(p_nor->read_proto);
    data_nbits = lm_spi_nor_get_protocol_data_nbits(p_nor->read_proto);

    /* convert the dummy cycles to the number of bytes */
    dummy = (dummy * addr_nbits) / 8;

    lm_spi_message_init(&message);
    memset(trans, 0, (sizeof trans));

    p_flash->command[0] = p_nor->read_opcode;
    __spi_flash_addr2cmd(p_nor, from, p_flash->command);

    trans[0].p_txbuf = p_flash->command;
    trans[0].tx_nbits = inst_nbits;
    trans[0].len = __spi_flash_cmdsz(p_nor) + dummy;
    lm_spi_message_add_tail(&trans[0], &message);

    /*
     * Set all dummy/mode cycle bits to avoid sending some manufacturer
     * specific pattern, which might make the memory enter its Continuous
     * Read mode by mistake.
     * Based on the different mode cycle bit patterns listed and described
     * in the JESD216B specification, the 0xff value works for all memories
     * and all manufacturers.
     */
    cmd_sz = trans[0].len;
    memset(p_flash->command + cmd_sz - dummy, 0xff, dummy);

    /* split the op code and address bytes into two transfers if needed. */
    data_idx = 1;
    if (addr_nbits != inst_nbits) {
        trans[0].len = 1;

        trans[1].p_txbuf = &p_flash->command[1];
        trans[1].tx_nbits = addr_nbits;
        trans[1].len = cmd_sz - 1;
        lm_spi_message_add_tail(&trans[1], &message);

        data_idx = 2;
    }

    trans[data_idx].p_rxbuf = buf;
    trans[data_idx].rx_nbits = data_nbits;
    trans[data_idx].len = len;

    lm_spi_message_add_tail(&trans[data_idx], &message);

    ret = lm_spi_sync(p_spi, &message);
    if (ret)
        return ret;

    ret = message.actual_length - cmd_sz;
    if (ret < 0)
        return -LM_EIO;
    return ret;
}

/*
 * 初始化 SPI Flash
 */
static int __spi_flash_init (lm_spi_flash_dev_t *p_flash)
{
    lm_spi_nor_dev_t *p_nor;
    lm_spi_dev_t           *p_spi = &p_flash->spi;

    struct spi_nor_hwcaps hwcaps = {
        .mask = LM_SNOR_HWCAPS_READ |
            LM_SNOR_HWCAPS_READ_FAST |
            LM_SNOR_HWCAPS_PP,
    };

    p_nor = &p_flash->spi_nor;

    /* install the hooks */
    p_nor->pfunc_read = __spi_flash_read;
    p_nor->pfunc_write = __spi_flash_write;
    p_nor->pfunc_write_reg = __spi_flash_write_reg;
    p_nor->pfunc_read_reg = __spi_flash_read_reg;

    p_nor->p_spi = &p_flash->spi;

    if (p_spi->mode & LM_SPI_RX_QUAD) {
        hwcaps.mask |= LM_SNOR_HWCAPS_READ_1_1_4;

        if (p_spi->mode & LM_SPI_TX_QUAD)
            hwcaps.mask |= (LM_SNOR_HWCAPS_READ_1_4_4 |
                    LM_SNOR_HWCAPS_PP_1_1_4 |
                    LM_SNOR_HWCAPS_PP_1_4_4);
    } else if (p_spi->mode & LM_SPI_RX_DUAL) {
        hwcaps.mask |= LM_SNOR_HWCAPS_READ_1_1_2;

        if (p_spi->mode & LM_SPI_TX_DUAL)
            hwcaps.mask |= LM_SNOR_HWCAPS_READ_1_2_2;
    }

    /* 初始化 SPI 设备 */
    p_nor->p_spi = &p_flash->spi;

    p_nor->priv = p_flash;

    return lm_spi_nor_scan(p_nor, p_flash->p_cfg->name, &hwcaps);
}

int lm_spi_flash_register (lm_spi_flash_dev_t       *p_flash,
                           const lm_spi_flash_cfg_t *p_cfg)
{
    int                 ret = LM_OK;

    /* 初始化 SPI设备 */
    lm_spi_dev_init(&p_flash->spi,
                    p_cfg->spi_id,
                    p_cfg->bits_per_word,
                    p_cfg->spi_mode,
                    p_cfg->spi_speed,
                    p_cfg->spi_flags,
                    p_cfg->cs_gpio);

    lm_spi_setup(&p_flash->spi);

    p_flash->p_cfg = p_cfg;

    /* 平台初始化 */
    if (p_cfg->pfunc_platform_init) {
        p_cfg->pfunc_platform_init();
    }

    ret = __spi_flash_init(p_flash);
    if (ret) {
        return ret;
    }

    p_flash->spi_nor.nvram.p_info = p_cfg->p_nvram_info;

    return lm_nvram_register(&p_flash->spi_nor.nvram);

}

