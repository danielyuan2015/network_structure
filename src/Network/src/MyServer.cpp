/*
 *TcpServerSocket.cpp
 *
 *  Created on: Dec 26, 2017
 *      Author: Daniel yuan
 */
 
//#include "MyServer.h"
//#include <iostream>
//#include <sys/socket.h>
//#include <sys/prctl.h>
//#include <string.h>
//#include <poll.h>
#include <errno.h>
#include <unistd.h>  
//#include <arpa/inet.h>
//#include <sys/wait.h>
//#include <algorithm>
#include "MyServer.h"
#include "realtime.h"
//#include "bmp.h"
#include "logging.h"
//#include "test.h"
//#include "FdManager.h"
//#include "DecoderMenuSettings.h"

//#define _DEBUG

#define LOG_TAG "MyServer"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)
#define PRINTFUNC LOGGING("%s\r\n",__func__);

MyServer::MyServer(EventLoop *loop,
                     const InetAddress &listenAddr,int maxCon)
	:server_(loop,listenAddr,"MyServer"),
	numConnected_(0),
	kMaxConnections_(maxCon)
{
	server_.setConnectionCallback(bind(&MyServer::onConnection,this,placeholders::_1));
	server_.setMessageCallback(bind(&MyServer::onMessage,
		this,placeholders::_1,placeholders::_2,placeholders::_3));
}

MyServer::~MyServer()
{
	PRINTFUNC;
}

void MyServer::start()
{
	server_.start();
}

void MyServer::onConnection(const TcpConnectionPtr& conn)
{
	PRINTFUNC;
	static bool flag = true;
	/*LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
			 << conn->localAddress().toIpPort() << " is "
			 << (conn->connected() ? "UP" : "DOWN");*/
	if (conn->connected()) {
		++numConnected_;
		if (numConnected_ > kMaxConnections_){
			conn->shutdown();
			conn->forceClose();
			//conn->forceCloseWithDelay(3.0);  // > round trip of the whole Internet.
		} else {
			if(true == flag) {
				flag = false;
				hostPtr_ = conn;
			}
		}
	} else {
	  --numConnected_;
	}
	LOGGING("numConnected = %d\r\n",numConnected_);
	//LOG_INFO << "numConnected = " << numConnected_;
}

void MyServer::onMessage(const TcpConnectionPtr& conn,char* buf,int size)
{
	PRINTFUNC;
	std::shared_ptr<TcpConnection> guard;
	guard = hostPtr_.lock();
	string str("hello world!");
	guard->send((char*)str.c_str(),str.size());
}


