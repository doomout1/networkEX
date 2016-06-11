#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>

int in_cksum(u_short *p, int n);

int main(int argc, char **argv)
{
	int icmp_socket;
	int ret;
	struct icmp *p, *rp;
	struct sockaddr_in addr, from;
	struct ip *ip;
	char buffer[1024];
	socklen_t sl;
	int hlen;
    //SOCK_RAW 옵션으로로 로우 소켓을 만든다. IPPROTO_ICMP로 imcp 프로토콜을 다룰 것임을 지정한다.
	icmp_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(icmp_socket < 0)
	{
		perror("socket error : ");
		exit(0);
	}

    //icmp 구조체를 초기화 한다. icmp 구조체에는 icmp 헤더 팰드에 대응되는 멤버 변수를 가지고 있다.
    //이 구조체를 이용해서 ICMP헤더의 정보를 변경할 수 있다.
	memset(buffer, 0x00, 1024);

	p = (struct icmp *)buffer;
	p->icmp_type=ICMP_ECHO;
	p->icmp_code=0;
	p->icmp_cksum=0;
	p->icmp_seq=15;
	p->icmp_id=getpid();

    //icmp 헤더의 체크섬에 사용될 값을 계산 한다.
	p->icmp_cksum = in_cksum((u_short *)p, 1000);
	memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	addr.sin_family = AF_INET;

    //sendto 함수를 이용해서 ICMP 요첨 데이터를 전송한다.
	ret=sendto(icmp_socket,p,sizeof(*p),MSG_DONTWAIT,(struct sockaddr *)&addr, sizeof(addr));
	if (ret< 0)
	{
		perror("sendto error : ");
	}

	sl=sizeof(from);

	//recvfrom 함수를 이용해서 ICMP 응답 데이터를 읽는다.
	ret = recvfrom(icmp_socket,buffer, 1024, 0, (struct sockaddr *)&from, &sl);
	if (ret < 0)
	{
		printf("%d %d %d\n", ret, errno, EAGAIN);
		perror("recvfrom error : ");
	}
     //읽은 데이터는 헤더가 제거되지 않은 상태로 전달된다. 그러므로 전체 패킷에서 ip 패킷과 ICMP 패킷을 구분해야 한다.
     //ip 패킷은 ip 구조체에 정의 되어 있으므로, 읽은 데이터를 형 변환 한다. 이제 ip 구조체는 IP 패킷을 가리킨다.
	ip = (struct ip *)buffer;

	//ip_hl 에는 IP 헤더의 길이가 저장되어 있다. 여기에 4를 곱하면 IP 헤더의 전체 크기가 된다.
	hlen = ip->ip_hl*4;

	//ICMP는 IP 헤더 아래 놓인다.
	//그러므로 recvfrom 함수로 읽은 buffer에서  ip 헤더 크기 만큼 뒤로 이동해서 ICMP 헤더의 시작 위치를 찾을 수 있다.
	rp = (struct icmp *)(buffer+hlen);

	//ICMP 정보를 출력한다.
	printf("reply from %s\n", inet_ntoa(from.sin_addr));
	printf("Type : %d \n", rp->icmp_type);
	printf("Code : %d \n", rp->icmp_code);
	printf("Seq  : %d \n", rp->icmp_seq);
	printf("Iden : %d \n", rp->icmp_id);
	return 1;
}

int in_cksum( u_short *p, int n )
{
	register u_short answer;
	register long sum = 0;
	u_short odd_byte = 0;

	while( n > 1 )
	{
		sum += *p++;
		n -= 2;

	}

	if( n == 1 )
	{
		*( u_char* )( &odd_byte ) = *( u_char* )p;
		sum += odd_byte;

	}

	sum = ( sum >> 16 ) + ( sum & 0xffff );
	sum += ( sum >> 16 );
	answer = ~sum;

	return ( answer );
}
