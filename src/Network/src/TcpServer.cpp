/*
 *TcpServerSocket.cpp
 *
 *  Created on: Dec 26, 2017
 *      Author: Daniel yuan
 */
 
#include "TcpServer.h"
//#include <iostream>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <string.h>
#include <poll.h>
#include <errno.h>

#include <unistd.h>  
#include <arpa/inet.h>
#include <sys/wait.h>
//#include <algorithm>

#include "realtime.h"
#include "bmp.h"
#include "logging.h"
#include "test.h"
#include "FdManager.h"
//#include "DecoderMenuSettings.h"

//#define _DEBUG

#define LOG_TAG "TcpServer"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

/*TcpServer::TcpServer(int port,int maxCon):tcpPort(port),maxConnections(maxCon)
{

	pDevFdVec_ = new FdManager(maxConnections);
	LOGGING("Max connectiongs is %d\r\n",maxConnections);
}


TcpServer::~TcpServer()
{
	//Close();
    //this->Stop();
	if(NULL != pDevFdVec_)
		delete pDevFdVec_;
}*/

TcpServer::TcpServer(EventLoop *loop,
                     const InetAddress &listenAddr,
                     const string &nameArg,int maxCon)
  : loop_(loop),
    hostport_(listenAddr.toIpPort()),
    name_(nameArg),
    maxConnections_(maxCon)
{

}

TcpServer::~TcpServer()
{
}

