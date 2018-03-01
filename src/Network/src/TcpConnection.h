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
	
	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();

private:
	Channel* channel_;
	EventLoop* loop_;
	Socket* socket_;
	const string name_;
	const InetAddress localAddr_;
	const InetAddress peerAddr_;

};

#endif
