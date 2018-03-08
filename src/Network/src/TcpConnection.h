/*
 *TcpConnection.h
 *
 *  Created on: March 01, 2018
 *      Author: Daniel yuan
 */
#ifndef _TCPCONNECTION_H_
#define _TCPCONNECTION_H_

#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "socket.h"

#include <memory> //shared_ptr

using namespace std;
class TcpConnection;

typedef shared_ptr<TcpConnection> TcpConnectionPtr;
typedef function<void()> TimerCallback;
typedef function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef function<void (const TcpConnectionPtr&)> CloseCallback;
typedef function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;
//typedef boost::function<void (const TcpConnectionPtr&,Buffer*,Timestamp)> MessageCallback;
// the data has been read to (buf, len)
typedef function<void (const TcpConnectionPtr&,char*,int)> MessageCallback;

void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr&,char*buf,int size/*Buffer* buf,Timestamp*/);

///
/// TCP connection, for both client and server usage.
///
/// This is an interface class, so don't expose too much details.
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
	/// Constructs a TcpConnection with a connected sockfd
	/// User should not create this object.
	TcpConnection(EventLoop* loop,
				  const string& name,
				  int sockfd,
				  const InetAddress& localAddr,
				  const InetAddress& peerAddr);
	~TcpConnection();
	
	EventLoop* getLoop() const { return loop_; }
	const string& name() const { return name_; }
	const InetAddress& localAddress() const { return localAddr_; }
	const InetAddress& peerAddress() const { return peerAddr_; }
	bool connected() const { return state_ == kConnected; }
  	bool disconnected() const { return state_ == kDisconnected; }

  	void shutdown(); // NOT thread safe, no simultaneous calling

	void setConnectionCallback(const ConnectionCallback& cb)
	{ connectionCallback_ = cb; }

	void setMessageCallback(const MessageCallback& cb)
	{ messageCallback_ = cb; }

	void setWriteCompleteCallback(const WriteCompleteCallback& cb)
	{ writeCompleteCallback_ = cb; }

	/*void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
	{ highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }*/
	/// Internal use only.
	void setCloseCallback(const CloseCallback& cb)
	{ closeCallback_ = cb; }
	

	// called when TcpServer accepts a new connection
	void connectEstablished();	 // should be called only once
	// called when TcpServer has removed me from its map
	void connectDestroyed();  // should be called only once

private:
	enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();

	void shutdownInLoop();
  
	void setState(StateE s) { state_ = s; }
	const char* stateToString() const;

	Channel* channel_;
	EventLoop* loop_;
	Socket* socket_;
	const string name_;
	const InetAddress localAddr_;
	const InetAddress peerAddr_;
	StateE state_;

	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;
	HighWaterMarkCallback highWaterMarkCallback_;
	CloseCallback closeCallback_;

};

#endif
