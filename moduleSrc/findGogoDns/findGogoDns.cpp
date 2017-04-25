/**************************************************************************************
*  Filename:               findGogoDns.cpp
*  Description:            gogoDns查找类实现文件
*  Author:                 Leo
*  Create:                 2016-06-13
*  Modification history:
*TODO:
1.参考gogotester上的延时，确定dns超时评价标准：
目前相同的ip，gogotest上延时大于本程序的；
2.innerIpSet中ip只有一个时，getIp()的-3出异常；
3.gogoIpSet中合法ip会重复
4.gogoCfg中为空时，如何启动；
5.搜索的循环机制，loop the innerIpSet；
6.搜索结果超时如何呈现：txt中直接标出来，还是只维持在内存:
0-100ms、100-200ms、200-300ms、300-500ms，区间内升序
7.curl退出时需要销毁的，在errExit和析构中完成
**************************************************************************************/

#include "common.h"
#include "findGogoDns.h"
#include "curl.h"
#include "cmdDispatcher.h"

/*静态成员变量初始化*/
C_FindGogoDns *C_FindGogoDns::ms_findGogoDnsObjPtr = NULL;

C_FindGogoDns::C_FindGogoDns()
{
    //TODO: set name
    pthread_mutex_t  m_initMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t  m_singleMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t  m_paramMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t  m_gogoMutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_t  m_initCond = PTHREAD_COND_INITIALIZER;

    ms_findGogoDnsObjPtr = NULL;
    m_bModuleInited = false;
}

C_FindGogoDns::~C_FindGogoDns()
{
    pthread_mutex_destroy(&m_paramMutex);
    pthread_mutex_destroy(&m_singleMutex);
    pthread_mutex_destroy(&m_initMutex);
    pthread_mutex_destroy(&m_gogoMutex);

    pthread_cond_destroy(&m_initCond);
}

/*************************************************
* Function:        getFindGogoDnsPtr()
* Description:     获取C_FindGogoDns唯一对象地址
* Access Level:    public
* Input:           N/A
* Output:          C_FindGogoDns唯一对象地址
* Return:          N/A
*************************************************/
/*静态成员变量声明*/
pthread_mutex_t C_FindGogoDns::m_singleMutex;

C_FindGogoDns *C_FindGogoDns::getFindGogoDnsPtr(void)
{
    if (NULL == ms_findGogoDnsObjPtr)
    {
        pthread_mutex_lock(&m_singleMutex);
        if (NULL == ms_findGogoDnsObjPtr)
        {
            /*创建唯一实例*/
            ms_findGogoDnsObjPtr = new C_FindGogoDns();
        }
        pthread_mutex_unlock(&m_singleMutex);
    }

    return ms_findGogoDnsObjPtr;
}

/*对外提供对象实例指针获取接口*/
C_FindGogoDns *getFindGogoDnsObjPtr()
{
    return C_FindGogoDns::getFindGogoDnsPtr();
}

/***********************************************************
* Function:       initFindGogoDns()
* Description:    初始化gogo dns查找类
* Access Level:   public
* Input:          N/A
* Output:         N/A
* Return:         0---成功/-1---失败
***********************************************************/
int C_FindGogoDns::initFindGogoDns(void)
{
    CURL *curl = NULL;
    C_CmdDispatcher *cmdDispatcherObjPtr = NULL;

    curl = curl_easy_init();
    if (NULL == curl)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: curl_easy_init() failed!\n", \
            __FUNCTION__, __LINE__);
        return -1;
    }

    levelDebug(INFO_LEV, "[%s][%d]: ... !\n", \
        __FUNCTION__, __LINE__);

    if (startFindGogoDnsTask() != 0)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: startFindGogoDnsTask() failed!\n", \
            __FUNCTION__, __LINE__);
        return -1;
    }

    cmdDispatcherObjPtr = getCmdDispatcherObjPtr();
    if (NULL == cmdDispatcherObjPtr)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: getCmdDispatcherObjPtr() failed!\n");
        return -1;
    }
    if (cmdDispatcherObjPtr->regInputCmd("findGogoDns", findGogoDns, (void *)this) != 0)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: regInputCmd(findGogoDns) failed!\n");
        return -1;
    }

    pthread_mutex_lock(&m_initMutex);
    m_bModuleInited = true;
    pthread_cond_broadcast(&m_initCond);
    pthread_mutex_unlock(&m_initMutex);

    return 0;
}
/***********************************************************
* Function:       initFindGogoDns()
* Description:    初始化gogo dns查找类
* Access Level:   public
* Input:          N/A
* Output:         N/A
* Return:         0---成功/-1---失败
***********************************************************/
int C_FindGogoDns::startFindGogoDnsTask(void)
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
    if (pthread_create(&tid, &attr, (START_ROUTINE)findGogoDnsTaskShell, (void *)this) != 0)
    {
        retVal = -1;
        goto exit;;
    }

