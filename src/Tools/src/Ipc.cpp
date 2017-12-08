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
#include "Ipc.h"

static int StartIpcThread(READER_CONFIGURATION *reader_configuration, IPC *pIpc);
static int StopIpcThread(void);

IPC::IPC(int len,READER_CONFIGURATION *pRc):bufLen(len)
{
    //checking IPC NODE
    if(-1 == access(IPC_NAME, F_OK)) {
        //creat named pipe
        printf("[IPC]NO IPC Node is found,generatinge...\n");
        int res = mkfifo(IPC_NAME, 0777);
        if(res != 0) {
            fprintf(stderr, "[IPC]Could not create fifo %s\n", IPC_NAME);
            //exit(EXIT_FAILURE);
            return;
        }
        pIpcBuf = new char[bufLen];
        memset(pIpcBuf,0,bufLen);
    } else
        printf("[IPC]IPC Node already exist\n");
    //printf("[IPC]Process id %d \n", getpid());

    //pipeFd = open(IPC_NAME, O_WRONLY|O_NONBLOCK);O_RDWR
    pipeFd = open(IPC_NAME, O_RDWR);
    printf("[IPC]Process %d result %d\n", getpid(), pipeFd);
    //if(pipe_fd < 0)
}

IPC::~IPC()
{
    if(pipeFd > 0)
        close(pipeFd);
    delete pIpcBuf;
    unlink(IPC_NAME);//delete fifo node
}

/*int IPC::IpcRead(char *buf, int len)
{
    return read(pipeFd, buf, len);
}*/

int IPC::IpcWrite(char *buf, int len)
{
    return write(pipeFd, buf, len);
}

int IPC::Starthread()
{
    StartIpcThread(pReaderConfiguration,this);
}

int IPC::StopThread()
{
    StopIpcThread();
}

int &IPC::GetFd()
{
    return pipeFd;
}

char *IPC::GetBUff()
{
    if(NULL!= pIpcBuf)
        return pIpcBuf;
    else
        return NULL;
}

/*int IPC::Polling()
{

}*/

static void IPCThread(bool &exit_thread,IPC *pIpc,READER_CONFIGURATION *pRConfig)
{
    //set_thread_name(__func__);
    printf("*************Start [IPC]Thread*****************\r\n");

    struct pollfd fds;

    fds.fd = pIpc->GetFd();
    fds.events = POLLIN;
    //char buf[IPC_BUF_MAX_LENGTH];
    char *buf = pIpc->GetBUff();
    int len = -1;

    while (!exit_thread) {
        if(poll(&fds, 1, -1) > 0) {//timeout=-1 to block until a requested event occurs
             printf("[IPC]tag1\r\n");
            //log_print(LOG_INFO, "KL", "[WaitKey] after poll %d %d %d\n", fds[0].revents, fds[1].revents);
            if((fds.revents & POLLIN) != 0) {
                printf("[IPC]tag2\r\n");
                len = read(fds.fd, buf, 512);
                if(len > 0) {
                    printf("[IPC]:received str len:%d\r\n",len);
                    printf("[IPC]:str is [%s]\r\n",buf);

                }
                memset(buf,0,len+1);
                //memset(buf,0,IPC_BUF_MAX_LENGTH);
            }
        }
    }
    printf("***********END [IPC]Thread***************\r\n");
}
static std::thread ipc_thread;
static bool ipc_exit = false;

static int StartIpcThread(READER_CONFIGURATION *reader_configuration,IPC *pIpc)
{
    ipc_exit = false;
    ipc_thread = std::move(
    std::thread(IPCThread,
            std::ref(ipc_exit),pIpc,reader_configuration)
    );
}

static int StopIpcThread(void)
{
    ipc_exit = true;
    ipc_thread.join();
}
