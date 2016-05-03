#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#define CLASS_MASK htonl(15<<4)

int main(int argc, char **argv)
{
//    char buf[256];
    char class_name[6][16] = {{"E class\0"},{"D class\0"},{"C class\0"},{"B class\0"},{"A class\0"},{"\0"}};
    unsigned int laddr;
    unsigned int haddr;
    int i;

    if(argc != 2)
    {
        fprintf(stderr, "Usage : %s [internet address]\n", argv[0]);
        return 1;
    }

    laddr = inet_addr(argv[1]);
    if(laddr == INADDR_ANY)
    {
        printf("INADDR_ANY\n");
        return 0;
    }
    else if(laddr == INADDR_NONE)
    {
        printf("INADDR_NONE\n");
        return 0;
    }
    haddr = ntohl(laddr);

    for(i = 0; i < 5; i++)
    {
        if(((CLASS_MASK << i)& (haddr))==(CLASS_MASK << i))
        {
            break;
        }
    }
    printf("%s -> %s\n", argv[1], class_name[i]);
    return 0;
}
