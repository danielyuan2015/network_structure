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

TcpServer::TcpServer(EventLoop *loop,
                     const InetAddress &listenAddr,
                     const string &nameArg,int maxCon)
  : loop_(loop),
    hostport_(listenAddr.toIpPort()),
    name_(nameArg),
    maxConnections_(maxCon),
    acceptor_(new Acceptor(loop, listenAddr,true))
{
	acceptor_->setNewConnectionCallback(
		std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
}

void TcpServer::start()
{
	loop_->runInLoop(
		std::bind(&Acceptor::listen, acceptor_));
}

//sockfd: new connection socket fd
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
	LOGGING("call newConnection\r\n");
	//loop_->assertInLoopThread();
	
	InetAddress localAddr(sockets::getLocalAddr(sockfd));

}

void TcpServer::removeConnection()
{
	LOGGING("call removeConnection\r\n");
}

