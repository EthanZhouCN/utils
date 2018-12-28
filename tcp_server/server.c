#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#define _PORT_ 9999
#define _BACKLOG_ 10
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>


uint8_t rent_check_bcc(uint8_t *pkg, uint16_t len)
{
	uint16_t index = 0;
	uint8_t  bcc_code = 0;
	
	for(index=2; index<len-1; index++){
		bcc_code ^= *(pkg+index);
	}
	
	if(*(pkg+len-1) == bcc_code)
	{
		return 1;
	}else{
		return 0;
	}
}

uint16_t make_bcc(uint8_t *pkg, uint16_t position)
{
	uint16_t index = 2;
	uint8_t  bcc_code = 0;
	
	for(; index<position; index++){
		bcc_code ^= *(pkg+index);
	}
	*(pkg+position) = bcc_code;
	
	return 1;
}



void *fun(void* arg)
{
    int client_sock = (int)arg;
	int ret = 0;
	int printf_index = 0;
	while(1)
	{  
		unsigned char buf[1024];
		memset(buf, '\0', sizeof(buf));
		ret = read(client_sock, buf, sizeof(buf));
		if(ret == 0)
		{	
			printf("client close socket connect.\n");
			break;
		}
		else if(ret > 1)
		{
			if((buf[0] == 0x23) && (buf[1] == 0x23))
			{
				printf("server recv : ");
				for(printf_index=0; printf_index<ret; printf_index++)
				{
					printf("%02X ", buf[printf_index]);
				}
				printf("\n");
				
				buf[3] = 0x01;
				make_bcc(buf, ret-1);
			}
		}

		printf("server send : ");
		for(printf_index=0; printf_index<ret; printf_index++)
		{
			printf("%02X ", buf[printf_index]);
		}
		printf("\n");
		write(client_sock,buf,ret);

	}
    close(client_sock);
}
int main(int argc, char **argv)
{
    
   

	int sock=socket(AF_INET,SOCK_STREAM,0);
	
    if(sock<0)
    {
        printf("socket()\n");
    }
	
	struct sockaddr_in server_socket;
    struct sockaddr_in socket;
	struct hostent *server;
    pthread_t thread_id; 
    bzero(&server_socket,sizeof(server_socket));
    server_socket.sin_family=AF_INET;
	server = gethostbyname(argv[1]);
	bcopy((char*)server->h_addr, (char*)&server_socket.sin_addr.s_addr, server->h_length);
	
    server_socket.sin_port=htons(atoi(argv[2]));
	
    if(bind(sock,(struct sockaddr*)&server_socket,sizeof(struct sockaddr_in))<0)
    {
        printf("bind()\n");
        close(sock);
        return 1;
    }
    if(listen(sock,_BACKLOG_)<0)
    {
        printf("listen()\n");
        close(sock);
        return 2;
    }
    printf("success\n");
    for(;;)
    {
        socklen_t len=0;
        int client_sock=accept(sock,(struct sockaddr*)&socket,&len);
        if(client_sock<0)
        {
            printf("accept()\n");
            return 3;
        }
		
      
        printf("ip : %s\n", inet_ntoa(socket.sin_addr));
        printf("port : %d\n", ntohs(socket.sin_port)); 
		
		printf("client connect success.\n");
        pthread_create(&thread_id, NULL, (void *)fun, (void *)client_sock);  
        pthread_detach(thread_id); 
    }
    close(sock);
    return 0;
}
