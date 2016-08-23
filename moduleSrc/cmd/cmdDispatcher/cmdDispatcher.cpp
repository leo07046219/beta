/**************************************************************************************
*  Filename:               cmdDispatcher.cpp
*  Description:            命令行调度器实现文件
*  Author:                 Leo
*  Create:                 2016-07-31
*  Modification history:
*
**************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"
#include "cmdDispatcher.h"

/*
kill main后立即启动main，socket不能立即建立
socket sock_fd=3
bind::Address already in use
*/

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

    memset(&(m_cmdDispatcherParam), 0, sizeof(m_cmdDispatcherParam));
    m_pReceiveBuf = NULL;
    m_pSendBuf = NULL;

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

    m_pReceiveBuf = (char *)malloc(RECEIVE_BUF_SIZE);
    if (NULL == m_pReceiveBuf)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: malloc(RECEIVE_BUF_SIZE[%d]) failed!\n", \
            __FUNCTION__, __LINE__, RECEIVE_BUF_SIZE);
        goto errExit;
    }
    memset(m_pReceiveBuf, 0, RECEIVE_BUF_SIZE);

    m_pSendBuf = (char *)malloc(SEND_BUF_SIZE);
    if (NULL == m_pSendBuf)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: malloc(SEND_BUF_SIZE[%d]) failed!\n", \
            __FUNCTION__, __LINE__, SEND_BUF_SIZE);
        goto errExit;
    }
    memset(m_pSendBuf, 0, SEND_BUF_SIZE);

    lstInit(&(m_cmdDispatcherParam.cmdNodeList));

    levelDebug(INFO_LEV, "[%s][%d]: ... \n", \
        __FUNCTION__, __LINE__);

    if (startCmdDispatcherTask() != 0)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: startCmdDispatcherTask() failed!\n");
        goto errExit;
    }
    levelDebug(INFO_LEV, "[%s][%d]: ... \n", \
        __FUNCTION__, __LINE__);

    //TODO: 通知相关交互线程初始化完成
    pthread_mutex_lock(&m_initMutex);
    m_bModuleInited = true;
    pthread_cond_broadcast(&m_initCond);
    pthread_mutex_unlock(&m_initMutex);

    if (regInputCmd("showInputCmdList", showInputCmdList, (void *)this) != 0)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: regInputCmd(showInputCmdList) failed!\n");
    }

    /*
    levelDebug(INFO_LEV, "[%s][%d]: regInputCmd[test] successfully!\n", \
        __FUNCTION__, __LINE__);
    */

    return 0;

