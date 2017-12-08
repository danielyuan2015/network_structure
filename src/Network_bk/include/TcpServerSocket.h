/*
 * TcpServerSocket.h
 *
 *  Created on: Aug 01, 2017
 *      Author: honeywell
 */

#ifndef SRC_TCPSERVERSOCKET_H_
#define SRC_TCPSERVERSOCKET_H_

#include "socket.h"
#include <thread>
#include "ScanDriver.h"
#include "DecodeManager.h"
#include "hstring.h"
#include "DataParser.h"
#include "Ipc.h"

#include "ReaderConfiguration.h"

#define BACKLOG 10
#define TCPSOCKETBUFLEN 1024

typedef enum
{
	TCP_SERVER_START_SENDING_IMAGE = TCP_SERVER_EVENT_BASE + 0x1,
	TCP_SERVER_STOP_SENDING_IMAGE = TCP_SERVER_EVENT_BASE + 0x2,		
} TcpServerEventId;

class TcpServerSocket:public cSocket
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
    int ParseExpAndGainData(const char *buff, int len);//later i will move it to other class

	int SocketRead(char *buff,int len);
	int SocketWrite(const char* buff,int len);

private:
	void tcpServerSocketConcurrentThread();
	void tcpServerSocketNoneConcurrentThread();//For sending image stream
	int ProcessDataFromHost(char *buf,int len);
	int ProcessAndSendingImageData(char *buf,int len);
	
	ScanDriver *pScanDriver;
	EventManager *pEventManager;
    DataParser *pDataParser;
    IPC *pIpc;
    READER_CONFIGURATION *pReaderConfig;//later i will move it to other class

	TcpServerType serverType;
	int tcpPort;
	bool tcpSocketIsValid;
	int numOfConnections;
	int remoteSockFd; 	// New Socket file descriptor
	struct sockaddr_in remoteSockAddr;
	std::thread thread;
	std::mutex mtx;
	bool isThreadRunning;
protected:
};
	
#endif /* SRC_TCPSERVERSOCKET_H_ */


