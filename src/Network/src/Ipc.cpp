#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <poll.h>
#include <thread>
//#include <sys/epoll.h>
#include "Ipc.h"
#include "realtime.h"
#include "logging.h"

#define BACKLOG 10

#define LOG_TAG "IPC"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

static int StartIpcThread(IPC *pIpc);
static int StopIpcThread(void);

IPC::IPC(const char*pStrPath,int len):bufLen(len)
{
	LOGGING("%s\r\n",__func__);
	
	pPath = new char[100];
	pIpcBuf = new char[bufLen];
	
	memcpy(pPath,pStrPath,strlen(pStrPath));
	LOGGING("IPC Server init,domain path:%s\r\n",pPath);
	
	if(-1 == access(pPath, F_OK)) {
        LOGGING("NO IPC Node is found,generatinge...\n");
    } else {
        LOGGING("IPC Node %s already exist,delete it\n",pPath);
		unlink(pPath);//delete node
	}
	Open();
}


IPC::~IPC()
{
	LOGGING("%s\r\n",__func__);
	
    if(ipcFd) {
		shutdown(ipcFd,SHUT_RDWR);
		close(ipcFd);
    }

    if(socketFd) {
		shutdown(socketFd,SHUT_RDWR);
		close(socketFd);
    }
	if(NULL!= pIpcBuf)
		delete pIpcBuf;

    if(NULL!= pPath)
		delete pPath;
	
	unlink(pPath);
}

int IPC::Open(void)
{
	LOGGING("%s\r\n",__func__);
	
	if(socketFd < 0) {
		socketFd = socket(PF_UNIX,SOCK_STREAM,0);
		if(socketFd < 0) {
			printf("Error: IPC server Obtain Socket Despcritor failed\r\n");
			perror("reason");
			//delete pPath;
			//pPath = NULL;
			return -1;
		}
		
		/* Fill the local socket address struct */
		serverSockAddr.sun_family = AF_UNIX;
		strncpy(serverSockAddr.sun_path,pPath,sizeof(serverSockAddr.sun_path)-1);

		Bind();
		Listen();		
	}
	return 0;
}

void IPC::Close(void)
{
	LOGGING("%s\r\n",__func__);
	if(socketFd) {
		close(socketFd);
		socketFd = -1;
	}
}

void IPC::Shutdown(int mode)
{
	LOGGING("%s\r\n",__func__);
	if(socketFd)
		shutdown(socketFd,SHUT_RDWR);
}

int IPC::Bind()
{
	if(bind(socketFd, (struct sockaddr*)&serverSockAddr, sizeof(struct sockaddr_un))==-1) {
		printf("[IPC]: Error:Bind port failed\r\n");
		perror("reason");
		return -1;
	} else {
		LOGGING(" OK:Bind port sucessfully\r\n");
		return 0;
	}
}

int IPC::Listen()
{
	if(listen(socketFd,BACKLOG) == -1)	{  
		printf("[IPC]: Error:Listen port failed\r\n");
		perror("reason");
		return -1;
	} else {
		LOGGING(" OK:Listening the Port sucessfully\r\n");
		return 0;
	}	
}

int IPC::Accept(void)
{
	LOGGING("%s\r\n",__func__);
	
	int sin_size = sizeof(struct sockaddr_un);

	/*	Wait a connection, and obtain a new socket file despriptor for single connection */
	if ((ipcFd = accept(socketFd,(struct sockaddr *)&remoteSockAddr,(socklen_t *__restrict)&sin_size))==-1) {  
		printf ("[IPC]: Error:Obtain client cooection failed\r\n");
		perror("reason");
		//unlink(pPath);
		return -1;
		//continue;
	} else {
		//LOGGING("OK:Server has got connect from %s\r\n",inet_ntoa(remoteSockAddr.sin_addr));
		LOGGING("Ok:IPC server got connection from client,fd %d\r\n",ipcFd);
	}
	return ipcFd;
}

int IPC::SocketRead(char *buf, int len)
{
	LOGGING("%s\r\n",__func__);
    //return read(ipcFd, buf, len);
}

int IPC::SocketWrite(const char* buf,int len)
{
	LOGGING("%s\r\n",__func__);
    //return write(ipcFd, buf, len);
}

int IPC::IpcRead(char *buf, int len)
{
	if(ipcFd > 0)
		return read(ipcFd, buf, len);
}