exit:

    if (pthread_attr_destroy(&attr) != 0)
    {
        retVal = -1;
    }

    //TODO: 通知相关交互线程初始化完成

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
void C_FindGogoDns::waitModuleInited(void)
{
    if (false == m_bModuleInited)
    {
        pthread_mutex_lock(&m_initMutex);
        if (false == m_bModuleInited)
        {
            /*启动时若线程初始化未完成，相关线程等待*/
            levelDebug(WARNING_LEV, "[%s][%d]:C_FindGogoDns not inited, wait!\n", \
                __FUNCTION__, __LINE__);

            pthread_cond_wait(&m_initCond, &m_initMutex);
        }
        pthread_mutex_unlock(&m_initMutex);
    }
    return;
}


/*************************************************
* Function:        findGogoDnsTaskShell()
* Description:     dns查找启动外壳
* Access Level:    private
* Input:           pThis --- 类对象指针
* Output:          N/A
* Return:          N/A
*************************************************/
void C_FindGogoDns::findGogoDnsTaskShell(void* pThis)
{
    //TODO: set thread name

    assert(pThis != NULL);

    ((C_FindGogoDns*)pThis)->findGogoDnsTask();
}

/*************************************************
* Function:        findGogoDnsTask()
* Description:     dns查找启动任务
* Access Level:    private
* Input:           N/A
* Output:          N/A
* Return:          N/A
*************************************************/
void C_FindGogoDns::findGogoDnsTask(void)
{
    levelDebug(INFO_LEV, "[%s][%d]: ... \n", \
        __FUNCTION__, __LINE__);

    if (checkIpFromIpSet() != 0)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: checkIpFromIpSet() failed! \n", \
            __FUNCTION__, __LINE__);
        return;
    }
    while (1)
    {
        levelDebug(INFO_LEV, "[%s][%d]: ... \n", \
            __FUNCTION__, __LINE__);

        sleep(63);
    }
    return;
}

/***********************************************************
* Function:       checkIp()
* Description:
* Access Level:   public
* Input:          N/A
* Output:         N/A
* Return:         0---成功/-1---失败
***********************************************************/
int C_FindGogoDns::checkIpFromIpSet(void)
{
    char ipStr[IP_V4_STR_LEN];

    memset(ipStr, 0, sizeof(ipStr));

#if 1
    if (getIp(ipStr) != 0)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: getIp() failed!\n", \
            __FUNCTION__, __LINE__);
        return -1;
    }
#endif

#if 0
    memcpy((ipStr), TEST_IP_STR, strlen(TEST_IP_STR));
    if (checkIp(ipStr) != 0)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: checkIp(%s) failed!\n", \
            __FUNCTION__, __LINE__, ipStr);
        return -1;
    }
#endif

    return 0;
}

/***********************************************************
* Function:       checkIp()
* Description:
* Access Level:   public
* Input:          N/A
* Output:         N/A
* Return:         0---成功/-1---失败
***********************************************************/
size_t headerCB(char *buffer, size_t size, size_t nitems, void *userdata)
{
    /* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
    /* 'userdata' is set with CURLOPT_HEADERDATA */

    levelDebug(INFO_LEV, "[%s][%d]: size[%d].nitems[%d].buffer[%s]\n", \
        __FUNCTION__, __LINE__, size, nitems, buffer);

    strcpy((char *)userdata, (char *)buffer);

    return nitems;

    /*
    每收到一行数据，就调一次回调，最后一次收到的是一个回车(覆盖了前一次的数据)，
    所以在main中打出的只有一个回车
    */
    /*
    [headerCB][251]: size[1].nitems[32].buffer[HTTP/1.1 301 Moved Permanently
    ]
    [headerCB][251]: size[1].nitems[34].buffer[Location: http://www.google.com/
    ]
    [headerCB][251]: size[1].nitems[40].buffer[Content-Type: text/html; charset=UTF-8
    ]
    [headerCB][251]: size[1].nitems[37].buffer[Date: Fri, 01 Jul 2016 15:20:20 GMT
    ]
    [headerCB][251]: size[1].nitems[40].buffer[Expires: Sun, 31 Jul 2016 15:20:20 GMT
    ]
    [headerCB][251]: size[1].nitems[40].buffer[Cache-Control: public, max-age=2592000
    ]
    [headerCB][251]: size[1].nitems[13].buffer[Server: gws
    ]
    [headerCB][251]: size[1].nitems[21].buffer[Content-Length: 219
    ]
    [headerCB][251]: size[1].nitems[33].buffer[X-XSS-Protection: 1; mode=block
    ]
    [headerCB][251]: size[1].nitems[29].buffer[X-Frame-Options: SAMEORIGIN
    ]
    [headerCB][251]: size[1].nitems[2].buffer[
    ]
    */
}

/***********************************************************
* Function:       checkIp()
* Description:
* Access Level:   public
* Input:          N/A
* Output:         N/A
* Return:         0---成功/-1---失败
***********************************************************/
size_t write_data(void *ptr, size_t size, size_t nmemb, void *pUser)
{
    /*
    levelDebug(INFO_LEV, "[%s][%d]: size[%d].nmemb[%d]\n", \
        __FUNCTION__, __LINE__, size, nmemb);
    */
    //strcpy((char *)pUser, (char *)ptr);
    //return size * nmemb;

    int retLen = min(size * nmemb, STD_1K);
    memcpy((char *)pUser, (char *)ptr, retLen);
    *((char *)pUser + STD_1K -1) = '\0';
    return retLen;

}

