#include "lm_utils.h"

int float_to_str(char *str, float number, uint8_t g, uint8_t l, bool flag)
{
    uint8_t i;
    uint32_t offset = g;
    int temp = 0;
    float t2 = 0.0;
    static char table[]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

    if (flag) {
        if (number < 0) {
            str[0] = '-';
        } else {
            str[0] = '+';
        }
        offset = g+1;
    }

    if (number < 0) {
        number = (-1)*number;
    }
    temp = number/1;

    for (i = 1; i<=g; i++)
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

    for(i=1; i<=l; i++)
    {
        temp = t2*10;
        str[offset+i] = table[temp%10];
        t2 = t2*10;
    }
    *(str+offset+l+1) = '\0';

    return LM_OK;
}

/* end of file */