int IPC::IpcWrite(const char* buf,int len)
{
	if(ipcFd > 0) {
		LOGGING("write ok %d\r\n",ipcFd);
		return write(ipcFd, buf, len);		
	}
	else
		LOGGING("write error %d\r\n",ipcFd);
}

int IPC::ProcessData(char *buf, int len)
{
    if((len ==0)||(NULL == buf))
        return -1;

    /*if((buf[0] == 'D')&&(buf[1] == 'E')) {
        printf("[IPC]:Process Decoder data\r\n");
        //parse decoder setting data
        if (true == pDataParser->ProcessData(buf,len)) {
            printf("[IPC]:parse data successful\r\n");
        }
    } else if ((buf[0] == 'P')&&(buf[1] == 'D')) {
        //set streaming presention exposure and gain
        if(NULL == pReaderConfiguration)
            return -1;
        HSTRING str(buf,len);
        printf("[IPC]:hstring is %s\r\n",str.Char);
        //std::cout<<"str is"<<str<<std::endl;
        int ret = pReaderConfiguration->Menu(&str);
        printf("[IPC]:ret is :%d\n",ret);
    }*/
	/*if (true == pDataParser->ProcessData(buf,len))
		printf("[IPC]:parse data successful\r\n");
	else
		printf("[IPC]:parse data error\r\n");*/
}

int IPC::Starthread()
{
	if(!isThreadRunning) {
		isThreadRunning = true;
		StartIpcThread(this);
	}
}

int IPC::StopThread()
{
	if(isThreadRunning) {
		isThreadRunning = false;
		StopIpcThread();
	}
}

int IPC::GetFd() const
{
    //return ipcFd;
	return socketFd;
}

int IPC::SetConnFd(int val)
{
	ipcFd = val;
}

char *IPC::GetBUff()
{
    if(NULL!= pIpcBuf)
        return pIpcBuf;
    else
        return NULL;
}
#define MAX_EVENTS 100
static void IPCThread(bool &exit_thread,IPC *pIpc)
{
    set_thread_name(__func__);
	
    LOGGING("*************Start [IPC] Thread*****************\r\n");
	int socketFd = -1,nfds = -1,connFd = -1,tmpSockFd=-1;
	int len = -1;
	struct epoll_event ev,events[MAX_EVENTS];
	char buf[1024];
	//pIpc->Bind();
	//pIpc->Listen();
	
	int epfd = epoll_create(256);
	socketFd = pIpc->GetFd();
	add_event(epfd,socketFd,EPOLLIN|EPOLLET);
	
	 while (!exit_thread) {
		 nfds=epoll_wait(epfd,events,MAX_EVENTS,500);
		 for(int i=0;i<nfds;++i) {
			 if(events[i].data.fd == socketFd) {
				printf("[Ipc epoll]:Ipc server begin accept connections!\r\n");
				connFd = pIpc->Accept();
				printf("[Ipc epoll]:connfd:%d\r\n",connFd);
				if(connFd < 0) {
					perror("[Ipc epoll]:connfd <0");
					_exit(1);
				}
				add_event(epfd,connFd,EPOLLIN|EPOLLET);
			 } else if(events[i].events&EPOLLIN) { //read
				tmpSockFd = events[i].data.fd;
				printf("[Ipc epoll]:pollin,fd:%d\r\n",tmpSockFd);
				len = read(tmpSockFd, buf, 512);
                if(len > 0) {
                    LOGGING("received str len:%d\r\n",len);
                    LOGGING("str is [%s]\r\n",buf);
					memset(buf,0,len);
                    //pIpc->ProcessData(buf,len);
					//pIpc->IpcWrite(buf,len);//echo test
                } else  if(len == 0) {
					printf("[Ipc epoll] close fd\r\n");
					delete_event(epfd,tmpSockFd,EPOLLIN|EPOLLET);
					shutdown(tmpSockFd,SHUT_RDWR);
					close(tmpSockFd);
					tmpSockFd = -1;
					connFd = -1;
					pIpc->SetConnFd(-1);
				}				
			 } else if(events[i].events&EPOLLOUT) { //write
				tmpSockFd = events[i].data.fd;
				printf("[Ipc epoll]:pollout:%d\r\n",tmpSockFd);
			 }
		 }
	 }
	
#if 0	
	pIpc->Accept();
	
    struct pollfd fds;

    fds.fd = pIpc->GetFd();
    fds.events = POLLIN;
    //char buf[IPC_BUF_MAX_LENGTH];
    char *buf = pIpc->GetBUff();
	if(NULL == buf)
		return;
    int len = -1;

    while (!exit_thread) {
        if(poll(&fds, 1, 100) > 0) {//timeout=-1 to block until a requested event occurs
             LOGGING("tag1\r\n");
            //log_print(LOG_INFO, "KL", "[WaitKey] after poll %d %d %d\n", fds[0].revents, fds[1].revents);
            if((fds.revents & POLLIN) != 0) {
                LOGGING("tag2\r\n");
                len = read(fds.fd, buf, 512);
                if(len > 0) {
                    LOGGING("received str len:%d\r\n",len);
                    LOGGING("str is [%s]\r\n",buf);
                    pIpc->ProcessData(buf,len);
					//pIpc->IpcWrite(buf,len);//echo test
                } else  if(len == 0) {
					LOGGING("close fd\r\n");
					close(fds.fd);
				}
				//printf("[IPC]:1\r\n");
                memset(buf,0,len+1);
				//printf("[IPC]:2\r\n");
                //memset(buf,0,IPC_BUF_MAX_LENGTH);
            }
        }
    }
#endif
    LOGGING("***********END [IPC] Thread***************\r\n");
}

