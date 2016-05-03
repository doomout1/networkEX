#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#define MAXBUF 1024
int main(int argc, char **argv)
{
	int server_sockfd, client_sockfd;
	int client_len, n;
	char buf[MAXBUF];
	struct sockaddr_in client_addr, server_addr;
	client_len = sizeof(client_addr);
	if ((server_sockfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP )) == -1)
	{
		perror("socket error : ");
		return 1;
	}
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(3600);

	bind (server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	listen(server_sockfd, 5);

	while(1)
	{
		memset(buf, 0x00, MAXBUF);
		client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_len);
		printf("New Client Connect: %s\n", inet_ntoa(client_addr.sin_addr));
		if ((n = read(client_sockfd, buf, MAXBUF)) <= 0)
		{
			close(client_sockfd);
			continue;
		}
		if (write(client_sockfd, buf, MAXBUF) <=0)
		{
			perror("write error : ");
			close(client_sockfd);
		}
		close(client_sockfd);
	}
	close(server_sockfd);
	return 0;
}
