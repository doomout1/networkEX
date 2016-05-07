#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE  1024

int client_sockfd;
void urg_handler(int signo)
{
	char buf;
	recv(client_sockfd, (void *)&buf, 1, MSG_OOB);
	printf("OOB DATA %c\n", buf);
}

int main(int argc, char **argv)
{
	int listenfd;
	int readn;
	socklen_t client_len;
	char buf[MAXLINE];
	struct sockaddr_in clientaddr, serveraddr;

	if(argc != 2)
	{
		printf("Usage : %s [port num]\n", argv[0]);
		return 1;
	}
	client_len = sizeof(clientaddr);

	if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error : ");
		return 1;
	}
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(atoi(argv[1]));

	bind (listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	listen(listenfd, 5);

	while(1)
	{
		memset(buf, 0x00, MAXLINE);
		client_sockfd = accept(listenfd, (struct sockaddr *)&clientaddr,
							&client_len);
		if(client_sockfd == -1 )
		{
			continue;
		}
		signal(SIGURG, urg_handler);
		fcntl(client_sockfd, F_SETOWN, getpid());
		while(1)
		{
			if ((readn = read(client_sockfd, buf, MAXLINE)) <= 0)
			{
				close(client_sockfd);
				continue;
			}
			if (write(client_sockfd, buf, strlen(buf)) <=0)
			{
				perror("write error : ");
				close(client_sockfd);
			}
		}
		close(client_sockfd);
	}
}

