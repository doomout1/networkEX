#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>


#define MAX_LINE 1024

void ReadLine(int maxfd, fd_set *readfds);

int main(int argc, char **argv)
{
    int tfd[2];
    int i = 0;
    int maxfd = 0;
    int fdn = 0;
    fd_set readfds;

    if(argc != 3)
    {
        fprintf(stderr, "Usage : %s [file 1] [file 2]\n", argv[0]);
        return 1;
    }
    FD_ZERO(&readfds);

    for(i=1; i<2; i++)
    {
        printf("Open %s\n", argv[i+1]);
        tfd[i] = open(argv[i+1], O_RDWR);
        if(tfd[i]==-1)
        {
            perror("Error");
            return 1;
        }
        if(tfd[i] > maxfd)
            maxfd = tfd[i];
        FD_SET(tfd[i], &readfds);
    }

    printf("Max Fd is %d\n", maxfd);
    while(1)
    {
        fdn = select(maxfd+1, &readfds, NULL, NULL, NULL);
        ReadLine(maxfd+1, &readfds);
    }
}

void ReadLine(int maxfd, fd_set *readfds)
{
    char buf[MAX_LINE];
    int i = 0;

    for(i = 0; i < maxfd; i++)
    {
        if(FD_ISSET(i, readfds))
        {
            while(1)
            {
                memset(buf, 0x00, MAX_LINE);
                if(read(i, buf, MAX_LINE-1) < MAX_LINE -1)
                    break;
                printf("(%d) -> %s", i, buf);
            }
            printf("(%d) -> %s", i, buf);
        }
        FD_SET(i, readfds);
    }
}
