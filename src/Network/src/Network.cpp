/*
 *Network.cpp
 *
 *  Created on: Aug 01, 2017
 *      Author: honeywell
 */

#include "Network.h"
#include "logging.h"
#include "realtime.h"
#include <unistd.h> //fork()
#include <sys/wait.h>

#define LOG_TAG "Network"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

volatile int tcpUdpPortVal = 0;

IPC *gIpcServerPtr = NULL;
IpcClient *gIpcClientPtr = NULL;
static Network *gNetworkPtr = NULL;

//static int StartListenerThread(Network *pNet);
//static int StopListenerThread();

Network::Network(int port):TcpServerPort(port),UdpPort(port)
{
    //Concurrent server for device and interface connections
    pTcpSocket = new TcpServerSocket(TcpServerPort+30,TcpServerSocket::TCP_SERVER_TYPE_CONCURRENT);

	//For recving and sending configuration data 
	pTcpSocketConfig = new TcpServerSocket(TcpServerPort,TcpServerSocket::TCP_SERVER_TYPE_NONE_CONCURRENT);

    //For sending image data to PC host
    //pTcpSocketImage = new TcpServerSocket(TcpServerPort+10,TcpServerSocket::TCP_SERVER_TYPE_NONE_CONCURRENT);

    //For sending decoder data and Image index to PC host
    //pTcpDecoderData = new TcpServerSocket(TcpServerPort+20,TcpServerSocket::TCP_SERVER_TYPE_NONE_CONCURRENT);

    //TODO:broadcasting server
    //pUdpSocket = new UdpSocket(TcpServerPort);

    //isThreadRunning = true;
    //isContinusWattingForConnection = true;
}

Network::~Network()
{
	if(NULL !=  pTcpSocket)
		delete pTcpSocket;
	if(NULL !=  pTcpSocketConfig)
		delete pTcpSocketConfig;
	if(NULL !=  pTcpSocketImage)
		delete pTcpSocketImage;
	if(NULL !=  pTcpDecoderData)
		delete pTcpDecoderData;
	if(NULL !=  pUdpSocket)
		delete pUdpSocket;
}

void Network::Start()
{
	log_print(LOG_LEVEL,LOG_TAG,"Network Start\r\n");
	if(NULL !=  pTcpSocket)
		pTcpSocket->Start();
	if(NULL !=  pTcpSocketConfig)
		pTcpSocketConfig->Start();
	if(NULL !=  pTcpSocketImage)
		pTcpSocketImage->Start();
	if(NULL !=  pTcpDecoderData)
		pTcpDecoderData->Start();
}

void Network::Stop()
{
	log_print(LOG_LEVEL,LOG_TAG,"Network Stop\r\n");
	if(NULL !=  pTcpSocket)
		pTcpSocket->Stop();
	if(NULL !=  pTcpSocketConfig)
		pTcpSocketConfig->Stop();
	if(NULL !=  pTcpSocketImage)
		pTcpSocketImage->Stop();
	if(NULL !=  pTcpDecoderData)
		pTcpDecoderData->Stop();
}

int Network::GetServerPort() const
{
    return TcpServerPort;
}

void Network::SetServerPort(int port)
{
    TcpServerPort = port;
}

int Network::RegisterScanDriver(ScanDriver *scan_driver, EventManager *event_manager)
{
	if(NULL != pTcpSocket)
		pTcpSocket->RegisterScanDriver(scan_driver,event_manager);
	if(NULL != pTcpSocketConfig)
		pTcpSocketConfig->RegisterScanDriver(scan_driver,event_manager);
	if(NULL != pTcpSocketImage)
		pTcpSocketImage->RegisterScanDriver(scan_driver,event_manager);
	if(NULL != pTcpDecoderData)
		pTcpDecoderData->RegisterScanDriver(scan_driver,event_manager);
    return 0;
}

int Network::UnRegisterScanDriver()
{
	if(NULL != pTcpSocket)
		pTcpSocket->UnRegisterScanDriver();
	if(NULL != pTcpSocketConfig)
		pTcpSocketConfig->UnRegisterScanDriver();
	if(NULL != pTcpSocketImage)
		pTcpSocketImage->UnRegisterScanDriver();
	if(NULL != pTcpDecoderData)	
		pTcpDecoderData->UnRegisterScanDriver();
    return 0;
}

int Network::RegisterConfiguration(READER_CONFIGURATION *pRConfg)
{
	if(NULL != pTcpSocket)
		pTcpSocket->RegisterConfiguration(pRConfg);
	if(NULL != pTcpSocketConfig)
		pTcpSocketConfig->RegisterConfiguration(pRConfg);
	if(NULL != pTcpDecoderData)	
		pTcpDecoderData->RegisterConfiguration(pRConfg);
    return 0;
}

