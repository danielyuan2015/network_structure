/*
 * UdpSocket.h
 *
 *  Created on: Aus 07, 2017
 *      Author: honeywell
 */

#ifndef UDPSOCKET_H_
#define UDPSOCKET_H_

#include "socket.h"

class UdpSocket:public cSocket
{
public:
	UdpSocket(int port = 0);
	virtual ~UdpSocket();
	void Close();
	void Shutdown(int mode);

	int SetSocketOpt();
	int Bind();

	int SocketRead(char *buff,int len);
	int SocketWrite(const char* buff,int len);
	
	int PollTest();
	int Run();
	
private:
	int udpPort;
	//static struct sockaddr_in socketConnAddr;
	//int remoteSockFd; 	// New Socket file descriptor
	struct sockaddr_in remoteSockAddr;
protected:
};

#endif /* UDPRSOCKET_H_ */
