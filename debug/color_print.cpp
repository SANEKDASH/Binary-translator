#include <stdarg.h>
#include <stdio.h>

#include "color_print.h"

void ColorPrintf(ColorCode_t color_code, const char *fmt, ...)
{
    va_list arg_list;
    va_start(arg_list, fmt);

    printf("\033[0;%dm", color_code);

    vprintf(fmt, arg_list);

    printf("\033[0m");

    va_end(arg_list);
}

