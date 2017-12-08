/*
 *UdpSocket.cpp
 *
 *  Created on: Aug 07, 2017
 *      Author: honeywell
 */
 
#include "UdpSocket.h"
#include <iostream>
#include <sys/socket.h>
#include <string.h>
//#include <stdio.h>
#include <unistd.h>  
#include <arpa/inet.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>

UdpSocket::UdpSocket(int port):udpPort(port)
{
	/* Fill the local socket address struct */
	bzero(&socketAddr, sizeof(struct sockaddr_in));
	socketAddr.sin_family = AF_INET;			     // Protocol Family
	socketAddr.sin_port = htons(udpPort); 			 // Port number
	socketAddr.sin_addr.s_addr	= htonl(INADDR_ANY); // AutoFill local address
	memset (socketAddr.sin_zero,0,8);
	
	bzero(&remoteSockAddr, sizeof(struct sockaddr_in));
	remoteSockAddr.sin_family = AF_INET;
	remoteSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if ((socketFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {   
		printf("upd socket init error...\r\n");
		return;
	} else {
		printf ("Ok: Udp sockfd = %d\n",socketFd);
		printf ("OK: Obtain Udp Socket Despcritor sucessfully.\n");
	}
	
}

UdpSocket::~UdpSocket()
{

}

void UdpSocket::Close()
{	
	close(socketFd);
}

void UdpSocket::Shutdown(int mode)
{
	switch(mode) {
		case 0:
			shutdown(socketFd,SHUT_RD);			
			break;
		case 1:
			shutdown(socketFd,SHUT_WR);			
			break;
		case 2:
			shutdown(socketFd,SHUT_RDWR);			
			break;
		default:
			shutdown(socketFd,SHUT_RDWR);		
			break;
	}
}

int UdpSocket::SetSocketOpt(void)
{
	int opt = 0;
	//set it as broadcast 
	if(setsockopt(socketFd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt)) == -1) {
		printf("set udp socket SO_BROADCAST error...\r\n");
		return -1;
	}

}

int UdpSocket::Bind()
{
	/*	Blind a special Port */
	if( bind(socketFd, (struct sockaddr*)&socketAddr, sizeof(struct sockaddr_in)) == -1 )
	{  
		printf ("ERROR: Failed to bind Port %d.\n",udpPort);
		printf("errno:%s\r\n",strerror(errno));
		return -1;
	} else {
		printf("OK: Bind the Port %d sucessfully.\n",udpPort);
		return 0;
	}
}

/* include sock_ntop */
char *sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
    char        portstr[8];
    static char str[128];     /* Unix domain is largest */
 
    switch (sa->sa_family) {
    case AF_INET: {
        struct sockaddr_in *sin = (struct sockaddr_in *) sa;
 
        if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
            return(NULL);
        if (ntohs(sin->sin_port) != 0) {
            snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
            strcat(str, portstr);
        }
        return(str);
    }
/* end sock_ntop */
 
#ifdef  IPV6

#endif
 
#ifdef  AF_UNIX

#endif
 
#ifdef  HAVE_SOCKADDR_DL_STRUCT

#endif
    default:
        snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, len %d",
                 sa->sa_family, salen);
        return(str);
    }
    return (NULL);
}
 

char *Sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
    char    *ptr;
 
    if ( (ptr = sock_ntop(sa, salen)) == NULL)
        printf("sock_ntop error");    /* inet_ntop() sets errno */
    return(ptr);
}

#define UDP_BUF_LEN 1024
int UdpSocket::PollTest()
{
	int ret = 0,num = 0;
	fd_set fdSetRead;
	char socket_buf[UDP_BUF_LEN];
	int len = sizeof(struct sockaddr_in);

	FD_ZERO(&fdSetRead);
	FD_SET(socketFd, &fdSetRead);

	ret = select(socketFd + 1, &fdSetRead, NULL, NULL, NULL);
	
	switch (ret) {
		case 0:
			//DBGINFO("udp poll timeout \r\n");
			break;
		case -1:
			printf("udp poll error\r\n");
			break; 
		default:
			if(FD_ISSET(socketFd,&fdSetRead)) {
				printf("udp poll:read socket\r\n");
				//num = recv(socket_fd, socket_buf, 256, 0);
				num = recvfrom(socketFd, socket_buf, UDP_BUF_LEN, 0, (struct sockaddr*)&remoteSockAddr,(socklen_t*)&len);
				if(num) {
					socket_buf[num] = '\0';
					printf("%s\t",socket_buf); 
					printf("received from %s\r\n",Sock_ntop((struct sockaddr *)&remoteSockAddr,len));
					char buf[]="hello world";
					sendto(socketFd, buf, strlen(buf), 0, (struct sockaddr*)&remoteSockAddr, sizeof(remoteSockAddr));
				} else if(num == 0) { //when num is 0, may receive a FIN form host,need close socket
					printf("udp terminated from host!\r\n");
					printf("do nothing!\r\n");
					//close(socket_fd);
					return -1;
				} else {
					printf("udp read error!\r\n");
				}
			}
			break;

	}
}

int UdpSocket::Run()
{
	SetSocketOpt();
	Bind();

	while(1) {
		if( -1 == PollTest() ) {
			printf("udp process exit!\r\n");
			break;	
		}	
	}
}

int UdpSocket::SocketRead(char *buff,int len)
{
	return 0;
}

int UdpSocket::SocketWrite(const char *buff,int len)
{

	return 0;
}

