#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE 1024
#define PORTNUM 3600
#define SOCK_SETSIZE 1021

int main(int argc, char **argv)
{
    int listen_fd, client_fd;
    socklen_t addrlen;
    int fd_num;
    int maxfd = 0;
    int sockfd;
    int i = 0;
    char buf[MAXLINE];
    fd_set readfds, allfds;

    struct sockaddr_in server_addr, client_addr;

    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        return 1;
    }
    memset((void*)&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORTNUM);

    if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr) == -1))
    {
        perror("bind error");
        return 1;
    }
    if(listen(listen_fd, 5) == -1)
    {
        perror("listen error");
        return 1;
    }
    FD_ZERO(&readfds); //초기화
    FD_SET(listen_fd, &readfds); //듣기 소켓 추가

    maxfd = listen_fd;  //듣기 소켓의 소켓 지정번호를 저장
    while(1)
    {
        allfds = readfds;
        printf("Select Wait %d\n", maxfd);
        fd_num = select(maxfd + 1, &allfds, (fd_set*)0, (fd_set *)0, NULL);

        //만약 듣기 소켓에 읽을 데이터가 있다면 클라이언트 연결 요청이 있음을 뜻함
        if(FD_ISSET(listen_fd, &allfds))
        {
            addrlen = sizeof(client_addr);
            client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
            FD_SET(client_fd, &readfds);
            if(client_fd > maxfd)
                maxfd = client_fd;
            printf("Accept OK\n");
            continue;
        }
        //읽을 데이터가 존재 하는 파일 개수 만큼 루프를 돔
        for(i=0; i<=maxfd; i++)
        {
            sockfd = i;
            if(FD_ISSET(sockfd, &allfds))
            {
                memset(buf, 0x00, MAXLINE);
                if(read(sockfd, buf, MAXLINE) <= 0)
                {
                    close(sockfd);
                    FD_CLR(sockfd, &readfds);
                }
                else
                {
                    if(strncmp(buf, "quit\n", 5) == 0)
                    {
                        close(sockfd);
                        FD_CLR(sockfd, &readfds);
                        break;
                    }
                    printf("Read : %s", buf);
                    write(sockfd, buf, strlen(buf));
                }
                if(--fd_num <- 0)
                    break;
            }
        }
    }
}
