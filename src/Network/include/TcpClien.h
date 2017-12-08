/*
 * TcpClient.h
 *
 *  Created on: Aus 01, 2017
 *      Author: honeywell
 */

#ifndef SRC_TCPCLIENT_H_
#define SRC_TCPCLIENT_H_

#include "TcpClientSocket.h"

class TcpClient
{
public:
	TcpClient();
	virtual ~TcpClient();
	int Run();
private:
	TcpClientSocket *pTcpSocket;

protected:

};

#endif /* SRC_TCPCLIENT_H_ */



