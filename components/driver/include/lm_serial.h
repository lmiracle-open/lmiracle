#ifndef __LM_SERIAL_H
#define __LM_SERIAL_H

#include "lmiracle.h"
#include "lm_list.h"
#include "lm_ringbuf.h"

/**
 * @brief 串口编号,当前最多支持八个
 */
enum lm_com_id {
    COM0 = 0,
    COM1,
    COM2,
    COM3,
    COM4,
    COM5,
    COM6,
    COM7,
    COM_MUX,
};

struct lm_serial_port;


/**
 * @brief 波特率
 */
#define BAUD_RATE_2400                    2400
#define BAUD_RATE_4800                    4800
#define BAUD_RATE_9600                    9600
#define BAUD_RATE_19200                   19200
#define BAUD_RATE_38400                   38400
#define BAUD_RATE_115200                  115200
#define BAUD_RATE_230400                  230400
#define BAUD_RATE_460800                  460800
#define BAUD_RATE_921600                  921600
#define BAUD_RATE_2000000                 2000000
#define BAUD_RATE_3000000                 3000000

/**
 * @brief 数据位
 */
#define LM_SERIAL_DATA_BITS_5                       5
#define LM_SERIAL_DATA_BITS_6                       6
#define LM_SERIAL_DATA_BITS_7                       7
#define LM_SERIAL_DATA_BITS_8                       8
#define LM_SERIAL_DATA_BITS_9                       9

/**
 * @brief 停止位
 */
#define LM_SERIAL_STOP_BITS_1                       0
#define LM_SERIAL_STOP_BITS_1_5                     1
#define LM_SERIAL_STOP_BITS_2                       2
#define LM_SERIAL_STOP_BITS_3                       3

/**
 * @brief 校验位
 */
#define LM_SERIAL_PARITY_NONE             0      /* 没有校验位 */
#define LM_SERIAL_PARITY_ODD              1      /* 奇校验 */
#define LM_SERIAL_PARITY_EVEN             2      /* 偶校验 */

/**
 * @brief 高低位在前
 */
#define LM_SERIAL_BIT_LSB                 0       /* 低位在前 */
#define LM_SERIAL_BIT_MSB                 1       /* 高位在前 */

/**
 * @brief 串口默认配置
 */
#define LM_SERIAL_CONFIG_DEFAULT                \
{                                               \
    BAUD_RATE_115200,                           \
    LM_SERIAL_DATA_BITS_8,                      \
    LM_SERIAL_STOP_BITS_1,                      \
    LM_SERIAL_PARITY_NONE,                      \
    LM_SERIAL_BIT_LSB,                          \
    0,                                          \
    0,                                          \
}

/**
 * @brief 串口相关配置
 */
struct lm_serial_config {
    unsigned int   baud_rate;                       /* 波特率 */

    uint32_t       data_bits                :4;     /* 数据位 */
    uint32_t       stop_bits                :2;     /* 停止位 */
    uint32_t       parity                   :2;     /* 校验位 */
    uint32_t       bit_order                :1;     /* LSB或MSB */
    uint32_t       fast_rect                :1;     /* 快速相应 */
    uint32_t       reserved                 :22;
};

#define LM_SERIAL_INFO_DEFAULT     \
{                                  \
        LM_SERIAL_CONFIG_DEFAULT,  \
        0x0,                       \
        0xFFFFFFFF                 \
}

struct lm_serial_info
{
    struct lm_serial_config     config;

    uint32_t       read_timeout;                /* 读超时 */
    uint32_t       idle_timeout;                /* 空闲超时 */
};


/**
 * @brief 串口服务函数
 */
typedef struct lm_serial_ops_t {

    /**
     * @brief 配置
     */
    int (*pfunc_set_config) (struct lm_serial_port         *p_serial,
                             const struct lm_serial_config *p_config);

    /**
     * @brief 发送数据
     */
    int (*pfunc_send) (struct lm_serial_port *p_serial,
                       const void            *p_buf,
                       size_t                 size);

    /**
     * @brief 发送一个字符
     */
    void (*pfunc_poll_put_char) (struct lm_serial_port *p_serial, uint8_t c);

    /**
     * @brief 获取一个字符
     */
    int (*pfunc_poll_get_char) (struct lm_serial_port *p_serial);
} lm_serial_ops_t;


/**
 * @brief
 */
struct lm_serial_port {
    struct lm_list_head         list;

    uint8_t                    *recv_buf;         /* 接收缓存区的地址 */
    uint32_t                    buf_size;         /* 接收缓存区大小 */
    int                         id;               /* 对应的串口ID */

    struct lm_ringbuf           rbuf;

    struct lm_serial_info       serial_info;
    lm_mutex_t              ro_mutex;
    lm_mutex_t              wr_mutex;
    lm_semb_t               ro_sync_semb;
    const lm_serial_ops_t         *p_ops;       /* 串口操作函数,由底层驱动去实现 */
};

static inline void
lm_uart_port_rx(struct lm_serial_port *p_serial, const uint8_t c)
{
    lm_ringbuf_putchar(&p_serial->rbuf, c);
}

/**
 * @brief 配置串口
 *
 * @param[in] com    串口号
 * @param[in] p_info 串口配置数据
 */
extern int lm_serial_set_info (int com, const struct lm_serial_info *p_info);

/**
 * @brief 获取串口配置信息
 *
 * @param[in] com    串口号
 * @param[in] p_info 存放串口配置数据的地址
 */
extern int lm_serial_get_info (int com, struct lm_serial_info *p_info);

/**
 * @brief 串口设备写数据
 *
 * @param[in] com   串口号
 * @param[in] p_buf 需要写入数据的缓存区地址
 * @param[in] size  需要写入数据的字节数
 *
 * @return 成功:返回的是写入数据的的个数,
 *         失败:返回负数(错误码).
 *
 * @note 该函数返回,串口不一定写完成,该函数只是将数据放入发送缓存区中
 */
extern int lm_serial_write (int com, const void *p_buf, size_t size);

/**
 * @brief 串口设备写数据
 *
 * @param[in] com   串口号
 * @param[in] p_buf 需要存放读取数据的缓存区地址
 * @param[in] size  需要读取数据的字节数
 *
 * @return 成功:返回的是读取数据的的个数,
 *         失败:返回负数(错误码).
 */
extern int lm_serial_read (int com, void *p_buf, size_t size);

/**
 * @brief 注册串口驱动
 */
extern int lm_serial_register (struct lm_serial_port *p_spi);

#endif /* __LM_SERIAL_H */

/* end of file */
