/*
 * MyServer.h
 *
 *  Created on: Dec 26, 2017
 *      Author: Daniel Yuan
 */
#ifndef _MYSERVER_H_
#define _MYSERVER_H_

//#include "socket.h"
//#include <thread>
//#include <map>
//#include <mutex>
//#include <vector>
//#include "ScanDriver.h"
//#include "DecodeManager.h"
//#include "hstring.h"
//#include "DataParser.h"
//#include "Ipc.h"
//#include <sys/epoll.h>

#include "EventLoop.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include "TcpServer.h"

using namespace std;

class MyServer
{
public:
	MyServer(EventLoop* loop,
				const InetAddress& listenAddr,int maxCon);
	~MyServer();
	
	void start();
	
	std::weak_ptr<TcpConnection> hostPtr_;;

private:
	void onConnection(const TcpConnectionPtr& conn);
	void onMessage(const TcpConnectionPtr& conn,char* buf,int size);

	//EventLoop *loop_;
	TcpServer server_;
	int numConnected_; // should be atomic_int
	const int kMaxConnections_;	
};

#endif
