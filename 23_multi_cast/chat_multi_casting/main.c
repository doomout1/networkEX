#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define MAXLINE 1024

char *msg_exit ="exit\n";

int main(int argc, char **argv)
{
	int read_fd;
	int send_fd;
	pid_t pid;
	int flag;
	struct sockaddr_in from;
	int addrlen;

	struct sockaddr_in mcast_group;
	struct ip_mreq mreq;

	char name[16];

	char message[MAXLINE];

	if(argc != 4)
	{
		printf("Usage : %s \n", argv[0]);
		return 1;
	}

	sprintf(name, "%s", argv[3]);
	memset(&mcast_group, 0x00, sizeof(mcast_group));

	mcast_group.sin_family = AF_INET;
	mcast_group.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &mcast_group.sin_addr);

	if( read_fd = socket(AF_INET, SOCK_DGRAM, 0) < 0 )
	{
		return 1;
	}
    //참여할 멀티캐스트 주소를 지정한다.
	mreq.imr_multiaddr = mcast_group.sin_addr;

	//멀티캐스트 데이터를 받을 인터페이스 주소 영역을 지정한다.
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	//setsockopt 함수를 이용해서 열린 소켓을 멀티캐스트 채널에 추가한다.
	if(setsockopt(read_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq))<0)
	{
		printf("error : add group\n");
		return 1;
	}
	flag = 1;
	if(setsockopt(read_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0)
	{
		printf("Socket Reuseaddr Error\n");
		return 1;
	}

	if(bind(read_fd, (struct sockaddr *)&mcast_group, sizeof(mcast_group)) < 0)
	{
		return 1;
	}

	if( (send_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
	{
		return 1;
	}

	pid = fork();
	if(pid < 0)
	{
		printf("fork error\n");
		return 1;
	}
	//자식 프로세스는 단지 데이터를 읽기만 한다.
	else if(pid == 0)
	{
		while(1)
		{
			addrlen = sizeof(from);
			memset(message, 0x00, MAXLINE);
			if(recvfrom(read_fd, message, MAXLINE, 0, (struct sockaddr *)&from, &addrlen))
			{
				printf("error : recvfrom\n");
					return 1;
			}
			printf("%s", message);

			//만약 'exit\n'이라는 문자열이 전달되면 멀티캐스트ㅡ 채널에서 제거한다. 즉, 채널을 빠져나간다.
			if(strstr(message, msg_exit) != NULL)
			{
				printf("EXIT\n");
				if(setsockopt(read_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq))	 < 0)
				{
					return 1;
				}
			}
		}
	}
	else
	{
        //부모 프로세스는 키보드 입력을 읽어서 멀티캐스트 채널로 전송한다.
		while(1)
		{
			memset(message, 0x00, MAXLINE);
			read(0, message, MAXLINE);
			sprintf(message, "%s : %s", name, message);
			if(sendto(send_fd, message, strlen(message), 0, (struct sockaddr *)&mcast_group,
                    sizeof(mcast_group)) < strlen(message))
			{
				return 1;
			}
		}
	}
	return 0;
}