errExit:

    if (m_pReceiveBuf != NULL)
    {
        free(m_pReceiveBuf);
        m_pReceiveBuf = NULL;
    }

    if (m_pSendBuf != NULL)
    {
        free(m_pSendBuf);
        m_pSendBuf = NULL;
    }

    return -1;
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

    //TODO: 指定线程栈大小创建线程

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
    INPUT_CMD_PTR pInputCmd = NULL;
    int sendLen = 0;
    int actSendLen = 0;

    int sock_fd = -1, client_fd = -1;    /*sock_fd：监听socket；client_fd：数据传输socket */
    int sin_size = 0;
    struct sockaddr_in my_addr;    /* 本机地址信息 */
    struct sockaddr_in remote_addr;    /* 客户端地址信息 */
    LIST        list;
    CMD_NODE    cmdNode;

    levelDebug(INFO_LEV, "[%s][%d]: ... \n", \
        __FUNCTION__, __LINE__);

    memset(&(cmdNode), 0, sizeof(cmdNode));

    //create socket  
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sock_fd)
    {
        perror("socket\n");
        exit(-1);
    }
    printf("socket sock_fd=%d\n", sock_fd);

    //build connection address  
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(CMD_INPUT_INTERACT_PORT);
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bzero(&(my_addr.sin_zero), 8);
    if (bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) 
    {
        perror("bind:");
        close(sock_fd);
        exit(-1);
    }
    printf("bind address successful!\n");

    if (listen(sock_fd, MAX_SERVER_LINTEN_CONNECTIONS) == -1)
    {
        perror("listen:");
        exit(-1);
    }
    printf("listen address successful!\n");

    while (1) 
    {
        sin_size = sizeof(struct sockaddr_in);
        if ((client_fd = accept(sock_fd, (struct sockaddr *)&remote_addr, (socklen_t *)&sin_size)) == -1)
        {
            perror("accept:");
            continue;
        }
        printf("accept a connection from %s\n", inet_ntoa(remote_addr.sin_addr));

        //接收inputCmd输入
        int recvbytes = 0;
        memset(m_pReceiveBuf, 0, RECEIVE_BUF_SIZE);
        if (-1 == (recvbytes = recv(client_fd, m_pReceiveBuf, RECEIVE_BUF_SIZE, 0)))
        {
            perror("recv error:");
            exit(1);
        }
        printf("received data from %s\n", inet_ntoa(remote_addr.sin_addr));

        //显示并处理cmd  TODO:
        pInputCmd = (INPUT_CMD_PTR)(m_pReceiveBuf);
        levelDebug(INFO_LEV, "[%s][%d]: receive from [%s:%d] : cmd[%s].param[%s]\n", \
            __FUNCTION__, __LINE__, \
            inet_ntoa(remote_addr.sin_addr), CMD_INPUT_INTERACT_PORT, \
            pInputCmd->inputCmdStr, pInputCmd->inputParamStr);
        
        if (matchCmd(pInputCmd->inputCmdStr, &cmdNode) != true)
        {
            levelDebug(INFO_LEV, "[%s][%d]: invalid input cmd : %s\n", \
                __FUNCTION__, __LINE__, pInputCmd->inputCmdStr);

            if (send(client_fd, "invalid input cmd\n", sizeof("invalid input cmd\n"), 0) == -1)
            {
                perror("send to client error :　");
            }

            goto closeClientConnect;
        }

        memset(m_pSendBuf, 0, SEND_BUF_SIZE);

        levelDebug(INFO_LEV, "[%s][%d]: ... \n", \
            __FUNCTION__, __LINE__);

        if (executeCmd(&cmdNode, pInputCmd->inputParamStr, m_pSendBuf) != 0)
        {
            levelDebug(ERROR_LEV, "[%s][%d]: executeCmd[%s] failed!\n", \
                __FUNCTION__, __LINE__, pInputCmd->inputCmdStr);

            goto closeClientConnect;
        }

        levelDebug(INFO_LEV, "[%s][%d]: ... \n", \
            __FUNCTION__, __LINE__);

        //向cmdInput返回cmd处理结果
        if (send(client_fd, m_pSendBuf, strlen(m_pSendBuf), 0) == -1) 
        {
            perror("send to client error :　");
        }

closeClientConnect:

        struct linger clientLinger;
        memset(&(clientLinger), 0, sizeof(clientLinger));
        clientLinger.l_onoff = 1;		    /* 强制关闭sockt属性*/
        clientLinger.l_linger = 0;	/* lingerSecs---延迟时间*/
        if (setsockopt(client_fd, SOL_SOCKET, SO_LINGER, (char *)&clientLinger, sizeof(clientLinger)) == -1)
        {
            levelDebug(ERROR_LEV, "[%s][%d]: setsockopt of client_fd[%s] failed : ", \
                __FUNCTION__, __LINE__, client_fd);
            perror(" : ");
            return;
        }


        if (client_fd > 0)
        {
            close(client_fd);
            client_fd = -1;
        }
    }

    //close socket
    if (sock_fd > 0)
    {
        close(sock_fd);
        sock_fd = -1;
    }

    return;
}

