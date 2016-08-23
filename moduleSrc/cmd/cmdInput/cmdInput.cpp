/**************************************************************************************
*  Filename:               cmdInput.cpp
*  Description:            ����������ʵ���ļ�
*  Author:                 Leo
*  Create:                 2016-07-31
*  Modification history:
*
**************************************************************************************/
/*TODO:
1.stop���ֹͣ������cmdDispatch����Ӧ
2.Ƶ�����ô��ڴ���time_wait -- inputCmd����Ƶ�����ã���ʱ���޸�
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cmdInput.h"

int main(int argc, char *argv[])//��ڲο�busybox�������ӻ���
{
    int     r;
    char    buf[255];
    int     fd = -1;
    char    sendBuf[SEND_BUF_SIZE];
    char    receiveBuf[SEND_BUF_SIZE];
    struct sockaddr_in addr;
    char    *pCmd = NULL;
    struct linger clientLinger;
    int recvbytes = 0;

    memset(&(clientLinger), 0, sizeof(clientLinger));
    memset(buf, 0, sizeof(buf));
    memset(sendBuf, 0, sizeof(sendBuf));
    memset(receiveBuf, 0, sizeof(receiveBuf));
    memset(&(addr), 0, sizeof(addr));
   
    fprintf(stderr, "\n[%s][%d]:argc[%d],\nargv[0]=[%s],\nargv[1]=[%s]\n\n", \
        __FUNCTION__, __LINE__, \
        argc, argv[0], argv[1]);

    if (argc != 2)
    {
        fprintf(stderr, "[%s][%d]:invalid input num:argc[%d]\n", \
            __FUNCTION__, __LINE__, argc);
        exit(-1);
    }

    //create socket  
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        exit(-1);
    }
    //printf("create socket OK!\n");

    //create an send address  
    addr.sin_family = AF_INET;
    addr.sin_port = htons(CMD_INPUT_INTERACT_PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bzero(&(addr.sin_zero), 8);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1) 
    {
        perror("connect:");
        goto closeConnect;
    }

    //��װcmd����
    pCmd = strchr(argv[0], '/');//���ݡ�./cmd������
    if(pCmd != NULL)
    {
    	pCmd = pCmd + 1;
    }
    else
    {
    	pCmd = argv[0];
    }
    memset(sendBuf, 0, sizeof(sendBuf));
    memcpy(sendBuf, pCmd, strlen(pCmd));
    memcpy(sendBuf + MAX_INPUT_CMD_STR_LEN, argv[1], strlen(argv[1]));
        
    //����cmd�����cmdDispatcher
    if (send(fd, sendBuf, (MAX_INPUT_CMD_STR_LEN + MAX_INPUT_PARAM_STR_LEN), 0) == -1)
    {
        perror("send to client error :��");
        goto closeConnect;
    }

    //����cmdDispatcher����    
    if ((recvbytes = recv(fd, buf, SEND_BUF_SIZE, 0)) <= 0) 
    {
        perror("recv error:");
        goto closeConnect;
    }
    buf[recvbytes] = '\0';
    printf("Received: %s", buf);


    //send the message to the specify address

#if 0
    while (1)
    {
        r = read(0, buf, sizeof(buf) - 1);
        if (r <= 0)
            break;
        sendto(fd, buf, r, 0, (struct sockaddr*)&addr, sizeof(addr));
    }
#endif



    //sendto(fd, sendBuf, (MAX_INPUT_CMD_STR_LEN + MAX_INPUT_PARAM_STR_LEN), 0, (struct sockaddr*)&addr, sizeof(addr));

    //receive data from inputDispatcher

closeConnect:

    clientLinger.l_onoff = 1;		    /* ǿ�ƹر�sockt����*/
    clientLinger.l_linger = 0;	/* lingerSecs---�ӳ�ʱ��*/
    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *)&clientLinger, sizeof(clientLinger)) == -1)
    {
        perror("setsockopt failed : ");
        return -1;
    }

    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }

    return 0;

}