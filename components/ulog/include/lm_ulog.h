/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_ulog.h
* Change Logs   :
* Date          Author          Notes
* 2021-03-04    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 日志管理模块
*******************************************************************************/

#ifndef __LM_ULOG_H
#define __LM_ULOG_H

#include "lmiracle.h"
#include "lm_kservice.h"

LM_BEGIN_EXTERN_C

#define STRBR                           "\r\n"          /* 回车换行符 */

/*******************************************************************************
* Description    : 日志配置数据定义
*******************************************************************************/
#define LM_ULOG_FILE_LINE_TOTAL        1000            /* 日志最大条数 */
#define LM_ULOG_FILE_LINE_SIZE         300             /* 每条日志的最大长度 */

/*******************************************************************************
* Description    : 日志输出标记控制
*******************************************************************************/
#define LM_ULOG_FLAG_DEBUG             0x01            /* 调试日志使能标记 */
#define LM_ULOG_FLAG_WARNING           0x02            /* 警告日志使能标记 */
#define LM_ULOG_FLAG_ERROR             0x04            /* 错误日志使能标记 */
#define LM_ULOG_FLAG_TIMESTAMP         0x08            /* 打印日志时间戳标记 */
#define LM_ULOG_FLAG_NAME              0x10            /* 打印模块名称标记 */
#define LM_ULOG_FLAG_SAVE_DEBUG        0x20            /* 保存调试日志 */
#define LM_ULOG_FLAG_SAVE_WARNING      0x40            /* 保存警告日志 */
#define LM_ULOG_FLAG_SAVE_ERROR        0x80            /* 保存错误标记 */

/*******************************************************************************
* Description    : 日志前导字符定义
*******************************************************************************/
#define LM_ULOG_PRE_DEBUG            "\033[32m[ DEBUG ]\033[0m"
#define LM_ULOG_PRE_WARING           "\033[33m[WARNING]\033[0m"
#define LM_ULOG_PRE_ERROR            "\033[31m[ ERROR ]\033[0m"
#define LM_ULOG_PRE_FRTOS            "\033[35m[ FRTOS ]\033[0m"

/*******************************************************************************
* Description    : 默认全局日志标记
*******************************************************************************/
#define LM_ULOG_GLOBAL_FLAG                                                    \
(                                                                              \
    LM_ULOG_FLAG_DEBUG         |                                               \
    LM_ULOG_FLAG_WARNING       |                                               \
    LM_ULOG_FLAG_ERROR         |                                               \
    LM_ULOG_FLAG_TIMESTAMP     |                                               \
    LM_ULOG_FLAG_NAME                                                          \
)

/*******************************************************************************
* Description    : 外部全局数据声明
*******************************************************************************/
extern uint8_t g_ulog_flag;                         /* 全局日志标记 */

/*******************************************************************************
* Description    : 日志管理数据类型
*******************************************************************************/
struct lm_ulog_mgr_info {
    const char  *name;                              /* 日志名称 */
    uint8_t     *flag;                              /* 日志控制标记指针 */
};

/*******************************************************************************
* Description    : 日志等级输出定义
*******************************************************************************/
/* debug日志 */
#define LM_LOGD(NAME, FLAG, ...)                                               \
        lm_ulog_output(LM_ULOG_PRE_DEBUG, #NAME, LM_ULOG_FLAG_DEBUG, FLAG,     \
                        __VA_ARGS__);
/* 警告日志 */
#define LM_LOGW(NAME, FLAG, ...)                                               \
        lm_ulog_output(LM_ULOG_PRE_WARING,#NAME, LM_ULOG_FLAG_WARNING, FLAG,   \
                        __VA_ARGS__);
/* 错误日志 */
#define LM_LOGE(NAME, FLAG, ...)                                               \
    lm_ulog_output(LM_ULOG_PRE_ERROR, #NAME, LM_ULOG_FLAG_ERROR, FLAG,         \
                        __VA_ARGS__);
/* 内核日志 */
#define LM_LOGS(...)                                                           \
    lm_ulog_output(LM_ULOG_PRE_FRTOS, "FRTOS_SYSTEM", LM_ULOG_GLOBAL_FLAG,     \
                        ulog_flag, __VA_ARGS__);

/*******************************************************************************
* Description    : 日志等级数据DUMP(十六进制输出)
*******************************************************************************/
#define LM_LOGD_DUMP(NAME, FLAG, DATA, LEN, ...)                               \
    lm_ulog_dump_hex(LM_ULOG_PRE_DEBUG, #NAME, ULOG_FLAG_DEBUG, FLAG, DATA,    \
                        LEN, __VA_ARGS__)
#define LM_LOGW_DUMP(NAME, FLAG, DATA, LEN, ...)                               \
    lm_ulog_dump_hex(LM_ULOG_PRE_WARING, #NAME, ULOG_FLAG_WARNING, FLAG,DATA,  \
                        LEN, __VA_ARGS__)
