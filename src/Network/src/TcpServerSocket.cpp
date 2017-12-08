/*
 *TcpServerSocket.cpp
 *
 *  Created on: Aug 01, 2017
 *      Author: honeywell
 */
 
#include "TcpServerSocket.h"
//#include <iostream>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <string.h>
#include <poll.h>
#include <errno.h>

#include <unistd.h>  
#include <arpa/inet.h>
#include <sys/wait.h>
#include <algorithm>

#include "realtime.h"
#include "bmp.h"
#include "logging.h"
#include "test.h"
//#include "DecoderMenuSettings.h"

#define _DEBUG

#define LOG_TAG "TcpServerSocket"
#define LOG_LEVEL LOG_PRINT //directly print in console
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)
//#define LOGGING(...) printf(__VA_ARGS__)

#define SYN 0X16
#define DC1 0X11
#define DC2 0X12
#define DC3 0X13
#define DC4 0X14
#define FE  0XFE
#define LF 0X0A
#define CR 0X0D

#define USE_IPC_CLASS
#define MAXCONNECTIONS 4

extern IPC *gIpcServerPtr;
extern IpcClient *gIpcClientPtr;

static int numOfConcurrentServer = 0;

int StartSendingImageStream(ScanDriver *scan_driver, EventManager *event_manager,TcpServerSocket *tcp_socket);
int StopSendingImageStream(EventManager *event_manager);
int StartSendDecoderData(ScanDriver *scan_driver, EventManager *event_manager,TcpServerSocket *tcp_socket);
int StopSendDecoderData(EventManager *event_manager);
int StartIpcThread(READER_CONFIGURATION *reader_configuration);
int StopIpcThread(void);

int RawToBmp(BYTE *pIn,BYTE *pOut,int width, int height);

TcpServerSocket::TcpServerSocket(int port,TcpServerType type):tcpPort(port),serverType(type)
{
	if((numOfConcurrentServer > 1)&&(TCP_SERVER_TYPE_CONCURRENT == serverType)) {
		printf("Failed init concurrent tcp server,only support 1 server in current system!\r\n");
		return;
	}

	if(serverType == TCP_SERVER_TYPE_CONCURRENT) {
		LOGGING("%s init,type:TCP_SERVER_TYPE_CONCURRENT\r\n",__func__);
		maxConnections = MAXCONNECTIONS;
		numOfConcurrentServer++;
		pInterfaceSocket = new TcpInterfaceSocket(2,55256);	
	} else {
		LOGGING("%s init,type:TCP_SERVER_TYPE_NONECONCURRENT\r\n",__func__);
		maxConnections = 1;
	}
	pDevFdVec_ = new FdManager(maxConnections);

	LOGGING("Max connectiongs is %d\r\n",maxConnections);

	//unsigned char testBuf[] = {"DECSET1a013001:1;."};
	//ParseDecoder(testBuf,sizeof(testBuf));
	//TODO:add heartbeat protocal
}

TcpServerSocket::~TcpServerSocket()
{
	Close();
    this->Stop();
	
	if(serverType == TCP_SERVER_TYPE_CONCURRENT) {
		if(--numOfConcurrentServer < 0)
			numOfConcurrentServer = 0;
		if(NULL != pInterfaceSocket) {
			delete pInterfaceSocket;
			pInterfaceSocket = NULL;			
		}
	}
	if(NULL != pDevFdVec_)
		delete pDevFdVec_;
}

