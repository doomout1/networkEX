#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <net/if.h>

int main(int argc, char **argv)
{
    struct if_nameindex *if_idx, *ifp;
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    struct sockaddr_in6 *sin6;
    int sock_fd;
    int connection = 0;

    if(argc != 3)
    {
        printf("Usage : %s [ip] [port]\n", argv[0]);
        return 1;
    }

    memset(&hints, 0x00, sizeof(hints));
//getaddrinfo 함수에 넘길 힌트 값을 설정한다.
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = 0;
//getaddrinfo 함수로 네트워크 주소 정보를 가져온다.
    if(getaddrinfo(argv[1], argv[2], &hints, &result) != 0 )
    {
        return 1;
    }
//가져온 주소 정보만큼 루프를 돌면서 연결을 시도한다. 도중에 성공하는 연결이 있으면 루프를 빠져나온다.
    for(rp = result; rp != NULL; rp = rp->ai_next)
    {
        sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sock_fd == -1)
        {
            continue;
        }
/*
주소 영역이 IPv6면 처리한다. getaddrinfo 함수는 sockaddr_in6 구조체의 sin6_scope_id 의 값을 설정하지 않는다.
때문에 직접 네트워크 인터페이스 인덱스 값을 조사해서 설정해줘야만 한다.
if_nameindex 함수를 이용하면 네트워크 인터페이스의 인덱스와 이름의 목록을 가져올 수 있다.
*/
        if(rp->ai_family == AF_INET6)
        {
            if_idx = if_nameindex();
            for(ifp = if_idx; ifp->if_name != NULL; ifp++)
            {
                sin6 = (void *)rp->ai_addr;
                sin6->sin6_scope_id = ifp->if_index;
				//루프백 인터페이스는 가상 인터페이스이므로 건너 뛴다.
                if(strcmp(ifp->if_name, "lo") == 0) continue;
                printf("Connection ID==>%d\n", ifp->if_index);
                if(connect(sock_fd, (struct sockaddr *)sin6, rp->ai_addrlen) != -1)
                {
                    printf("ok IPv6 connect\n");
                    connection = 1;
                    break;
                }
                else
                {
                    perror("Connection failed\n");
                }
            }
        }
        else
        {
			//IPv4에서의 연결이다.
            if(connect(sock_fd, rp->ai_addr, rp->ai_addrlen) != -1)
            {
                printf("ok IPv4connect\n");
                connection = 1;
                break;
            }
        }
        if(connection) break;
        close(sock_fd);
    }

    if(rp == NULL)
    {
        printf("Connection Failed\n");
        return 1;
    }
    freeaddrinfo(result);


    return 1;
}