/*************************************************
* Function:        cmdDispatcherTask()
* Description:     命令调度器任务
* Access Level:    private
* Input:           N/A
* Output:          N/A
* Return:          N/A
*************************************************/
int C_CmdDispatcher::regInputCmd(const char *pCmdName, INPUT_CALL_BACK_FUNC inputCallBackFunc, void *pInputParam)
{
    int             retVal = 0;
    CMD_NODE_PTR    pCmdNode = NULL;
    CMD_NODE_PTR    pTestCmdNode = NULL;
    char            symblicLinkFolder[STD_STR_SIZE];

    if (NULL == pCmdName)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: pCmdName is NULL!\n", \
            __FUNCTION__, __LINE__);
        retVal = -1;
        goto errExit;
    }

    if (NULL == inputCallBackFunc)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: inputCallBackFunc is NULL!\n", \
            __FUNCTION__, __LINE__);
        retVal = -1;
        goto errExit;
    }
     
    if (NULL == pInputParam)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: pInputParam is NULL!\n", \
            __FUNCTION__, __LINE__);
        retVal = -1;
        goto errExit;
    }

    if (strlen(pCmdName) + 1 > MAX_INPUT_CMD_STR_LEN)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: cmdName length [%d] > %d !\n", \
            __FUNCTION__, __LINE__, strlen(pCmdName), MAX_INPUT_CMD_STR_LEN - 1);
        retVal = -1;
        goto errExit;
    }

    waitModuleInited();

    pCmdNode = (CMD_NODE_PTR)malloc(sizeof(CMD_NODE));
    if (NULL == pCmdNode)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: malloc(CMD_NODE[%d B]) failed!\n", \
            __FUNCTION__, __LINE__, sizeof(CMD_NODE));
        retVal = -1;
        goto errExit;
    }

    /*检查命令是否已经被注册过*/
    if (isCmdExist(pCmdName) == true)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: cmd[%s] has already been reged!\n", \
            __FUNCTION__, __LINE__, pCmdName);
        retVal = -1;
        goto errExit;
    }

    pthread_mutex_lock(&m_paramMutex);
    memcpy(pCmdNode->cmdName, pCmdName, min(MAX_INPUT_CMD_STR_LEN, strlen(pCmdName)+1));
    pCmdNode->inputCbFun = inputCallBackFunc;
    pCmdNode->inputParam = pInputParam;
    lstAdd(&(m_cmdDispatcherParam.cmdNodeList), &(pCmdNode->node));
    pthread_mutex_unlock(&m_paramMutex);
    
    //注册的时候创建sbin目录下的软链接   symlink
    memset(symblicLinkFolder, 0, sizeof(symblicLinkFolder));
    snprintf(symblicLinkFolder, STD_STR_SIZE, "/sbin/%s", pCmdName);
    if (symlink(INPUT_CMD_FLODER, symblicLinkFolder) != 0)
    {
        perror("symlink[/sbin/inputCmd] failed : ");
        retVal = -1;
        goto errExit;
    }

    return retVal;

errExit:

    if (pCmdNode != NULL)
    {
        free(pCmdNode);
        pCmdNode = NULL;
    }
    return retVal;
}

/*************************************************
* Function:        isCmdExist()
* Description:     命令名称是否已经被注册
* Access Level:    private
* Input:           pCmdName
* Output:          N/A
* Return:          true -- 已经被注册/false -- 没有被注册
*************************************************/
bool C_CmdDispatcher::isCmdExist(const char *pCmdName)
{
    CMD_NODE_PTR pCmdNode = NULL;

    assert(pCmdName != NULL);

    pthread_mutex_lock(&m_paramMutex);
    for (pCmdNode = (CMD_NODE_PTR)(lstFirst(&(m_cmdDispatcherParam.cmdNodeList))); \
        pCmdNode != NULL; \
        pCmdNode = (CMD_NODE_PTR)(lstNext(&(pCmdNode->node))))
    {
        if (strlen(pCmdName) != strlen(pCmdNode->cmdName))
        {
            continue;
        }

        if (memcmp(pCmdName, pCmdNode->cmdName, strlen(pCmdNode->cmdName)) == 0)
        {
            return true;
        }
    }
    pthread_mutex_unlock(&m_paramMutex);

    return false;
}

