#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define SOCK 0
#define MAXLINE 1024
int main(int argc, char **argv)
{
	char buf[MAXLINE];
	struct sockaddr addr;
	socklen_t addrlen;

	addrlen = sizeof(addr);
	if(getpeername(0, &addr, &addrlen) < 0)
	{
		printf("Socket Open Failure!!!\n");
		return 1;
	}

	while(1)
	{
		memset(buf, 0x00, MAXLINE);
		if(read(SOCK, buf, MAXLINE-1) <=0)
		{
			return 1;
		}
		if(write(SOCK, buf, strlen(buf)) <=0 )
		{
			return 1;
		}
	}
	return 0;
}
