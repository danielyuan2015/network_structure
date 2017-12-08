/*
 *TcpServerSocket.cpp
 *
 *  Created on: Aug 01, 2017
 *      Author: honeywell
 */
 
#include "TcpServerSocket.h"
#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include <poll.h>

#include <unistd.h>  
#include <arpa/inet.h>
#include <sys/wait.h>

#include "realtime.h"
#include "bmp.h"
#include "logging.h"
#include "test.h"
//#include "DecoderMenuSettings.h"

#define _DEBUG
#define LOG_TAG "TCPSERVERSOCKET"

#define SYN 0X16
#define DC1 0X11
#define DC2 0X12
#define DC3 0X13
#define DC4 0X14
#define FE  0XFE
#define LF 0X0A
#define CR 0X0D

static int pipefd[2]; //IPC

int StartSendingImageStream(ScanDriver *scan_driver, EventManager *event_manager,TcpServerSocket *tcp_socket);
int StopSendingImageStream(void);
int StartSendDecoderData(ScanDriver *scan_driver, EventManager *event_manager,TcpServerSocket *tcp_socket);
int StopSendDecoderData(void);
int StartIpcThread(READER_CONFIGURATION *reader_configuration);
int StopIpcThread(void);

int RawToBmp(BYTE *pIn,BYTE *pOut,int width, int height);


