#include "lm_utils.h"

int float_to_str(char *str, float number, uint8_t g, uint8_t l)
{
    uint8_t i;
    int temp = number/1;
    float t2 = 0.0;
    static char table[]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

    for (i = 1; i<=g; i++)
    {
        if (temp==0)
            str[g-i] = table[0];
        else
            str[g-i] = table[temp%10];
        temp = temp/10;
    }
    *(str+g) = '.';
    temp = 0;
    t2 = number;
    for(i=1; i<=l; i++)
    {
        temp = t2*10;
        str[g+i] = table[temp%10];
        t2 = t2*10;
    }
    *(str+g+l+1) = '\0';

    return LM_OK;
}

/* end of file */
