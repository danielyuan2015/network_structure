/*
 *TcpClientSocket.cpp
 *
 *  Created on: Aug 01, 2017
 *      Author: honeywell
 */
 
#include "TcpClientSocket.h"
#include <iostream>
#include <sys/socket.h>
#include <string.h>
//#include <stdio.h>
#include <unistd.h>  
#include <arpa/inet.h>

TcpClientSocket::TcpClientSocket(int port):tcpPort(port)
{
	/* Get the Socket file descriptor */  
	if( (socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )  {   
		printf ("ERROR: Failed to obtain Client Socket Despcritor.\n");
		tcpSocketIsValid = false;
		//return (0);
	} else {
		printf ("sockfd = %d\n",socketFd);
		printf ("OK: Obtain Client Socket Despcritor sucessfully.\n");
	}
	
	/* Fill the local socket address struct */
	remoteSockAddr.sin_family = AF_INET;			// Protocol Family
	remoteSockAddr.sin_port = htons(tcpPort); 			// Port number
	inet_pton(AF_INET,ipTab,&remoteSockAddr.sin_addr); 	// Net Address
	remoteSockAddr.sin_addr.s_addr	= htonl(INADDR_ANY);  // AutoFill local address
	memset (remoteSockAddr.sin_zero,0,8);				// Flush the rest of struct
	tcpSocketIsValid = true;
}

TcpClientSocket::~TcpClientSocket()
{
	if(true == tcpSocketIsValid) {
		tcpSocketIsValid = false;
		close(socketFd);
	}
}

void TcpClientSocket::Close()
{
	close(socketFd);
}

void TcpClientSocket::Shutdown(int mode)
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

int TcpClientSocket::SetSocketOpt(void)
{
	int optval = 1;//on
	if( setsockopt( socketFd , SOL_SOCKET, SO_REUSEADDR,( char *)&optval, sizeof( optval ) ) < 0 )
		printf( " set socket SO_REUSEADDR error /n" );
	if( setsockopt( socketFd , SOL_SOCKET, SO_REUSEPORT,( char *)&optval, sizeof( optval ) ) < 0 )
		printf( " set socket SO_REUSEPORT error /n" );
	return 0;
}

int TcpClientSocket::Connect(void)
{
	/* Try to connect the remote */
	if (connect(socketFd, (struct sockaddr *)&remoteSockAddr, sizeof(struct sockaddr)) == -1) {
		printf ("ERROR: Failed to connect to the host!\n");
		return (0);
	} else {
		printf ("OK: Have connected to the %s\n",ipTab);
	}
	return 0;
}

int TcpClientSocket::SocketRead(char *buff,int len)
{
	//std::cout<<"SocketRead\r\n";
	if(true == tcpSocketIsValid) {
		return read(socketFd, buff, len);	
	} else
		return -1;
}

int TcpClientSocket::SocketWrite(const char *buff,int len)
{
	//std::cout<<"SocketWrite\r\n";
	if(true == tcpSocketIsValid) {
		if((send(socketFd, buff, len ,0)) == -1) {
			printf("ERROR: Failed to sent string.\n");
			return -1;
		}	
	} else
		return -1;
}

