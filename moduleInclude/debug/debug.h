/**************************************************************************************
*  Filename:               debug.h
*  Description:            ����ͷ�ļ�
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

/*������ɫ����*/
const unsigned int color_serialCom[] = 
{ 
    7,   
    7,  //��     --- trace
    2,  //��     --- info
    3,  //��     --- warning
    5,  //Ʒ��   --- error
    1 };//��     --- fault

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