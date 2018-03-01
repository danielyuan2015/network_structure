/*
 *TcpConnection.cpp
 *
 *  Created on: March 01, 2018
 *      Author: Daniel yuan
 */
#include "TcpConnection.h"
#include "logging.h"

#define LOG_TAG "TcpConnection"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

 TcpConnection::TcpConnection(EventLoop* loop,
                             const string& nameArg,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
  : loop_(loop),
    name_(nameArg),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr)
{
	channel_->setReadCallback(
		bind(&TcpConnection::handleRead, this/*, std::placeholders::_1*/));
	channel_->setWriteCallback(
		bind(&TcpConnection::handleWrite, this));
	channel_->setCloseCallback(
		bind(&TcpConnection::handleClose, this));
	channel_->setErrorCallback(
		bind(&TcpConnection::handleError, this));

	LOGGING("Connection:[%s] at fd=%d\r\n",name_.c_str(),sockfd);
	//socket_->setKeepAlive(true);
}

void TcpConnection::handleRead()
{
	LOGGING("handleRead\r\n");
}

void TcpConnection::handleWrite()
{
	LOGGING("handleWrite\r\n");
}

void TcpConnection::handleClose()
{
	LOGGING("handleClose\r\n");
}

void TcpConnection::handleError()
{
	LOGGING("handleError\r\n");
}