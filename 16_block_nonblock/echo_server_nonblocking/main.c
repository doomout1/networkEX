#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>

#define MAXLINE  1024

int set_nonblock_socket(int fd)
{
	int flags;
	if((flags = fcntl(fd, F_GETFL,0)) == -1)
	{
		perror("fnctl error");
		flags = 0;
	}
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
int main(int argc, char **argv)
{
	int server_sockfd, client_sockfd;
	int client_len, n;
	char buf[MAXLINE];
	struct sockaddr_in clientaddr, serveraddr;
	int client_fd_array[1024];
	int maxfd = 0;
	int i = 0;

	memset((void *)client_fd_array, -1, sizeof(int)*1024);
	client_len = sizeof(clientaddr);
	if ((server_sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error : ");
		exit(0);
	}
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(atoi(argv[1]));

	if(bind (server_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
	{
		perror("Error");
	}
	if(listen(server_sockfd, 5) == -1)
	{

	}
	set_nonblock_socket(server_sockfd);
	maxfd = server_sockfd;
	while(1)
	{
		memset(buf, 0x00, MAXLINE);
		client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr	,
							&client_len);
		if(client_sockfd == -1)
		{
			if(errno == EAGAIN)
			{
				//printf("NON BLOCKING SOCKET : Temporarily unavailable\n");
			}
			else
			{
				printf("Accept Error %d\n", errno);
				continue;
			}
		}
		else
		{
			set_nonblock_socket(client_sockfd);
			client_fd_array[client_sockfd] = 1;
			if(client_sockfd > maxfd)
				maxfd = client_sockfd;
			continue;
		}
		memset(buf, 0x00, sizeof(buf));
		for(i = 0; i < maxfd +1; i++)
		{
			if(client_fd_array[i] != 1) continue;
			client_sockfd = i;
			n = read(client_sockfd, buf, MAXLINE);
			if(n == -1)
			{
				if(errno == EAGAIN)
				{
					//printf("NON BLOCKING SOCKET : Temporarily unavailable\n");
				}
				else
				{
					printf("read Error %d\n", errno);
					close(client_sockfd);
					client_fd_array[client_sockfd] = -1;
					break;
				}
			}
			else if(n == 0)
			{
				printf("close %d\n", errno);
					close(client_sockfd);
				client_fd_array[client_sockfd] = -1;
				break;
			}
			else
			{
				printf("Read Data %s", buf);
				write(client_sockfd, buf, MAXLINE);
			}
		}
	}
}

