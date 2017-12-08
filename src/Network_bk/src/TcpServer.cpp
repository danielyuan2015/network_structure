/*
 *TcpServer.cpp
 *
 *  Created on: Aug 01, 2017
 *      Author: honeywell
 */

#include "TcpServer.h"
//#include "logging.h"
#include <unistd.h> //fork()
#include <sys/wait.h>

#define LOG_TAG "TcpServer"

volatile int tcpUdpPortVal = 0;

TcpServer::TcpServer(int port):TcpServerPort(port)
{
    //Concurrent server for recving menu command form PC and other devices
    pTcpSocket = new TcpServerSocket(TcpServerPort,TcpServerSocket::TCP_SERVER_TYPE_CONCURRENT);

    //sending image data to PC
    pTcpSocketImage = new TcpServerSocket(TcpServerPort+10,TcpServerSocket::TCP_SERVER_TYPE_NONE_CONCURRENT);

    //TODO:I will put sending decoder data server to CONCURRENT,combine them together
    pTcpDecoderData = new TcpServerSocket(TcpServerPort+20,TcpServerSocket::TCP_SERVER_TYPE_NONE_CONCURRENT);

    //TODO:broadcasting server
    pUdpSocket = new UdpSocket(TcpServerPort);

    isThreadRunning = true;
    isContinusWattingForConnection = true;
}

TcpServer::~TcpServer()
{
    delete pTcpSocket;
    delete pTcpSocketImage;
    delete pTcpDecoderData;
    delete pUdpSocket;
}

void TcpServer::Start()
{
    printf("Tcp Server Start\r\n");
}

void TcpServer::Stop()
{
    printf("Tcp Server Stop\r\n");
}

#if 0
int TcpServer::Run()
{
    pid_t pid;
    pTcpSocket->SetSocketOpt();
    pTcpSocket->Bind();
    pTcpSocket->Listen();

    while(isContinusWattingForConnection) {

        if( (remoteSocketFd = pTcpSocket->Accept()) > 0) { //should block here waitting for tcp connection
            //step 1 :fork
            if((pid = fork()) < 0) {
                printf("fork error\r\n");
                exit(-1);
            } else if(pid == 0) {/* Child process */
                //step 2 :creat a thread here?
                /*if (!ThreadRunning) {
                    log_print(LOG_INFO, LOG_TAG, "TCP thread start\n");
                    ThreadRunning = 1;
                    Thread = std::thread(std::bind(&TcpServer::TcpProcessThread, this));
                    while(1) {

                    }
                    printf("child process finished!\r\n");
                }*/
                //int b =pTcpSocket->GetSocketFd();
                //printf("child process fd:%d!\r\n",b);
                char buf[]="hello world\r\n";
                pTcpSocket->SocketWrite(buf,sizeof(buf));

                pTcpSocket->Close();//close listening socket
                TcpProcessThread();

            } else {/*parent process*/
                printf("Parent process:%d!\r\n",getpid());
                printf("Child process:%d!\r\n",pid);
                //pTcpSocket->Close();
                close(remoteSocketFd);
                while(waitpid(-1, NULL, WNOHANG) > 0);
                printf("parent process finished!\r\n");
                printf("block for accept()\r\n");
            }
        }

    }
}
#else
int TcpServer::Run()
{
    //pTcpSocket->Run();
    //pUdpSocket->Run();
    pTcpSocket->Start();
    pTcpSocketImage->Start();
    pTcpDecoderData->Start();

}
#endif
/*
void TcpServer::TcpProcessThread()
{
    printf("start TcpProcessThread!\r\n");
    while(1) {
        //pTcpSocket->PollingData();
        if(-1 == poll_process()) {
            printf("Child process exit!\r\n");
            break;
        }
    }
    printf("start TcpProcessThread finsihed!\r\n");
}*/

int TcpServer::GetServerPort() const
{
    return TcpServerPort;
}

