//SQLite에서 제공하는 C함수를 사용하기 위해서 헤더파일을 인클루드한다.
#include <sqlite3.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <time.h>


#define PORTNUM 3800
#define MAXLINE 1024

//서버, 클라이언트 간 약속된 요청 번호다.
#define Q_UPLOAD	1 //업로드
#define Q_DOWNLOAD  2 //다운로드
#define Q_LIST	  3 //목록요청

#define LOWER_BOUNDER  "========================================="
//서버, 클라이언트 간 주고받을 프로토콜을 정의한 구조체이다.
struct Cquery
{
	int command;
	char f_name[64];
};

//중요 코드는 함수로 분리했다.
int callback(void *not, int argc, char **argv, char **ColName);
int proc(int sockfd);
int serv_file_list();
int gettime(char *tstr);

int main(int argc, char **argv)
{
	struct sockaddr_in addr;
	int sockfd, cli_sockfd;
	int clilen;
	int pid;
	char tstr[36];
	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket error");
		return 1;
	}
	printf("Start\n");

	memset(&addr, 0x00, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORTNUM);
	if( bind (sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
	{
		perror("bind error");
		return 1;
	}

	if( listen(sockfd, 5) != 0)
	{
		perror("listen error");
		return 1;
	}
	//이 프로그램은 멀티 프로세스를 이용해서 다수의 클라이언트를 처리한다.
	//좀비 프로세스를 막기 위해 자식 프로세스가 종료할 때 wait 함수 호출이 아닌
	//시그널 처리했다.
	signal(SIGCHLD, SIG_IGN);

	//클라이언트가 연결되면 fork 함수를 이용해서 자식 프로세스를 만든다.
	//이후 클라이언트 요청 처리는 proc 함수를 통해서 이루어진다.
	while(1)
	{
		clilen = sizeof(addr);
		cli_sockfd = accept(sockfd, NULL, &clilen);
		if(cli_sockfd < 0) exit(0);
		pid = fork();
		if(pid == 0)
		{
			proc(cli_sockfd);
			printf("close socket\n");
			sleep(5);
			close(cli_sockfd);
			exit(0);
		}
		else if(pid == -1)
		{
			printf("fork error\n");
			exit(0);
		}
		else
		{
			close(cli_sockfd);
		}
	}
}

int proc(int sockfd)
{
	char buff[MAXLINE];
	struct Cquery query;
	printf("OK Server\n");
	while(1)
	{
		//클라이언트의 요청을 읽어서, 어떤 종류의 서비를 요청하는지를 확인한다.
		if(recv(sockfd, (void *)&query, sizeof(query), 0) <= 0)
		{
			return -1;
		}
		printf("read ok\n");
		query.command = ntohl(query.command);
		//업로드, 다운로드, 파일목록 요청 스위치
		switch(query.command)
		{
			case (Q_UPLOAD):
				serv_file_upload(sockfd, query.f_name);
			break;
			case (Q_DOWNLOAD):
				serv_file_download(sockfd, query.f_name);
			break;
			case (Q_LIST):
				serv_file_list(sockfd);
			break;
		}
		break;
	}
	return 1;
}

int serv_file_upload(int sockfd, char *filename)
{
	int fd;
	int readn;
	int writen;
	char buf[MAXLINE];
	struct sockaddr_in addr;
	int addrlen;
	sqlite3 *db;
	int ret;
	//클라이언트가 전송하는 데이터를 파일 형태로 저장한다.
	//매개변수로 주어진 filename의 파일을 쓰기 전용으로 연다.
	if( (fd = open(filename, O_WRONLY)) == -1)
	{
		return -1;
	}

	memset(buf, 0x00, MAXLINE);
	//소켓에서 읽은 데이터를 그대로 파일로 쓴다.
	//파일로 쓴 다음에는 파일의 정보를 SQLite 에 저장한다.
	while((readn = recv(sockfd, buf, MAXLINE, 0)) > 0)
	{
		writen = write(fd, buf, readn);
		if(writen < 0) break;
		memset(buf, 0x00, MAXLINE);
	}

	ret = sqlite3_open("mydb.db", &db);
	if(ret != 0)
	{
		return -1;
	}
	addrlen = sizeof(addr);
	getpeername(sockfd, (struct sockaddr *)&addr, &addrlen);
	printf("File Upload %s\n", inet_ntoa(addr.sin_addr));

	return 1;
}

int serv_file_download(int sockfd, char *filename)
{
	int fd;
	int readn;
	int sendn;
	char buf[MAXLINE];
	//이름이 filename인 파일을 읽기 전용으로 연다.
	if( (fd = open(filename, O_RDONLY)) == -1)
	{
		return -1;
	}

	memset(buf, 0x00, MAXLINE);
	//파일의 내용을 읽어서 클라이언트에 전송한다.
	while( (readn = read(fd, buf, MAXLINE)) > 0)
	{
		sendn = send(sockfd, buf, readn, 0);
		if(sendn < 0) break;
		memset(buf, 0x00, MAXLINE);
	}
	if (sendn == -1) return -1;

	return 1;
}

int callback(void *farg, int argc, char **argv, char **ColName)
{
	int i;
	int sockfd = *(int *)farg;
	int sendn = 0;
	char buf[MAXLINE];
	//sqlite3_exec 가 쿼리를 성공적으로 실행하면 결과는 argc, argv, ColName 을 통해 넘어간다.
	//argc 는 쿼리 결과로 파일 목록을 저장한 레코드 수가 되낟.
	//argv 는 각 컬럼의 값
	//ColName는 컬럼의 이름이다.
	for(i = 0; i < argc; i++)
	{
		sprintf(buf, "%10s = %s\n", ColName[i], argv[i] ? argv[i] : "NULL");
		sendn = send(sockfd, buf, strlen(buf), 0);
		printf("--> %d\n", sendn);
	}
	sprintf(buf, "%s\n", LOWER_BOUNDER);
	sendn = send(sockfd, buf, strlen(buf), 0);
	printf("--> %d\n", sendn);
	return 0;
}
//현재 시간을 yyyy/mm/dd hh:mm:ss 형식으로 변환한다.
int gettime(char *tstr)
{
	struct tm *tm_ptr;
	time_t the_time;

	time(&the_time);
	tm_ptr = localtime(&the_time);

	sprintf(tstr, "%d/%d/%d %d:%d:00",
		tm_ptr->tm_year + 1900,
		tm_ptr->tm_mon + 1,
		tm_ptr->tm_mday,
		tm_ptr->tm_hour,
		tm_ptr->tm_min
		);
}

int serv_file_list(int sockfd)
{
	char *ErrMsg;
	int ret;
	sqlite3 *db;
	//파일 정보가 들어있는 mydb.db 파일을 연다.
	ret = sqlite3_open("mydb.db", &db);
	if(ret != 0)
	{
		printf("Database Open Error\n");
		return -1;
	}
	printf("Database Open Success\n");
	//파일 정보가 들어있는 file_info 테이블의 내용을 읽기 위해서
	//sqlite3_exec 함수로 select * from file_info 쿼리를 실행한다.
	//callback 함수를 이용해서 쿼리 결과를 처리한다.
	//처리한 결과를 소켓으로 클라이언트에 전달해야 하는데, 이를 위해 sockfd를 넘겼다.
	ret= sqlite3_exec(db,
			"select * from file_info",
			callback, (void *)&sockfd, &ErrMsg);
	if(ret != 0)
	{
		printf("Error %s\n", ErrMsg);
		return 1;
	}
	sqlite3_close(db);
	return 1;
}