/*************************************************
* Function:        matchCmd()
* Description:     匹配命令
* Access Level:    private
* Input:           pCmdName
* Output:          N/A
* Return:          true -- 已经被注册/false -- 没有被注册
*************************************************/
int C_CmdDispatcher::matchCmd(const char *pCmdName, CMD_NODE_PTR pMatchedCmdNode)
{
    CMD_NODE_PTR pCmdNode = NULL;

    assert(pCmdName != NULL);
    assert(pMatchedCmdNode != NULL);

    levelDebug(INFO_LEV, "[%s][%d]: ... \n", \
        __FUNCTION__, __LINE__);

    pthread_mutex_lock(&m_paramMutex);

    for (pCmdNode = (CMD_NODE_PTR)(lstFirst(&(m_cmdDispatcherParam.cmdNodeList))); \
        pCmdNode != NULL; \
        pCmdNode = (CMD_NODE_PTR)(lstNext(&(pCmdNode->node))))
    {
        levelDebug(INFO_LEV, "[%s][%d]: %s\n", \
            __FUNCTION__, __LINE__, pCmdNode->cmdName);
        if (strlen(pCmdName) != strlen(pCmdNode->cmdName))
        {
            continue;
        }

        if (memcmp(pCmdName, pCmdNode->cmdName, strlen(pCmdNode->cmdName)) == 0)
        {
            memcpy(pMatchedCmdNode, pCmdNode, sizeof(*pCmdNode));

            levelDebug(TRACE_LEV, "[%s][%d]: match input cmd : %s\n", \
                __FUNCTION__, __LINE__, pMatchedCmdNode->cmdName);

            pthread_mutex_unlock(&m_paramMutex);
            return true;
        }
    }

    pthread_mutex_unlock(&m_paramMutex);

    return false;
}

/*************************************************
* Function:        executeCmd()
* Description:     执行命令
* Access Level:    private
* Input:           pCmdName
* Output:          N/A
* Return:          0--成功/-1--失败
*************************************************/
int C_CmdDispatcher::executeCmd(CMD_NODE_PTR pCmdNode, const char *pInputBuf, char *pOutputBuf)
{
    if (NULL == pCmdNode)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: pCmdNode is null!\n", \
            __FUNCTION__, __LINE__);
        return -1;
    }

    if (NULL == pCmdNode->inputCbFun)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: pCmdNode->inputCbFun is null!\n", \
            __FUNCTION__, __LINE__);
        return -1;
    }

    return pCmdNode->inputCbFun(pCmdNode->inputParam, pInputBuf, pOutputBuf);
}

/*************************************************
* Function:        executeCmd()
* Description:     执行命令
* Access Level:    private
* Input:           pCmdName
* Output:          N/A
* Return:          0--成功/-1--失败
*************************************************/
int C_CmdDispatcher::showInputCmdList(void *pInputParam, const char *pInputBuf, char *pOutputBuf)
{
    CMD_NODE_PTR pCmdNode = NULL;

    assert(ms_cmdDispatcherObjPtr != NULL);
    for (pCmdNode = (CMD_NODE_PTR)(lstFirst(&(ms_cmdDispatcherObjPtr->m_cmdDispatcherParam.cmdNodeList))); \
        pCmdNode != NULL; \
        pCmdNode = (CMD_NODE_PTR)(lstNext(&(pCmdNode->node))))
    {
        sprintf(pOutputBuf, "the inputCmdList is:\n");
        sprintf(pOutputBuf + strlen(pOutputBuf), "%s\n", \
            pCmdNode->cmdName);
    }

    return 0;
}