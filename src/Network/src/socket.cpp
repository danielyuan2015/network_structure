/*
 * cSocket.cpp
 *
 *  Created on: Dec 17, 2017
 *      Author: Daniel Yuan
 */
#include "socket.h"
#include <sys/epoll.h>

cSocket::cSocket()
{
}

cSocket::~cSocket()
{
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


#include "SocketOps.h"
#include <string.h>

using namespace sockets;

Socket::~Socket()
{
	sockets::close(sockfd_);
}
void Socket::Listen()
{
	sockets::listenOrDie(sockfd_);
}

void Socket::Bind(const InetAddress& addr)
{
	sockets::bindOrDie(sockfd_, addr.getSockAddr());
}

  /// On success, returns a non-negative integer that is
  /// a descriptor for the accepted socket, which has been
  /// set to non-blocking and close-on-exec. *peeraddr is assigned.
  /// On error, -1 is returned, and *peeraddr is untouched.
int Socket::Accept(InetAddress* peeraddr)
{
	struct sockaddr_in addr;
	bzero(&addr, sizeof addr);
	int connfd = sockets::accept(sockfd_, &addr);
	if (connfd >= 0) {
		peeraddr->setSockAddr(addr);
	}
	return connfd;
}

void Socket::ShutdownWrite()
{
	sockets::shutdownWrite(sockfd_);
}

void Socket::ShutdownReadWrite()
{
	sockets::shutdownReadWrite(sockfd_);
}