/***********************************************************
* Function:       recordIp()
* Description:    将gogo IP 以及超时 写入txt，暂时也写入超时，TODO: 后面超时放在内存中，txt只记录ip
* Access Level:   public
* Input:          N/A
* Output:         N/A
* Return:         0---成功/-1---失败
***********************************************************/
int C_FindGogoDns::recordIp(const char *pIpStr, const float delayTime)
{
    int     retVal = 0;
    FILE    *fileHandle = NULL;
    int     errNo = 0;
    char    lineBuf[LINE_BUF_SIZE];
    int     writeLen = 0;

    memset(lineBuf, 0, sizeof(lineBuf));
    //TODO: 目前，锁整个文件操作过程
    pthread_mutex_lock(&m_gogoMutex);

    if (NULL == (fileHandle = fopen(GOGO_IP_SET_FILE, "ab+")))
    {
        levelDebug(ERROR_LEV, "[%s][%d]: fopen[%s] failed![fileHandle=%d]", \
            __FUNCTION__, __LINE__, GOGO_IP_SET_FILE, fileHandle);
        perror(":");
        retVal = -1;
        goto exit;
    }

    //TODO: 将ip和超时写入txt

    snprintf(lineBuf, LINE_BUF_SIZE, "%s-%d\n", pIpStr, (int)delayTime);

    //TODO:实际写的还是LINE_BUF_SIZE

    levelDebug(INFO_LEV, "[%s][%d]: strlen(lineBuf) +　1 = %d\n", \
        __FUNCTION__, __LINE__, strlen(lineBuf) + 1);

    if ((writeLen = fwrite(lineBuf, sizeof(char), strlen(lineBuf), fileHandle)) != strlen(lineBuf))
    {
        levelDebug(ERROR_LEV, "[%s][%d]: wtire file[%s] error : writeLen[%d]", \
            __FUNCTION__, __LINE__, GOGO_IP_SET_FILE, writeLen);
        perror(":");
        retVal = -1;
        goto exit;
    }

exit:
    if (fileHandle != NULL)
    {
        fclose(fileHandle);
        fileHandle = NULL;
    }
    pthread_mutex_unlock(&m_gogoMutex);

    return retVal;
}
/***********************************************************
* Function:       checkIp()
* Description:
* Access Level:   public
* Input:          N/A
* Output:         N/A
* Return:         0---成功/-1---失败
***********************************************************/
int C_FindGogoDns::checkIp(const char *pIpStr)
{
    CURL        *pCurl;
    CURLcode    retCode;
    //CURLINFO    curlInfo;
    double      time = 0;
    char        urlRetBuf[STD_1K];
    char        headBuf[REQUEST_LEN];
    char        *pGogoPosition = NULL;

    memset(urlRetBuf, 0, sizeof(urlRetBuf));
    memset(headBuf, 0, sizeof(headBuf));

    /*
    levelDebug(TRACE_LEV, "[%s][%d]: ... pIpStr[%s]\n", \
        __FUNCTION__, __LINE__, pIpStr);
    */

    //调试多线程，暂时屏蔽
#if 1
    pCurl = curl_easy_init();

    if (pCurl != NULL)
    {
        curl_easy_setopt(pCurl, CURLOPT_URL, pIpStr);
        curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, CURL_ACCESS_TIMEOUT);    //秒

        curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1);    //屏蔽其它信号---暂时不用

        /*
        目前不通过header来判断google的dns，接收header的成本很高，
        需要触发多次header回调才能完整收到包含serverName的header内容
        */
        /*
        curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, headerCB);
        curl_easy_setopt(pCurl, CURLOPT_HEADERDATA, headBuf);
        */

        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, write_data); //设置下载数据的回调函数
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, urlRetBuf);

        curl_easy_setopt(pCurl, CURLOPT_HEADER, 1L);     //下载数据包括HTTP头部 类似于"curl -I ..."
        //curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);  //交互信息打印，调试用

        retCode = curl_easy_perform(pCurl);

        curl_easy_getinfo(pCurl, CURLINFO_NAMELOOKUP_TIME, &time);

        curl_easy_cleanup(pCurl);

#if 0 
        levelDebug(TRACE_LEV, "[%s][%d]: nameLookUpTime[%f]\n", \
            __FUNCTION__, __LINE__, time);

        levelDebug(TRACE_LEV, "[%s][%d]: strlen(%d)\nurlRetBuf[\n%s]\n", \
            __FUNCTION__, __LINE__, strlen(urlRetBuf), urlRetBuf);
