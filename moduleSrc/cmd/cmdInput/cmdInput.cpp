/**************************************************************************************
*  Filename:               cmdInput.cpp
*  Description:            ����������ʵ���ļ�
*  Author:                 Leo
*  Create:                 2016-07-31
*  Modification history:
*
**************************************************************************************/
//TODO:stop���ֹͣ������cmdDispatch����Ӧ

#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>  
#include <string.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>

int main(void)//��ڲο�busybox
{
    //create socket  
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
    {
        perror("socket");
        exit(-1);
    }
    printf("create socket OK!\n");

    //create an send address  
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6666);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //send the message to the specify address  
    int r;
    char buf[255];
    while (1)
    {
        r = read(0, buf, sizeof(buf) - 1);
        if (r <= 0)
            break;
        sendto(fd, buf, r, 0, (struct sockaddr*)&addr, sizeof(addr));
    }

    //close socket  
    close(fd);
    return 0;

}