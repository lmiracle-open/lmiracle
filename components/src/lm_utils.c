/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : lm_utils.c
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : 单元模块(包含一些常用算法 字符串操作 类型转换等)
*******************************************************************************/

#include "lm_utils.h"

/**
 * @brief float转字符串     eg: 2.134667
 */
int lm_utils_float_to_str ( char        *str,       \
                            float       number,     \
                            uint8_t     i_n,        \
                            uint8_t     d_n,        \
                            bool        flag)
{
    uint8_t i;
    uint32_t offset = i_n;
    int temp = 0;
    float t2 = 0.0;
    static char table[]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

    if (flag) {
        if (number < 0) {
            str[0] = '-';
        } else {
            str[0] = '+';
        }
        offset = i_n+1;
    }

    if (number < 0) {
        number = (-1)*number;
    }
    temp = number/1;

    for (i = 1; i<=i_n; i++)
    {
        if (temp==0)
            str[offset-i] = table[0];
        else
            str[offset-i] = table[temp%10];
        temp = temp/10;
    }
    *(str+offset) = '.';

    temp = 0;
    t2 = number;

    for(i=1; i<=d_n; i++)
    {
        temp = t2*10;
        str[offset+i] = table[temp%10];
        t2 = t2*10;
    }
    *(str+offset+d_n+1) = '\0';

    return LM_OK;
}

/**
 * @brief 将float类型转换成uint16保存，该函数转化范围为小数点后面两位
 */
inline int lm_float_convert_byte (  float       *data,      \
                                    uint16_t    *out,       \
                                    uint16_t    len)
{
    int ret = LM_OK;
    uint16_t      temp = 0;
    uint16_t    integer, decimal;
    bool symbol_flag = false;

    /* 1. 参数有效性检查 */
    if (NULL == data || NULL == out) {
        return -LM_ERROR;
    }

    for (int i = 0; i < len; i++) {
        /* 将小数转换为整数 */
        temp = (uint16_t)((data[i]*100)/1);

        /* 温度值为负数时 设置符号位 并将负数转换为正数参与计算 */
        if (temp < 0) {
            symbol_flag = true;
            temp = ~temp + 1;
        }

        /* eg: 27.43->2743->27,43 */
        integer = (uint16_t)(temp/100);   /* 取出正数部分 */
        decimal = (uint16_t)(temp%100);   /* 取出小数部分 */

        *out = (integer*100) + (decimal & 0xff);

        if (symbol_flag) {
            *out |= (1 << 15);
        }

        out ++;
    }

    return ret;
}

/**
 * @brief 将byte类型转换成float保存，该函数转化范围为小数点后面两位
 */
inline int lm_byte_convert_float (  uint16_t        *data,      \
                                    float           *out,       \
                                    uint16_t        len)
{
    int ret = LM_OK;

    uint16_t     integer, decimal;
    float temp = 0.0;

    /* 1. 参数有效性检查 */
    if (NULL == data || NULL == out) {
        return -LM_ERROR;
    }

    for (int i = 0; i < len; i ++) {

        /* 2. 解析数据 */
        integer = (data[i]/100);
        decimal = (data[i]%100);

        /* 3. 组合数据 */
        temp = integer + (0.01 * decimal);

        /* 4. 设置符号位 */
        if (data[i] & 0x8000) {
            temp *= -1;
        }

        *out = temp;

        out ++;
    }

    return ret;
}

/* end of file */
