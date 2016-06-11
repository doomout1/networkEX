#include <netinet/in.h>
#include <stdio.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PACKET_LENGTH  65536

void PrintPacket(unsigned char* , int);
void PrintTcp(unsigned char *, int size);
void PrintData (unsigned char *, int Size);

int main(int argc, char **argv)
{
	int readn;
	socklen_t addrlen;
	int sock_raw;
	struct sockaddr_in saddr;

	unsigned char *buffer = (unsigned char *)malloc(PACKET_LENGTH);

	//로웃 소켓을 만든다.
	sock_raw = socket(AF_INET , SOCK_RAW , IPPROTO_TCP);
	if(sock_raw < 0)
	{
		return 1;
	}
	while(1)
	{
		addrlen = sizeof(saddr);
		memset(buffer, 0x00, PACKET_LENGTH);

		//로우 소켓으로 데이터를 읽는다.
		readn = recvfrom(sock_raw , buffer , PACKET_LENGTH , 0 , (struct sockaddr *)&saddr , &addrlen);
		if(readn <0 )
		{
			return 1;
		}
		//로우 소켓이서 읽은 데이터는 패킷 헤더를 포함하고 있다.
		//PrintPacket 함수로 패킷 헤더를 분석하고 나서, 프로토콜에 맞는 함수를 호출한다.
		//여기에서는  TCP 데이터만 처리하도록 한다.
		PrintPacket(buffer , readn);
	}
	close(sock_raw);
	return 0;
}

void PrintPacket(unsigned char* buffer, int size)
{
	//iphdr 구조체로 ip헤더를 가리킨다.
	struct iphdr *iph = (struct iphdr*)buffer;
	switch (iph->protocol)
	{
		case 1:
			printf("I recv icmp Packet\n");
			break;
		case 6:
			PrintTcp(buffer , size);
			break;
		case 17:
			printf("I recv UDP Packet\n");
			break;
		default:
			break;
	}
}

void PrintTcp(unsigned char* buf, int size)
{
	unsigned short iphdrlen;
	unsigned char *data;

	//패킷 캡처는 헤더를 제외한 사용자 데이터에만 이루어져야 한다.
	//그러므로 ip 패킷 헤더와 tcp 패킷 헤더를 건너 뛰어서 유저 데이터를 가리키는 주소로 이동해야 한다.
	//tcp 데이터는 ip 패킷 + tcp 패킷 + 데이터로 구성되어 있으므로,
	//읽은 데이터가 저장된 buf의 처음 위치에서 ip 헤더 크기 + TCP 헤더 크기 만큼만 포인터를 이동하면 유저 데이터의 시작 지점이다.
	struct iphdr *iph = (struct iphdr *)buf;
	iphdrlen = iph->ihl*4;
	struct tcphdr *tcph=(struct tcphdr*)(buf + iphdrlen);

	data = (unsigned char *)(buf + (iph->ihl*4) + (tcph->doff*4));
	printf("%s", data);
}