#endif

        pGogoPosition = strstr(urlRetBuf, GOGO_WEB_SITE_STR);
        if (pGogoPosition != NULL)
        {
            levelDebug(TRACE_LEV, "[%s][%d]: pIpStrnameLookUpTime[%f]\n", \
                __FUNCTION__, __LINE__, time);

            levelDebug(TRACE_LEV, "[%s][%d]: pIpStr[%s]\n", \
                __FUNCTION__, __LINE__, pIpStr);

            /*
            levelDebug(TRACE_LEV, "\n[%s][%d]: strlen(%d)\nurlRetBuf[\n%s]\n", \
                __FUNCTION__, __LINE__, strlen(urlRetBuf), urlRetBuf);
            */

            levelDebug(WARNING_LEV, "[%s][%d]: find GOGO:ip[%s].delay[%f ms]\n", \
                __FUNCTION__, __LINE__, \
                pIpStr, time * 1000);

            if (recordIp(pIpStr, time * 1000) != 0)
            {
                levelDebug(ERROR_LEV, "[%s][%d]: recordIp(%s, %fs) failed!\n", \
                    __FUNCTION__, __LINE__, \
                    pIpStr, time * 1000);
            }
        }

        //TODO: curl多线程 + 减少curl超时
        //TODO: 评价gogo dns好坏的，是什么超时？

        /*
        HTTP/1.1 301 Moved Permanently
        Location: http://www.google.com/
        Content-Type: text/html; charset=UTF-8
        Date: Wed, 29 Jun 2016 15:00:45 GMT
        Expires: Fri, 29 Jul 2016 15:00:45 GMT
        Cache-Control: public, max-age=2592000
        Server: gws
        Content-Length: 219
        X-XSS-Protection: 1; mode=block
        X-Frame-Options: SAMEORIGIN

        <HTML><HEAD><meta http-equiv="content-type" content="text/html;charset=utf-8">  //通过下面数据判断gogo
        <TITLE>301 Moved</TITLE></HEAD><BODY>
        <H1>301 Moved</H1>
        The document has moved
        <A HREF="http://www.google.com/">here</A>.
        </BODY></HTML>
        */


        //levelDebug(ERROR_LEV, "[%s][%d]: () failed!\n");

    }
#endif

    return 0;
}
/*************************************************
* Function:        checkIpTaskFuncTaskShell()
* Description:     dns查找启动外壳
* Access Level:    private
* Input:           pThis --- 类对象指针
* Output:          N/A
* Return:          N/A
*************************************************/
void C_FindGogoDns::checkIpTaskFuncTaskShell(CHECK_IP_TASK_INFO_PTR pCheckIpTaskInfo)
{
    //TODO: set thread name
    void *pUsr = NULL;

    assert(pCheckIpTaskInfo != NULL);

    pUsr = pCheckIpTaskInfo->pUsr;

    if (NULL == pUsr)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: invald pUsr[NULL]\n", \
            __FUNCTION__, __LINE__);
        return ;
    }

    ((C_FindGogoDns*)pUsr)->checkIpTaskFunc(&(pCheckIpTaskInfo->checkIpInfo));
}
/***********************************************************
* Function:       multiThreadGetIp()
* Description:
* Access Level:   public
* Input:          N/A
* Output:         N/A
* Return:         0---成功/-1---失败
* NOTE:
1.ip分配，线程控制--什么时候停,
将每个线程需要处理的ip数目给他，处理完，返回
***********************************************************/
int C_FindGogoDns::checkIpTaskFunc(CHECK_IP_INFO_PTR pCheckIpInfo)
{
    assert(pCheckIpInfo != NULL);

    levelDebug(TRACE_LEV, "[%s][%d]:threadId[0x%x]:[%d - %d]\n", \
        __FUNCTION__, __LINE__, \
        pthread_self(), \
        pCheckIpInfo->startIpIdx, pCheckIpInfo->endIpIdx);

    for (int ipIdx = 0; ipIdx < (pCheckIpInfo->endIpIdx - pCheckIpInfo->startIpIdx); ipIdx++)
    {
        if (checkIp(pCheckIpInfo->pIpStrBuf[ipIdx].ip) != 0)
        {
            levelDebug(ERROR_LEV, "[%s][%d]: checkIp(idx[%d].[%s]) failed!\n", \
                __FUNCTION__, __LINE__, ipIdx, pCheckIpInfo->pIpStrBuf[ipIdx].ip);
            continue;
        }
    }

    return 0;
}

