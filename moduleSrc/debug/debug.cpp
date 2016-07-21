/**************************************************************************************
*  Filename:               debug.cpp
*  Description:            调试打印实现文件 --- 实现不同级别的彩色打印信息输出
*  Author:                 Leo
*  Create:                 2016-06-09
*  Modification history:
*
**************************************************************************************/

#include "common.h"

static void setDebugColor(DEBUG_LEVEL debugLevel)
{
    fprintf(stderr, "\033[1;40;3%dm", color_serialCom[debugLevel]);
}

static void resetColor(void)
{
    fprintf(stderr, "\033[0m");
}

static void coloredOutput(DEBUG_LEVEL debugLevel ,const char *pOutputStr)
{
    setDebugColor(debugLevel);
    fprintf(stderr, pOutputStr);
    fflush(stderr);

    resetColor();
}

void levelDebug(DEBUG_LEVEL debugLevel, const char *fmt, ...)
{
    va_list args;
    char printBuf[MAX_DEBUG_LINE + 1];

    memset(printBuf, 0, sizeof(printBuf));

    va_start(args, fmt);
    vsprintf(printBuf, fmt, args);
    coloredOutput(debugLevel, printBuf);
    va_end(args);

    return ;
}