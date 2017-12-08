#ifndef IPC_H
#define IPC_H

//#include "ReaderConfiguration.h"
#include "hstring.h"
#include "ReaderConfiguration.h"

class IPC
{
#define  IPC_NAME "/tmp/hf800_fifo"
//#define  IPC_BUF_MAX_LENGTH 1024
public:
    IPC(int len, READER_CONFIGURATION *pRc);
    virtual ~IPC();
    //int IpcRead(char *buf,int len);
    int IpcWrite(char *buf,int len);
    //int Polling();
    int Starthread();
    int StopThread();
    int &GetFd();
    char *GetBUff();

private:
    READER_CONFIGURATION *pReaderConfiguration = NULL;
    char *pIpcBuf = NULL;
    int pipeFd = -1;
    int bufLen = -1;
protected:
};

#endif // IPC_H
