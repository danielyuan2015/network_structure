/*
 * Acceptor.h
 *
 *  Created on: March 01, 2018
 *      Author: Daniel Yuan
 */
#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

#include "EventLoop.h"
#include "InetAddress.h"
#include "socket.h"
#include "Channel.h"
#include "SocketOps.h"

using namespace sockets;

class Acceptor
{
public:
	typedef std::function<void (int sockfd,
		const InetAddress&)> NewConnectionCallback;

	Acceptor(EventLoop* loop, 
			const InetAddress& listenAddr, bool reuseport);
	~Acceptor();
	
	//bool listenning() const { return listenning_; }
	void listen();
	
	void setNewConnectionCallback(const NewConnectionCallback& cb);

private:
	void handleRead();
	EventLoop *loop_;
	Socket acceptSocket_;
	Channel acceptChannel_;
	NewConnectionCallback newConnectionCallback_;
};
#endif