/***********************************************************
* Function:       multiThreadGetIp()
* Description:
* Access Level:   public
* Input:          N/A
* Output:         N/A
* Return:         0---成功/-1---失败
* NOTE:
1.ip分配，线程控制--什么时候停
***********************************************************/
int C_FindGogoDns::multiThreadGetIp(int ipNumInSeg, IP_STR_PTR pIpStrBuf)
{
    int                 ipNumForEachThread = 0;
    CHECK_IP_TASK_INFO  checkIpTaskInfo[MAX_CHECK_IP_THREAD_NUMBER];
    pthread_t           tid[MAX_CHECK_IP_THREAD_NUMBER];
    int                 threadIdx = 0;
    int                 error = 0;
    int                 actualThreadNum = 0;

    assert(pIpStrBuf != NULL);
    if (ipNumInSeg <= 0)
    {
        levelDebug(WARNING_LEV, "[%s][%d]: invalid ipNumInSeg[%d]\n", \
            __FUNCTION__, __LINE__, ipNumInSeg);
        return -1;
    }

    memset(&(checkIpTaskInfo), 0, sizeof(checkIpTaskInfo));

    ipNumForEachThread = \
        ((ipNumInSeg / MAX_CHECK_IP_THREAD_NUMBER) == 0) ? 1: (ipNumInSeg / MAX_CHECK_IP_THREAD_NUMBER);

    //ip段划分

    //线程信息封装

    //启动线程

    //等待线程结束

    /* Must initialize libcurl before any threads are started */
    curl_global_init(CURL_GLOBAL_ALL);//TODO :放在这里初始化是否合适？还是要放在最外面？？

    levelDebug(TRACE_LEV, "[%s][%d]: ipNumForEachThread[%d]\n", \
        __FUNCTION__, __LINE__, ipNumForEachThread);

    for (threadIdx = 0; threadIdx < MAX_CHECK_IP_THREAD_NUMBER;  threadIdx++)
    {
        checkIpTaskInfo[threadIdx].pUsr = this;
        checkIpTaskInfo[threadIdx].checkIpInfo.pIpStrBuf = pIpStrBuf;

        if ((threadIdx * ipNumForEachThread + ipNumForEachThread) <= ipNumInSeg)
        {
            actualThreadNum++;
            checkIpTaskInfo[threadIdx].checkIpInfo.startIpIdx = threadIdx * ipNumForEachThread;
            checkIpTaskInfo[threadIdx].checkIpInfo.endIpIdx = checkIpTaskInfo[threadIdx].checkIpInfo.startIpIdx + ipNumForEachThread;

            levelDebug(INFO_LEV, "[%s][%d]: checkIpTaskInfo[%02d].[%d-%d]\n", \
                __FUNCTION__, __LINE__, \
                threadIdx, \
                checkIpTaskInfo[threadIdx].checkIpInfo.startIpIdx, \
                checkIpTaskInfo[threadIdx].checkIpInfo.endIpIdx);

#if 1
            //TODO: start thread
            error = pthread_create(&tid[threadIdx], 
                NULL, /* default attributes please */
                (START_ROUTINE)checkIpTaskFuncTaskShell,
                (void *)&checkIpTaskInfo[threadIdx]);

            if (0 != error)
                fprintf(stderr, "Couldn't run thread number %d, errno %d\n", threadIdx, error);
            else
                fprintf(stderr, "Thread %d, finished.\n", threadIdx);
#endif

        }

    }

#if 1
    /*等待所有是线程退出*/
    for (threadIdx = 0; threadIdx < actualThreadNum; threadIdx++)
    {
        error = pthread_join(tid[threadIdx], NULL);
        fprintf(stderr, "Thread %d terminated\n", threadIdx);
    }
#endif

    return 0;
}

/***********************************************************
* Function:     getPresentIpSeg()
* Description:  获取当前ip段，用于恢复之前的检查进度
* Access Level: public
* Input:        N/A
* Output:       N/A
* Return:       0---成功/-1---失败
***********************************************************/
int C_FindGogoDns::resumeIpSeg(char *pIpStr)
{
    int     retVal = 0;
    FILE    *fileHandle = NULL;
    int     errNo = 0;
    int     writeLen = 0;

    assert(pIpStr != NULL);

    if (NULL == (fileHandle = fopen(GOGO_CFG_FILE, "r")))
    {
        levelDebug(ERROR_LEV, "[%s][%d]: fopen[%s] failed![fileHandle=%d]", \
            __FUNCTION__, __LINE__, GOGO_CFG_FILE, fileHandle);
        perror(":");
        retVal = -1;
        goto exit;
    }

    if (fgets(pIpStr, IP_SEGMENT_STR_LEN, fileHandle) == NULL)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: fgets failed![fileHandle=%d]", \
            __FUNCTION__, __LINE__, GOGO_CFG_FILE, fileHandle);
        perror(":");
        retVal = -1;
        goto exit;
    }

    //pIpStr[strlen(pIpStr) - 1] = '\0';  /*去掉换行符*/

exit:
    if (fileHandle != NULL)
    {
        fclose(fileHandle);
        fileHandle = NULL;
    }

    return retVal;
}

/***********************************************************
* Function:     storeIpSeg()
* Description:  保存当前ip端，用于恢复之前的检查进度
* Access Level: public
* Input:        N/A
* Output:       N/A
* Return:       0---成功/-1---失败
***********************************************************/
int C_FindGogoDns::storeIpSeg(char *pIpStr)
{
    int     retVal = 0;
    FILE    *fileHandle = NULL;
    int     errNo = 0;
    char    lineBuf[LINE_BUF_SIZE];
    int     writeLen = 0;

    memset(lineBuf, 0, sizeof(lineBuf));

    if (NULL == (fileHandle = fopen(GOGO_CFG_FILE, "w+")))
    {
        levelDebug(ERROR_LEV, "[%s][%d]: fopen[%s] failed![fileHandle=%d]", \
            __FUNCTION__, __LINE__, GOGO_CFG_FILE, fileHandle);
        perror(":");
        retVal = -1;
        goto exit;
    }

    snprintf(lineBuf, LINE_BUF_SIZE, "%s\n", pIpStr);

    //TODO:实际写的还是LINE_BUF_SIZE

    levelDebug(INFO_LEV, "[%s][%d]: strlen(lineBuf) +　1 = %d\n", \
        __FUNCTION__, __LINE__, strlen(lineBuf) + 1);

    if ((writeLen = fwrite(lineBuf, sizeof(char), strlen(lineBuf), fileHandle)) != strlen(lineBuf))
    {
        levelDebug(ERROR_LEV, "[%s][%d]: wtire file[%s] error : writeLen[%d]", \
            __FUNCTION__, __LINE__, GOGO_CFG_FILE, writeLen);
        perror(":");
        retVal = -1;
        goto exit;
    }


exit:
    if (fileHandle != NULL)
    {
        fclose(fileHandle);
        fileHandle = NULL;
    }

    return retVal;

    levelDebug(TRACE_LEV, "[%s][%d]: ... \n", \
        __FUNCTION__, __LINE__);
    return 0;
}

