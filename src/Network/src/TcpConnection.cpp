/*
 *TcpConnection.cpp
 *
 *  Created on: March 01, 2018
 *      Author: Daniel yuan
 */
#include "TcpConnection.h"
#include "logging.h"
#include "assert.h"

#define LOG_TAG "TcpConnection"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)
#define PRINTFUNC LOGGING("%s\r\n",__func__)
//#define LOG_TRACE std::cout
//#define LOG_DEBUG std::cout

void defaultConnectionCallback(const TcpConnectionPtr& conn)
{
	/*LOG_TRACE << conn->localAddress().toIpPort() << " -> "
	        << conn->peerAddress().toIpPort() << " is "
	        << (conn->connected() ? "UP" : "DOWN");*/
	LOGGING("%s -> %s %s\r\n",conn->localAddress().toIpPort().c_str(),
		conn->peerAddress().toIpPort().c_str(),(conn->connected() ? "UP" : "DOWN"));
	// do not call conn->forceClose(), because some users want to register message callback only.
}

void defaultMessageCallback(const TcpConnectionPtr&,char*buf,int size/*Buffer* buf,Timestamp*/)
{
	PRINTFUNC;
	//buf->retrieveAll();
}
										
 TcpConnection::TcpConnection(EventLoop* loop,
                             const string& nameArg,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
  : loop_(loop),
    name_(nameArg),
    state_(kConnecting),
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
	
TcpConnection::~TcpConnection()
{
	/*LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this
	        << " fd=" << channel_->fd()
	        << " state=" << stateToString();*/
	LOGGING("TcpConnection::dtor[%s] at fd=%d state=%s\r\n",
		name_.c_str(),channel_->fd(),stateToString());	      
	assert(state_ == kDisconnected);
}

void TcpConnection::handleRead()
{
	PRINTFUNC;
}

void TcpConnection::handleWrite()
{
	PRINTFUNC;
}

void TcpConnection::handleClose()
{
	PRINTFUNC;
}

void TcpConnection::handleError()
{
	PRINTFUNC;
}

const char* TcpConnection::stateToString() const
{
	switch (state_) {
		case kDisconnected:
			return "kDisconnected";
		case kConnecting:
			return "kConnecting";
		case kConnected:
			return "kConnected";
		case kDisconnecting:
			return "kDisconnecting";
		default:
			return "unknown state";
	}
}