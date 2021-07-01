/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_ulog.c
* Change Logs   :
* Date          Author          Notes
* 2021-03-04    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 日志管理模块(未实现文件存储功能)
*******************************************************************************/

#include "lm_ulog.h"
#include "lm_time.h"
#include "lm_serial.h"
#include "lm_kservice.h"

#include "lm_ulog_interface.h"

/* 日志管理安装地址声明(定义在链接文件中) */
extern struct lm_ulog_mgr_info * __gp_ulog_start[];
extern struct lm_ulog_mgr_info * __gp_ulog_end[];

/* 全局日志标记定义 */
uint8_t g_ulog_flag = LM_ULOG_GLOBAL_FLAG;

/* 日志参数指针定义 */
static lm_ulog_t *__gp_ulog_info = NULL;

/******************************************************************************/
/**
 * @brief 日志可变参数输出函数
 */
static int __ulog_vsout (   const char      *pre,   \
                            const uint8_t   type,   \
                            const char      *name,  \
                            uint8_t         flag,   \
                            const char      *fmt,   \
                            va_list         va)
{
    int ret = LM_OK;

    lm_tm_t datetime;

    /* 1. 参数有效性检查 */
    if (NULL == pre || NULL == name || NULL == __gp_ulog_info) {
        return -LM_ERROR;
    }

    /* 2. 打印时间戳 */
    if ((g_ulog_flag & LM_ULOG_FLAG_TIMESTAMP) && \
       (flag & LM_ULOG_FLAG_TIMESTAMP)) {
        lm_time_get(&datetime);
        snprintf((void *)__gp_ulog_info->ulog_out_buf, __gp_ulog_info->ulog_s, \
                    "%02d %02d %02d:%02d:%02d ",
                    datetime.tm_mon, datetime.tm_yday, \
                    datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
    } else {
        __gp_ulog_info->ulog_out_buf[0] = '\0';
    }

    /* 3. 打印日志级别 */
    strcat((void *)__gp_ulog_info->ulog_out_buf, (void *)pre);
    strcat((void *)__gp_ulog_info->ulog_out_buf, (void *)" ");

    /* 4. 打印设备或模块名称 */
    if ((g_ulog_flag & LM_ULOG_FLAG_NAME) && (flag & LM_ULOG_FLAG_NAME)) {
        strcat((void *)__gp_ulog_info->ulog_out_buf, (void *)name);
        strcat((void *)__gp_ulog_info->ulog_out_buf, (void *)" : ");
    }

    /* 5. 打印日志内容 */
    vsnprintf((void *)&(__gp_ulog_info[strlen((void *)__gp_ulog_info->ulog_out_buf)]), \
            __gp_ulog_info->ulog_s - strlen(STRBR) - \
            strlen((void *)__gp_ulog_info->ulog_out_buf) - 1, fmt, va);

    /* 6. 添加换行符 */
    strcat((void *)__gp_ulog_info->ulog_out_buf, (void *)STRBR);

    /* 7. 打印 */
    lm_serial_write(__gp_ulog_info->com, (void *)__gp_ulog_info->ulog_out_buf, \
                    strlen((void *)__gp_ulog_info->ulog_out_buf));

    /* 8. 检查日志是否需要保存 */
    switch (type) {
    case LM_ULOG_FLAG_DEBUG:
        if (0 == (flag & LM_ULOG_FLAG_SAVE_DEBUG) || \
            0 == (g_ulog_flag & LM_ULOG_FLAG_SAVE_DEBUG)) {
            goto RETURN_2;
        }
        break;
    case LM_ULOG_FLAG_WARNING:
        if (0 == (flag & LM_ULOG_FLAG_SAVE_WARNING) || \
            0 == (g_ulog_flag & LM_ULOG_FLAG_SAVE_WARNING)) {
            goto RETURN_2;
        }
        break;
    case LM_ULOG_FLAG_ERROR:
        if (0 == (flag & LM_ULOG_FLAG_SAVE_ERROR) || \
            0 == (g_ulog_flag & LM_ULOG_FLAG_SAVE_ERROR)) {
            goto RETURN_2;
        }
        break;
    default:
        goto RETURN_2;
    }

    RETURN_2:

    return ret;
}

/**
 * @brief 日志输出函数
 */
int lm_ulog_output (const char      *pre,   \
                    const char      *name,  \
                    const uint8_t   type,   \
                    uint8_t         flag,   \
                    const char      *fmt,   \
                    ...)
{
    int ret = LM_OK;

    va_list va;

    /* 1. 检查参数有效性 */
    if (NULL == pre || NULL == name || NULL == fmt) {
        return -LM_ERROR;
    }

    /* 2. 检查日志是否需要打印 */
    if (0 == (g_ulog_flag & type) || 0 == (flag & type)) {
        return ret;
    }

    /* 3. 开始解析参数 */
    va_start(va, fmt);

    /* 4. 输出数据 */
    __ulog_vsout(pre, type, name, flag, fmt, va);

    /* 5. 结束参数解析 */
    va_end(va);

    return ret;
}

/**
 * @brief 十六进制dump
 */
int lm_utils_hex_dump (void *data, uint16_t len)
{
    int ret = LM_OK;

    uint16_t i = 0;

    uint8_t str[32] = {0};

    /* 1. 检查参数有效性 */
    if (NULL == data || 0 == len) {
        return -LM_ERROR;
    }

    /* 2. DUMP数据 */
    for (i = 0; i < len; i++) {
        if (i % 16 == 0) {
            if (i > 0) {
                lm_kprintf(" |%s", str);
            }
            lm_kprintf(STRBR "%08X: ", i);
        }
        lm_kprintf("%02x ", ((uint8_t *)data)[i]);
        if (isprint(((uint8_t *)data)[i])) {
            str[i % 16] = ((uint8_t *)data)[i];
        } else {
            str[i % 16]=' ';
        }
    }
    while (i % 16 != 0) {
        lm_kprintf("   ");
        str[i % 16]=' ';
        i++;
    }
    lm_kprintf(" |%s" STRBR STRBR, str);

    return ret;
}

/**
 * @brief 日志十六进制DUMP
 */
int lm_ulog_dump_hex (  const char      *pre,   \
                        const char      *name,  \
                        const uint8_t   type,   \
                        const uint8_t   flag,   \
                        void            *dat,   \
                        uint16_t        len,    \
                        const char      *fmt,   \
                        ...)
{
    int ret = LM_OK;

    va_list va;

    /* 1. 检查参数有效性 */
    if (NULL == name || NULL == fmt) {
        return -LM_ERROR;
    }

    /* 2. 检查日志是否需要打印 */
    if (0 == (g_ulog_flag & type) || 0 == (flag & type)) {
        return ret;
    }

    /* 3. 开始解析参数 */
    va_start(va, fmt);

    /* 4. 输出数据 */
    __ulog_vsout(pre, type, name, flag, fmt, va);

    /* 5. DUMP数据 */
    if (0 != (g_ulog_flag & type) && 0 != (flag & type)) {
        lm_utils_hex_dump(dat, len);
    }

    /* 6. 结束参数解析 */
    va_end(va);

    return ret;
}

/**
 * @brief DUMP日志文件(todo: 暂时没有实现存文件功能)
 */
int lm_ulog_dump_file (void)
{
    int ret = LM_OK;

    return ret;
}

/**
 * @brief 日志标记设置
 */
void lm_ulog_flag_set (const char *name, const uint8_t flag)
{
    struct lm_ulog_mgr_info **ulog = NULL;

    /* 1. 使能日志标记 */
    for (ulog = __gp_ulog_start; ulog < __gp_ulog_end; ulog++) {
        if (0 != strstr((*ulog)->name, name)) {
            *((*ulog)->flag) |= flag;
        }
    }
}

/**
 * @brief 日志标记清除
 */
void lm_ulog_flag_clear (const char *name, const uint8_t flag)
{
    struct lm_ulog_mgr_info **ulog = NULL;

    /* 1. 禁用日志标记 */
    for (ulog = __gp_ulog_start; ulog < __gp_ulog_end; ulog++){
        if(0 != strstr((*ulog)->name, name)) {
            *((*ulog)->flag) &= ~flag;
        }
    }
}

/**
 * @brief 日志使能
 */
void lm_ulog_enable (const char *name)
{
    struct lm_ulog_mgr_info **ulog = NULL;

    /* 1. 使能日志 */
    for (ulog = __gp_ulog_start; ulog < __gp_ulog_end; ulog++) {
        if (0 != strstr((*ulog)->name, name)) {
            *((*ulog)->flag) |= (LM_ULOG_GLOBAL_FLAG);
        }
    }
}

/**
 * @brief 日志禁用
 */
void lm_ulog_disable (const char *name)
{
    struct lm_ulog_mgr_info **ulog = NULL;

    /* 1. 禁用日志 */
    for (ulog = __gp_ulog_start; ulog < __gp_ulog_end; ulog++) {
        if (0 != strstr((*ulog)->name, name)) {
            *((*ulog)->flag) &= ~(LM_ULOG_GLOBAL_FLAG);
        }
    }
}

/**
 * @brief 日志注册
 */
int lm_ulog_register (lm_ulog_t *p_ulog)
{
    int ret = LM_OK;

    /* 1. 检查输入参数是否有效 */
    lm_assert(NULL != p_ulog);

    /* 2. 注册数据结构  */
    __gp_ulog_info = p_ulog;

    return ret;
}

/* end of file */

