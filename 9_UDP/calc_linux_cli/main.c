#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT_NUM 3800
#define MAXLEN 256

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
	int sockfd;
	struct sockaddr_in addr;
	struct cal_data sdata, recvaddr;

	char msg[MAXLEN];
	int left_num;
	int right_num;
	socklen_t addrlen;

	char op[2];

	if (argc != 2)
	{
		printf("Usage : %s [ipaddress]\n", argv[0]);
		return 1;
	}
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 )
	{
		return 1;
	}
	memset((void *)&addr, 0x00, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	addr.sin_port = htons(PORT_NUM);

	while(1)
	{
		printf("> ");
		fgets(msg, MAXLEN-1, stdin);
		if(strncmp(msg, "quit\n",5) == 0)
		{
			break;
		}
		sscanf(msg, "%d%[^0-9]%d", &left_num, op, &right_num);
		memset((void *)&sdata, 0x00, sizeof(sdata));
		sdata.left_num = htonl(left_num);
		sdata.right_num = htonl(right_num);
		sdata.op = op[0];

		addrlen = sizeof(addr);
		sendto(sockfd, (void *)&sdata, sizeof(sdata), 0,
			(struct sockaddr *)&addr, addrlen);

		recvfrom(sockfd, (void *)&sdata, sizeof(sdata), 0,
			(struct sockaddr *)&recvaddr, &addrlen);
		printf("%d %c %d = %d\n", ntohl(sdata.left_num), sdata.op, ntohl(sdata.right_num), ntohl(sdata.result));
	}
	close(sockfd);
}


