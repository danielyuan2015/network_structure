/*
 * TcpServer.h
 *
 *  Created on: Aus 01, 2017
 *      Author: honeywell
 */

#ifndef SRC_TCPSERVER_H_
#define SRC_TCPSERVER_H_

#include "TcpServerSocket.h"
#include "UdpSocket.h"
#include <thread>
#include "ScanDriver.h"

//class TcpSocket;
extern volatile int tcpUdpPortVal;

class TcpServer
{
public:
    TcpServer(int port = 0);
    virtual ~TcpServer();
    int GetServerPort() const;
    void SetServerPort(int);
    void Start();
    void Stop();
    //int poll_process();

    //int RegisterScanDriver(ScanDriver *scan_driver, EventManager *event_manager);
    //int UnRegisterScanDriver();
    //int RegisterConfiguration(READER_CONFIGURATION *pRConfg);
    //int UnRegisterConfiguration();

    int Run();
private:
    //void TcpProcessThread();
    //bool isContinusWattingForConnection;
    TcpServerSocket *pTcpSocket;
    TcpServerSocket *pTcpSocketImage;
    TcpServerSocket *pTcpDecoderData;
    UdpSocket *pUdpSocket;
    int  TcpServerPort;
    //struct sockaddr_in addrRemote;
    //bool isThreadRunning;
    //int remoteSocketFd;
    //std::thread Thread;

protected:

};
int StartNetworkServer(int tcpPort/*,int udpPort,ScanDriver *scan_driver, EventManager *event_manager,READER_CONFIGURATION *pRConfg*/);
int StopNetworkServer();

#endif /* SRC_TCPSERVER_H_ */



