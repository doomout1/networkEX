#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <string.h>
#include <stdio.h>

#define PORT_NUM 3600
#define EPOLL_SIZE 20
#define MAXLINE 1024
//epoll_event 구조체로 넘길 유저 데이터 정보를 포함한 구조체이다.
//이 구조체는 파일 지정 번호와 유저 이름을 저장하기 위한 멤버 변수를 가지고 있다.
//구조체를 이용해서 파일 지정번호와 기타 부가 정보를 하나로 묶은 셈이다.
struct udata
{
	int fd;
	char name[80];
};
//서버는 현재 접속한 모든 쿨라이언트에 메시지를 전송한다. 이를 위해서는 접속한 모든 클라이언트의 파일 지정 정보를 가지고 있어야 한다.
int user_fds[1024];
void send_msg(struct epoll_event ev, char *msg);
int main(int argc, char **argv)
{
	struct sockaddr_in addr, clientaddr;
	struct epoll_event ev, *events;
	struct udata *user_data;
	int listenfd;
	int clientfd;
	int i;
	socklen_t addrlen, clilen;
	int readn;
	int eventn;
	int epollfd;
	char buf[MAXLINE];
//이벤트 풀을 만든다. 이벤트 풀의 크기는 20으로 했다.
	events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * EPOLL_SIZE);
	if((epollfd = epoll_create(100)) == -1)
		return 1;

	addrlen = sizeof(addr);
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return 1;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT_NUM);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind (listenfd, (struct sockaddr *)&addr, addrlen) == -1)
		return 1;
	listen(listenfd, 5);
	//epoll_cti 함수로 듣기 소켓을 이벤트 풀에 추가했다.
	ev.events = EPOLLIN;
	ev.data.fd = listenfd;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);

	memset(user_fds, -1, sizeof(int) * 1024);

	while(1)
	{
		//epoll_wait 함수로 이벤트를 기다린다.
		eventn = epoll_wait(epollfd, events, EPOLL_SIZE, -1);
		if(eventn == -1)
		{
			return 1;
		}
		for(i = 0; i < eventn ; i++)
		{
			//듣기 소켓에서 이벤트가 발생했다면, accept 함수를 호출해서 연결 소켓을 만든다.
			//연결 소켓은 epoll_ctl 함수로 이벤트 풀에 추가한다.
			//유저 이름과 소켓 지정번호를 저장하기 위한 user_data 구조체를 만들어서 event_poll 구조체가 가리키게 했다.
			if(events[i].data.fd == listenfd)
			{
				clilen = sizeof(struct sockaddr);
				clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clilen);
				user_fds[clientfd] = 1;

				user_data = malloc(sizeof(user_data));
				user_data->fd = clientfd;
				sprintf(user_data->name, "user(%d)",clientfd);

				ev.events = EPOLLIN;
				ev.data.ptr = user_data;

				epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
			}
			else
			{
				//연결 소켓에서 이벤트가 발생하면 데이터를 읽는다. 만약 읽는 중에 에러가 발생하면 이벤트 풀에서 제거하고 소켓을 닫는다.
				//반드시 free 함수로 유저 데이터를 해제해줘야 하낟. 그렇지 않으면 메모리 누수가 발생한다.
				user_data = events[i].data.ptr;
				memset(buf, 0x00, MAXLINE);
				readn = read(user_data->fd, buf, MAXLINE);
				if(readn <= 0)
				{
					epoll_ctl(epollfd, EPOLL_CTL_DEL, user_data->fd, events);
					close(user_data->fd);
					user_fds[user_data->fd] = -1;
					free(user_data);
				}
				else
				{
					send_msg(events[i], buf);
				}
			}
		}
	}
}
//연결된 모든 클라이언트에 읽은 데이터를 전송한다.
void send_msg(struct epoll_event ev, char *msg)
{
	int i;
	char buf[MAXLINE+24];
	struct udata *user_data;
	user_data = ev.data.ptr;
	for(i =0; i < 1024; i++)
	{
		memset(buf, 0x00, MAXLINE+24);
		sprintf(buf, "%s %s", user_data->name, msg);
		if((user_fds[i] == 1))
		{
			write(i, buf, MAXLINE+24);
		}
	}
}