int TcpServerSocket::Open(void)
{
	if(socketFd < 0) {
		LOGGING("%s\r\n",__func__);
		/* Get the Socket file descriptor */  
		if( (socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )  {   
			printf("Failed: Obtain Tcp Socket Despcritor failed,port:[%d]\r\n",tcpPort);
			perror("reason");
			//return;
		} else {
			LOGGING("OK: Obtain Tcp Socket fd sucessfully,sockfd:[%d],port:[%d]\n",socketFd,tcpPort);
		}
		
		/* Fill the local socket address struct */
		socketAddr.sin_family = AF_INET;			// Protocol Family
		socketAddr.sin_port = htons(tcpPort); 			// Port number
		socketAddr.sin_addr.s_addr	= htonl(INADDR_ANY);  // AutoFill local address
		memset (socketAddr.sin_zero,0,8);				// Flush the rest of struct
	}
}

void TcpServerSocket::Close()
{	
	LOGGING("TcpServerSocket::%s\r\n",__func__);

	if(socketFd > 0) {
		shutdown(socketFd,SHUT_RDWR);
		close(socketFd);
		socketFd = -1;
	}
}

void TcpServerSocket::Shutdown(int mode)
{
	if(socketFd > 0) {
		switch(mode) {
			case 0:
				shutdown(socketFd,SHUT_RD);			
				break;
			case 1:
				shutdown(socketFd,SHUT_WR);			
				break;
			case 2:
				shutdown(socketFd,SHUT_RDWR);			
				break;
			default:
				shutdown(socketFd,SHUT_RDWR);		
				break;
		}		
	}	
}

void TcpServerSocket::Close(int fd)
{
	if(fd > 0) {
		shutdown(fd,SHUT_RDWR);
		close(fd);
		pDevFdVec_->Delete(fd);
	}
}

void TcpServerSocket::Shutdown(int fd, int mode)
{
	switch(mode) {
		case 0:
			shutdown(fd,SHUT_RD);			
			break;
		case 1:
			shutdown(fd,SHUT_WR);			
			break;
		case 2:
			shutdown(fd,SHUT_RDWR);			
			break;
		default:
			shutdown(fd,SHUT_RDWR);		
			break;
	}
}

int TcpServerSocket::Bind()
{
	/*	Blind a special Port */
	if( bind(socketFd, (struct sockaddr*)&socketAddr, sizeof(struct sockaddr_in)) == -1 )
	{  
		printf ("ERROR: Failed to bind Port %d.\n",tcpPort);
		printf("errno:%s\r\n",strerror(errno));
		return -1;
	} else {
		printf("OK: Bind the Port %d sucessfully.\n",tcpPort);
		return 0;
	}
	
}

int TcpServerSocket::Listen()
{
	/*	Listen remote connect/calling */
	if(listen(socketFd,BACKLOG) == -1)	
	{  
		printf ("ERROR: Failed to listen Port %d.\n", tcpPort);
		return -1;
	} else {
		printf ("OK: Listening the Port %d sucessfully.\n", tcpPort);
		return 0;
	}	
}

int TcpServerSocket::Accept(void)
{
	int sin_size = sizeof(struct sockaddr_in);
	if(pDevFdVec_ == NULL) {
		return -1;
	}
	/*	Wait a connection, and obtain a new socket file despriptor for single connection */
	if ((connFd = accept(socketFd, (struct sockaddr *)&remoteSockAddr, (socklen_t *__restrict)&sin_size)) == -1)
	{  
		printf ("[%d]ERROR: Obtain new Socket Despcritor error.\n",tcpPort);
		return -1;
		//continue;
	} else {
		printf ("[%d]OK: Server has got connect from %s\n", tcpPort,inet_ntoa(remoteSockAddr.sin_addr));
		printf ("[%d]Ok: Remote socket fd is:%d.\n",tcpPort,connFd);
		//if(serverType == TCP_SERVER_TYPE_CONCURRENT) {
			if(CheckConnections() > 0) {
				LOGGING("Insert connfd!\r\n");
				pDevFdVec_->Insert(connFd);
			} else {
				LOGGING("Max interface reached,can not accept any more connections\r\n");
				Close(connFd);
				connFd = -1;
			}
		//}
		//numOfConnections++;
	}
	pDevFdVec_->DumpAllData();
	return connFd;
}

int TcpServerSocket::SetSocketOpt(void)
{
	int optval = 1;//on
	if( setsockopt( socketFd , SOL_SOCKET, SO_REUSEADDR,( char *)&optval, sizeof( optval ) ) < 0 )
		printf( " set socket SO_REUSEADDR error /n" );
	if( setsockopt( socketFd , SOL_SOCKET, SO_REUSEPORT,( char *)&optval, sizeof( optval ) ) < 0 )
		printf( " set socket SO_REUSEPORT error /n" );
	return 0;
}

int TcpServerSocket::GetSocketFd(void)
{
	return socketFd;
}

int TcpServerSocket::GetRemoteSocketFd(void)
{
	return connFd;
}

int TcpServerSocket::PollingData()
{
	int ret = -1,num = 0;
		
	fd_set fdSetRead;
    //char buf[TCPSOCKETBUFLEN+1];
	char socket_buf[TCPSOCKETBUFLEN+1];
	
	//struct timeval tm;
	//tm.tv_sec = 0;
	//tm.tv_usec = 0;

	FD_ZERO(&fdSetRead);
	FD_SET (connFd, &fdSetRead);
	ret = select(connFd + 1, &fdSetRead, NULL, NULL, NULL);
	switch (ret) {
		case 0:
			//printf("time out \r\n");
		break;

		case -1:
			//perror("select error\r\n");
			printf("select error\r\n");
		break; 
	
		default:
			if(FD_ISSET(connFd,&fdSetRead)) {
				printf("poll:read socket\r\n");
				//num = recv(socket_fd, socket_buf, 256, 0);
				num = SocketRead(socket_buf,256);
				if(num) {
#ifdef _DEBUG					
                    printf ("[%d]OK :Receviced numbytes = %d\n",tcpPort,num);
					socket_buf[num] = '\0';
					printf ("[%d]OK :Receviced string is: %s\n",tcpPort,socket_buf);	
#endif
					switch (serverType) {
						case TCP_SERVER_TYPE_CONCURRENT:
							ProcessDataFromHost(socket_buf,num);
							break;
						case TCP_SERVER_TYPE_NONE_CONCURRENT:
							//if(tcpPort == 55266)
								//pDataParser->ProcessData(socket_buf,num);
							//else
								ProcessAndSendingImageData(socket_buf,num);
							break;
						default:
							break;
					};
                    memset(socket_buf,0,TCPSOCKETBUFLEN+1);
					//SocketWrite(socket_buf,num); //test only,send back data
				} else if(num == 0) { //when num is 0, may receive a FIN form host,need close socket
					printf("[%d] Terminated from host!\r\n",tcpPort);
                    memset(socket_buf,0,TCPSOCKETBUFLEN+1);
					//TODO:need colse or shutdown connection fd here?
					//close(connFd);
					//Close();
					return -1;
				}
			}
		break;
	}
	
		return 0;

}

int TcpServerSocket::SocketRead(char *buff,int len)
{
	std::unique_lock<std::mutex> lck(rmtx);
	return read(connFd, buff, len);	
}

int TcpServerSocket::SocketWrite(const char *buff,int len)
{
	std::unique_lock<std::mutex> lck(wmtx);
	if((send(connFd, buff, len ,0)) == -1) {
		printf("ERROR: Failed to sent string.\n");
		return -1;
	}
	return 0;
}

int TcpServerSocket::Start()
{
	switch (serverType) {
		case TCP_SERVER_TYPE_CONCURRENT: {
			LOGGING("Start TCP_SERVER_TYPE_CONCURRENT\r\n");
			//IPC class
			if(NULL != pReaderConfig ) {
				//pIpc = new IPC("/tmp/hf800_fifo",1024,pReaderConfig);
				//pIpc->Starthread();
			 }
#ifdef USE_IPC_CLASS			
			if (!isThreadRunning) {
				isThreadRunning = true;
				ConcurrentProcess();
			}
#else
			if (!isThreadRunning) {
				isThreadRunning = true;
				thread = std::thread(std::bind(&TcpServerSocket::ConcurrentThread, this));
                //must set as detach thread,as child process may exit anytime
                //thread.detach();
			}
#endif			
			break;
		}
		case TCP_SERVER_TYPE_NONE_CONCURRENT:
			Open();
			if(55266 == tcpPort) {
				LOGGING("register Dataparser class\r\n");
				//pDataParser = new DataParser(pReaderConfig,pScanDriver,pEventManager,this);
			}
			if (!isThreadRunning) {
				isThreadRunning = true;
				thread = std::thread(std::bind(&TcpServerSocket::NoneConcurrentThread, this));
			}			
			break;
		default:
			break;
	}
	//StartSendingImageStream(pScanDriver,pEventManager,this);

}

int TcpServerSocket::Stop()
{
	if(TCP_SERVER_TYPE_CONCURRENT == serverType) {
		LOGGING("Stop TCP_SERVER_TYPE_CONCURRENT\r\n");
#ifdef USE_IPC_CLASS
		char buf[]={0x01,0x02,0x03};
		if (isThreadRunning) {
			isThreadRunning = false;
		}
		gIpcServerPtr->IpcWrite(buf,sizeof(buf));
#else
		if (isThreadRunning) {
			isThreadRunning = false;
			 thread.join();
		}
#endif		
    } else if(TCP_SERVER_TYPE_NONE_CONCURRENT == serverType) {
		if (isThreadRunning) {
			isThreadRunning = false;
	        //if(TCP_SERVER_TYPE_NONE_CONCURRENT == serverType)
	        thread.join();
	    }
		Close();
	}
    //child process is exiting...
   /* if(childProcessNeedExit)
        thread.join();*/
	//StopSendingImageStream();
}

int TcpServerSocket::CheckConnections()
{
	//return ((numOfConnections < maxConnections)?1:-1);
	return ((pDevFdVec_->GetTotalCount() < maxConnections)?1:-1);
}

int TcpServerSocket::CheckFd(int fd)
{
	if(pDevFdVec_->Find(fd)) {
		pDevFdVec_->Delete(fd);
	}
	pDevFdVec_->DumpAllData();
}

int TcpServerSocket::CloseAllConnections()
{
	LOGGING("%s\r\n",__func__);
	int fd = -1;
	for(int i =0;i<pDevFdVec_->GetTotalCount();i++) {
		fd = pDevFdVec_->Get(i);
		LOGGING("fd:%d\r\n",fd);
		shutdown(fd,SHUT_RDWR);
		close(fd);
	}
	pDevFdVec_->ClearAll();
}

#if 0
//this thread for sending image stream
void TcpServerSocket::NoneConcurrentThread(void)
{
	set_thread_name(__func__);
	//set_sched_fifo(50);// test here do we need tead-time thread?
	
	LOGGING("*****Start [%s]*****\r\n",__func__);
	SetSocketOpt();
	Bind();
	Listen();
	
	while(isThreadRunning) {
		if(Accept() > 0) { //should block here waitting for tcp connection
			//wait signale here
			while(1) {
                if( -1 == PollingData() ) {
					printf("exit!\r\n");
					break;	
				}		
			}
			//close(connFd);
			Shutdown(connFd,2);
			close(connFd);
			LOGGING("[NoneConcurrentThread] begine wait for next connection,port:[%d]:\r\n",tcpPort);
		}
	}
	close(socketFd);
	LOGGING("*****End [%s]*****\r\n",__func__);
}
#endif
void TcpServerSocket::NoneConcurrentThread(void)
{
	LOGGING("*****Start [%s]*****\r\n",__func__);
	
	set_thread_name(__func__);
	int nfds = -1,connFd = -1,tmpSockFd=-1;
	int num = -1;
	char rBuff[1024];
	
	Open();
	SetSocketOpt();
	Bind();
	Listen();
	
	int epfd = epoll_create(256);
	
	add_event(epfd,socketFd,EPOLLIN|EPOLLET); //add event for PCconnection
	
	while(isThreadRunning) {
		nfds = epoll_wait(epfd,events,MAX_EVENTS,500);
		for(int i=0;i<nfds;++i) {
			if(events[i].data.fd == socketFd) { //wait for Pc client connections
				printf("[NoneConurrent epoll]:Server begin accept connections!\r\n");
				connFd = Accept();
				printf("[NoneConurrent epoll]:connfd:%d\r\n",connFd);
				if(connFd < 0) {
					perror("[NoneConurrent epoll]:Server Get connection fd failed\r\n");
					continue;
				} else {
					add_event(epfd,connFd,EPOLLIN|EPOLLET);
					connFd = -1;	
				}	
			} else if(events[i].events&EPOLLIN) { //read
				tmpSockFd = events[i].data.fd;
				printf("[NoneConurrent epoll]:Poll in sockfd:%d\r\n",tmpSockFd);
				if ( (num = read(tmpSockFd, rBuff, 512)) < 0) {
					if (errno == ECONNRESET) {
						close(tmpSockFd);
						delete_event(epfd,tmpSockFd,EPOLLIN|EPOLLET);
					} else
						printf("[NoneConurrent epoll]:readline error\r\n");
				} else if (num == 0) {
					printf("[NoneConurrent epoll]:Socket terminated from host!\r\n");
					delete_event(epfd,tmpSockFd,EPOLLIN|EPOLLET);
					//check and delete fd
					CheckFd(tmpSockFd);
					//close and shutdown
					shutdown(tmpSockFd,SHUT_RDWR);	
					close(tmpSockFd);
				} else {
					printf("[epoll]:read %d bytes:[%s]\r\n",num,rBuff);
					//process data
					ProcessAndSendingImageData(rBuff,num);
					memset(rBuff,0,num);							
				}
				//if ( (tmpSockFd = events[i].data.fd) < 0)
					//continue;
				//printf("current sockfd:%d\r\n",tmpSockFd);
			}
		}
	}
	printf("[epoll]:exit epoll\r\n");
	delete_event(epfd,socketFd,EPOLLIN|EPOLLET);

	CloseAllConnections();
	Close(); //close server socketfd

	LOGGING("*****End [%s]*****\r\n",__func__);
}
void TcpServerSocket::ConcurrentProcess(void)
{
	LOGGING("Start %s\r\n",__func__);
	
	int tmpSockFd = -1,ipcFd = -1,ifcFd = -1;
	int num = -1,nfds = 0;
	char buff[1024];
	char IPCbuff[1024];
	pid_t pid;
	
	/*if(NULL == gIpcClientPtr)
		return;
	if(-1 == gIpcClientPtr->TryConnect())
		return;*/

	/*SetSocketOpt();
	Bind();
	Listen();
	
	pInterfaceSocket->Open();
	pInterfaceSocket->SetUpTcpInterfaceSocket();*/

	if((pid = fork()) < 0) {
		printf("[epoll]:fork error\r\n");
		_exit(-1);
	} else if(pid == 0) {/* Child process */
		prctl(PR_SET_PDEATHSIG, SIGHUP);//kill this process if father process exit unexpectly!
		
		if(NULL == gIpcClientPtr)
			return;
		if(-1 == gIpcClientPtr->TryConnect())
			return;

		Open();
		SetSocketOpt();
		Bind();
		Listen();
		
		pInterfaceSocket->Open();
		pInterfaceSocket->SetUpTcpInterfaceSocket();
		
		int epfd = epoll_create(256);
		ifcFd = pInterfaceSocket->GetFd();
		ipcFd = gIpcClientPtr->GetFd();
		
		add_event(epfd,socketFd,EPOLLIN|EPOLLET); //add event for device connection
		add_event(epfd,ifcFd,EPOLLIN|EPOLLET); //add event for net interface
		add_event(epfd,ipcFd,EPOLLIN|EPOLLET); //add event for ipc
		
		printf("[epoll]:Starting epoll,current process id:%d!\r\n",getpid());

		while(isThreadRunning) {
			
			nfds = epoll_wait(epfd,events,MAX_EVENTS,500);
			
			for(int i=0;i<nfds;++i) {
				
		        if(events[i].data.fd == socketFd) { //wait for device client connections
					printf("[epoll]:Device server begin accept connections!\r\n");
					connFd = Accept();
					printf("[epoll]:connfd:%d\r\n",connFd);
					if(connFd < 0) {
						perror("[epoll]:Device server Get connection fd failed\r\n");
						continue;
					} else {
						//setnonblocking(connfd);
						//char *str = inet_ntoa(remoteSockAddr.sin_addr);
						//printf("[epoll]:accapt a connection from %s \r\n",str);
						add_event(epfd,connFd,EPOLLIN|EPOLLET);
						connFd = -1;	
					}		
		        } else if(events[i].data.fd == ifcFd) { //wait for interface client connections
					printf("[epoll]:Tcp interface server begin accept connections!\r\n");
		        	//Net Interface concurrent acceptance
	                connFd = pInterfaceSocket->Accept();
					//printf("[epoll]:interface server:connfd:%d\r\n",connFd);
	                if(connFd < 0) {
	                    perror("[epoll]:Tcp interface server Get connection fd failed\r\n");
						continue;
	                    //exit(-1);
	                } else {
		                //char *str = inet_ntoa(remoteSockAddr.sin_addr);
						//printf("[epoll]:accapt a connection from %s \r\n",str);
						add_event(epfd,connFd,EPOLLIN|EPOLLET);
						connFd = -1; //set to defalt;
					}
				} else if(events[i].events&EPOLLIN) { //read
					tmpSockFd = events[i].data.fd;
					printf("[epoll]:current sockfd:%d\r\n",tmpSockFd);

					if(tmpSockFd == ipcFd) {
						//probably sending interface data to host
						printf("[epoll]:IPC EPOLLIN\r\n");
						num = gIpcClientPtr->SocketRead(IPCbuff, 512);
						//num = gIpcPtr->IpcRead(buff, 512);
						printf("[epoll]:IPC EPOLLIN read %d bytes: %s \r\n",num,IPCbuff);
						if(num = 3) { //TODO:change the protocol
							if((0x01 == IPCbuff[0])&&(0x02==IPCbuff[1])&&(0x03==IPCbuff[02])) {
								printf("[epoll]:gonna exit this process \r\n");
								isThreadRunning = false;
							}
						}
						//SocketWrite(IPCbuff,num);
						//modify_event(epfd,tmpSockFd,EPOLLOUT|EPOLLET);
						memset(IPCbuff,0,num);
					} else {
						printf("[epoll]:Socket EPOLLIN\r\n");
		                if ( (num = read(tmpSockFd, buff, 512)) < 0) {
		                    if (errno == ECONNRESET) {
		                        close(tmpSockFd);
								delete_event(epfd,tmpSockFd,EPOLLIN|EPOLLET);
		                    } else
								printf("[epoll]:readline error\r\n");
		                } else if (num == 0) {
							printf("[epoll]:socket terminated from host!\r\n");
							delete_event(epfd,tmpSockFd,EPOLLIN|EPOLLET);
							//checking if it's device fd
							CheckFd(tmpSockFd);
							//checking if it's interface fd
							pInterfaceSocket->CheckFd(tmpSockFd);
							/*if(pInterfaceSocket->CheckFd(tmpSockFd)) {
								printf("[epoll]:yes!it's interface fd:%d,delete it!\r\n",tmpSockFd);
								pInterfaceSocket->DelFdVector(tmpSockFd);
								pInterfaceSocket->DumpFdVector();
							}*/
							shutdown(tmpSockFd,SHUT_RDWR);	
							close(tmpSockFd);
		                } else {
							printf("[epoll]:read %d bytes:[%s]\r\n",num,buff);
							//notify IPC
							//add_event(epfd,IpcFd,EPOLLOUT|EPOLLET);
							//gIpcReadPtr->IpcWrite(buff,num);
							gIpcClientPtr->SocketWrite((const char*)buff, num);
							//modify_event(epfd,IpcFd,EPOLLOUT|EPOLLET);
							//gIpcPtr->IpcWrite(buff,num);
							//add_event(epfd,gIpcPtr->GetFd(),EPOLLOUT|EPOLLET);
							//modify_event(epfd,tmpSockFd,EPOLLOUT);
							memset(buff,0,num);							
						}
					}
                	//if ( (tmpSockFd = events[i].data.fd) < 0)
                    	//continue;
					//printf("current sockfd:%d\r\n",tmpSockFd);

            	} else if(events[i].events&EPOLLOUT) { //write
	                tmpSockFd = events[i].data.fd;
					
					if(tmpSockFd == ipcFd) {
						printf("[epoll]:IPC EPOLLOUT\r\n");
						//gIpcPtr->IpcWrite(buff,num);
						//delete_event(epfd,IpcFd,EPOLLOUT|EPOLLET);
						//modify_event(epfd,IpcFd,EPOLLIN|EPOLLET);
						memset(buff,0,num);
					} else {
						printf("[epoll]:Socket EPOLLOUT\r\n");
		                write(tmpSockFd, buff, num);
						modify_event(epfd,tmpSockFd,EPOLLIN|EPOLLET);
						memset(buff,0,num);				
					}
	            }

			}
			
		}
		printf("[epoll]:exit epoll\r\n");
		//close(ipcFd);
		delete_event(epfd,ipcFd,EPOLLIN|EPOLLET);
		delete_event(epfd,ifcFd,EPOLLIN|EPOLLET);
		delete_event(epfd,socketFd,EPOLLIN|EPOLLET);
		
		pInterfaceSocket->CloseAllConnections();
		pInterfaceSocket->Close();
		
		gIpcClientPtr->TryDisconnect(); //close ipc client socketfd
		Close(); //close concurrent server socketfd
		
		printf("end child process\r\n");
		printf("child exit!\r\n");
		_exit(0);
	} else {/*parent process*/
		printf("Parent process id:%d!\r\n",getpid());
		printf("Child process id:%d!\r\n",pid);
		waitpid(-1,NULL,WNOHANG);
		//printf("parent process finished!\r\n");
		//printf("block for accept()\r\n");
		//while(isThreadRunning) {		
		//}
	}
	//printf("*****End [%s]*****\r\n",__func__);
	LOGGING("End %s\r\n",__func__);
}

void TcpServerSocket::ConcurrentThread(void)
{
	set_thread_name(__func__);
	LOGGING("Start %s\r\n",__func__);
	
	int tmpSockFd = -1,ipcFd = -1,ifcFd = -1;
	int num = -1,nfds = 0;
	char buff[1024];
	char IPCbuff[1024];
	
	Open();
	SetSocketOpt();
	Bind();
	Listen();
		
	pInterfaceSocket->Open();
	pInterfaceSocket->SetUpTcpInterfaceSocket();
		
	int epfd = epoll_create(256);
	ifcFd = pInterfaceSocket->GetFd();
	//ipcFd = gIpcClientPtr->GetFd();
		
	add_event(epfd,socketFd,EPOLLIN|EPOLLET); //add event for device connection
	add_event(epfd,ifcFd,EPOLLIN|EPOLLET); //add event for net interface
	//add_event(epfd,ipcFd,EPOLLIN|EPOLLET); //add event for ipc
		
	printf("[epoll]:Starting epoll,current process id:%d!\r\n",getpid());

	while(isThreadRunning) {

		nfds = epoll_wait(epfd,events,MAX_EVENTS,500);

		for(int i=0;i<nfds;++i) {
			
			if(events[i].data.fd == socketFd) { //wait for device client connections
				printf("[epoll]:Device server begin accept connections!\r\n");
				connFd = Accept();
				printf("[epoll]:connfd:%d\r\n",connFd);
				if(connFd < 0) {
					perror("[epoll]:Device server Get connection fd failed\r\n");
					continue;
				} else {
					char *str = inet_ntoa(remoteSockAddr.sin_addr);
					printf("[epoll]:accapt a connection from %s \r\n",str);
					add_event(epfd,connFd,EPOLLIN|EPOLLET);
					connFd = -1;	
				}		
			} else if(events[i].data.fd == ifcFd) { //wait for interface client connections
				printf("[epoll]:Tcp interface server begin accept connections!\r\n");
				//Net Interface concurrent acceptance
				connFd = pInterfaceSocket->Accept();
				//printf("[epoll]:interface server:connfd:%d\r\n",connFd);
				if(connFd < 0) {
					perror("[epoll]:Tcp interface server Get connection fd failed\r\n");
					continue;
				} else {
					add_event(epfd,connFd,EPOLLIN|EPOLLET);
					connFd = -1; //set to defalt;
				}
			} else if(events[i].events&EPOLLIN) { //read
				tmpSockFd = events[i].data.fd;
				printf("[epoll]:current sockfd:%d\r\n",tmpSockFd);

				printf("[epoll]:Socket EPOLLIN\r\n");
				if ( (num = read(tmpSockFd, buff, 512)) < 0) {
					if (errno == ECONNRESET) {
						close(tmpSockFd);
						delete_event(epfd,tmpSockFd,EPOLLIN|EPOLLET);
					} else
						printf("[epoll]:readline error\r\n");
				} else if (num == 0) {
					printf("[epoll]:socket terminated from host!\r\n");
					delete_event(epfd,tmpSockFd,EPOLLIN|EPOLLET);
					//checking if it's device fd
					//CheckFd(tmpSockFd);
					//checking if it's interface fd
					pInterfaceSocket->CheckFd(tmpSockFd);
					//close and shutdown
					shutdown(tmpSockFd,SHUT_RDWR);	
					close(tmpSockFd);
				} else {
					printf("[epoll]:read %d bytes:[%s]\r\n",num,buff);
					//notify IPC
					//add_event(epfd,IpcFd,EPOLLOUT|EPOLLET);
					//gIpcReadPtr->IpcWrite(buff,num);
					//gIpcClientPtr->SocketWrite((const char*)buff, num);
					//modify_event(epfd,IpcFd,EPOLLOUT|EPOLLET);
					//gIpcPtr->IpcWrite(buff,num);
					//add_event(epfd,gIpcPtr->GetFd(),EPOLLOUT|EPOLLET);
					//modify_event(epfd,tmpSockFd,EPOLLOUT);
					memset(buff,0,num);							
				}
				//if ( (tmpSockFd = events[i].data.fd) < 0)
					//continue;
				//printf("current sockfd:%d\r\n",tmpSockFd);
			} else if(events[i].events&EPOLLOUT) { //write
				tmpSockFd = events[i].data.fd;
				printf("[epoll]:Socket EPOLLOUT\r\n");
				//write(tmpSockFd, buff, num);
				//modify_event(epfd,tmpSockFd,EPOLLIN|EPOLLET);
				memset(buff,0,num);				
			}
		}	
	}
		printf("[epoll]:exit epoll\r\n");

		//delete_event(epfd,ipcFd,EPOLLIN|EPOLLET);
		delete_event(epfd,ifcFd,EPOLLIN|EPOLLET);
		delete_event(epfd,socketFd,EPOLLIN|EPOLLET);
		
		pInterfaceSocket->CloseAllConnections();
		pInterfaceSocket->Close();
		

		//gIpcClientPtr->TryDisconnect(); //close ipc client socketfd
		CloseAllConnections();
		Close(); //close concurrent server socketfd
		
		//printf("end child process\r\n");
		//printf("child exit!\r\n");
		//_exit(0);	
		LOGGING("End %s\r\n",__func__);
}

void LittleEndien4bytes(unsigned int x, char *ptr) {*ptr++ = x & 0xff; x >>= 8; *ptr++ = x & 0xff; x >>= 8; *ptr++ = x & 0xff; x >>= 8; *ptr++ = x & 0xff;}
void LittleEndien2bytes(unsigned int x, char *ptr) {*ptr++ = x & 0xff; x >>= 8; *ptr++ = x & 0xff;}

static void SendPgmHeader(int X, int Y, int size, char *comment,TcpServerSocket *tp)
{
	// Create common header first
	char hdr[2048] ={0};
	/*if (comment[0] == '#') 
		sprintf((char *)(hdr), "\x16\xfe....\rIMGSHP\x1dP5\n%s%d %d\n255\n",comment,X,Y);
	else 
		sprintf((char *)(hdr), "\x16\xfe....\rIMGSHP\x1dP5\n#%s%d %d\n255\n",comment,X,Y);*/
	sprintf((char *)(hdr), "\x16\xfe....\rIMGSHP2P0L%dR%dB0T0M8D10S6F\x1F%s\x1d",X,Y,comment);	
	unsigned int Size = strlen(hdr);
	/*printf("size is %d\r\n",Size);
	for(int i=0;i<Size;i++) {
		printf("%x ",hdr[i]);
	}
	printf("\r\n");*/
	LittleEndien4bytes(Size-7+size, &hdr[2]);
	tp->SocketWrite(hdr, Size);
}

void SendPgmImage(int X, int Y,int size, char *img, char *comment,TcpServerSocket *tp)
{
	SendPgmHeader(X, Y, size,comment,tp);
	tp->SocketWrite(img,size);
}

static unsigned char *buf_capture_image = NULL;
static unsigned char *pBmp = NULL;
static int buf_capture_image_width = 0;
static int buf_capture_image_height = 0;
static uint64_t  buf_capture_time_stamp = 0;
# if 0
static void SendImage(DataEvent<EventDataDecode> *pDecEvent,TcpServerSocket *tcp_socket)
{
	char commentLine[2048]={0};
	
	if(buf_capture_image) {
		/*Event *pEvent;
		EventQueue<Event*>* eventQueue;
		eventQueue = event_manager->CreateEventQueue(EVENT_QUEUE_MAX_SIZE,
					{ DECODE_EVENT_NEW_DECODE });
		
		if(true == eventQueue->TryWait(pEvent)) {
			printf("***Barcode dected***\r\n");
		}*/
		//int m;
		//int image_length = buf_capture_image_width*buf_capture_image_height;
		//unsigned char *image;
		//memset(commentLine,0,2048);
		/*if(NULL != pDecEvent) {
			EventDataDecode *pData = (EventDataDecode *)pDecEvent->GetData();
			//pData->result.MainBounds.corners[0]
			//pData->result.Length
			sprintf(commentLine,"%s\x1f%d\x1f%d\x1f%d\x1f%d\x1f%d\x1f%d\x1f%d\x1f%d\x1f%lld\x1f%lld",pData->result.Data,\
			pData->result.MainBounds.corners[0].x,pData->result.MainBounds.corners[0].y,\
			pData->result.MainBounds.corners[1].x,pData->result.MainBounds.corners[1].y,\
			pData->result.MainBounds.corners[2].x,pData->result.MainBounds.corners[2].y,\
			pData->result.MainBounds.corners[3].x,pData->result.MainBounds.corners[3].y,\
			buf_capture_time_stamp,pData->decodedTimestamp\
			);
			//sprintf(commentLine,"1:%ld,2:%ld",buf_capture_time_stamp,pData->decodedTimestamp);
			//printf("recall:%s\r\n",commentLine);
		} else {
			//sprintf(commentLine,"NULL\x1f\x1f\x1f\x1f\x1f\x1f\x1f\x1f\x1f%lld\x1f",\
			//buf_capture_time_stamp);
		}*/
		int len = RawToBmp(buf_capture_image,pBmp,buf_capture_image_width,buf_capture_image_height);
		//printf("len = %d\r\n",len);
		if(len)
			SendPgmImage(buf_capture_image_width,buf_capture_image_height,len,(char *)pBmp,commentLine,tcp_socket);
		//event_manager->DestroyEventQueue(eventQueue);
	}
}

static void tcp_capture_image(unsigned char *image, int width, int height,uint64_t timeStamp,DataEvent<EventDataDecode> *pDecEvent,TcpServerSocket *tcp_socket)
{
	if(buf_capture_image)
		return;

	buf_capture_image_width = width;
	buf_capture_image_height = height;
	buf_capture_image = image;
	buf_capture_time_stamp = timeStamp;
	SendImage(pDecEvent,tcp_socket);
	buf_capture_image = NULL;
}

static bool tcp_capture_image_empty(void)
{
	return buf_capture_image == NULL;
}
#endif
static void ImageLiveTask(bool &exit_thread, ScanDriver *scan_driver, EventManager *event_manager,TcpServerSocket *tcp_socket)
{
	set_thread_name(__func__);

	printf("*************ImageLiveTask*****************\r\n");
#if 0	
	int capture_count = 0;
	uint64_t capture_image_timestamp = 0;
	uint64_t tcp_image_timestamp = 0;
	Image tcp_image;
	DataEvent<EventDataDecode> *pDecEvent = NULL;
	
	int image_length =640*480*2;//temp fixed size
	pBmp = (unsigned char*)malloc(image_length);
	if(!pBmp) {
		syslog(LOG_ERR, "ERROR allocating memory (%d)\n", image_length);
		return ;
	}
	
	Event *pEvent;
	EventQueue<Event*>* eventQueue;
	eventQueue = event_manager->CreateEventQueue(EVENT_QUEUE_MAX_SIZE,
				{ SCAN_DRIVER_NEW_IMAGE_AVAILABLE, TCP_SERVER_STOP_SENDING_IMAGE/*,DECODE_EVENT_NEW_DECODE*/ });
	event_manager->DumpLists();
	
	while(!exit_thread) {
		//printf("b w\r\n");
        eventQueue->Wait(pEvent);
		//printf("e w\r\n");
		switch(pEvent->GetId()) {
            case SCAN_DRIVER_NEW_IMAGE_AVAILABLE: {
				//printf("IMAGE_AVAILABLE\r\n");
				uint64_t imageTimeStamp = scan_driver->GetNewerImage(capture_image_timestamp);
                if(imageTimeStamp == 0) {
					delete pEvent;
                    //std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
				capture_image_timestamp = imageTimeStamp;
				Image image;
				if(scan_driver->LockBuffer(capture_image_timestamp) &&
                        scan_driver->GetImageInfo(capture_image_timestamp, &image)) {
					if(tcp_capture_image_empty()) {
						scan_driver->UnlockBuffer(tcp_image_timestamp);
						tcp_image_timestamp = capture_image_timestamp;
						tcp_image.buffer = image.buffer;
						tcp_image.width = image.width;
						tcp_image.height = image.height;
						tcp_image.timestamp_us = capture_image_timestamp;
					} else {
						scan_driver->UnlockBuffer(capture_image_timestamp);
					}
					//trying get barcode data
					/*if (true == eventQueue->TryWait(pEvent)) {	
						if(DECODE_EVENT_NEW_DECODE == pEvent->GetId()) {
							pDecEvent = (DataEvent<EventDataDecode>*)pEvent;
							//printf("***Barcode dected2***\r\n");
							//pDecEvent->GetData();
						} else 
							pDecEvent = NULL; 
					} else 
						pDecEvent = NULL; */
					
					if(tcp_image.buffer)
						tcp_capture_image(
								tcp_image.buffer,
								tcp_image.width, tcp_image.height,capture_image_timestamp,pDecEvent,tcp_socket);
					
					/*struct capture_status status;
					if (scan_driver.GetStatus(&status))
					{
						wss_capture_status(status.exposure, status.vmax);
					}*/

				}
				break;
			}
			case TCP_SERVER_STOP_SENDING_IMAGE:{
				printf("***TCP_SERVER_STOP_SENDING_IMAGE***\r\n");
				//exit_thread = true;
				break;
			}
		    default:
                break;
		}
		delete pEvent;
	}
	
	printf("***********END ImageLiveTask***************\r\n");
	scan_driver->UnlockBuffer(tcp_image_timestamp);
	free(pBmp);
	event_manager->DestroyEventQueue(eventQueue);
#endif
}

static void SendDecoderDataTask(bool &exit_thread, ScanDriver *scan_driver, EventManager *event_manager,TcpServerSocket *tcp_socket)
{
	printf("*************SendDecoderDataTask*****************\r\n");
#if 0	
	DataEvent<EventDataDecode> *pDecEvent = NULL;
	EventDataDecode *pData = NULL;
	Event *pEvent;
	EventQueue<Event*>* eventQueue;
	eventQueue = event_manager->CreateEventQueue(EVENT_QUEUE_MAX_SIZE,
				{ DECODE_EVENT_NEW_DECODE,TCP_SERVER_STOP_SENDING_DATA });
	//event_manager->DumpLists();
	char commentLine[1028]={0};
	char hdr[2048]={0};
	
	while(!exit_thread) {
		//printf("b w\r\n");
		eventQueue->Wait(pEvent);
		//printf("e w\r\n");
		switch(pEvent->GetId()) {
			case DECODE_EVENT_NEW_DECODE: {
				//printf("DECODE_EVENT_NEW_DECODE\r\n");
				pDecEvent = (DataEvent<EventDataDecode>*)pEvent;
				if(NULL != pDecEvent) {
					pData = (EventDataDecode *)pDecEvent->GetData();
					sprintf(commentLine,"%s\x1f%d\x1f%d\x1f%d\x1f%d\x1f%d\x1f%d\x1f%d\x1f%d\x1f%d\x1f%d\x1f%d",pData->result.Data,\
					pData->result.MainBounds.corners[0].x,pData->result.MainBounds.corners[0].y,\
					pData->result.MainBounds.corners[1].x,pData->result.MainBounds.corners[1].y,\
					pData->result.MainBounds.corners[2].x,pData->result.MainBounds.corners[2].y,\
					pData->result.MainBounds.corners[3].x,pData->result.MainBounds.corners[3].y,\
					pData->total_decode_cnt,pData->success_decode_cnt,pData->decode_time/*buf_capture_time_stamp,pData->decodedTimestamp*/\
					);
				
					sprintf((char *)(hdr), "\x16\xfe....\rDECRES2P0\x1F%s\x1d",commentLine);
					unsigned int Size = strlen(hdr);
					LittleEndien4bytes(Size-7, &hdr[2]);
					tcp_socket->SocketWrite(hdr,Size);	
				} else {
					//sprintf(commentLine,"NULL\x1f\x1f\x1f\x1f\x1f\x1f\x1f\x1f\x1f%lld\x1f",\
					//buf_capture_time_stamp);
				}

				break;
			}
			case TCP_SERVER_STOP_SENDING_DATA: {
				printf("***DECODE_EVENT_NEW_DECODE***\r\n");
				break;	
			}
		    default:
                break;		
		}
		delete pEvent;
	}
	event_manager->DestroyEventQueue(eventQueue);
#endif
	printf("***********END SendDecoderDataTask***************\r\n");
}



int TcpServerSocket::ProcessDataFromHost(char *buf,int len)
{
	//pleased be noted that we are in a totally different process
	//not the main process!!!
	//so we must use IPC method notify other process

	//char *p;
	//int ret = -1;
	//char commentLine[200];

	if(NULL == buf)
		return -1;
#if 1
    //if((buf[0] == 'D')&&(buf[1] == 'E')){
		//notify main process
        //write(pipefd[1], buf, len);
       //IPC *wPIpc = new IPC(1024,pReaderConfig);
           // wPIpc->IpcWrite(buf,len);
        //delete wPIpc;
        printf("process id:%d!\r\n",getpid());
        //pIpc->IpcWrite( buf, len);
		//gIpcPtr->IpcWrite( buf, len);

		/*if (true == ParseDecoder((unsigned char*)buf,len)) {
			printf("parse data successful\r\n");
		}*/
    //}
#endif
	return 0;
}

int TcpServerSocket::ProcessAndSendingImageData(char *buf,int len)
{
	//char *p;
	//int ret = -1;
	//char commentLine[200];

	if(NULL == buf)
		return -1;
	if((buf[0] == SYN)&&(buf[1] == DC1)) {
		switch(buf[2]) {
			case 0X01:{
				//start sending image stram thread
				//step 1: checking pScanDriver and pEventManager
				if((NULL == pScanDriver)||(NULL == pEventManager))
					break;
				printf("begin sending image thread\r\n");
				//step 2:Creat a thread for sending image
				StartSendingImageStream(pScanDriver,pEventManager,this);
				//notify main process
				//write(pipefd[1], "1", 1);
				break;
				}
			case 0X02:
				//stop sending image stram thread
				printf("stop sending image thread\r\n");
				StopSendingImageStream(pEventManager);
				//notify main process
				//write(pipefd[1], "0", 1);
				break;
				
			case 0X03:
				if((NULL == pScanDriver)||(NULL == pEventManager))
					break;
					printf("begine send decoder data thread!\r\n");
					StartSendDecoderData(pScanDriver,pEventManager,this);
				break;
			case 0X04:
					printf("Stop send decoder data thread!\r\n");
					StopSendDecoderData(pEventManager);
				break;

			default:
				break;
		}
	}
	return 0;
}

int TcpServerSocket::RegisterScanDriver(ScanDriver *scan_driver, EventManager *event_manager)
{
	if(NULL == pScanDriver)
		pScanDriver = scan_driver;
	if(NULL == pEventManager)
		pEventManager = event_manager;
	return 0;
}

int TcpServerSocket::UnRegisterScanDriver()
{
	pScanDriver = NULL;
	pEventManager = NULL;

	return 0;
}

int TcpServerSocket::RegisterConfiguration(READER_CONFIGURATION *pRc)
{
    if(NULL == pReaderConfig)
        pReaderConfig = pRc;
    else
        return -1;
    return 0;
}

int TcpServerSocket::UnRegisterConfiguration()
{
    if(NULL != pReaderConfig)
        delete pReaderConfig;
    else
       return -1;
    return 0;
}

static std::thread tcp_status_thread;
static bool tcp_status_exit = false;
static std::thread tcp_decoder_thread;
static bool tcp_decoder_exit = false;

int StartSendingImageStream(ScanDriver *scan_driver, EventManager *event_manager,TcpServerSocket *tcp_socket)
{
	tcp_status_exit = false;
	tcp_status_thread = std::move(
			//std::thread(/*ImageLiveThread*/ImageLiveTask,
					//std::ref(tcp_status_exit), std::ref(scan_driver), std::ref(event_manager),std::ref(tcp_socket))
	std::thread(/*ImageLiveThread*/ImageLiveTask,
			std::ref(tcp_status_exit), scan_driver, event_manager,tcp_socket)

	);

}

int StopSendingImageStream(EventManager *event_manager)
{
	tcp_status_exit = true;
	//event_manager->Send( TCP_SERVER_STOP_SENDING_IMAGE);
	tcp_status_thread.join();
	
}

int StartSendDecoderData(ScanDriver *scan_driver, EventManager *event_manager,TcpServerSocket *tcp_socket)
{
	tcp_decoder_exit = false;
	tcp_decoder_thread = std::move(
	std::thread(SendDecoderDataTask,
			std::ref(tcp_decoder_exit), scan_driver, event_manager,tcp_socket)
	);

}

int StopSendDecoderData(EventManager *event_manager)
{
	/*EventDataDecode decodeData;
	DecoderResult result ={0};
	
	decodeData.result = result;
	decodeData.triggerPulledTimestamp = 0;
	decodeData.decodedTimestamp = 0;
	decodeData.decode_time =0;
	tcp_decoder_exit = true;
	
	event_manager->Send(DECODE_EVENT_NEW_DECODE, decodeData);//send empty data for notifying blocking thread

	tcp_decoder_thread.join();*/
	
	tcp_decoder_exit = true;
	//event_manager->Send(TCP_SERVER_STOP_SENDING_DATA);
	tcp_decoder_thread.join();
}


#define LOG_TAG2 "TcpInterfaceSocket"
#define LOGGING2(...) log_print(LOG_LEVEL,LOG_TAG2,__VA_ARGS__)
//==============interface class================================
//=======================================================
TcpInterfaceSocket::TcpInterfaceSocket(int maxConn, int port):
	tcpPort(port),maxConnections(maxConn)
{
/*	if( (socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )  {
		printf("Failed: Obtain Socket Despcritor failed,port[%d]\r\n",tcpPort);
		perror("reason");
		//exit(-1);
		//tcpSocketIsValid = false;
		return;
	} else {
		LOGGING2("Ok: TcpInterfaceSocket sockfd:[%d],port:[%d]\n",socketFd,tcpPort);
		//LOGGING("[%d]OK: Obtain Tcp Socket Despcritor sucessfully.\n",tcpPort);
	}
	
	LOGGING2("Max connectiongs is %d\r\n",maxConnections);
	
	socketAddr.sin_family = AF_INET;					// Protocol Family
	socketAddr.sin_port = htons(tcpPort);				// Port number
	socketAddr.sin_addr.s_addr	= htonl(INADDR_ANY);	// AutoFill local address
	memset (socketAddr.sin_zero,0,8);					// Flush the rest of struct
*/
	LOGGING2("Max connectiongs is %d\r\n",maxConnections);
	pIfcFdVec_ = new FdManager(maxConn);
	//tcpSocketIsValid = true;
	//numOfConnections = 0;
}

TcpInterfaceSocket::~TcpInterfaceSocket()
{
	socketFd = -1;
	delete pIfcFdVec_;
}

int TcpInterfaceSocket::GetFd()
{
	return socketFd;
}

int TcpInterfaceSocket::SetUpTcpInterfaceSocket()
{
	int optval = 1;//on
	if( setsockopt( socketFd , SOL_SOCKET, SO_REUSEADDR,( char *)&optval, sizeof( optval ) ) < 0 )
		perror("set socket SO_REUSEADDR error");
	if( setsockopt( socketFd , SOL_SOCKET, SO_REUSEPORT,( char *)&optval, sizeof( optval ) ) < 0 )
		perror("set socket SO_REUSEPORT error");

	/*	Blind a specified Port */
	if( bind(socketFd, (struct sockaddr*)&socketAddr, sizeof(struct sockaddr_in)) == -1 )
	{  
		printf("Failed to bind Port %d\r\n",tcpPort);
		perror("reason");
		//perror("errno:%s\r\n",strerror(errno));
		exit(-1);
	} else {
		LOGGING2("OK:Bind the Port %d sucessfully.\n",tcpPort);
	}
	
	/*	Listen remote connect/calling */
	if(listen(socketFd,BACKLOG) == -1)	
	{  
		printf("Failed to listen Port %d\r\n",tcpPort);
		perror("reason");
		exit(-1);
	} else {
		LOGGING2("OK: Listening the Port %d sucessfully\r\n",tcpPort);
	}		
}

int TcpInterfaceSocket::Accept(void)
{
	int sin_size = sizeof(struct sockaddr_in);

	if ((connFd = accept(socketFd, (struct sockaddr *)&connSockAddr, (socklen_t *__restrict)&sin_size)) == -1)
	{  
		printf("ERROR: Obtain new Socket Despcritor error,port:[%d]\r\n",tcpPort);
		perror("reason");
		//return -1;
		//continue;
	} else {
		LOGGING2("OK: Server has got connect from %s,port:[%d]\r\n",inet_ntoa(connSockAddr.sin_addr),tcpPort);
		LOGGING2("Ok: Connected socket fd is:%d,port:[%d]\r\n",connFd,tcpPort);

		if(CheckConnections() > 0) {
			LOGGING2("Insert connfd!\r\n");
			//InsertFdVevtor(connFd);
			pIfcFdVec_->Insert(connFd);
			//numOfConnections=pIfcFdVec_->GetTotalCount();
		} else {
			printf("[TcpInterfaceSocket]:Max interface reached,can not accept any more connections\r\n");
			Close(connFd);
			connFd = -1;
		}
		LOGGING2("Total numOfConnections:%d\r\n",pIfcFdVec_->GetTotalCount());
	}
	//DumpFdVector();
	pIfcFdVec_->DumpAllData();

	return connFd;
}

int TcpInterfaceSocket::Open(void)
{
	LOGGING2("%s\r\n",__func__);

	if(socketFd < 0) {
		if( (socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )  {
			printf("Failed: Obtain Socket Despcritor failed,port[%d]\r\n",tcpPort);
			perror("reason");
			//exit(-1);
			//tcpSocketIsValid = false;
			return -1;
		} else {
			LOGGING2("Ok: TcpInterfaceSocket sockfd:[%d],port:[%d]\n",socketFd,tcpPort);
			//LOGGING("[%d]OK: Obtain Tcp Socket Despcritor sucessfully.\n",tcpPort);
		}	
		/* Fill the local socket address struct */
		socketAddr.sin_family = AF_INET;					// Protocol Family
		socketAddr.sin_port = htons(tcpPort);				// Port number
		socketAddr.sin_addr.s_addr	= htonl(INADDR_ANY);	// AutoFill local address
		memset (socketAddr.sin_zero,0,8);					// Flush the rest of struct
	}
	
}

void TcpInterfaceSocket::Close(void)
{
	LOGGING2("%s\r\n",__func__);
	if(socketFd > 0) {
		shutdown(socketFd,SHUT_RDWR);
		close(socketFd);
		socketFd = -1;
	}
}

void TcpInterfaceSocket::Close(int fd)
{
	LOGGING2("%s %d\r\n",__func__,fd);
	if(fd > 0) {
		shutdown(fd,SHUT_RDWR);
		close(fd);
		//DelFdVector(fd);
		pIfcFdVec_->Delete(fd);
		//numOfConnections=pIfcFdVec_->GetTotalCount();
	}
}

void TcpInterfaceSocket::Shutdown(int mode)
{
	LOGGING2("%s\r\n",__func__);
	if(socketFd > 0)
		shutdown(socketFd,SHUT_RDWR);
}

int TcpInterfaceSocket::SocketRead(char *buff,int len)
{
	LOGGING2("%s\r\n",__func__);
	/*return read(connFd, buff, len);*/
}

int TcpInterfaceSocket::SocketWrite(const char* buff,int len)
{
	LOGGING2("%s\r\n",__func__);
	/*if((send(connFd, buff, len ,0)) == -1) {
		perror("Failed to sent string\r\n");
		return -1;
	}*/
	return 0;
}

int TcpInterfaceSocket::SendDataAll(const char* buff,int len)
{
	LOGGING2("%s\r\n",__func__);
	
	/*std::vector<int>::iterator iter;
	for(iter = connFdVec.begin(); iter != connFdVec.end(); ++iter) {
		if((send(*iter, buff, len ,0)) == -1) {
			perror("Failed to sent string\r\n");
			return -1;
		}
	}*/
	int fd = -1;
	for(int i =0;i<pIfcFdVec_->GetTotalCount();i++) {
		fd = pIfcFdVec_->Get(i);
		if((send(fd, buff, len ,0)) == -1) {
			perror("Failed to sent string\r\n");
			return -1;
		}
	}
	return 0;
}

int TcpInterfaceSocket::CheckConnections()
{
	//return ((numOfConnections < maxConnections)?1:-1);
	LOGGING2("%s %d\r\n",__func__,pIfcFdVec_->GetTotalCount());
	return ((pIfcFdVec_->GetTotalCount() < maxConnections)?1:-1);
}

int TcpInterfaceSocket::CheckFd(int fd)
{
	if(pIfcFdVec_->Find(fd)) {
		pIfcFdVec_->Delete(fd);
	}
	pIfcFdVec_->DumpAllData();
}

/*
int TcpInterfaceSocket::CheckFd(int fd)
{
	std::vector<int>::iterator iter;
	iter = std::find(connFdVec.begin(),connFdVec.end(),fd);
	if(connFdVec.end() == iter) {
		LOGGING2("can not find fd:%d\r\n",fd);
		return -1;
	} else {
		return *iter;
	}
}
/*
int TcpInterfaceSocket::InsertFdVevtor(int fd)
{
	//connFdMap.insert(std::map<int, int>::value_type(numOfConnections,connFd)); 
	connFdVec.push_back(fd);
	numOfConnections++;
}

int TcpInterfaceSocket::DelFdVector(int fd)
{
	std::vector<int>::iterator iter;
	iter = std::find(connFdVec.begin(),connFdVec.end(),fd);
	if(connFdVec.end() == iter) {
		LOGGING2("can not find fd:%d\r\n",fd);
		return -1;
	} else {
		connFdVec.erase(iter);
		if(--numOfConnections < 0)
			numOfConnections = 0;
		//LOGGING2("numOfConnections:%d\r\n",numOfConnections);
	}
	return 0;
}

void TcpInterfaceSocket::DumpFdVector()
{
	LOGGING2("**********%s*********\r\n",__func__);

	std::vector<int>::iterator iter;
	for(iter = connFdVec.begin(); iter != connFdVec.end(); ++iter) {
		printf("[fd]:%d\r\n",*iter);	
	}
	LOGGING2("******end of %s******\r\n",__func__);
}*/

int TcpInterfaceSocket::CloseAllConnections()
{
	LOGGING2("%s\r\n",__func__);

	/*std::vector<int>::iterator iter;
	for(iter = connFdVec.begin(); iter != connFdVec.end(); ++iter) {
		shutdown(*iter,SHUT_RDWR);
		close(*iter);
	}
	connFdVec.clear();*/
	int fd = -1;
	for(int i =0;i<pIfcFdVec_->GetTotalCount();i++) {
		fd = pIfcFdVec_->Get(i);
		LOGGING2("fd:%d\r\n",fd);
		shutdown(fd,SHUT_RDWR);
		close(fd);
	}
	pIfcFdVec_->ClearAll();
}

#define LOG_TAG3 "FdManager"
#define LOGGING3(...) log_print(LOG_LEVEL,LOG_TAG3,__VA_ARGS__)
//-------------------------FdMamager class---------------------------------
//-------------------------------------------------------------------------
FdManager::FdManager(int num):maxSize_(num)
{
}

FdManager::~FdManager()
{
	fdVec_.clear();	
}

int FdManager::Insert(int fd)
{
	LOGGING3("%s %d %d\r\n",__func__,fdVec_.size(),maxSize_);
	if(fdVec_.size() <= maxSize_) {
		LOGGING3("%s\r\n",__func__);
		fdVec_.push_back(fd);
		LOGGING3("size:%d\r\n",fdVec_.size());
	} else {
		LOGGING3("Max Size reached!\r\n");
	}
}

int FdManager::Delete(int fd)
{
	LOGGING3("%s\r\n",__func__);
	
	std::vector<int>::iterator iter;
	iter = std::find(fdVec_.begin(),fdVec_.end(),fd);
	if(fdVec_.end() == iter) {
		LOGGING2("can not find fd:%d\r\n",fd);
		return -1;
	} else {
		fdVec_.erase(iter);
	}
	return 0;
}

int FdManager::Find(int fd)
{
	LOGGING3("%s\r\n",__func__);
	
	std::vector<int>::iterator iter;
	iter = std::find(fdVec_.begin(),fdVec_.end(),fd);
	if(fdVec_.end() == iter) {
		LOGGING3("can not find fd:%d\r\n",fd);
		return -1;
	} else {
		return *iter;
	}
}

int FdManager::ClearAll()
{
	LOGGING3("%s\r\n",__func__);
	fdVec_.clear();
}

int FdManager::Get(int num)
{
	return fdVec_[num];
}

int FdManager::GetTotalCount()
{
	return fdVec_.size();
}

void FdManager::DumpAllData()
{
	LOGGING3("**********%s*********\r\n",__func__);

	std::vector<int>::iterator iter;
	for(iter = fdVec_.begin(); iter != fdVec_.end(); ++iter) {
		printf("[fd]:%d\r\n",*iter);	
	}

	LOGGING3("******end of %s******\r\n",__func__);
}