int Network::UnRegisterConfiguration()
{
	if(NULL != pTcpSocket)
		pTcpSocket->UnRegisterConfiguration();
	if(NULL != pTcpSocketConfig)
		pTcpSocketConfig->UnRegisterConfiguration();
	if(NULL != pTcpDecoderData)	
		pTcpDecoderData->UnRegisterConfiguration();
    return 0;
}

int Network::StarThread()
{
	if(!isThreadRunning) {
		isThreadRunning = true;
		//listenerT = std::move(std::thread(&Network::NetworkListenerThread.this));
		listenerT = std::thread(std::bind(&Network::NetworkListenerThread,this));
	}
}

int Network::StopThread()
{
	if(isThreadRunning) {
		isThreadRunning = false;
		cv.notify_one();
		listenerT.join();
	}
}

void Network::NetworkListenerThread()
{
	set_thread_name(__func__);
	std::unique_lock<std::mutex> lck(mtx);
	int count = 0;
	
	LOGGING("***********Start NetworkListenerThread***************\r\n");

	while (isThreadRunning) {
		count++;
		if(cv.wait_for(lck,std::chrono::seconds(5)) == std::cv_status::timeout) {
			LOGGING("[%d]time up!\r\n",count);
		}
	}
	
	LOGGING("***********End NetworkListenerThread***************\r\n");	
}

int InitNetworkServer(int tcpPort,int udpPort,ScanDriver *scan_driver, EventManager *event_manager,READER_CONFIGURATION *pRConfg)
{
	LOGGING("InitNetworkServer\r\n");

#ifdef USE_IPC_CLASS
	gIpcServerPtr = new IPC("/tmp/hf800_rfifo",1024);
	gIpcServerPtr->Starthread();
	gIpcClientPtr = new IpcClient("/tmp/hf800_rfifo",1024);
#endif

	gNetworkPtr = new Network(tcpPort);
	gNetworkPtr->RegisterScanDriver(scan_driver,event_manager);
	gNetworkPtr->RegisterConfiguration(pRConfg);
}

int StartNetworkServer()
{
	LOGGING("StartNetworkServer\r\n");
	//gIpcServerPtr = new IPC("/tmp/hf800_rfifo",1024);
	//gIpcClientPtr = new IpcClient("/tmp/hf800_rfifo",1024);
	//if(NULL!=gIpcServerPtr)
		//gIpcServerPtr->Starthread();
	
	if(NULL!=gNetworkPtr) {
		//gNetworkPtr->RegisterScanDriver(scan_driver,event_manager);
	    //gNetworkPtr->RegisterConfiguration(pRConfg);
		gNetworkPtr->Start();
		gNetworkPtr->StarThread();
	}
    //tp = new Network(tcpPort);
    //tp->RegisterScanDriver(scan_driver,event_manager);
    //tp->RegisterConfiguration(pRConfg);
	//tp->Run();
    return 0;
}

int StopNetworkServer()
{
	LOGGING("StopNetworkServer\r\n");
	//if(NULL!=gIpcServerPtr)
		//gIpcServerPtr->StopThread();
	
	if(NULL!=gNetworkPtr) {
		gNetworkPtr->StopThread();
		gNetworkPtr->Stop();
	    //gNetworkPtr->UnRegisterConfiguration(pRConfg);
		//gNetworkPtr->UnRegisterScanDriver();	
	}
	/*tp->Stop();
	tp->UnRegisterConfiguration();
	tp->UnRegisterScanDriver();
	delete tp;
	
	gIpcServerPtr->StopThread();
	delete gIpcServerPtr;*/
    return 0;
}
#if 0
std::mutex mtx;
std::unique_lock<std::mutex> lck(mtx);
std::condition_variable cv;

static void NetworkListenerThread(bool &exit_thread,Network *pNet)
{
	set_thread_name(__func__);
	
	LOGGING("***********Start NetworkListenerThread***************\r\n");

	
	while (!exit_thread) {
		if(cv.wait_for(lck,std::chrono::seconds(1)) == std::cv_status::timeout) {
			LOGGING("time is up!\r\n");
		}
	}
	
	LOGGING("***********End NetworkListenerThread***************\r\n");
}

static std::thread ipc_thread;
static bool ipc_exit = false;

static int StartListenerThread(Network *pNet)
{
    ipc_exit = false;
	ipc_thread = std::move(std::thread(NetworkListenerThread,std::ref(ipc_exit),pNet));

	return 0;
}

static int StopListenerThread()
{
    ipc_exit = true;
	cv.notify_one();
	ipc_thread.join();

	return 0;
}
#endif