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
#define PRINTFUNC LOGGING("%s\r\n",__func__);

TcpServer::TcpServer(EventLoop *loop,
                     const InetAddress &listenAddr,
                     const string &nameArg,int maxCon)
  : loop_(loop),
    hostport_(listenAddr.toIpPort()),
    name_(nameArg),
    maxConnections_(maxCon),
    acceptor_(new Acceptor(loop, listenAddr,true)),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    nextConnId_(1)
{
	acceptor_->setNewConnectionCallback(
		std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
	loop_->assertInLoopThread();
	//LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";
	LOGGING("TcpServer::~TcpServer [%s] destructing\r\n",name_.c_str());

	for (ConnectionMap::iterator it(connections_.begin());
		it != connections_.end(); ++it) {
		TcpConnectionPtr conn = it->second;
		it->second.reset();
		conn->getLoop()->runInLoop(bind(&TcpConnection::connectDestroyed, conn));
		conn.reset();
	}

}

void TcpServer::start()
{
	//assert(!acceptor_->listenning());
	loop_->runInLoop(bind(&Acceptor::listen, acceptor_));
}

//sockfd: new connection socket fd
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
	PRINTFUNC;
	loop_->assertInLoopThread();
	//EventLoop* ioLoop = threadPool_->getNextLoop();
	char buf[64];
	snprintf(buf, sizeof buf, "-%s#%d", hostport_.c_str(), nextConnId_);
	++nextConnId_;
	string connName = name_ + buf;
	
	/*LOG_INFO << "TcpServer::newConnection [" << name_
			 << "] - new connection [" << connName
			 << "] from " << peerAddr.toIpPort();*/
	LOGGING("TcpServer::newConnection [%s] - new connection [%s] from %s\r\n",
		name_.c_str(),connName.c_str(),peerAddr.toIpPort().c_str());

	InetAddress localAddr(sockets::getLocalAddr(sockfd));
	// FIXME poll with zero timeout to double confirm the new connection
	// FIXME use make_shared if necessary
	TcpConnectionPtr conn(new TcpConnection(loop_,
											connName,
											sockfd,
											localAddr,
											peerAddr));
	connections_[connName] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(bind(&TcpServer::removeConnection, this,placeholders::_1)); // FIXME: unsafe
	loop_->runInLoop(bind(&TcpConnection::connectEstablished, conn));


}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
	PRINTFUNC;
	// FIXME: unsafe
	loop_->runInLoop(bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
	PRINTFUNC;
	loop_->assertInLoopThread();
	/*LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
	       << "] - connection " << conn->name();*/
	LOGGING("TcpServer::removeConnectionInLoop [%s] - connection %s\r\n",name_.c_str(),conn->name().c_str());
	size_t n = connections_.erase(conn->name());
	//(void)n;
	//assert(n == 1);
	EventLoop* ioLoop = conn->getLoop();
	ioLoop->queueInLoop(bind(&TcpConnection::connectDestroyed, conn));
}

