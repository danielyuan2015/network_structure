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
	PRINTFUNC;
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
	LOGGING("received data:[%s] size=%d\r\n",buf,size);
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
	//assert(state_ == kDisconnected);
	//test here
	//sockets::shutdownReadWrite(channel_->fd());
	//sockets::close(channel_->fd());
}

void TcpConnection::shutdown()
{
	// FIXME: use compare and swap
	if (state_ == kConnected) {
		setState(kDisconnecting);
		// FIXME: shared_from_this()?
		loop_->runInLoop(bind(&TcpConnection::shutdownInLoop, this));
	}
}

void TcpConnection::shutdownInLoop()
{
	loop_->assertInLoopThread();
	if (!channel_->isWriting()) {
		// we are not writing
		socket_->ShutdownWrite();
	}
}

void TcpConnection::connectEstablished()
{
	PRINTFUNC;
	loop_->assertInLoopThread();
	assert(state_ == kConnecting);
	setState(kConnected);
	channel_->tie(shared_from_this());
	channel_->enableReading();

	connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
	PRINTFUNC;
	loop_->assertInLoopThread();
	if (state_ == kConnected) {
		setState(kDisconnected);
		channel_->disableAll();

		connectionCallback_(shared_from_this());
	}
	channel_->remove();
}
void TcpConnection::handleRead()
{
	char buf[1024] = {0};
	PRINTFUNC;
	loop_->assertInLoopThread();
	//int savedErrno = 0;
	//ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
	ssize_t n = sockets::read(channel_->fd(),buf,1024);
	if (n > 0) {
		//messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
		messageCallback_(shared_from_this(),buf,n);
	} else if (n == 0) {
		handleClose();
	} else {
		//errno = savedErrno;
		//LOG_SYSERR << "TcpConnection::handleRead";
		perror("TcpConnection::handleRead");
		handleError();
	}
}

void TcpConnection::handleWrite()
{
	PRINTFUNC;
}

void TcpConnection::handleClose()
{
	PRINTFUNC;
	loop_->assertInLoopThread();
	//LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
	LOGGING("fd = %d stata = %s\r\n",channel_->fd(),stateToString());
	//assert(state_ == kConnected || state_ == kDisconnecting);	
	if(state_ == kConnected || state_ == kDisconnecting) {
		// we don't close fd, leave it to dtor, so we can find leaks easily.
		setState(kDisconnected);
		channel_->disableAll();

		TcpConnectionPtr guardThis(shared_from_this());
		connectionCallback_(guardThis);
		// must be the last line
		closeCallback_(guardThis);
	}
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