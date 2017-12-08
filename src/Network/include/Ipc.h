#ifndef IPC_H
#define IPC_H

//#include "hstring.h"
#include "socket.h"
#include <sys/un.h>
//#include "ReaderConfiguration.h"
//#include "DataParser.h"

class IPC:public cSocket,cNonCopyable
{
public:
	//IPC(HSTRING strPath, int len, READER_CONFIGURATION *pRc);
	//IPC(const char* pStrPath, int len, READER_CONFIGURATION *pRc);
	IPC(const char*pStrPath,int len);
    virtual ~IPC();
	void Close();
	int Open();
	void Shutdown(int mode);
	int SocketRead(char *buff,int len);
	int SocketWrite(const char* buff,int len);
	
	int Bind();
	int Listen();
	int Accept();	

    int IpcRead(char *buf,int len);
    int IpcWrite(const char *buf,int len);

    int ProcessData(char *buf,int len);
    //int ProcessData(HSTRING &str);
    int Starthread();
    int StopThread();
    int GetFd() const;
	int SetConnFd(int val);
    char *GetBUff();

	
private:
    //READER_CONFIGURATION *pReaderConfiguration = NULL;
    //DataParser *pDataParser = NULL;
    //struct sockaddr_in remoteSockAddr;
	int ipcFd = -1;
	bool isThreadRunning = false;
	struct sockaddr_un serverSockAddr; //unix domain
	struct sockaddr_un remoteSockAddr;
	char* pPath = NULL;
    char *pIpcBuf = NULL;
    int bufLen = -1;
protected:

};

class IpcClient:public cSocket,cNonCopyable
{
public:
	IpcClient(const char*pStrPath,int len);
    virtual ~IpcClient();
	int Open();
	void Close();
	void Shutdown(int mode);
	int SocketRead(char *buff,int len);
	int SocketWrite(const char* buff,int len);
	int TryConnect();
	int TryDisconnect();
	int GetFd() const;

private:
	//int connFd = -1;
	struct sockaddr_un clientSockAddr;
	char* pPath = NULL;
	char *pIpcBuf = NULL;
	int bufLen = -1;
protected:
};

#endif // IPC_H
