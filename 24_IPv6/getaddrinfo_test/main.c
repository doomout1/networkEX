#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>


int main(int argc, char **argv)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;
	int *listen_fd;
	int listen_fd_num=0;

	char buf[80] = {0x00,};
	int i = 0;
	if(argc != 2)
	{
		printf("Usage : %s [port]\n", argv[0]);
		return 1;
	}

	memset(&hints, 0x00, sizeof(struct addrinfo));
/*
getaddrinfo 함수에 사용할 힌트 정보를 만든다.
ai_family는 AF_UNSPEC로 했는데 이는 IPv4와 IPv6모두에 대한 정보를 얻겠다는 의미다.
ai_socktype은 SOCK_STREAM으로 TCP 소켓을 사용하겠다고 명시했다.
ai_flags로 AI_PASSIVE를 사용했는데, 이 값을 사용하면 bind 함수에 사용할 주소 정보를 얻어올 수 있다.
bind 주소로는 IPv4에서 INADDR_ANY, IPv6에서 IN6ADDR_ANY를 사용한다.
*/
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

/*
위에서 만든 힌트를 이용해서 사용할 수 있는 주소 정보를 가져온다.
가져온 주소 정보는 매개변수 result로 넘어온다.
이 예제는 서버용이기에 nodename는 NULL로 입력했다.
*/
	if(getaddrinfo(NULL, argv[1], &hints, &result) != 0 )
	{
		perror("getaddrinfo");
		return 1;
	}

	//몇 개의 주소 정보를 가져왔는지 확인한다.
	for(rp = result ; rp != NULL; rp = rp->ai_next)
	{
		listen_fd_num++;
	}
	//가져온 주소 정보 만큼의 소켓 지정 번호를 저장하기 위한 배열을 만들었다.
	listen_fd = malloc(sizeof(int)*listen_fd_num);

	for(rp = result, i=0 ; rp != NULL; rp = rp->ai_next, i++)
	{
		//개발자에게 디버깅 정보를 출력하기 위한 코드다. 각 소켓의 주소 정보를 출력한다.
		if(rp->ai_family == AF_INET)
		{
			sin = (void *)rp->ai_addr;
			inet_ntop(rp->ai_family, &sin->sin_addr, buf, sizeof(buf));
			printf("<bind 정보 %d %d %s>\n", rp->ai_protocol, rp->ai_socktype, buf);
		}
		else
		{
			sin6 = (void *)rp->ai_addr;
			inet_ntop(rp->ai_family, &sin6->sin6_addr, buf, sizeof(buf));
			printf("<bind 정보 %d %d %s>\n", rp->ai_protocol, rp->ai_socktype, buf);
		}
		if((listen_fd[i] = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) < 0)
		{
			printf("Socket Create Error\n");
		}
/*
IPv6는 IPv4를 포함한다. 그러므로 만약 IPv4 소켓이 만들어진 다음 IPv6 소켓을 만들게 되면,
IPv6 소켓이 IPv4 주소를 포함해 버리기 때문에 기존 IPv4 소켓과 중복 된다.
결국 bind에서 에러가 생긴다. setsockopt 함수를 이용해서 해당 소켓이 단지 IPv6 주소만 가지도록 제한했다.
네트워크 인터페이스가 IPv4와 IPv6 주소를 모두 가지고 있고, IPv6_V6ONLY 소켓 옵션을 활성화 하면
결국 두개의 소켓이 만들어진다.
*/
		if(rp->ai_family == AF_INET6)
		{
			int opt = 1;
			setsockopt(listen_fd[i], IPPROTO_IPV6, IPV6_V6ONLY, (char *)&opt, sizeof(opt));
		}
		//bind 함수 호출
		if(bind(listen_fd[i], rp->ai_addr, rp->ai_addrlen) != 0)
		{
			if(errno != EADDRINUSE);
			{
				perror("bind error\n");
				return 1;
			}
		}
		//listen 함수 호출
		if(listen(listen_fd[i], 5) != 0)
		{
			perror("listen error\n");
			return 1;
		}
	}
	//getaddrinfo 함수로 가져온 주소 정보는 전역 변수로 저장된다.
	//더이상 쓰지 않는 경우에는 freeaddrinfo 함수를 호출해서 메모리를 해제하자.
	freeaddrinfo(result);
	pause();
	return 0;
}

