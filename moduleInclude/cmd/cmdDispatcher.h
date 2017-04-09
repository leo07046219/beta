/**************************************************************************************
*  Filename:               cmdDispatcher.h
*  Description:            �����е�����ͷ�ļ�
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

#define MAX_INPUT_NUM                   2                   //������������� xxx 1 ,  xxx 1,1
#define CMD_INPUT_INTERACT_PORT         6543                //�����˿�
#define MAX_INPUT_CMD_STR_LEN           128                 //��������
#define MAX_INPUT_PARAM_STR_LEN         128                 //����������
#define RECEIVE_BUF_SIZE                (1 * 1024)          //���ͻ�������С
#define SEND_BUF_SIZE                   (10 * 1024 * 1024)  //���ջ�������С10M
#define MAX_SERVER_LINTEN_CONNECTIONS   5                   //���ͬʱ����������
#define INPUT_CMD_FLODER                "/sbin/inputCmd"    //inputCmd�������ļ�����·��
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
    * Description:  ��ʼ�������������
    * Access Level: public
    * Input:        N/A
    * Output:       N/A
    * Return:       0---�ɹ�/-1---ʧ��
    ***********************************************************/
    int initCmdDispatcher(void);

    /*************************************************
    * Function:       getCmdDispatcherPtr()
    * Description:    ��ȡC_CmdDispatcherΨһ�����ַ
    * Access Level:   public
    * Input:          N/A
    * Output:         C_CmdDispatcherΨһ�����ַ
    * Return:         N/A
    *************************************************/
    static C_CmdDispatcher *getCmdDispatcherPtr(void);

    int regInputCmd(const char *pCmdName, INPUT_CALL_BACK_FUNC inputCallBackFunc, void *pInputParam);

private:
    C_CmdDispatcher();    //���캯��˽�л�

    void waitModuleInited(void);
    /*TODO: ȷ���Ƿ��б�Ҫ��pthread_mutexattr_init����ʼ��mutex*/
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

/*�����ṩ����ʵ��ָ���ȡ�ӿ�,����ʹ��ʱҪ��������*/
C_CmdDispatcher *getCmdDispatcherObjPtr(void);

#ifdef  __cplusplus
}
#endif

#endif