/***********************************************************
* Function:       testCurl()
* Description:
* Access Level:   public
* Input:          N/A
* Output:         N/A
* Return:         0---成功/-1---失败
***********************************************************/
int C_FindGogoDns::getIp(char *pIpStr)
{
    int     retVal = 0;
    int     ipSegmentCnt = 0;
    char    ipSegment[IP_SEGMENT_STR_LEN];
    char    resumedSegment[IP_SEGMENT_STR_LEN];
    FILE    *fileHandle = NULL;
    IP_STR_PTR  pIpStrBuf = NULL;
    int     ipSegLen = 0;
    int     ipNumInSeg = 0;
    int     ipIdx = 0;
    int     totalIpNumInSeg = 0;    //所有ip段中的ip个数汇总
    bool    noValidIpSeg = true;
    assert(pIpStr != NULL);

    memset(ipSegment, 0, sizeof(ipSegment));
    memset(resumedSegment, 0, sizeof(resumedSegment));

    if (NULL == (fileHandle = fopen(INNER_IP_SET_FILE, "r")))
    {
        levelDebug(ERROR_LEV, "[%s][%d]: fopen[%s] failed![fileHandle=%d]\n", \
            __FUNCTION__, __LINE__, INNER_IP_SET_FILE, fileHandle);
        return -1;
    }

    /*一个ip段最多256*256个ip*/
    pIpStrBuf = (IP_STR_PTR)malloc(sizeof(IP_STR) * MAX_IP_NUM_IN_A_SEGMENT);
    if (NULL == pIpStrBuf)
    {
        levelDebug(ERROR_LEV, "[%s][%d]:calloc memmory of pIpStrBuf[%d B] failed!", \
            __FUNCTION__, __LINE__, sizeof(IP_STR) * MAX_IP_NUM_IN_A_SEGMENT);
        retVal = -1;
        goto exit;
    }

    /*获取保存的最近一次查询的ip段，用于恢复之前进度*/
    memset(resumedSegment, 0, sizeof(resumedSegment));
    if (resumeIpSeg(resumedSegment) != 0)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: storeIpSeg() failed!\n", \
            __FUNCTION__, __LINE__);
        retVal = -1;
        goto exit;
    }
    levelDebug(TRACE_LEV, "[%s][%d]: resumedSegment[%s]\n", \
        __FUNCTION__, __LINE__, resumedSegment);

    memset(pIpStrBuf, 0, sizeof(pIpStrBuf));
    ipSegmentCnt = 0;
    /*!!!txt中的ip段后面必须跟回车!!!*/
    while (fgets(ipSegment, IP_SEGMENT_STR_LEN, fileHandle) != NULL)
    {
        ipSegmentCnt++;

        if (true == noValidIpSeg)
        {
            if (strncmp(ipSegment, resumedSegment, strlen(resumedSegment) - 3) != 0)//TODO: ip串比较需要-3，dig deep reason
            {
                /*
                levelDebug(TRACE_LEV, "[%s][%d]: ipSegment[%s].resumedSegment[%s].strlen(resumedSegment)=[%d]\n", \
                    __FUNCTION__, __LINE__, ipSegment, resumedSegment, strlen(resumedSegment));
                */
                continue;
            }
            noValidIpSeg = false;
        }

        ipSegLen = strlen(ipSegment);
        ipSegment[ipSegLen - 1] = '\0';  /*去掉换行符*/

        /*
        levelDebug(INFO_LEV, "[%s][%d]: ipSeg[%04d]: %s\n", \
        __FUNCTION__, __LINE__, ipSegmentCnt, ipSegment);
        */
#if 0
        if (strncmp(ipSegment, resumedSegment, strlen(resumedSegment)) != 0)
        {
            levelDebug(TRACE_LEV, "[%s][%d]: ipSegment[%s].resumedSegment[%s]\n", \
                __FUNCTION__, __LINE__, ipSegment, resumedSegment);
            continue;
        }
#endif

        levelDebug(TRACE_LEV, "[%s][%d]: storeIpSeg[%s]\n", \
            __FUNCTION__, __LINE__, ipSegment);
        if (storeIpSeg(ipSegment) != 0)
        {
            levelDebug(ERROR_LEV, "[%s][%d]: storeIpSeg(%s) failed!\n", \
                __FUNCTION__, __LINE__, ipSegment);
            retVal = -1;
            goto exit;
        }
         //get ip from  xxx.xxx.0-255.0-255
        ipNumInSeg = 0;
        if (getIpFromIpSegment(ipSegment, &ipNumInSeg, pIpStrBuf) != 0)
        {
            levelDebug(ERROR_LEV, "[%s][%d]: getIpFromIpSegment(%s) failed!\n", \
                __FUNCTION__, __LINE__, ipSegment);
            retVal = -1;
            goto exit;
        }

        totalIpNumInSeg += ipNumInSeg;

#if 0
        for (ipIdx = 0; ipIdx < ipNumInSeg; ipIdx++)
        {
            levelDebug(INFO_LEV, "[%s][%d]: ipSeg[%03d].ip[%05d]:%s \n", \
                __FUNCTION__, __LINE__, \
                ipSegmentCnt, ipIdx, pIpStrBuf[ipIdx].ip);

            //memcpy(&(pIpStrBuf[ipIdx].ip), TEST_IP_STR, strlen(TEST_IP_STR));
            //TODO: 如何并发？
            if (checkIp(pIpStrBuf[ipIdx].ip) != 0)
            {
                levelDebug(ERROR_LEV, "[%s][%d]: checkIp(%s) failed!\n", \
                    __FUNCTION__, __LINE__, pIpStrBuf[ipIdx].ip);
                return -1;
            }

        }
#endif

#if 1
        if (multiThreadGetIp(ipNumInSeg, pIpStrBuf) != 0)
        {
            levelDebug(ERROR_LEV, "[%s][%d]: multiThreadGetIp() failed!\n", \
                __FUNCTION__, __LINE__);
            return -1;
        }
#endif
        //debug:只对txt中的第一行操作
        //retVal = 0;
        //goto exit;
        memset(ipSegment, 0, sizeof(ipSegment));

    }

    levelDebug(INFO_LEV, "[%s][%d]: totalIpNumInSeg[%d]\n", \
        __FUNCTION__, __LINE__, totalIpNumInSeg);


    memcpy(pIpStr, pIpStrBuf->ip, sizeof(IP_STR));

