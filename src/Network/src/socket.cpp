/*
 * cSocket.cpp
 *
 *  Created on: Aug 01, 2017
 *      Author: honeywell
 */
#include "socket.h"
#include <sys/epoll.h>

cSocket::cSocket()
{
	// TODO Auto-generated constructor stub

}

cSocket::~cSocket()
{
	// TODO Auto-generated destructor stub
}


void modify_event(int epollfd,int fd,int state)
{
     struct epoll_event ev;
     ev.events = state;
     ev.data.fd = fd;
     epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
}

void add_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
}

void delete_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
}