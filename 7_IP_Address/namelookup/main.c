#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
    struct hostent *myent;

    if(argc !=2)
    {
        fprintf(stderr, "Usage : %s [domain name]\n", argv[0]);
        return 1;
    }
    myent = gethostbyname(argv[1]);
    if(myent == NULL){
        perror("ERROR ");
        return 1;
    }
    printf("Host Name : %s\n", myent -> h_name);
    while(*myent -> h_addr_list != NULL)
    {
        printf("%s\n", inet_ntoa(*(struct in_addr *)*myent -> h_addr_list));
        myent -> h_addr_list++;
    }
    return 0;
}
