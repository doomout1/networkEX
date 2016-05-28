#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <string.h>
#include <stdio.h>

#define PORT_NUM 3600
#define MAXLINE 1024

int user_fds[1024];

//매개변수로 주어진 파일 지정번호 fd르 ㄹ비동기 입출력 모드로 변경하고,
//입출력이 있을 때 SIGRTMIN 시그널을 발생하도록 한다.
//F_SETOWN은 시그널을 받을 프로세스를 지정하기 위해서 사용한다.
//시그널은 자기 프로세스에게 전달되어야 하므로 getid함수로 자신을 지정했다.
int setup_sigio(int fd)
{
	if(fcntl(fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC) < 0)
	{
		return -1;
	}
	if(fcntl(fd, F_SETSIG, SIGRTMIN) < 0)
	{
		return -1;
	}
	if(fcntl(fd, F_SETOWN, getpid()) < 0)
	{
		return -1;
	}
	return 1;
}
//socket() -> bind() -> listen() 과정을 함수화 했다.
//이 과정에서 만들어진 듣기 소켓은 setup_sigio 함수로 리얼 타임 시그널을 발생하도록 했다.
//이제 클라이언트 연결이 들어오면 듣기 소켓에서 SIGRTMIN 시그널이 발생한다.
int make_listener()
{
	int addrlen;
	int state;
	int listenfd;

	struct sockaddr_in addr;
	addrlen = sizeof(addr);

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd < 0)
	{
		return -1;
	}
	memset((void *)&addr, 0x00, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORT_NUM);
	if(bind (listenfd, (struct sockaddr *)&addr, addrlen) == -1)
	{
		return -1;
	}
	if(listen(listenfd, 5) == -1)
	{
		return -1;
	}
	if(setup_sigio(listenfd))
		return listenfd;
	else
		return -1;
}
//accept 함수로 연결 소켓을 만든다. 연결 소켓은 setup_sigio 함수로 리얼 타임 시그널을 발생하도록 한다.
//이 함수는 sigwaitinfo 함수 반환 뒤에 실행한다.
int make_connected_fd(int listen_fd)
{
	int sockfd;
	socklen_t addrlen;
	struct sockaddr_in addr;
	sockfd = accept(listen_fd, (struct sockaddr *)&addr, &addrlen);
	if(sockfd == -1)
	{
		return -1;
	}
	if(setup_sigio(sockfd) == -1)
	{
		return -1;
	}
	user_fds[sockfd] = 1;
	return sockfd;
}
//연결된 모든 클라이언트에 메시지를 전송한다. 유저 이름은 파일 지정번호로 했다.
int send_chat_msg(int fd, char *msg)
{
	int i;
	char buf[MAXLINE+24];
	memset(buf, 0x00, MAXLINE+24);
	sprintf(buf, "user name (%d) %s", fd, msg);
	for(i = 0; i < 1024; i++)
	{
		if(user_fds[i] == 1)
		{
			write(i, buf, strlen(buf));
		}
	}
}

int main(int argc, char **argv)
{
	struct siginfo si;
	sigset_t set;
	int listenfd;
	int ev_num;
	char buf[MAXLINE];
	int readn;

	//SIGRTMIN 함수가 블럭되도록 설정했다.
	memset((void *)user_fds, 0x00, sizeof(int) * 1024);
	sigemptyset(&set);
	sigaddset(&set, SIGRTMIN);
	sigprocmask(SIG_BLOCK, &set, NULL);

	//make_listener 함수로 듣기 소켓을 가져온다. 듣기 소켓은 리얼 타임 시그널로 설정되어 있다.
	listenfd = make_listener();
	if(listenfd == -1)
	{
		perror("listener make error ");
		return -1;
	}

	while(1)
	{
		//sigwaitinfo 함수로 SIGRTMIN 시그널을 기다린다.
		ev_num = sigwaitinfo(&set, &si);
		if(ev_num == SIGRTMIN)
		{
			//만약 듣기 소켓에서 시그널이 발생했다면 make_connected_fd 함수를 호출해서 연결 소켓을 만든다.
			if(si.si_fd== listenfd)
			{
				if(make_connected_fd(listenfd) == -1)
				{
					perror("accept error ");
				}
			}
			else
			{
				//연결 소켓에서 발생한 시그널이라면 send_chat_msg 함수로 모든 클라이언트에 메시지를 전달한다.
				memset(buf, 0x00, MAXLINE);
				readn = read(si.si_fd, buf, MAXLINE);
				if(readn <= 0)
				{
					user_fds[si.si_fd] = 0;
					close(si.si_fd);
				}
				else
				{
					send_chat_msg(si.si_fd, buf);
				}
			}
		}
	}
}