exit:

    if (pIpStrBuf != NULL)
    {
        free(pIpStrBuf);
        pIpStrBuf = NULL;
    }

    if (fileHandle != NULL)
    {
        fclose(fileHandle);
        fileHandle = NULL;
    }

    return 0;
}

/***********************************************************
* Function:     countCharInStr()
* Description:  统计字符串中特定字符的个数
* Access Level: public
* Input:        pStr        --- 待统计字符串
* Input:        charToCount --- 待统计字符
* Output:       pCharCnt    --- 字符个数
* Return:       0---成功/-1---失败
***********************************************************/
static int countCharInStr(const char *pStr, const char charToCount, int *pCharCnt)
{
    const char *pChar = NULL;

    if ((NULL == pStr) || (NULL == pCharCnt))
    {
        levelDebug(ERROR_LEV, "[%s][%d]: invalid input\n", \
            __FUNCTION__, __LINE__);
        return -1;
    }

    /*
    levelDebug(INFO_LEV, "[%s][%d]: charToCount[%c].str[%s]\n", \
    __FUNCTION__, __LINE__, charToCount, pStr);
    */

    pChar = pStr;

    while ((*pChar) != '\0')
    {
        //printf("char[%c]\n", *pChar);
        if ((*pChar) == charToCount)
        {
            //printf("find a \'-\'\n");
            (*pCharCnt)++;
        }
        pChar++;
    }

    return 0;
}

