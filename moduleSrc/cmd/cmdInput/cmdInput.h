/**************************************************************************************
*  Filename:               cmdInput.h
*  Description:            ����������ͷ�ļ�
*  Author:                 Leo
*  Create:                 2016-07-31
*  Modification history:
*
**************************************************************************************/
#ifndef _CMD_INPUT_H
#define _CMD_INPUT_H

#ifdef  __cplusplus
extern "C"
{
#endif

#define MAX_INPUT_NUM               2       //������������� xxx 1 ,  xxx 1,1
#define CMD_INPUT_INTERACT_PORT     6543    //�����˿�
#define MAX_INPUT_CMD_STR_LEN       128     //��������
#define MAX_INPUT_PARAM_STR_LEN     128     //����������
#define SEND_BUF_SIZE               (1 * 1024)  //���ͻ�������С
#define RECEIVE_BUF_SIZE            (10 * 1024 * 1024)  //���ջ�������С

typedef struct
{
    char inputCmdStr[MAX_INPUT_CMD_STR_LEN];
    char inputParamStr[MAX_INPUT_PARAM_STR_LEN];
}INPUT_CMD, *INPUT_CMD_PTR;

#ifdef  __cplusplus
}
#endif

#endif