static std::thread ipc_thread;
static bool ipc_exit = false;

static int StartIpcThread(IPC *pIpc)
{
    ipc_exit = false;
    ipc_thread = std::move(
    std::thread(IPCThread,
            std::ref(ipc_exit),pIpc)
    );
}

static int StopIpcThread(void)
{
    ipc_exit = true;
    ipc_thread.join();
}


//-------------------------------------------------------------
IpcClient::IpcClient(const char*pStrPath,int len):bufLen(len)
{
	pPath = new char[100];
	pIpcBuf = new char[bufLen];
	
	memcpy(pPath,pStrPath,strlen(pStrPath));
	LOGGING("IPC Client init,domain path:%s\r\n",pPath);
	
	/*if(-1 == access(pPath, F_OK)) {
        LOGGING("NO IPC Node is found,exit...\n");
		delete pPath;
		return;
    }*/

	/*socketFd = socket(PF_UNIX,SOCK_STREAM,0);
	if(socketFd<0) {
		printf("Error: IPC Client Obtain Socket Despcritor failed\r\n");
		perror("reason");
		delete pPath;
		return;
	}
	pIpcBuf = new char[bufLen];
	
	clientSockAddr.sun_family=AF_UNIX;  
    strcpy(clientSockAddr.sun_path,pPath);*/
}

IpcClient::~IpcClient()
{
	Close();
	delete pIpcBuf;
	delete pPath;
}

int IpcClient::Open(void)
{
	socketFd = socket(PF_UNIX,SOCK_STREAM,0);
	if(socketFd < 0) {
		printf("Error: IPC Client Obtain Socket Despcritor failed\r\n");
		perror("reason");
		socketFd = -1;
		//delete pPath;
		return -1;
	}
	//pIpcBuf = new char[bufLen];
	clientSockAddr.sun_family=AF_UNIX;  
    strcpy(clientSockAddr.sun_path,pPath);
	
	return 0;
}

void IpcClient::Close(void)
{
	if(socketFd > 0) {
		shutdown(socketFd,SHUT_RDWR);
		close(socketFd);
		socketFd = -1;
	}
}

void IpcClient::Shutdown(int mode)
{
}

int IpcClient::SocketRead(char *buf, int len)
{
	if(socketFd > 0)
    	return read(socketFd, buf, len);
}

int IpcClient::SocketWrite(const char* buf,int len)
{
	if(socketFd > 0)
    	return write(socketFd, buf, len);
}

int IpcClient::TryConnect()
{
	LOGGING("TryConnect,%s\r\n",pPath);
	Open();
	if(-1 == access(pPath, F_OK)) {
        LOGGING("NO IPC Node is found,exiting...\n");
		return -1;
    }

	int ret = connect(socketFd,(struct sockaddr*)&clientSockAddr,sizeof(clientSockAddr));
	if(-1 == ret) {
        perror("cannot connect to the Ipc server"); 
        //Close();  
        return -1;  		
	}
	LOGGING("Connect to Ipc server,%d\r\n",socketFd);
	
	return 0;
}

int IpcClient::TryDisconnect()
{
	LOGGING("TryDisconnect\r\n");
	Close();
	return 0;
}

int IpcClient::GetFd() const
{
    return socketFd;
}

