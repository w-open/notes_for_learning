#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE); \
	}while(0)

void echo_cli(int sock)
{
	char sendbuf[1024]={0};
	char recvbuf[1024]={0};
	//不停地从标准输入获取一行数据到一个缓冲区当中
	while(fgets(sendbuf,sizeof(sendbuf),stdin)!=NULL)	
	{
		//发送给服务器端
		write(sock,sendbuf,strlen(sendbuf));
		//接收回来
		read(sock,recvbuf,sizeof(recvbuf));
		fputs(recvbuf,stdout);
		memset(sendbuf,0,sizeof(sendbuf));
		memset(recvbuf,0,sizeof(recvbuf));
	}
	close(sock);
}
int main(void)
{
	int sock;
	if((sock=socket(PF_UNIX,SOCK_STREAM,0))<0)
		ERR_EXIT("socket");

	struct sockaddr_un servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sun_family=AF_UNIX;
	strcpy(servaddr.sun_path,"/tmp/test_socket");

	if(connect(sock,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
		ERR_EXIT("connect");

	//连接成功后，执行回射客户端的函数
	echo_cli(sock);
	return 0;
}