TcpServerSocket::TcpServerSocket(int port,TcpServerType type):tcpPort(port),serverType(type)
{
	/* Get the Socket file descriptor */  
	if( (socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )  {   
		printf ("[%d]ERROR: Failed to obtain Socket Despcritor.\n",tcpPort);
		tcpSocketIsValid = false;
		return;
	} else {
		printf ("[%d]Ok: Tcp sockfd = %d\n",tcpPort,socketFd);
		printf ("[%d]OK: Obtain Tcp Socket Despcritor sucessfully.\n",tcpPort);
	}
	
	/* Fill the local socket address struct */
	socketAddr.sin_family = AF_INET;			// Protocol Family
	socketAddr.sin_port = htons(tcpPort); 			// Port number
	socketAddr.sin_addr.s_addr	= htonl(INADDR_ANY);  // AutoFill local address
	memset (socketAddr.sin_zero,0,8);				// Flush the rest of struct

	isThreadRunning	= false;
	tcpSocketIsValid = true;
	numOfConnections = 0;
	
	pScanDriver = NULL;
	pEventManager = NULL;
    pReaderConfig = NULL;
    pIpc = NULL;
	//unsigned char testBuf[] = {"DECSET1a013001:1;."};
	//ParseDecoder(testBuf,sizeof(testBuf));
	//TODO:add heartbeat protocal
}

TcpServerSocket::~TcpServerSocket()
{
	if(true == tcpSocketIsValid) {
		tcpSocketIsValid = false;
		close(socketFd);
	}
}

void TcpServerSocket::Close()
{	
	if(true == tcpSocketIsValid)
		close(socketFd);
}

void TcpServerSocket::Shutdown(int mode)
{
	if(true == tcpSocketIsValid) {
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
	close(fd);
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

	/*	Wait a connection, and obtain a new socket file despriptor for single connection */
	if ((remoteSockFd = accept(socketFd, (struct sockaddr *)&remoteSockAddr, (socklen_t *__restrict)&sin_size)) == -1)
	{  
		printf ("[%d]ERROR: Obtain new Socket Despcritor error.\n",tcpPort);
		return -1;
		//continue;
	} else {
		printf ("[%d]OK: Server has got connect from %s\n", tcpPort,inet_ntoa(remoteSockAddr.sin_addr));
		printf ("[%d]Ok: Remote socket fd is:%d.\n",tcpPort,remoteSockFd);
		numOfConnections++;
	}
	return remoteSockFd;
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
	if(true == tcpSocketIsValid)
		return socketFd;
	else
		return -1;
}

int TcpServerSocket::GetRemoteSocketFd(void)
{
	return remoteSockFd;
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
	FD_SET (remoteSockFd, &fdSetRead);
	ret = select(remoteSockFd + 1, &fdSetRead, NULL, NULL, NULL);
	switch (ret) {
		case 0:
			//printf("time out \r\n");
		break;

		case -1:
			//perror("select error\r\n");
			printf("select error\r\n");
		break; 
	
		default:
			if(FD_ISSET(remoteSockFd,&fdSetRead)) {
				printf("poll:read socket\r\n");
				//num = recv(socket_fd, socket_buf, 256, 0);
				num = SocketRead(socket_buf,256);
				if(num) {
#ifdef _DEBUG					
					printf ("[%d]OK :eceviced numbytes = %d\n",tcpPort,num);
					socket_buf[num] = '\0';
					printf ("[%d]OK :Receviced string is: %s\n",tcpPort,socket_buf);	
#endif
					switch (serverType) {
						case TCP_SERVER_TYPE_CONCURRENT:
							ProcessDataFromHost(socket_buf,num);
							break;
						case TCP_SERVER_TYPE_NONE_CONCURRENT:
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
					close(remoteSockFd);
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
	return read(remoteSockFd, buff, len);	
}

int TcpServerSocket::SocketWrite(const char *buff,int len)
{
	//std::unique_lock<std::mutex> lck(mtx);
	if((send(remoteSockFd, buff, len ,0)) == -1) {
		printf("ERROR: Failed to sent string.\n");
		return -1;
	}
	return 0;
}

int TcpServerSocket::Start()
{
	switch (serverType) {
		case TCP_SERVER_TYPE_CONCURRENT: {
            //create pipe for informing the end of waiting
			if (pipe(pipefd) == -1) {
				printf("pipe error %d, %s\n", errno, strerror(errno));
			}
            //IPC class
            if(NULL != pReaderConfig ){
                pIpc = new IPC(1024,pReaderConfig);
                //pIpc->Starthread();
             }

			if (!isThreadRunning) {
				isThreadRunning = true;
				thread = std::thread(std::bind(&TcpServerSocket::tcpServerSocketConcurrentThread, this));
			}
            //StartIpcThread(pReaderConfig);
			break;
		}
		case TCP_SERVER_TYPE_NONE_CONCURRENT:
			if (!isThreadRunning) {
				isThreadRunning = true;
				thread = std::thread(std::bind(&TcpServerSocket::tcpServerSocketNoneConcurrentThread, this));
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
        //StopIpcThread();
		close(pipefd[0]);//read
		close(pipefd[1]);//write

        if(NULL != pIpc) {
            pIpc->StopThread();
            delete pIpc;
        }
	}
	if (isThreadRunning) {
		isThreadRunning = false;
		thread.join();
	}
	
	//StopSendingImageStream();
}

//this thread for sending image stream
void TcpServerSocket::tcpServerSocketNoneConcurrentThread(void)
{
	set_thread_name(__func__);
	//set_sched_fifo(50);// test here do we need tead-time thread?
	
	printf("*******start tcpServerSocketNoneConcurrentThread*******\r\n");
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
			close(remoteSockFd);
			printf("[%d]begine wait for next connection\r\n",tcpPort);
		}
	}
	close(socketFd);
}

void TcpServerSocket::tcpServerSocketConcurrentThread(void)
{
	pid_t pid;
	
	set_thread_name(__func__);
	//set_sched_fifo(45);// test here do we need tead-time thread?

	printf("*****start tcpServerSocketThread*****\r\n");

	SetSocketOpt();
	Bind();
	Listen();
	
	while(isThreadRunning) {
		if(Accept() > 0) { //should block here waitting for tcp connection
			//step 1 :fork
			if((pid = fork()) < 0) {
				printf("fork error\r\n");
				exit(-1);
			} else if(pid == 0) {/* Child process */

				//close(socketFd);
				while(1) {
                    if( -1 == PollingData() ) {
						printf("Child process exit!\r\n");
						break;	
					}		
				}	
			} else {/*parent process*/
				printf("Parent process:%d!\r\n",getpid());
				printf("Child process:%d!\r\n",pid);
				close(remoteSockFd);
				while(waitpid(-1, NULL, WNOHANG) > 0);
				printf("parent process finished!\r\n");
				printf("block for accept()\r\n");
			}
		}		
	}
	//set_sched_other(0);
}

//#include <fcntl.h>
//#define BUFFER_SIZE 1024
//char* gptr =NULL;

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

#if 0
static int testOpenBmpFile(const char* path,char* buf)
{
	int fd = -1;
	int bytes_read = 0;
	char * pbuff;
	int total=0;
	
	if(path == NULL)
		return -1;
	
    if ((fd = open(path, O_RDONLY)) == -1) {
        fprintf(stderr, "Open %s Error：%s\n", path, strerror(errno));
        exit(1);
    } else {
		if(gptr== NULL)
			gptr = (char*)malloc(2*1024*1024);
		buf = gptr;
	}
	
	while (bytes_read = read(fd, buf, BUFFER_SIZE)) {
		if ((bytes_read == -1) && (errno != EINTR))
			break;
		 else if (bytes_read > 0) {
		 	total += bytes_read;
		 	buf += bytes_read;
		 }
		
	}
	printf("total=%d\r\n",total);
	close(fd);
	
	return total;
}
#endif

static unsigned char *buf_capture_image = NULL;
static unsigned char *pBmp = NULL;
static int buf_capture_image_width = 0;
static int buf_capture_image_height = 0;
static uint64_t  buf_capture_time_stamp = 0;

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

static void ImageLiveTask(bool &exit_thread, ScanDriver *scan_driver, EventManager *event_manager,TcpServerSocket *tcp_socket)
{
	set_thread_name(__func__);

	printf("*************ImageLiveTask*****************\r\n");
	
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
				{ SCAN_DRIVER_NEW_IMAGE_AVAILABLE/*,DECODE_EVENT_NEW_DECODE*/ });
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
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
			/*case DECODE_EVENT_NEW_DECODE:{
				printf("***Barcode dected***\r\n");
				break;
			}*/
		    default:
                break;
		}
	}
	
	printf("***********END ImageLiveTask***************\r\n");
	scan_driver->UnlockBuffer(tcp_image_timestamp);
	free(pBmp);
	event_manager->DestroyEventQueue(eventQueue);
}

static void SendDecoderDataTask(bool &exit_thread, ScanDriver *scan_driver, EventManager *event_manager,TcpServerSocket *tcp_socket)
{
	printf("*************SendDecoderDataTask*****************\r\n");
	
	DataEvent<EventDataDecode> *pDecEvent = NULL;
	Event *pEvent;
	EventQueue<Event*>* eventQueue;
	eventQueue = event_manager->CreateEventQueue(EVENT_QUEUE_MAX_SIZE,
				{ DECODE_EVENT_NEW_DECODE });
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
					EventDataDecode *pData = (EventDataDecode *)pDecEvent->GetData();
					sprintf(commentLine,"%s\x1f%d\x1f%d\x1f%d\x1f%d\x1f%d\x1f%d\x1f%d\x1f%d\x1f%lld\x1f%lld",pData->result.Data,\
					pData->result.MainBounds.corners[0].x,pData->result.MainBounds.corners[0].y,\
					pData->result.MainBounds.corners[1].x,pData->result.MainBounds.corners[1].y,\
					pData->result.MainBounds.corners[2].x,pData->result.MainBounds.corners[2].y,\
					pData->result.MainBounds.corners[3].x,pData->result.MainBounds.corners[3].y,\
					buf_capture_time_stamp,pData->decodedTimestamp\
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
		    default:
                break;		
		}
	}
	
	event_manager->DestroyEventQueue(eventQueue);	
	printf("***********END SendDecoderDataTask***************\r\n");
}

#if 1
//static void IPCThread(bool &exit_thread, ScanDriver *scan_driver, EventManager *event_manager,TcpServerSocket *tcp_socket)
static void IPCThread(bool &exit_thread,READER_CONFIGURATION *pRConfig)
{
    printf("*************Start IPCThread*****************\r\n");
	struct pollfd fds;
	
	fds.fd = pipefd[0];
	fds.events = POLLIN;
    char buf[TCPSOCKETBUFLEN];
	int len = -1;
	//event_manager->DumpLists();
	
	while (!exit_thread) {
		if(poll(&fds, 1, -1) > 0) {//timeout=-1 to block until a requested event occurs
			//log_print(LOG_INFO, "KL", "[WaitKey] after poll %d %d %d\n", fds[0].revents, fds[1].revents);
			
			if((fds.revents & POLLIN) != 0) {
				//char in;
				len = read(fds.fd, buf, 512);
				if(len > 0) {
                    printf("[IPC]:received decoder config command\r\n");
                    printf("[IPC]:str is [%s]\r\n",buf);

                    if((buf[0] == 'D')&&(buf[1] == 'E')) {
                        //parse decoder setting data
                        if (true == ParseDecoder((unsigned char*)buf,len)) {
                            printf("parse data successful\r\n");
                        }
                    } else if ((buf[0] == 'P')&&(buf[1] == 'D')) {
                        //set streaming presention exposure and gain
                        if(NULL == pRConfig)
                            return;
                        HSTRING str(buf,len);
                         printf("hstring is %s\r\n",str.Char);
                        //std::cout<<"str is"<<str<<std::endl;
                        int ret = pRConfig->Menu(&str);
                        printf("ret is :%d\n",ret);
                    }
				}
                memset(buf,0,TCPSOCKETBUFLEN);
				/*if (read(fds.fd, &in, 1) == 1) {
					if('1' == in){
						printf("IPC:received decoder config command\r\n");
						
						//ImageLiveTask(exit_thread,scan_driver,event_manager,tcp_socket);
					} else if('0' == in){
						printf("request close\r\n");	
					}
				}*/
			}
		}		
	}	
	printf("***********END IPCThread***************\r\n");
	/*Event *pEvent;
	EventQueue<Event*>* eventQueue;
	eventQueue = event_manager->CreateEventQueue(EVENT_QUEUE_MAX_SIZE,
	{ TCP_SERVER_START_SENDING_IMAGE,TCP_SERVER_STOP_SENDING_IMAGE });
	
	event_manager->DumpLists();
	while(!exit_thread) {
        eventQueue->Wait(pEvent);
		switch(pEvent->GetId()) {
            case TCP_SERVER_START_SENDING_IMAGE: {
				printf("receive TCP_SERVER_START_SENDING_IMAGE signal\r\n");
				ImageLiveTask(exit_thread,scan_driver,event_manager,tcp_socket);
				break;
			}
			case TCP_SERVER_STOP_SENDING_IMAGE: {
				printf("receive TCP_SERVER_STOP_SENDING_IMAGE signal\r\n");
				break;
			}
			default:
				break;
		}
	}*/

}
#endif
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
        pIpc->IpcWrite( buf, len);
		/*if (true == ParseDecoder((unsigned char*)buf,len)) {
			printf("parse data successful\r\n");
		}*/
    //}
#endif
#if 0
	if((buf[0] == SYN)&&(buf[1] == DC1)) {
		switch(buf[2]) {
			case 0X01:{
				//start sending image stram thread
				if((NULL == pScanDriver)||(NULL == pEventManager))
					break;
				//pEventManager->Send(TCP_SERVER_START_SENDING_IMAGE);
				//printf("begin sending image thread\r\n");
				//StartSendingImageStream(pScanDriver,pEventManager,this);
				//notify main process
				//write(pipefd[1], "1", 1);
				break;
				}
			case 0X02:
				//stop sending image stram thread
				//pEventManager->Send(TCP_SERVER_STOP_SENDING_IMAGE);
				//printf("stop sending image thread\r\n");
				//StopSendingImageStream();
				//notify main process
				//write(pipefd[1], "0", 1);
				break;
			case 0X03:
				//if((NULL == pScanDriver)||(NULL == pEventManager))
					//break;
					//printf("begine send decoder data thread!\r\n");
					//StartSendDecoderData(pScanDriver,pEventManager,this);
				break;
			case 0X04:
					//printf("Stop send decoder data thread!\r\n");
					//StopSendDecoderData();
				break;
			default:
				break;
		}
	}
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
				StopSendingImageStream();
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
					StopSendDecoderData();
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

int TcpServerSocket::ParseExpAndGainData(const char *buff, int len)
{
    if(NULL == pReaderConfig)
        return -1;
    HSTRING str(buff,len);

    int ret = pReaderConfig->Menu(&str);
    printf("ret is :%d\n",ret);
    return 0;
}

static std::thread tcp_status_thread;
static bool tcp_status_exit = false;
static std::thread tcp_decoder_thread;
static bool tcp_decoder_exit = false;
static std::thread tcp_ipc_thread;
static bool tcp_ipc_exit = false;

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

int StopSendingImageStream(void)
{
	tcp_status_exit = true;
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

int StopSendDecoderData(void)
{
	tcp_decoder_exit = true;
	tcp_decoder_thread.join();
}

int StartIpcThread(READER_CONFIGURATION *reader_configuration)
{
	tcp_ipc_exit = false;
	tcp_ipc_thread = std::move(
	std::thread(IPCThread,
            std::ref(tcp_ipc_exit),reader_configuration)
	);

}

int StopIpcThread(void)
{
	tcp_ipc_exit = true;
	tcp_ipc_thread.join();
}

