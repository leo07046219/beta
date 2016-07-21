/**************************************************************************************
*  Filename:               debug.h
*  Description:            调试头文件
*  Author:                 Leo
*  Create:                 2016-06-09
*  Modification history:
*
**************************************************************************************/
#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef  __cplusplus
extern "C"
{
#endif

#define MAX_DEBUG_LINE 512 

/*串口着色方案*/
const unsigned int color_serialCom[] = 
{ 
    7,   
    7,  //白     --- trace
    2,  //绿     --- info
    3,  //黄     --- warning
    5,  //品红   --- error
    1 };//红     --- fault

typedef enum _debugLevel
{
    INVALID_LEVEL = 0,
    TRACE_LEV,
    INFO_LEV,
    WARNING_LEV,
    ERROR_LEV,
    FAULT_LEV,
}DEBUG_LEVEL;

void levelDebug(DEBUG_LEVEL debugLevel, const char *fmt, ...);


#ifdef  __cplusplus
}
#endif

#endif