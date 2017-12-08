/*
 * cSocket.h
 *
 *  Created on: Aus 01, 2017
 *      Author: honeywell
 */

#ifndef SRC_SOCKET_H_
#define SRC_SOCKET_H_

#include <sys/socket.h>
#include<netinet/in.h>   // sockaddr_in

/*#include "Interface.h"
#include "Hardware.h"
#include "HardwareDetect.h"
#include "ProductConfiguration.h"
#include "ReaderConfiguration.h"
#include "interfacetypes.h"
#include "CommandParser.h"*/

class cSocket 
{
public:
	cSocket();
	virtual ~cSocket();
	virtual void Close() = 0;
	virtual void Shutdown(int) = 0;
	virtual int SocketRead(char *,int) = 0;
	virtual int SocketWrite(const char*,int) = 0;
	
protected:
	struct sockaddr_in socketAddr;
	int socketFd;
};

#endif /* SRC_SOCKET_H_ */

