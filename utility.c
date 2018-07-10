#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void remove_char_from_string(char c, char *str)
{
    int i = 0;
    int len = strlen(str) + 1;

    for(i = 0; i < len; i++)
    {
        if(str[i] == c)
        {
            strncpy(&str[i], &str[i + 1], len - i);
        }
    }
}