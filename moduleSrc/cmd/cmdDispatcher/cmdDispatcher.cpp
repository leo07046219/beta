/**************************************************************************************
*  Filename:               cmdDispatcher.cpp
*  Description:            命令行调度器实现文件
*  Author:                 Leo
*  Create:                 2016-07-31
*  Modification history:
*
**************************************************************************************/

#include "common.h"
#include "cmdDispatcher.h"

#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <netinet/in.h>  
#include <arpa/inet.h> 

/*静态成员变量初始化*/
C_CmdDispatcher *C_CmdDispatcher::ms_cmdDispatcherObjPtr = NULL;

C_CmdDispatcher::C_CmdDispatcher()
{
    //TODO: set name
    pthread_mutex_t  m_initMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t  m_singleMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t  m_paramMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t  m_gogoMutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_t  m_initCond = PTHREAD_COND_INITIALIZER;

    ms_cmdDispatcherObjPtr = NULL;
    m_bModuleInited = false;
}

C_CmdDispatcher::~C_CmdDispatcher()
{
    pthread_mutex_destroy(&m_paramMutex);
    pthread_mutex_destroy(&m_singleMutex);
    pthread_mutex_destroy(&m_initMutex);

    pthread_cond_destroy(&m_initCond);
}

/*************************************************
* Function:        getCmdDispatcherPtr()
* Description:     获取C_CmdDispatcher唯一对象地址
* Access Level:    public
* Input:           N/A
* Output:          C_CmdDispatcher唯一对象地址
* Return:          N/A
*************************************************/
/*静态成员变量声明*/
pthread_mutex_t C_CmdDispatcher::m_singleMutex;

C_CmdDispatcher *C_CmdDispatcher::getCmdDispatcherPtr(void)
{
    if (NULL == ms_cmdDispatcherObjPtr)
    {
        pthread_mutex_lock(&m_singleMutex);
        if (NULL == ms_cmdDispatcherObjPtr)
        {
            /*创建唯一实例*/
            ms_cmdDispatcherObjPtr = new C_CmdDispatcher();
        }
        pthread_mutex_unlock(&m_singleMutex);
    }

    return ms_cmdDispatcherObjPtr;
}

/*对外提供对象实例指针获取接口*/
C_CmdDispatcher *getCmdDispatcherObjPtr()
{
    return C_CmdDispatcher::getCmdDispatcherPtr();
}

/***********************************************************
* Function:       initCmdDispatcher()
* Description:    初始化命令调度器类
* Access Level:   public
* Input:          N/A
* Output:         N/A
* Return:         0---成功/-1---失败
***********************************************************/
int C_CmdDispatcher::initCmdDispatcher(void)
{
    levelDebug(INFO_LEV, "[%s][%d]: ... !\n", \
        __FUNCTION__, __LINE__);

    if (startCmdDispatcherTask() != 0)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: startCmdDispatcherTask() failed!\n");
        return -1;
    }

    return 0;
}
/***********************************************************
* Function:       startCmdDispatcherTask()
* Description:    启动命令调度器线程
* Access Level:   public
* Input:          N/A
* Output:         N/A
* Return:         0---成功/-1---失败
***********************************************************/
int C_CmdDispatcher::startCmdDispatcherTask(void)
{
    pthread_attr_t  attr;
    pthread_t       tid;
    int             retVal = 0;

    if (pthread_attr_init(&attr) != 0)
    {
        retVal = -1;
        goto exit;
    }

    /*创建线程*/
    if (pthread_create(&tid, &attr, (START_ROUTINE)cmdDispatcherTaskShell, (void *)this) != 0)
    {
        retVal = -1;
        goto exit;;
    }

exit:

    if (pthread_attr_destroy(&attr) != 0)
    {
        retVal = -1;
    }

    return retVal;
}

/*************************************************
* Function:        waitModuleInited()
* Description:     等待初始化完毕
* Access Level:    private
* Input:           N/A
* Output:          N/A
* Return:          N/A
*************************************************/
void C_CmdDispatcher::waitModuleInited(void)
{
    if (false == m_bModuleInited)
    {
        pthread_mutex_lock(&m_initMutex);
        if (false == m_bModuleInited)
        {
            /*启动时若线程初始化未完成，相关线程等待*/
            levelDebug(WARNING_LEV, "[%s][%d]:C_CmdDispatcher not inited, wait!\n", \
                __FUNCTION__, __LINE__);

            pthread_cond_wait(&m_initCond, &m_initMutex);
        }
        pthread_mutex_unlock(&m_initMutex);
    }
    return;
}


/*************************************************
* Function:     cmdDispatcherTaskShell()
* Description:  命令调度线程外壳
* Access Level: private
* Input:        pThis --- 类对象指针
* Output:       N/A
* Return:       N/A
*************************************************/
void C_CmdDispatcher::cmdDispatcherTaskShell(void* pThis)
{
    //TODO: set thread name

    assert(pThis != NULL);

    ((C_CmdDispatcher*)pThis)->cmdDispatcherTask();
}

/*************************************************
* Function:        cmdDispatcherTask()
* Description:     命令调度器任务
* Access Level:    private
* Input:           N/A
* Output:          N/A
* Return:          N/A
*************************************************/
void C_CmdDispatcher::cmdDispatcherTask(void)
{
    levelDebug(INFO_LEV, "[%s][%d]: ... \n", \
        __FUNCTION__, __LINE__);

    //create socket  
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
    {
        perror("socket\n");
        exit(-1);
    }
    printf("socket fd=%d\n", fd);

    //build connection address  
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6666);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int r;
    r = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (r == -1)
    {
        perror("bind");
        close(fd);
        exit(-1);
    }
    printf("bind address successful!\n");
    //accept or send message  
    char buf[255];
    struct sockaddr_in from;
    socklen_t len;
    len = sizeof(from);

    while (1)
    {
        r = recvfrom(fd, buf, sizeof(buf) - 1, 0, (struct sockaddr*)&from, &len);
        if (r > 0)
        {
            buf[r] = 0;
            printf("The message from %s is:%s\n", inet_ntoa(from.sin_addr), buf);
        }
        else
        {
            break;
        }

#if 0
        levelDebug(INFO_LEV, "[%s][%d]: ... \n", \
            __FUNCTION__, __LINE__);

        sleep(5);
#endif

    }

    //close socket  
    close(fd);

    return;
}