/*
 *Ppoller.cpp
 *
 *  Created on: Dec 26, 2017
 *      Author: Daniel yuan
 */
 
#include <sys/socket.h>
#include <sys/prctl.h>
#include <string.h>
#include <poll.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>  
#include <arpa/inet.h>
#include <sys/wait.h>
//#include <algorithm>

#include "realtime.h"
#include "logging.h"
#include "Poller.h"

#define LOG_TAG "Poller"
#define LOG_LEVEL LOG_PRINT //directly print in console
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

static void epoll_modify_event(int epollfd,int fd,int state)
{
     struct epoll_event ev;
     ev.events = state;
     ev.data.fd = fd;
     epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
}

static void epoll_add_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
}

static void epoll_delete_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
}

Poller::Poller()
{

}

Poller::~Poller()
{
	close(epollfd_);
}

int Poller::Poll(int timeoutMs)
{
	
}
