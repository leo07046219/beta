/**************************************************************************************
*  Filename:               main.cpp
*  Description:            主函数实现文件
*  Author:                 Leo
*  Create:                 2016-06-09
*  Modification history:
*
**************************************************************************************/

#include "common.h"
#include "findGogoDns.h"
#include "cmdDispatcher.h"

int main(void)
{
    C_FindGogoDns   *findGogoDnsObjPtr = NULL;
    C_CmdDispatcher *cmdDispatcherObjPtr = NULL;
    
    cmdDispatcherObjPtr = getCmdDispatcherObjPtr();
    if (NULL == cmdDispatcherObjPtr)
    {
        levelDebug(ERROR_LEV, "[%s][%d]:getCmdDispatcherObjPtr() failed!\n", \
            __FUNCTION__, __LINE__);
        return -1;
    }
    if (cmdDispatcherObjPtr->initCmdDispatcher() != 0)
    {
        levelDebug(ERROR_LEV, "[%s][%d]:initCmdDispatcher() failed!\n", \
            __FUNCTION__, __LINE__);
        return -1;
    }

    findGogoDnsObjPtr = getFindGogoDnsObjPtr();
    if (NULL == findGogoDnsObjPtr)
    {
        levelDebug(ERROR_LEV, "[%s][%d]:getFindGogoDnsObjPtr() failed!\n", \
            __FUNCTION__, __LINE__);
        return -1;
    }

#if 1
    if (findGogoDnsObjPtr->initFindGogoDns() != 0)
    {
        levelDebug(ERROR_LEV, "[%s][%d]:initFindGogoDns() failed!\n", \
            __FUNCTION__, __LINE__);
        return -1;
    }
#endif

#if 1
    while (1)
    {
        sleep(1);
    }
#endif

    //sleep(3);

    return 0;
}