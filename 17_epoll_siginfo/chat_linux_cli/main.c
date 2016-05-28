#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 1024

int main(int argc, char **argv)
{
  int socketfd;
  struct sockaddr_in sockaddr;
  int socklen;
  fd_set readfds, oldfds;
  int maxfd;
  int fd_num;
  char buf[MAXLINE];
  int readn, writen;

  if(argc != 3)
  {

  printf("Usage : %s [ip address] [port]\n", argv[0]);
 	 return 1;
  }
  if( (socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
  {
 	 return 1;
  }
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = inet_addr(argv[1]);
  sockaddr.sin_port = htons(atoi(argv[2]));

  socklen = sizeof(sockaddr);
  if(connect(socketfd, (struct sockaddr *)&sockaddr, socklen) == -1)

  {
 	 perror("connect error");
 	 return 1;
  }

  FD_ZERO(&readfds);
  FD_SET(socketfd, &readfds);
  FD_SET(0, &readfds);

  maxfd = socketfd;

  oldfds = readfds;
  while(1)
  {
 	 readfds = oldfds;
 	 fd_num = select(maxfd + 1, &readfds, NULL, NULL, NULL);
 	 if(FD_ISSET(socketfd, &readfds))
 	 {
 		 memset(buf, 0x00, MAXLINE);
 		 readn = read(socketfd, buf, MAXLINE);
 		 if(readn <= 0)
 		 {
 			 return 1;
 		 }
 		 writen = write(1, buf, readn);
 		 if(writen != readn)
 		 {
 			 return 1;
 		 }
 	 }
 	 if(FD_ISSET(0, &readfds))
 	 {
 		 memset(buf, 0x00, MAXLINE);
 		 readn = read(0, buf, MAXLINE);
 		 printf("> %s", buf);
 		 if(readn <= 0)
 		 {
 			 return 1;
 		 }
 		 writen = write(socketfd, buf, readn);
 		 if(readn != writen)
 		 {
 			 return 1;
 		 }
 	 }
  }
}
