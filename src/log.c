#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "log.h"

void ll_log(int level, char *format, ...)
{
    char szLevel[8] = {0};
    char *buf = NULL;
    int buf_len;

    va_list ap;

    switch (level)
    {
    case LOG_DEBUG:
        strcpy(szLevel, "debug");
        break;
    case LOG_INFORMATION:
        strcpy(szLevel, "info");
        break;
    case LOG_WARNNING:
        strcpy(szLevel, "warn");
        break;
    case LOG_ERROR:
        strcpy(szLevel, "error");
        break;
    }

    va_start(ap, format);
    buf_len = vsnprintf(buf, 0, format, ap) + 1;
    buf = (char *)malloc(buf_len);
    vsnprintf(buf, buf_len, format, ap);
    printf("[%s] %s\n", szLevel, buf);
    free(buf);
    va_end(ap);
}