/***********************************************************
* Function:     getIpFromIpSegment()
* Description:  从ip地址段(XXX.XXX-XXX.XXX-XXX.XXX-XXX or XXX.XXX.XXX-XXX.XXX-XXX or XXX.XXX.XXX.XXX-XXX)中解析出ip地址
* Access Level: public
* Input:        N/A
* Output:       N/A
* Return:       0---成功/-1---失败
***********************************************************/
int C_FindGogoDns::getIpFromIpSegment(const char *ipSegment, int *pIpNum, IP_STR_PTR pIpStrArray)
{
    int     retVal = 0;
    int     charCnt = 0;
    int     a = 0, bStart = 0, bEnd = 0, cStart = 0, cEnd = 0, dStart = 0, dEnd = 0; //a.b.c.d.点分十进制
    int ipNumInSeg = 0;
    assert(ipSegment != NULL);
    assert(pIpNum != NULL);
    assert(pIpStrArray != NULL);

    ///*
    levelDebug(INFO_LEV, "[%s][%d]: %s\n", \
        __FUNCTION__, __LINE__, ipSegment);
    //*/

    /*统计ipSeg中‘-’个数*/
    if (countCharInStr(ipSegment, '-', &charCnt) != 0)
    {
        levelDebug(ERROR_LEV, "[%s][%d]: countCharInStr() failed!\n", \
            __FUNCTION__, __LINE__);
        retVal = -1;
        goto exit;
    }

    levelDebug(INFO_LEV, "[%s][%d]: charCnt = %d\n", \
        __FUNCTION__, __LINE__, charCnt);

    switch (charCnt)
    {
    case 0:
        /*没有‘-’,ipSeg = ip*/
        *pIpNum = 1;
        memcpy((pIpStrArray[0].ip), ipSegment, strlen(ipSegment) - 1);//不-1，curl不返回，ip地址有问题
        /*或者用下面的方法重新封装ip*/
        /*
        sscanf(ipSegment, "%d.%d.%d.%d", &a, &bStart, &cStart, &dStart);
        sprintf((char *)&(pIpStrArray[0].ip), "%d.%d.%d.%d", a, bStart, cStart, dStart);
        */
        break;

    case 1:
        /*只有1个‘-’--一个动态ip范围(在最后一个点分十进制上)：XXX.XXX.XXX.XXX-XXX*/
        //a.bStart.cStart-cEnd.dStart-dEnd
        sscanf(ipSegment, "%d.%d.%d.%d-%d", &a, &bStart, &cStart, &dStart, &dEnd);

        levelDebug(INFO_LEV, "[%s][%d]:xxx.xxx.xxx.[%d-%d]\n", \
            __FUNCTION__, __LINE__, dStart, dEnd);

        *pIpNum = dEnd - dStart + 1;
        for (int dIdx = 0; dIdx < *pIpNum; dIdx++)
        {
            sprintf((char *)&(pIpStrArray[dIdx].ip), "%d.%d.%d.%d", a, bStart, cStart, dStart + dIdx);
        }
        break;

    case 2:
        /*有2个‘-’--一个动态ip范围(在第3、4个点分十进制上)：XXX.XXX.XXX-XXX.XXX-XXX*/
        //a.b.cStart-cEnd.dStart-dEnd
        sscanf(ipSegment, "%d.%d.%d-%d.%d-%d", &a, &bStart, &cStart, &cEnd, &dStart, &dEnd);

        levelDebug(INFO_LEV, "[%s][%d]:xxx.xxx.[%d-%d].[%d-%d]\n", \
            __FUNCTION__, __LINE__, cStart, cEnd, dStart, dEnd);

        *pIpNum = (cEnd - cStart + 1) * (dEnd - dStart + 1);

        ipNumInSeg = 0;
        for (int cIdx = 0; cIdx < ((cEnd - cStart + 1)); cIdx++)
        {
            for (int dIdx = 0; dIdx < ((dEnd - dStart + 1)); dIdx++)
            {
                sprintf((char *)&(pIpStrArray[ipNumInSeg].ip), \
                    "%d.%d.%d.%d", a, bStart, cStart + cIdx, dStart + dIdx);

                ipNumInSeg++;
            }
        }

        break;

    case 3:
        /*有3个‘-’--一个动态ip范围(在第2、3、4个点分十进制上)：XXX.XXX-XXX.XXX-XXX.XXX-XXX*/
        //a.bStart-bEnd.cStart-cEnd.dStart-dEnd
        sscanf(ipSegment, "%d.%d-%d.%d-%d.%d-%d", &a, &bStart, &bEnd, &cStart, &cEnd, &dStart, &dEnd);

        levelDebug(INFO_LEV, "[%s][%d]:xxx.[%d-%d].[%d-%d].[%d-%d]\n", \
            __FUNCTION__, __LINE__, bStart, bEnd, cStart, cEnd, \
            dStart, dEnd);

        *pIpNum = (bEnd - bStart + 1) * (cEnd - cStart + 1) * (dEnd - dStart + 1);

        ipNumInSeg = 0;

        for (int bIdx = 0; bIdx < (bEnd - bStart + 1); bIdx++)
        {
            for (int cIdx = 0; cIdx < ((cEnd - cStart + 1)); cIdx++)
            {
                for (int dIdx = 0; dIdx < ((dEnd - dStart + 1)); dIdx++)
                {
                    sprintf((char *)&(pIpStrArray[ipNumInSeg].ip), \
                        "%d.%d.%d.%d", a, bStart + bIdx, cStart + cIdx, dStart + dIdx);

                    ipNumInSeg++;
                }
            }
        }

        break;

    default:
        levelDebug(ERROR_LEV, "[%s][%d]: invalid charCnt[%d]\n", \
            __FUNCTION__, __LINE__, charCnt);
        retVal = -1;
        goto exit;
    }

    levelDebug(INFO_LEV, "[%s][%d]: ipNum[%d]\n\n", \
        __FUNCTION__, __LINE__, \
        *pIpNum);

exit:

    return retVal;
}

/*************************************************
* Function:        findGogoDns()
* Description:     执行命令
* Access Level:    private
* Input:           pCmdName
* Output:          N/A
* Return:          0--成功/-1--失败
*************************************************/
int C_FindGogoDns::findGogoDns(void *pInputParam, const char *pInputBuf, char *pOutputBuf)
{
    levelDebug(INFO_LEV, "[%s][%d]: \n", \
        __FUNCTION__, __LINE__);

    sprintf(pOutputBuf + strlen(pOutputBuf), " ... findGOgoDns cmd\n");

    return 0;
}
