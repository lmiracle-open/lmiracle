
#ifndef __LM_SPI_H
#define __LM_SPI_H

#include "lmiracle.h"
#include "lm_list.h"
#include <string.h>


#define     SPI_NAME_SIZE               10

typedef struct lm_spi_dev {
    uint8_t                     bus_id;
    uint8_t                     bits_per_word;
    uint16_t                    mode;
#define LM_SPI_CPHA        0x01                    /* clock phase */
#define LM_SPI_CPOL        0x02                    /* clock polarity */
#define LM_SPI_MODE_0      (0|0)                   /* (original MicroWire) */
#define LM_SPI_MODE_1      (0|LM_SPI_CPHA)
#define LM_SPI_MODE_2      (LM_SPI_CPOL|0)
#define LM_SPI_MODE_3      (LM_SPI_CPOL|LM_SPI_CPHA)
#define LM_SPI_CS_HIGH     0x04                    /* chipselect active high? */
#define LM_SPI_LSB_FIRST   0x08                    /* per-word bits-on-wire */
#define LM_SPI_3WIRE       0x10                    /* SI/SO signals shared */
#define LM_SPI_LOOP        0x20                    /* loopback mode */
#define LM_SPI_NO_CS       0x40                    /* 1 dev/bus, no chipselect */
#define LM_SPI_READY       0x80                    /* slave pulls low to pause */
#define LM_SPI_TX_DUAL     0x100                   /* transmit with 2 wires */
#define LM_SPI_TX_QUAD     0x200                   /* transmit with 4 wires */
#define LM_SPI_RX_DUAL     0x400                   /* receive with 2 wires */
#define LM_SPI_RX_QUAD     0x800                   /* receive with 4 wires */

    uint16_t                    flags;
#define LM_SPI_MASTER_HALF_DUPLEX  BIT(0)          /* 不支持全双工 */
#define LM_SPI_MASTER_NO_RX        BIT(1)          /* 不支持缓存区读 */
#define LM_SPI_MASTER_NO_TX        BIT(2)          /* 不支持缓存区写 */
#define LM_SPI_MASTER_MUST_RX      BIT(3)          /* requires rx */
#define LM_SPI_MASTER_MUST_TX      BIT(4)          /* requires tx */

    /* 设备的最大频率 */
    uint32_t                    max_speed_hz;

    void                       *cs_gpio;           /* 片选 */

}lm_spi_dev_t;

typedef struct lm_spi_master lm_spi_master_t;

/**
 * @brief SPI传输
 */
typedef struct lm_spi_transfer{
    const void *p_txbuf;      
    void       *p_rxbuf;     
    
    size_t      len;             /* 发送接受缓存区的数据长度 */

    uint8_t     cs_change:1;     /* 传输是否影响片选 */
    uint8_t     tx_nbits:3;      /*  */
    uint8_t     rx_nbits:3;      /*  */
#define LM_SPI_NBITS_SINGLE        0x01 /* 1bit transfer */
#define LM_SPI_NBITS_DUAL          0x02 /* 2bits transfer */
#define LM_SPI_NBITS_QUAD          0x04 /* 4bits transfer */

    uint8_t     bits_per_word;

    uint16_t    delay_usece;

    uint32_t    speed_hz;             /* 传输速度,使用默认速度时设置为0 */

    struct lm_list_head      transfer_list;
} lm_spi_transfer_t; 

/**
 * @brief spi message
 */
typedef struct lm_spi_message {  
    struct lm_list_head        transfers;

    lm_spi_dev_t              *p_spi;

    lm_spi_master_t           *p_master;

    unsigned                   is_dma_mapped:1;

    void (*pfunc_complete)(void *p_arg);
    void                      *p_arg;

    /* 发送成功字节总数 */
    size_t                     actual_length;

    /* 消息状态 */
    int                        status;

    struct   lm_list_head      queue;


}lm_spi_message_t;

/**
 * @brief spi master服务函数
 */
typedef struct lm_spi_funcs{
    int (*pfunc_setup)(lm_spi_master_t *p_master,
                       lm_spi_dev_t    *p_spi);

    int (*pfunc_transfer)(lm_spi_master_t   *p_master,
                          lm_spi_dev_t      *p_spi,
                          lm_spi_transfer_t *p_trans);

    int (*pfunc_set_cs) (lm_spi_master_t     *p_master,
                         uint8_t              enable);

}lm_spi_funcs_t;



