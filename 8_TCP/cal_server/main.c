#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdio.h>

#define PORT 3600

struct cal_data
{
        int left_num;
        int right_num;
        char op;
        int result;
        short int error;
};

int main(int argc, char **argv)
{
        struct sockaddr_in client_addr, sock_addr;
        int listen_sockfd, client_sockfd;
        socklen_t addr_len;
        struct cal_data rdata;
        int left_num, right_num;
        short int cal_result;
        printf("1");

        if( (listen_sockfd  = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
                perror("Error ");
                return 1;
        }

        memset((void *)&sock_addr, 0x00, sizeof(sock_addr));
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        sock_addr.sin_port = htons(PORT);

        if( bind(listen_sockfd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1)
        {
                perror("Error ");
                return 1;
        }

        if(listen(listen_sockfd, 5) == -1)
        {
                perror("Error ");
                return 1;
        }

        for(;;)
        {
                addr_len = sizeof(client_addr);
                client_sockfd = accept(listen_sockfd, (struct sockaddr *)&client_addr, &addr_len);
                if(client_sockfd == -1)
                {
                        perror("Error ");
                        return 1;
                }
                read(client_sockfd, (void *)&rdata, sizeof(rdata));

                rdata.error = 0;

                left_num = ntohs(rdata.left_num);
                right_num = ntohs(rdata.right_num);

                switch(rdata.op)
                {
                        case '+':
                                cal_result = left_num + right_num;
                                break;
                        case '-':
                                cal_result = left_num  - right_num;
                                break;
                        case '*':
                                cal_result = left_num * right_num;
                                break;
                        case '/':
                                if(right_num == 0)
                                {
                                        rdata.error = 2;
                                        break;
                                }
                                cal_result = left_num / right_num;
                                break;
                        default:
                                rdata.error = 1;

                }

                rdata.result = htonl(cal_result);
                rdata.error = htons(cal_result);
                write(client_sockfd, (void *)&rdata, sizeof(rdata));
                close(client_sockfd);
        }

        close(listen_sockfd);
        return 0;
}