void TcpServer::SetServerPort(int port)
{
    TcpServerPort = port;
}
/*
int TcpServer::RegisterScanDriver(ScanDriver *scan_driver, EventManager *event_manager)
{
    pTcpSocket->RegisterScanDriver(scan_driver,event_manager);
    pTcpSocketImage->RegisterScanDriver(scan_driver,event_manager);
    pTcpDecoderData->RegisterScanDriver(scan_driver,event_manager);
    return 0;
}

int TcpServer::UnRegisterScanDriver()
{
    pTcpSocket->UnRegisterScanDriver();
    pTcpSocketImage->UnRegisterScanDriver();
    pTcpDecoderData->UnRegisterScanDriver();
    return 0;
}

int TcpServer::RegisterConfiguration(READER_CONFIGURATION *pRConfg)
{
    pTcpSocket->RegisterConfiguration(pRConfg);
    //pTcpSocketImage->RegisterScanDriver(scan_driver,event_manager);
    pTcpDecoderData->RegisterConfiguration(pRConfg);
    return 0;
}

int TcpServer::UnRegisterConfiguration()
{
    pTcpSocket->UnRegisterConfiguration();
    //pTcpSocketImage->UnRegisterScanDriver();
    pTcpDecoderData->UnRegisterConfiguration();
    return 0;
}*/
/*
scan_driver->SetMaximumExposure(readerConfiguration->GetNumber("PDCEMX"));
scan_driver->SetMaxGain(readerConfiguration->GetNumber("PDCGMX"));
scan_driver->SetFixedExposure(readerConfiguration->GetNumber("PDCFEX"));
scan_driver->SetFixedGain(readerConfiguration->GetNumber("PDCFGX"));*/

int StartNetworkServer(int tcpPort)
{
    TcpServer *tp = new TcpServer(tcpPort);
    //tp->RegisterScanDriver(scan_driver,event_manager);
    //tp->RegisterConfiguration(pRConfg);
    tp->Run();
    return 0;
}

int StopNetworkServer()
{
	//tp->Stop();
	//delete tp;
    return 0;
}
#if 0
#define SOCK_SD_LEN 1024
#define _DEBUG
int TcpServer::poll_process(void)
{
    int ret = -1,num = 0,i =0;
    int pros_ret = 0;

    fd_set fdSetRead;
    char buf[SOCK_SD_LEN+1];
    char socket_buf[SOCK_SD_LEN+1];
    struct timeval tm;
    tm.tv_sec = 0;
    tm.tv_usec = 0;

    //socketFd = pTcpSocket->GetSocketFd();
    FD_ZERO(&fdSetRead);
    //FD_SET (fd, &fdSetRead);
    FD_SET (remoteSocketFd, &fdSetRead);

    ret = select(remoteSocketFd + 1, &fdSetRead, NULL, NULL, &tm);
    switch (ret) {
        case 0:
            //printf("time out \r\n");
        break;

        case -1:
            //perror("select error\r\n");
            printf("select error\r\n");
        break;

        default:
            if(FD_ISSET(remoteSocketFd,&fdSetRead)) {
                printf("poll:read socket\r\n");
                //num = recv(socket_fd, socket_buf, 256, 0);
                num = pTcpSocket->SocketRead(socket_buf,256);
                if(num) {
#ifdef _DEBUG
                    printf ("OK: Receviced numbytes = %d\n", num);
                    socket_buf[num] = '\0';
                    printf ("OK: Receviced string is: %s\n", socket_buf);
#endif
                    pTcpSocket->SocketWrite(socket_buf,num);
                } else if(num == 0) { //when num is 0, may receive a FIN form host,need close socket
                    printf("terminated from host!\r\n");
                    //close(socket_fd);
                    //pTcpSocket->Close();
                    close(remoteSocketFd);
                    return -1;
                }
            }
        break;
    }

    return 0;
}
#endif
