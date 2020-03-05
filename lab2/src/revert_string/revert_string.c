#include "revert_string.h"

void RevertString(char *str)
{
    int length = 0;
    while(str[length] != '\0' && str[length] != '\n')
    {
        length++;
    }
    for (int cnt = 0; cnt < length / 2; cnt++)
    {
        char t = str[cnt];
        str[cnt] = str[length - cnt - 1];
        str[length - cnt - 1] = t;
    }
}

