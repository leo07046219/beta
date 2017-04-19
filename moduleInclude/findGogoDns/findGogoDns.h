/**************************************************************************************
*  Filename:               findGogoDns.h
*  Description:            gogoDns������ͷ�ļ�
*  Author:                 Leo
*  Create:                 2016-06-13
*  Modification history:
*
**************************************************************************************/
#ifndef _FIND_GOGO_DNS_H
#define _FIND_GOGO_DNS_H

#include "common.h"

#ifdef  __cplusplus
extern "C"
{
#endif

#define     IP_SEGMENT_STR_LEN  28      //XXX.XXX-XXX.XXX-XXX.XXX-XXX\n
#define     IP_V4_STR_LEN       16      //"XXX.XXX.XXX.XXX\n"
#define     INNER_IP_SET_FILE   "/home/leo/alpha/findGogoDns/innerIpSet.txt"
#define     GOGO_IP_SET_FILE    "/home/leo/alpha/findGogoDns/gogoIpSet.txt"
#define     GOGO_CFG_FILE       "/home/leo/alpha/findGogoDns/gogoCfg.txt"
#define     MAX_IP_NUM_IN_A_SEGMENT     (256 * 256 * 10) //һ��IP����ຬ�е�ip����
#define     TEST_IP_STR         "115.239.211.112"//111.92.162.4-59
#define     CURL_ACCESS_TIMEOUT 1       //curl����ip�ĳ�ʱʱ��---��
#define     REQUEST_LEN         256     //curl������ַ��������С
#define     GOGO_WEB_SITE_STR   "www.google.com"

#define     MAX_CHECK_IP_THREAD_NUMBER  50          //��curl����ip�Ĳ�����Ŀ
#define     MAX_GOGO_IP_NUM     100          //ά�ֵ������Чip��Ŀ
#define     LINE_BUF_SIZE       30          //д��gogoTxtÿ�еĻ���������
/*ip��ַ�ṹ*/
typedef struct  
{
    char ip[IP_V4_STR_LEN];
}IP_STR, *IP_STR_PTR;

typedef struct
{
    IP_STR  ipStr;          //ip��
    float   respondTime;    //��Ӧʱ��--����
}GOGO_IP, *GOGO_IP_PTR;

#if 0
typedef struct  
{
    int             validIpNum;
    GOGO_IP_PTR     head;
    GOGO_IP_PTR     tail;
}GOGO_IP_SET;
#endif

typedef struct
{
    int             validIpNum;
    GOGO_IP         gogoIp[MAX_GOGO_IP_NUM];
}GOGO_IP_SET, *GOGO_IP_SET_PTR;

typedef struct
{
    void        *pUsr;
    int         startIpIdx;
    int         endIpIdx;
    IP_STR_PTR  pIpStrBuf;
}CHECK_IP_INFO, *CHECK_IP_INFO_PTR;

typedef struct  
{
    void            *pUsr;
    CHECK_IP_INFO   checkIpInfo;
}CHECK_IP_TASK_INFO, *CHECK_IP_TASK_INFO_PTR;

typedef struct
{
    GOGO_IP_SET     ipSet;
}MODULE_PARAM, *MODULE_PARAM_PTR;

class C_FindGogoDns
{
public:
    ~C_FindGogoDns();

    /***********************************************************
    * Function:     initFindGogoDns()
    * Description:  ��ʼ��gogo dns������
    * Access Level: public
    * Input:        N/A
    * Output:       N/A
    * Return:       0---�ɹ�/-1---ʧ��
    ***********************************************************/
    int initFindGogoDns(void);

    /*************************************************
    * Function:       getFindGogoDnsPtr()
    * Description:    ��ȡC_FindGogoDnsΨһ�����ַ
    * Access Level:   public
    * Input:          N/A
    * Output:         C_FindGogoDnsΨһ�����ַ
    * Return:         N/A
    *************************************************/
    static C_FindGogoDns *getFindGogoDnsPtr(void);

private:
    C_FindGogoDns();    //���캯��˽�л�

    void waitModuleInited(void);
    /*TODO: ȷ���Ƿ��б�Ҫ��pthread_mutexattr_init����ʼ��mutex*/ 
    static pthread_mutex_t  m_singleMutex;
    pthread_mutex_t     m_paramMutex;

    bool                m_bModuleInited;
    pthread_mutex_t     m_initMutex;
    pthread_cond_t      m_initCond;
    
    pthread_mutex_t     m_gogoMutex;


    static C_FindGogoDns *ms_findGogoDnsObjPtr;

    static void findGogoDnsTaskShell(void* pThis);
    void        findGogoDnsTask(void);

    int         startFindGogoDnsTask(void);
    int         checkIpFromIpSet(void);
    int         recordIp(const char * pIpStr, const float delayTime);
    int         checkIp(const char *pIpStr);

    static void checkIpTaskFuncTaskShell(CHECK_IP_TASK_INFO_PTR pCheckIpTaskInfo);
    int         checkIpTaskFunc(CHECK_IP_INFO_PTR pCheckIpInfo);

    static int  findGogoDns(void *pInputParam, const char *pInputBuf, char *pOutputBuf);

    int         multiThreadGetIp(int ipNumInSeg, IP_STR_PTR pIpStrBuf);
    int         resumeIpSeg(char * pIpStr);
    int         storeIpSeg(char * pIpStr);
    int         getIp(char *pIpStr);
    int         getIpFromIpSegment(const char *ipSegment, int *pIpNum, IP_STR_PTR pIpStrArray);
};

/*�����ṩ����ʵ��ָ���ȡ�ӿ�,����ʹ��ʱҪ��������*/
C_FindGogoDns *getFindGogoDnsObjPtr(void);

#ifdef  __cplusplus
}
#endif

#endif