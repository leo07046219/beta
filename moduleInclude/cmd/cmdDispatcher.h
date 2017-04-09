/**************************************************************************************
*  Filename:               cmdDispatcher.h
*  Description:            命令行调度器头文件
*  Author:                 Leo
*  Create:                 2016-07-31
*  Modification history:
*
**************************************************************************************/
#ifndef _CMD_DISPATCHER_H
#define _CMD_DISPATCHER_H

#ifdef  __cplusplus
extern "C"
{
#endif

#define MAX_INPUT_NUM                   2                   //输入参数个数， xxx 1 ,  xxx 1,1
#define CMD_INPUT_INTERACT_PORT         6543                //交互端口
#define MAX_INPUT_CMD_STR_LEN           128                 //最大命令长度
#define MAX_INPUT_PARAM_STR_LEN         128                 //最大参数长度
#define RECEIVE_BUF_SIZE                (1 * 1024)          //发送缓冲区大小
#define SEND_BUF_SIZE                   (10 * 1024 * 1024)  //接收缓冲区大小10M
#define MAX_SERVER_LINTEN_CONNECTIONS   5                   //最大同时连接请求数
#define INPUT_CMD_FLODER                "/sbin/inputCmd"    //inputCmd二进制文件绝对路径
typedef struct
{
    char inputCmdStr[MAX_INPUT_CMD_STR_LEN];
    char inputParamStr[MAX_INPUT_PARAM_STR_LEN];
}INPUT_CMD, *INPUT_CMD_PTR;

typedef int (*INPUT_CALL_BACK_FUNC)(void *pInputParam, const char *pInputBuf, char *pOutputBuf);

typedef struct
{
    NODE                    node;
    char                    cmdName[MAX_INPUT_CMD_STR_LEN];
    void                    *inputParam;
    INPUT_CALL_BACK_FUNC    inputCbFun;
}CMD_NODE, *CMD_NODE_PTR;

typedef struct
{
    LIST cmdNodeList;
}CMD_DISPATCHER_PARAM, *CMD_DISPATCHER_PARAM_PTR;

class C_CmdDispatcher
{
public:
    ~C_CmdDispatcher();

    /***********************************************************
    * Function:     initCmdDispatcher()
    * Description:  初始化命令调度器类
    * Access Level: public
    * Input:        N/A
    * Output:       N/A
    * Return:       0---成功/-1---失败
    ***********************************************************/
    int initCmdDispatcher(void);

    /*************************************************
    * Function:       getCmdDispatcherPtr()
    * Description:    获取C_CmdDispatcher唯一对象地址
    * Access Level:   public
    * Input:          N/A
    * Output:         C_CmdDispatcher唯一对象地址
    * Return:         N/A
    *************************************************/
    static C_CmdDispatcher *getCmdDispatcherPtr(void);

    int regInputCmd(const char *pCmdName, INPUT_CALL_BACK_FUNC inputCallBackFunc, void *pInputParam);

private:
    C_CmdDispatcher();    //构造函数私有化

    void waitModuleInited(void);
    /*TODO: 确认是否有必要用pthread_mutexattr_init来初始化mutex*/
    static pthread_mutex_t  m_singleMutex;
    pthread_mutex_t     m_paramMutex;

    bool                m_bModuleInited;
    pthread_mutex_t     m_initMutex;
    pthread_cond_t      m_initCond;

    static C_CmdDispatcher *ms_cmdDispatcherObjPtr;

    static void cmdDispatcherTaskShell(void* pThis);
    void        cmdDispatcherTask(void);

    int         startCmdDispatcherTask(void);

    bool isCmdExist(const char *pCmdName);
    int matchCmd(const char *pCmdName, CMD_NODE_PTR pMatchedCmdNode);
    int executeCmd(CMD_NODE_PTR pCmdNode, const char *pInputBuf, char *pOutputBuf);

    static int showInputCmdList(void *pInputParam, const char *pInputBuf, char *pOutputBuf);

    CMD_DISPATCHER_PARAM    m_cmdDispatcherParam;
    char        *m_pReceiveBuf;
    char        *m_pSendBuf;

};

/*对外提供对象实例指针获取接口,否则使用时要加上类域*/
C_CmdDispatcher *getCmdDispatcherObjPtr(void);

#ifdef  __cplusplus
}
#endif

#endif