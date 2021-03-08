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

/* end of file */
