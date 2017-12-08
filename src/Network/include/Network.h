/*
 * Network.h
 *
 *  Created on: Aus 01, 2017
 *      Author: honeywell
 */

#ifndef SRC_NETWORK_H_
#define SRC_NETWORK_H_

#include "TcpServerSocket.h"
#include "UdpSocket.h"
#include <thread>
#include "ScanDriver.h"
#include "Ipc.h"

//class TcpSocket;
extern volatile int tcpUdpPortVal;

class Network
{
public:
    Network(int port = 0);
    virtual ~Network();
    int GetServerPort() const;
    void SetServerPort(int);
    void Start();
    void Stop();
    //int poll_process();
    int RegisterScanDriver(ScanDriver *scan_driver, EventManager *event_manager);
    int UnRegisterScanDriver();
    int RegisterConfiguration(READER_CONFIGURATION *pRConfg);
    int UnRegisterConfiguration();
    //int Run();
	
private:

    //bool isContinusWattingForConnection = false;
    TcpServerSocket *pTcpSocket = NULL;
	
	TcpServerSocket *pTcpSocketConfig = NULL; //For recving and sending configuration data 
    TcpServerSocket *pTcpSocketImage = NULL;  //For sending image data to PC host
    TcpServerSocket *pTcpDecoderData = NULL;  //For sending decoder data and Image index to PC host
    
    UdpSocket *pUdpSocket = NULL;
    int  TcpServerPort = -1;
	int  UdpPort = -1;
    //struct sockaddr_in addrRemote;
    bool isThreadRunning = false;
    //int remoteSocketFd;
    //std::thread Thread;

protected:

};

int InitNetworkServer(int tcpPort,int udpPort,ScanDriver *scan_driver, EventManager *event_manager,READER_CONFIGURATION *pRConfg);
int StartNetworkServer();
int StopNetworkServer();

#endif /* SRC_NETWORK_H_ */



