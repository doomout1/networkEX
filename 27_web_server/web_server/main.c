#include <arpa/inet.h>
#include <stdio.h>
#include <signal.h>

#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define PORTNUM 80 //80포트 사용. 시스템 포트이므로 루트 권한으로 실행해야 한다.
#define MAXLINE 1024
#define HEADERSIZE 1024 //응답 메시지의 헤더 정보 크기

#define SERVER "myserver/1.0 (Linux)" //서버 프로그램의 이름과 버전. 이정보는 웹로봇에서 브라우저 방문 통계등을 작성할 때 사용된다.

struct user_request //사용자 요청을 저장하기 위한 구조체
{
	char method[20];   // 요청 방법
	char page[255];    // 페이지 이름
	char http_ver[80]; // HTTP 프로토콜 버전
};

char root[MAXLINE]; //웹문서 파일이 존재하는 루트 디렉터리다.

int webserv(int sockfd); //자식 프로세스가 실행하며, 클라이언트의 요청을 분석해서 응답한다.
int protocol_parser(char *str, struct user_request *request); //클라이언트의 요청을 분석해서 user_request 구조체에 넣는다.
int sendpage(int sockfd, char *filename, char *http_ver, char *codemsg); //sendpage 함수는 클라이언트에 요청한 웹페이지를 전송한다.


int main(int argc, char **argv)
{
	int listenfd;
	int clientfd;
	socklen_t clilen;
	int pid;
	int optval = 1;
	struct sockaddr_in addr, cliaddr;

	if(argc !=2 )
	{
		printf("Usage : %s [root directory]\n", argv[0]);
		return 1;
	}
	memset(root, 0x00, MAXLINE);
	sprintf(root, "%s", argv[1]);

	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		return 1;
	}
//SO_REUSEADDR 옵션으로 소켓을 재사용하도록 했다.
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	memset(&addr, 0x00, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORTNUM);

	if(bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		return 1;
	}
	if(listen(listenfd, 5) == -1)
	{
		return 1;
	}

	signal(SIGCHLD, SIG_IGN);
	while(1)
	{
		clilen = sizeof(clilen);
		clientfd = accept(listenfd , (struct sockaddr *)&cliaddr, &clilen);
		if(clientfd == -1)
		{
			return 1;
		}
		pid = fork();
		//자식 프로세스는 webserv 함수를 호출해서 클라이언트 요청을 처리한다.
		if(pid == 0)
		{
			webserv(clientfd);
			close(clientfd);
			exit(0);
		}
		if(pid == -1)
		{
			return 1;
		}
		close(clientfd);
	}
}
//자식 프로세스에서 호출하는 함수로, 매개변수로 연결 소켓이 넘어간다.
//이 연결 소켓을 이용해서 클라이언트의 요청을 읽고 처리한 결과를 전송한다.
int webserv(int sockfd)
{
	char buf[MAXLINE];
	char page[MAXLINE];
	struct user_request request;

	memset(&request, 0x00, sizeof(request));
	memset(buf, 0x00, MAXLINE);
	//클라이언트이 요청을 읽는다.
	if(read(sockfd, buf, MAXLINE) <= 0)
	{
		return -1;
	}
	//protocol_parser를 이용해서 클라이언트 요청을 분석한다. 분석한 정보는 user_request 에 저장된다.
	protocol_parser(buf, &request);

	//GET 요청이라면, 클라이언트가 요청한 페이지를 찾아서 읽은 다음 소켓으로 전송한다.
	//access 함수는 파일이 상태를 점검한다. R_OK는 읽을 수 있는 파일인지 검사하라는 의미다.
	//만약 파일이 존재하지 않거나 하는 등의 이유로 읽을 수 없다면 '404 Not Found' 에러를,
	//읽을 파일이 존재한다면 '200 OK' 메시지와 함께 파일의 내용을 클라이언트로 전송한다.
	if(strcmp(request.method, "GET") == 0)
	{
		sprintf(page, "%s%s", root, request.page);
		if(access(page, R_OK) != 0)
		{
			sendpage(sockfd, NULL, request.http_ver, "404 Not Found");
		}
		else
		{
			sendpage(sockfd, page, request.http_ver, "200 OK");
		}
	}
	else //GET 이외의 요청에는 '500 Internal Server Error'를 전송한다.
	{
		sendpage(sockfd, NULL, request.http_ver, "500 Internal Server Error");
	}
	return 1;
}

