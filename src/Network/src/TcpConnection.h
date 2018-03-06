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

///
/// TCP connection, for both client and server usage.
///
/// This is an interface class, so don't expose too much details.
class TcpConnection
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
	
private:
	enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();
	
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