/**
 * @brief SPI 控制器
 */
struct lm_spi_master {
    struct lm_list_head         list;

    int                         bus_id;

    /* 指向当前设备的指针 */
    lm_spi_dev_t               *p_spi;

    lm_mutex_t                  bus_lock_mutex;

    /* 驱动私有数据，由对应的驱动程序分配内存 */
    void                       *p_driver_data;

    uint32_t                    bits_per_word_mask;
#define LM_SPI_BPW_MASK(bits)       BIT((bits) - 1)
#define LM_SPI_BIT_MASK(bits)       (((bits) == 32) ? ~0U : (BIT(bits) - 1))
#define LM_SPI_BPW_RANGE_MASK(min, max)  \
                (LM_SPI_BIT_MASK(max) - LM_SPI_BIT_MASK(min - 1))

    /* 传输速度 */
    uint32_t                    min_speed_hz;
    uint32_t                    max_speed_hz;

    /*
     * lm_spi_dev.mode  定义了spi master支持的模式
     */
    uint16_t                    mode_bits;

    const lm_spi_funcs_t       *p_funcs;
};

/**
 * @brief spi设备初始化
 */
static inline void
lm_spi_dev_init (lm_spi_dev_t *p_spi,
                 uint8_t       bus_id,
                 uint8_t       bits_per_word,
                 uint16_t      mode,
                 uint16_t      flags,
                 void         *cs_gpio)
{
    p_spi->bus_id        = bus_id;
    p_spi->bits_per_word = bits_per_word;
    p_spi->mode          = mode;
    p_spi->flags         = flags;
    p_spi->cs_gpio       = cs_gpio;
}

/**
 * @brief 设置传输
 */
static inline void
lm_spi_set_transfer (lm_spi_transfer_t *p_trans,
                     const void        *p_txbuf,
                     void              *p_rxbuf,
                     size_t             len,
                     uint8_t            cs_change,
                     uint8_t            bits_per_word,
                     uint16_t           delay_usecs,
                     uint32_t           speed_hz)
{
    p_trans->p_txbuf       = p_txbuf;
    p_trans->p_rxbuf       = p_rxbuf;
    p_trans->len           = len;
    p_trans->bits_per_word = bits_per_word;
    p_trans->delay_usece   = delay_usecs;
    p_trans->speed_hz      = speed_hz;
}


/**
 * @brief 初始化消息
 */
static inline void 
lm_spi_message_init (struct lm_spi_message *p_msg)
{
    memset(p_msg, 0, sizeof(*p_msg));

    LM_INIT_LIST_HEAD(&p_msg->transfers);
}

/**
 * @brief 在消息中添加传输
 */
static inline void 
lm_spi_message_add_tail (struct lm_spi_transfer *t, struct lm_spi_message *m)
{
    lm_list_add_tail(&t->transfer_list, &m->transfers);
}

/**
 * @brief 设置spi设备
 */
extern int lm_spi_setup (lm_spi_dev_t *p_spi);

/**
 * @brief spi 同步传输
 */
extern int lm_spi_sync (lm_spi_dev_t     *p_spi,
                        lm_spi_message_t *p_msg);

/**
 * @brief 先写后读
 *
 * @paran[in]  id     总线ID
 * @paran[in]  txbuf  发送数据缓存区
 * @paran[in]  n_tx   需要发送的数据
 * @paran[out] rx_buf 接受数据缓存区
 * @paran[in]  n_rx   需要接受的数据
 *
 *
 * @return  LM_OK     成功
 *          其他      失败
 */
extern int lm_spi_write_then_read (lm_spi_dev_t  *p_spi,
                                   const uint8_t *txbuf,
                                   size_t         n_tx,
                                   uint8_t       *rx_buf,
                                   size_t         n_rx);


/**
 * @brief 注册SPI驱动
 */
extern int lm_spi_register (lm_spi_master_t *p_spi);

#endif     /* __LM_SPI_H */

/* end of file */
