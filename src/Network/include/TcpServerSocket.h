/*
 * TcpServerSocket.h
 *
 *  Created on: Dec 26, 2017
 *      Author: Daniel Yuan
 */

#ifndef SRC_TCPSERVERSOCKET_H_
#define SRC_TCPSERVERSOCKET_H_

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

//#include "ReaderConfiguration.h"

#define BACKLOG 10
#define TCPSOCKETBUFLEN 1024
#define MAX_EVENTS 100
#define MAX_TCP_CONNECTIONS 50

//#define USE_IPC_CLASS

class TcpInterfaceSocket;
class FdManager;
//class DataParser;
#define TCP_SERVER_EVENT_BASE 0x800

typedef enum
{
	//TCP_SERVER_START_SENDING_IMAGE = TCP_SERVER_EVENT_BASE + 0x1,
	TCP_SERVER_STOP_SENDING_DATA = TCP_SERVER_EVENT_BASE + 0x01,
	TCP_SERVER_STOP_SENDING_IMAGE = TCP_SERVER_EVENT_BASE + 0x02,		
} TcpServerEventId;

class TcpServerSocket:public cSocket,cNonCopyable
{
public:
	typedef enum
	{
        //TCP_SERVER_TYPE_NONE_CONCURRENT,
        //TCP_SERVER_TYPE_CONCURRENT,
        TCP_SERVER_TYPE_CONCURRENT = 0,
        TCP_SERVER_TYPE_NONE_CONCURRENT
	} TcpServerType;

	TcpServerSocket(int port = 0,TcpServerType type = TCP_SERVER_TYPE_CONCURRENT);
	virtual ~TcpServerSocket();
	int Open();
	void Close();
	void Close(int fd);
	void Shutdown(int mode);
	void Shutdown(int fd,int mode);

	int SetSocketOpt();
	int Bind();
	int Listen();
	virtual int Accept();
	
	int GetSocketFd();
	int GetRemoteSocketFd();

	int Start();
	int Stop();
    int PollingData();

	int RegisterScanDriver(ScanDriver *scan_driver, EventManager *event_manager);
	int UnRegisterScanDriver();

    int RegisterConfiguration(READER_CONFIGURATION *pRc);//later i will move it to other class
    int UnRegisterConfiguration();//later i will move it to other class

	int SocketRead(char *buff,int len);
	int SocketWrite(const char* buff,int len);
	
	int CheckConnections();
	int CloseAllConnections();
	int CheckFd(int fd);
	
private:
	void ConcurrentProcess();
	void ConcurrentThread();
	void NoneConcurrentThread();//For sending image stream
	int ProcessDataFromHost(char *buf,int len);
	int ProcessAndSendingImageData(char *buf,int len);
	
	ScanDriver *pScanDriver = NULL;
	EventManager *pEventManager = NULL;
    //DataParser *pDataParser = NULL;
    READER_CONFIGURATION *pReaderConfig = NULL;
	TcpInterfaceSocket *pInterfaceSocket = NULL;
	FdManager *pDevFdVec_ = NULL;
	
	TcpServerType serverType;
	int tcpPort;
	//bool tcpSocketIsValid = false;
	bool isThreadRunning = false;
	//int numOfConnections = 0;
	int maxConnections = 0;
	//static int numOfConcurrentServer;
	//int remoteSockFd; 	// New Socket file descriptor
	int connFd = -1;				// connected file descriptor
	int hostFd = -1;
	struct sockaddr_in remoteSockAddr;
	std::thread thread;
	std::mutex wmtx;
	std::mutex rmtx;
	struct epoll_event ev,events[MAX_EVENTS];
	
protected:
};

class TcpInterfaceSocket:public cSocket,cNonCopyable
{
public:
	TcpInterfaceSocket(int maxConn, int port);
	virtual ~TcpInterfaceSocket();
	int Open();
	void Close();
	void Close(int fd);
	void Shutdown(int mode);
	int SocketRead(char *buff,int len);
	int SocketWrite(const char* buff,int len);
	int SendDataAll(const char* buff,int len);
	int SetUpTcpInterfaceSocket();
	int Accept();
	int GetFd();

	int CheckConnections();
	//int InsertFdVevtor(int fd);
	//int DelFdVector(int fd);
	int CheckFd(int fd);
	//void DumpFdVector();
	
	int CloseAllConnections();
	
private:
	struct sockaddr_in connSockAddr;
	int connFd = -1;
	int maxConnections = 0;
	//int numOfConnections = 0;
	int tcpPort = 0;
	//std::map<int,int> connFdMap;
	std::vector<int> connFdVec;
	FdManager *pIfcFdVec_ = NULL;

protected:
};


#endif /* SRC_TCPSERVERSOCKET_H_ */


