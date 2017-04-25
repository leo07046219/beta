#include "hello.h"
#include "stdio.h"

void hello(const char *pNameStr)
{
	if(NULL == pNameStr)
	{
		fprintf(stderr, "[%s][%d]:pNameStr is null!\n", __FUNCTION__, __LINE__);
		return;
	}

	printf("[%s][%d]::HELLO::[%s]\n", __FUNCTION__, __LINE__, pNameStr);

	return;
}
