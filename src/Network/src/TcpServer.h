/*
 * TcpServerSocket.h
 *
 *  Created on: Dec 26, 2017
 *      Author: Daniel Yuan
 */
#ifndef SRC_TCPSERVER_H_
#define SRC_TCPSERVER_H_

#include "socket.h"
#include <thread>
#include <map>
#include <mutex>
#include <vector>
#include "ScanDriver.h"
//#include "DecodeManager.h"
//#include "hstring.h"
//#include "DataParser.h"
#include "Ipc.h"
#include <sys/epoll.h>

#include "EventLoop.h"
#include "InetAddress.h"

/*class TcpServer:public cSocket,cNonCopyable
{
public:
	TcpServer(int port = 0);
	virtual ~TcpServer();
	int Open();
	void Close();
	void Close(int fd);
	void Shutdown(int mode);
	void Shutdown(int fd,int mode);

	int SetSocketOpt();
	int Bind();
	int Listen();
	virtual int Accept();

	int Start();
	int Stop();

	int RegisterScanDriver(ScanDriver *scan_driver, EventManager *event_manager);
	int UnRegisterScanDriver();

    int RegisterConfiguration(READER_CONFIGURATION *pRc);
    int UnRegisterConfiguration();

	int SocketRead(char *buff,int len);
	int SocketWrite(const char* buff,int len);
	
	int CheckConnections();
	int CloseAllConnections();
	int CheckFd(int fd);
	
private:
	#define BACKLOG 10
	#define TCPSOCKETBUFLEN 1024
	#define MAX_EVENTS 100
	#define MAX_TCP_CONNECTIONS 50
	ScanDriver *pScanDriver_ = NULL;
	EventManager *pEventManager_ = NULL;
    READER_CONFIGURATION *pReaderConfig_ = NULL;
	FdManager *pDevFdVec_ = NULL;

	int tcpPort = 0;
	bool isThreadRunning = false;
	int maxConnections = 0;
	int connFd = -1;				// connected file descriptor
	int hostFd = -1;
	struct sockaddr_in remoteSockAddr;
	std::thread thread;
	std::mutex wmtx;
	std::mutex rmtx;
	struct epoll_event ev,events[MAX_EVENTS];
	
protected:
};*/
using namespace std;

class TcpServer:cNonCopyable
{
public:
	TcpServer(EventLoop* loop,
				const InetAddress& listenAddr,
				const string& nameArg,int maxCon);
	~TcpServer();

private:
	EventLoop* loop_;  // the acceptor loop
	const string hostport_;
	const string name_; //server name
	int maxConnections_;
};

#endif
