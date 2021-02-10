#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<fcntl.h>
#include<arpa/inet.h>


typedef unsigned char bool;

#define true    1
#define false   0

#define SERVER_ROLE '1'
#define CLIENT_ROLE '2'

#define DEFAULT_PORT 8888
#define MAXLINE 4096


fd_set readFds;
int nFds = 0;


#define WATCH_OBJ_NUM   100

#define INVALID_FD     0
#define VALID_FD       1
typedef struct _watch_table
{
    bool flag;
    int sock_fd;
}TWatchTable, *TWatchTablePtr;

TWatchTable watch[WATCH_OBJ_NUM];


void set_fd_non_block(int fd)
{
    int flags;
    flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
    return;
}

int init_server(void)
{
    int socket_fd, on = 1;
    struct sockaddr_in  servaddr;

    if( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
    {
        printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
        exit(0);
    }

    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(DEFAULT_PORT);

    if(bind(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
        exit(0);
    }

    if(listen(socket_fd, 10) == -1)
    {
        printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
        exit(0);
    }

    return socket_fd;
}


int init_client(char *ip_str)
{
    int sock_fd;

    struct sockaddr_in    servaddr;

    if( (sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(DEFAULT_PORT);
    if(inet_pton(AF_INET, ip_str, &servaddr.sin_addr) <= 0)
    {
        printf("inet_pton error for %s\n",ip_str);
        exit(0);
    }

    if( connect(sock_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
        exit(0);
    }

    return sock_fd;
}

void print_usage(char *role, char *ip_info)
{
    char buf[100] =  {0};
    printf(
"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n\
    Please select link mode:    \n\
    %c server                   \n\
    %c client                   \n\
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\
=========>input:", SERVER_ROLE, CLIENT_ROLE);
    //*role = getchar();
    scanf("%c", role);
    if (CLIENT_ROLE == *role)
    {
        printf("Please input the ip of server:");
        scanf("%s", ip_info);
    }
    return;
}


void add_to_watch_table(int fd)
{
    int index;

    if(!FD_ISSET(fd, &readFds))
        FD_SET(fd, &readFds);

    if (nFds <= fd)
       nFds = fd + 1;

    for(index = 0; index < WATCH_OBJ_NUM; index++)
    {
        if( INVALID_FD == watch[index].flag)
        {
            watch[index].flag = VALID_FD;
            watch[index].sock_fd = fd;

            break;
        }
    }

    return;
}


void delete_from_watch_talbe(int fd)
{
    int index;

    if(FD_ISSET(fd, &readFds))
        FD_CLR(fd, &readFds);

    for(index = 0; index < WATCH_OBJ_NUM; index++)
    {
        if( VALID_FD == watch[index].flag && fd == watch[index].sock_fd)
        {
            watch[index].flag = INVALID_FD;
            watch[index].sock_fd = INVALID_FD - 1;      //set fd is -1

            break;
        }
    }

    for(index = 0; index < WATCH_OBJ_NUM; index++)
    {
        if( VALID_FD == watch[index].flag && nFds <= watch[index].sock_fd)
        {
            nFds = watch[index].sock_fd + 1;
        }
    }

    return;
}

void init_global_var(void)
{
    int index;
    FD_ZERO(&readFds);
    memset(watch, 0, sizeof(watch));
    return;
}

int main(int argc, char* argv[])
{
    char role;
    char ip_info[16] = {0}, buff[MAXLINE];
    int  server_fd, client_fd, sock_fd;
    int n, index;
    int count;

    struct sockaddr_in peeraddr;
    socklen_t socklen = sizeof (peeraddr);

    init_global_var();

    print_usage(&role, ip_info);

    if(SERVER_ROLE == role)
    {
        server_fd = init_server();
        set_fd_non_block(server_fd);
        add_to_watch_table(server_fd);
    }
    else if (CLIENT_ROLE == role)
    {
        client_fd = init_client(ip_info);
        set_fd_non_block(client_fd);
        add_to_watch_table(client_fd);
        send(client_fd, "hello", 5,0);
    }

    fd_set tempFds;

    while(1)
    {
        printf("start to select~~~~~~\n");
	memcpy(&tempFds, &readFds, sizeof(fd_set));
	n = select(nFds, &tempFds, NULL, NULL, NULL);
        printf("select return n = %d\n", n);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return 0;
        }

        //循环测试所有的FD
        for (index = 0; index < WATCH_OBJ_NUM; index++)
        {
            if(VALID_FD == watch[index].flag && FD_ISSET(watch[index].sock_fd, &tempFds))
            {
                if(SERVER_ROLE == role && server_fd == watch[index].sock_fd)
                {
                    if( (sock_fd = accept(server_fd, (struct sockaddr*)&peeraddr, &socklen)) == -1)
                    {
                        printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
                        continue;
                    }
                    printf(">>> new connection[fd:%d] from client[%s:%x] done!! <<<\n", \
                            sock_fd, inet_ntoa(peeraddr.sin_addr), peeraddr.sin_port);
                    set_fd_non_block(sock_fd);
                    add_to_watch_table(sock_fd);

                    continue;
                }

                memset(buff, 0, sizeof(buff));
                n = recv(watch[index].sock_fd, buff, MAXLINE, 0);
                if (0 >= n)
		{
		
			printf("errno:%d(%s)", errno, strerror(errno));
			continue;
		};
		printf("[count = %d]recv return n = %d\n", count++,n);
                if (server_fd != watch[index].sock_fd && n <= 0)
                {
                    delete_from_watch_talbe(watch[index].sock_fd);
                    close(watch[index].sock_fd);
                    continue;
                }
                printf("Received message ===>%s\n", buff);

                if(0 == memcmp("end", buff, strlen("end")))
                {
                    if(SERVER_ROLE ==  role)
                    {
                        send(watch[index].sock_fd, "end", strlen("end"), 0);
                        delete_from_watch_talbe(watch[index].sock_fd);
                        close(watch[index].sock_fd);
                        printf("~~~Wait for new connection~~~\n");
                        continue;
                    }
                    else if (CLIENT_ROLE == role)
                    {
                        exit(0);
                    }
                }

		
	/*	if( SERVER_ROLE != role )
		 {
		        memset(buff, 0, sizeof(buff));
               		printf("Reply message ===>");
               		scanf("%s", buff);
                	send(watch[index].sock_fd, buff, strlen(buff),0);
            	}*/
//                	send(watch[index].sock_fd, buff, strlen(buff),0);
	    }
        }
    }

    return 0;
}

