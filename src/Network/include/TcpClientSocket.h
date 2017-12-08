/*
 * TcpClientSocket.h
 *
 *  Created on: Aus 01, 2017
 *      Author: honeywell
 */

#ifndef SRC_TCPCLIENTSOCKET_H_
#define SRC_TCPCLIENTSOCKET_H_

#include "socket.h"

class TcpClientSocket:public cSocket
{
public:
	TcpClientSocket(int port = 0);
	virtual ~TcpClientSocket();
	void Close();
	void Shutdown(int);
	int SetSocketOpt();
	int Connect();
	
	int SocketRead(char *buff,int len);
	int SocketWrite(const char* buff,int len);

private:
	char ipTab[100];
	int  tcpPort;
	bool tcpSocketIsValid;
	int remoteSockFd; 	// New Socket file descriptor
	struct sockaddr_in remoteSockAddr;
	
protected:
};

#endif /* SRC_TCPCLIENTSOCKET_H_ */