#define LM_LOGE_DUMP(NAME, FLAG, DATA, LEN, ...)                               \
    lm_ulog_dump_hex(LM_ULOG_PRE_ERROR, #NAME, ULOG_FLAG_ERROR, FLAG, DATA,    \
                        LEN, __VA_ARGS__)
#define LM_LOGS_DUMP(DATA, LEN, ...)                                           \
    lm_ulog_dump_hex(LM_ULOG_PRE_FRTOS, "FRTOS_SYSTEM", ULOG_GLOBAL_FLAG,      \
                        ulog_flag, DATA, LEN, __VA_ARGS__);

/**
 * @brief 日志管理安装
 *
 * @param[in]     name   日志名称
 * @param[in]     flag   日志标记
 *
 * @return  None
 */
#define LM_ULOG_MGR_INSTALL(name, flag) \
    static struct lm_ulog_mgr_info \
    __attribute__((unused)) __ulog_mgr_install_##name##__ = {#name, &flag}; \
    static __const struct lm_ulog_mgr_info * \
    __attribute__((unused, section(".install_ulog"))) \
    __ulog_mgr_install_##name##_point__ = & __ulog_mgr_install_##name##__

/**
 * @brief 日志输出函数
 *
 * @param[in]   pre     预字符串
 * @param[in]   name    日志名称
 * @param[in]   type    日志类型
 * @param[in]   flag    日志标记
 * @param[in]   fmt     格式化字符串
 * @param[in]   ...     可变参数
 *
 * @return  错误码
 */
extern int lm_ulog_output ( const char      *pre,       \
                            const char      *name,      \
                            const uint8_t   type,       \
                            uint8_t         flag,       \
                            const char      *fmt,       \
                            ...);
/**
 * @brief 十六进制dump
 *
 * @param[in]   data    数据指针
 * @param[in]   len     数据长度
 *
 * @return  错误码
 */
extern int lm_utils_hex_dump (void *data, uint16_t len);

/**
 * @brief 日志十六进制DUMP
 *
 * @param[in]   pre     预字符串
 * @param[in]   name    日志名称
 * @param[in]   type    日志类型
 * @param[in]   flag    日志标记
 * @param[in]   dat     DUMP数据
 * @param[in]   len     数据长度
 * @param[in]   fmt     格式化字符串
 * @param[in]   ...     可变参数
 *
 * @return  错误码
 */
extern int lm_ulog_dump_hex (   const char      *pre,   \
                                const char      *name,  \
                                const uint8_t   type,   \
                                const uint8_t   flag,   \
                                void            *dat,   \
                                uint16_t        len,    \
                                const char      *fmt,   \
                                ...);
/**
 * @brief DUMP日志文件
 *
 * @param[in]   None
 *
 * @return  错误码
 */
extern int lm_ulog_dump_file (void);

/**
 * @brief 格式化输出参数设置
 *
 * @param[in] com       日志输出串口号
 * @param[in] buf       日志输出缓存指针
 * @param[in] size      日志输出缓存长度
 *
 * @return  错误码
 */
extern int lm_ulog_param_set (uint8_t com, uint8_t *buf, uint16_t size);


/**
 * @brief 日志标记设置
 *
 * @param[in]   name    日志名称
 * @param[in]   flag    使能日志标记
 *
 * @return  None
 */
extern void lm_ulog_flag_set (const char *name, const uint8_t flag);

/**
 * @brief 日志标记清除
 *
 * @param[in]   name    日志名称
 * @param[in]   flag    禁用日志标记
 *
 * @return  None
 */
extern void lm_ulog_flag_clear (const char *name, const uint8_t flag);

/**
 * @brief 日志使能
 *
 * @param[in]   name    日志名称
 *
 * @return  None
 */
extern void lm_ulog_enable (const char *name);

/**
 * @brief 日志禁用
 *
 * @param[in]   name    日志名称
 *
 * @return  None
 */
extern void lm_ulog_disable (const char *name);

/**
 * @brief 全局日志标记设置
 *
 * @param[in]   flag    要使能的日志标记
 *
 * @return  None
 */
static inline void lm_ulog_global_set (const uint8_t flag)
{
    g_ulog_flag |= flag;
}

/**
 * @brief 全局日志标记清除
 *
 * @param[in]   flag    要清除的日志标记
 *
 * @return  None
 */
static inline void lm_ulog_global_clear (const uint8_t flag)
{
    g_ulog_flag &= ~flag;
}

/**
 * @brief 全局日志使能
 *
 * @param[in]   None
 *
 * @return  None
 */
static inline void lm_ulog_global_enable (void)
{
    g_ulog_flag |= (LM_ULOG_GLOBAL_FLAG);
}

/**
 * @brief 全局日志禁用
 *
 * @param[in]   None
 *
 * @return  None
 */
static inline void lm_ulog_global_disable (void)
{
    g_ulog_flag &= ~(LM_ULOG_GLOBAL_FLAG);
}

LM_END_EXTERN_C

#endif /* __LM_ULOG_H */

/* end of file */

