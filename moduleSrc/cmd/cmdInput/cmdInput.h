/**************************************************************************************
*  Filename:               cmdInput.h
*  Description:            命令行输入头文件
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

#define MAX_INPUT_NUM               2       //输入参数个数， xxx 1 ,  xxx 1,1
#define CMD_INPUT_INTERACT_PORT     6543    //交互端口
#define MAX_INPUT_CMD_STR_LEN       128     //最大命令长度
#define MAX_INPUT_PARAM_STR_LEN     128     //最大参数长度
#define SEND_BUF_SIZE               (1 * 1024)  //发送缓冲区大小
#define RECEIVE_BUF_SIZE            (10 * 1024 * 1024)  //接收缓冲区大小

typedef struct
{
    char inputCmdStr[MAX_INPUT_CMD_STR_LEN];
    char inputParamStr[MAX_INPUT_PARAM_STR_LEN];
}INPUT_CMD, *INPUT_CMD_PTR;

#ifdef  __cplusplus
}
#endif

#endif
