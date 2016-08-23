/**************************************************************************************
*  Filename:               common.h
*  Description:            公共头文件
*  Author:                 Leo
*  Create:                 2016-06-25
*  Modification history:
*
**************************************************************************************/
#ifndef _COMMON_H
#define _COMMON_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h> //sleep()
#include <assert.h> //assert()
#include <string.h> 
#include <errno.h>
#include <stdarg.h> //va_list
#include <math.h>
#include <unistd.h>


#include "../debug/debug.h"
#include "../framework/list/lstLib.h"

#define     STD_STR_SIZE    256     //普通字符串长度 256B

#define     STD_1K          (1024)
#define     STD_1M          (STD_1K * STD_1K)
#define     STD_1G          (STD_1M * STD_1K)

#define     min(a, b)        (((a) < (b)) ? (a) : (b))
#define     max(a, b)        (((a) > (b)) ? (a) : (b))

typedef void *(*START_ROUTINE)(void *);


#endif