int sendpage(int sockfd, char *filename, char *http_ver, char *codemsg)
{
	struct tm *tm_ptr;
	time_t the_time;
	struct stat fstat;
	char header[HEADERSIZE];
	int fd;
	int readn;
	int content_length=0;
	char buf[MAXLINE];
	char date_str[80];
	//time 함수로 알아낸 유닉스 시간을 문자열로 변환해줘야 한다. 월과 요일을 문자열로 변환해주기 위한 테이블이다.
	char *daytable = "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat\0";
	char *montable = "Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec\0";

	memset(header, 0x00, HEADERSIZE);

	//time 함수로 현재 시간을 얻어오고 localtime 함수를 이용해서 시간 구조체인 tm 구조체에 담는다.
	time(&the_time);
	tm_ptr = localtime(&the_time);

	sprintf(date_str, "%s, %d %s %d %02d:%02d:%02d GMT",
			daytable+(tm_ptr->tm_wday*4),
			tm_ptr->tm_mday,
			montable+((tm_ptr->tm_mon)*4),
			tm_ptr->tm_year+1900,
			tm_ptr->tm_hour,
			tm_ptr->tm_min,
			tm_ptr->tm_sec
			);
	//파일 이름이 NULL 이 아니라면 파일의 크기를 가져온다. 이 값은 HTTP 헤더에서 문서의 크기를 의미하는 Content-Length 설정에 쓰인다.
	//브라우저에 따라서 이 크기와 실제 문서의 크기가 일치하지 않으면 에러를 발생하기도 한다.
	//파일이 존재하지않으면 문서 본문으로 codemsg 를 그대로 전송할 것이다.
	//그러므로 codemsg 의 길이를 content_length 로 지정해야 한다.
	if(filename != NULL)
	{
		stat(filename, &fstat);
		content_length = (int)fstat.st_size;
	}
	else
	{
		content_length = strlen(codemsg);
	}
	//HTTP 헤더를 만든다.
	sprintf(header, "%s %s\nDate: %s\nServer: %s\nContent-Length: %d\nConnection: close\nContent-Type: text/html; charset=UTF8\n\n",
		http_ver, date_str, codemsg, SERVER, content_length);
	//먼저 헤더를 보낸다.
	write(sockfd, header, strlen(header));
	//파일의 내용을 읽어서 클라이언트로 전송한다.
	if(filename != NULL)
	{
		fd = open(filename, O_RDONLY);
		memset(buf, 0x00, MAXLINE);
		while((readn = read(fd,buf,MAXLINE)) > 0)
		{
			write(sockfd, buf, readn);
		}
		close(fd);
	}
	//만약 파일의 내용이 존재하지 않을 경우 에러 메시지를 전송한다.
	else
	{
		write(sockfd, codemsg, strlen(codemsg));
	}

	return 1;
}
//protocol_parser 함수는 클라이언트 요청을 분석해서 user_request 구조체에 저장한다.
int protocol_parser(char *str, struct user_request *request)
{
	char *tr;
	char token[] = " \r\n";
	int i;
	tr = strtok(str, token);
	for (i = 0; i < 3; i++)
	{
		if(tr == NULL) break;
		if(i == 0)
			strcpy(request->method, tr);
		else if(i == 1)
			strcpy(request->page, tr);
		else if(i == 2)
			strcpy(request->http_ver, tr);
		tr = strtok(NULL, token);
	}
	return 1;
}
