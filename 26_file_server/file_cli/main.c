#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include <unistd.h>
#include <getopt.h>

#define PORTNUM 3800
#define MAXLINE 1024

#define Q_UPLOAD	1
#define Q_DOWNLOAD  2
#define Q_LIST	  3
//클라이언트가 요청에 사용할 구조체를 정의했다.
//이 구조체는 서버와 클라이언트이 약속이므로 서로 다르면 안된다.
//원칙적으로는 서버와 클라이언트가 함께 사용하는 이런 종류의 구조체는 별도의 헤더파일로 관리해야 한다.
struct Cquery
{
	int command;
	char fname[64];
};
//클라이언트이 도움말을 포함한다.
//프로그램의 실행인자를 잘못 사용했거나 -h 옵션을 주면 이 도움말이 출력된다.
void help(char *progname)
{
	printf("Usage : %s -h -i [ip] -u [upload filename] -d [download filename] -l\n", progname);
}

int main(int argc, char **argv)
{
	struct sockaddr_in addr;
	int sockfd;
	int clilen;
	int opt;
	int optflag=0;
	char ipaddr[36]={0x00,};
	int command_type=0;
	char fname[80]={0x00,};
	char buf[MAXLINE];
//프로그램 실행인자를 이용해서 프로그램의 실행방식을 결정한다.
//getopt 함수를 이용하면 쉽게 프로그램 실행인자를 처리할 수 있다.
	while( (opt = getopt(argc, argv, "hli:u:d:")) != -1)
	{
		switch(opt)
		{
			case 'h':
				help(argv[0]);
				return 1;

			case 'i':
				sprintf(ipaddr, "%s", optarg);
				break;

			case 'u':
				command_type = Q_UPLOAD;
				sprintf(fname, "%s", optarg);
				optflag = 1;
				break;

			case 'd':
				command_type = Q_DOWNLOAD;
				sprintf(fname, "%s", optarg);
				optflag = 1;
				break;

			case 'l':
				command_type = Q_LIST;
				break;

			default:
				help(argv[0]);
				return 1;
		}
	}

	if(ipaddr[0] == '\0')
	{
		printf ("ip address not setting\n");
		return 0;
	}

	if((fname[0] == '\0') && (optflag == 1))
	{
		printf ("fname error\n");
		return 0;
	}

	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("socket error");
		return 1;
	}

	memset((void *)&addr, 0x00, sizeof(addr));
	addr.sin_family  = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ipaddr);
	addr.sin_port = htons(PORTNUM);

	clilen = sizeof(addr);
	if(connect(sockfd, (struct sockaddr *)&addr, clilen) < 0)
	{
		perror("connect error:");
		return 0;
	}
	//클라이언트 요청에 종류에 따른 함수를 호출한다.
	while(1)
	{
		switch(command_type)
		{
			case (Q_LIST):
				get_list(sockfd);
				break;
			break;
			case (Q_DOWNLOAD):
				download(sockfd, fname);
			break;
			case (Q_UPLOAD):
				upload(sockfd, fname);
			break;
			default:
				printf("Unknown command\n");
			break;
		}
		break;
	}
	close(sockfd);
}

int download(int sockfd, char *file)
{
	struct Cquery query;
	int fd;
	int readn, writen;
	char buf[MAXLINE];
	//file 을 이름으로 하는 파일을 쓰기 전용으로 연다
	if( (fd = open(file, O_WRONLY)) == -1 )
	{
		return -1;
	}
	memset(buf, 0x00, MAXLINE);
	memset(&query, 0x00, sizeof(query));

	//사용자 요청을 전달하기 위해서 query 구조체를 채운다.
	query.command = htonl(Q_DOWNLOAD);
	sprintf(query.fname, "%s", file);

	if(send(sockfd, (void *)&query, sizeof(query), 0) <=0)
	{
		return -1;
	}
	//서버로부터 읽은 데이터를 파일로 저장한다.
	while((readn = recv(sockfd, buf, MAXLINE, 0)) > 0)
	{
		writen = write(fd, buf, readn);
		if(writen != readn)
		{
			return -1;
		}
		memset(buf, 0x00, MAXLINE);
	}
	close(fd);
	return 1;
}

int get_list(int sockfd)
{
	struct Cquery query;
	char buf[MAXLINE];
	int len;

	//서버로 요청을 전달한다.
	memset(&query, 0x00, sizeof(query));
	query.command = htonl(Q_LIST);

	if(send(sockfd, (void *)&query, sizeof(query), 0) <=0 )
	{
		perror("Send Error\n");
		return -1;
	}
	memset(buf, 0x00, MAXLINE);
	//서버로 부터 읽은 데이터를 출력한다.
	while(1)
	{
		len = recv(sockfd, buf, MAXLINE, 0);
		if(len <= 0) break;
		printf("%s", buf);
		memset(buf, 0x00, MAXLINE);
	}
	printf("End!\n");
}

int upload(int sockfd, char *file)
{
	struct Cquery query;
	int fd;
	int readn;
	int sendn;
	char buf[MAXLINE];
	//업로드 할 파일을 읽기 전용으로 연다.
	if( (fd = open(file, O_RDONLY)) == -1 )
	{
		return -1;
	}
	//서버에 업로드 요청을 한다.
	memset(&query, 0x00, sizeof(query));
	query.command = htonl(Q_UPLOAD);
	sprintf(query.fname, "%s", file);

	if(send(sockfd, (void *)&query, sizeof(query), 0) <=0)
	{
		return -1;
	}
	//파일에서 읽은 내용을 서버에 전송한다.
	while((readn = read(fd, buf, MAXLINE)) > 0)
	{
		sendn = send(sockfd, buf, readn, 0);
		if(sendn != readn)
		{
			printf("Upload Error\n");
			return -1;
		}
	}
	close(fd);
	return 1;
}
