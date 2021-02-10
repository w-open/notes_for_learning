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

void echo_srv(int conn)
{
	char recvbuf[1024];
	int n;
	while(1)
	{
		memset(recvbuf,0,sizeof(recvbuf));
		//不断接收一行数据到recvbuf中
		n=read(conn,recvbuf,sizeof(recvbuf));
		if(n==-1)
		{
			if(n==EINTR)
				continue;
			ERR_EXIT("read");
		}
		//客户端关闭
		else if(n==0)
		{
			printf("client close\n");
			break;
		}
		fputs(recvbuf,stdout);
		write(conn,recvbuf,strlen(recvbuf));
	}
	close(conn);
}

int main(void)
{
	int listenfd;
	//创建一个监听套接字
	//它的协议家族是PF_UNIX,用流式套接字SOCK_STREAM
	if((listenfd=socket(PF_UNIX,SOCK_STREAM,0))<0)
		ERR_EXIT("socket");

	//unlink表示删除这个文件,先删除，再绑定，重新创造了一个文件
	unlink("/tmp/test_socket");

	//初始化一个地址绑定监听
	struct sockaddr_un servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sun_family=AF_UNIX;
	strcpy(servaddr.sun_path,"/tmp/test_socket");

	//绑定
	//绑定的时候会产生test_socket文件
	if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
		ERR_EXIT("bind");
	//监听
	//监听队列的最大值SOMAXCONN
	if(listen(listenfd,SOMAXCONN)<0)
		ERR_EXIT("listen");

	int conn;
	pid_t pid;
	//接受客户端的连接
	while(1)
	{
		//返回一个已连接套接字
		conn=accept(listenfd,NULL,NULL);
		if(conn==-1)
		{
			if(conn==EINTR)
				continue;
			ERR_EXIT("accept");
		}		
		
		pid=fork();
		if(pid==-1)
			ERR_EXIT("fork");
	
		//pid==0(子进程)说明是客户端，执行回射
		if(pid==0)
		{
			//子进程不需要处理监听
			close(listenfd);
			echo_srv(conn);
			exit(EXIT_SUCCESS);
		}
		//父进程不需要处理连接
		close(conn);
	}
	return 0;
}
