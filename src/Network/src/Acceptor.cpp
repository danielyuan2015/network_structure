/*
 * Acceptor.cpp
 *
 *  Created on: March 01, 2018
 *      Author: Daniel Yuan
 */
#include "Acceptor.h"
#include "logging.h"

#define LOG_TAG "Acceptor"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
  : loop_(loop),
    acceptSocket_(sockets::createNonblockingOrDie()),
    acceptChannel_(loop,acceptSocket_.fd())
{
	acceptSocket_.setReuseAddr(true);
	acceptSocket_.setReusePort(reuseport);
	acceptSocket_.Bind(listenAddr);
	acceptChannel_.setReadCallback(
		std::bind(&Acceptor::handleRead, this));
	printf("acceptSocket_ fd = %d\r\n",acceptSocket_.fd());
}
	
Acceptor::~Acceptor()
{
}

void Acceptor::listen()
{
	//loop_->assertInLoopThread();
	//listenning_ = true;
	acceptSocket_.Listen();
	acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
	LOGGING("call handleRead\r\n");
	//loop_->assertInLoopThread();
	InetAddress peerAddr;
	
	int connfd = acceptSocket_.Accept(&peerAddr);
	if (connfd >= 0) {
		// string hostport = peerAddr.toIpPort();
		if (newConnectionCallback_) {
			newConnectionCallback_(connfd,peerAddr);
		} else {
			sockets::close(connfd);
		}
	}
}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback& cb)
{
	newConnectionCallback_ = cb;
}

