/*
 *SocketOps.cpp
 *
 *  Created on: Dec 26, 2017
 *      Author: Daniel yuan
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  // snprintf
//#include <strings.h>  // bzero
#include <string.h> //strerror bzero
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>

#include "SocketOps.h"
//#include "realtime.h"
#include "logging.h"

#define LOG_TAG "Sockets"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

//using namespace sockets;
struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in *addr)
{
	//return static_cast<struct sockaddr*>(addr);
	return (struct sockaddr*)(addr);
}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in *addr)
{
	return (const struct sockaddr*)(addr);
}

void sockets::bindOrDie(int sockfd, const struct sockaddr* addr)
{
	int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
	if (ret < 0) {
		//LOG_SYSFATAL << "sockets::bindOrDie";
		printf ("sockets::bindOrDie\r\n");
		printf("errno:%s\r\n",strerror(errno));
	}
}

void sockets::listenOrDie(int sockfd)
{
	int ret = ::listen(sockfd, SOMAXCONN);
	if (ret < 0) {
		//LOG_SYSFATAL << "sockets::listenOrDie";
		printf ("sockets::listenOrDie\r\n");
		printf("errno:%s\r\n",strerror(errno));
	}
}

int sockets::createNonblockingOrDie()
{
	int sockfd = ::socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC,IPPROTO_TCP);
	if (sockfd < 0) {
		printf("sockets::createNonblockingOrDie\r\n");
		printf("errno:%s\r\n",strerror(errno));
	}
	return sockfd;
}

int sockets::accept(int sockfd, struct sockaddr_in *addr)
{
	socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);

	int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
	//setNonBlockAndCloseOnExec(connfd);
	if (connfd < 0) {
	    int savedErrno = errno;
	    //LOG_SYSERR << "Socket::accept";
		printf("Socket::accept\r\n");
		perror("reason");
		switch (savedErrno) {
			case EAGAIN:
			case ECONNABORTED:
			case EINTR:
			case EPROTO: // ???
			case EPERM:
			case EMFILE: // per-process lmit of open file desctiptor ???
				// expected errors
				errno = savedErrno;
			break;
			case EBADF:
			case EFAULT:
			case EINVAL:
			case ENFILE:
			case ENOBUFS:
			case ENOMEM:
			case ENOTSOCK:
			case EOPNOTSUPP:
				// unexpected errors
				printf("unexpected error of ::accept %d\r\n",savedErrno);
				//LOG_FATAL << "unexpected error of ::accept " << savedErrno;
			break;
			default:
				printf("unknown error of ::accept %d\r\n",savedErrno);
				//LOG_FATAL << "unknown error of ::accept " << savedErrno;
			break;
		}
	}
	return connfd;
}

int sockets::connect(int sockfd, const struct sockaddr* addr)
{
	return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}

ssize_t sockets::read(int sockfd, void *buf, size_t count)
{
	return ::read(sockfd, buf, count);
}

/*ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt)
{
  return ::readv(sockfd, iov, iovcnt);
}*/

ssize_t sockets::write(int sockfd, const void *buf, size_t count)
{
	return ::write(sockfd, buf, count);
}

void sockets::close(int sockfd)
{
	if (::close(sockfd) < 0) {
		//LOG_SYSERR << "sockets::close";
		perror("sockets::close");
	}
}

void sockets::shutdownWrite(int sockfd)
{
	if (::shutdown(sockfd, SHUT_WR) < 0) {
		//LOG_SYSERR << "sockets::shutdownWrite";
		perror("sockets::shutdownWrite");
	}
}

void sockets::shutdownReadWrite(int sockfd)
{
	if (::shutdown(sockfd, SHUT_RDWR) < 0) {
		perror("sockets::shutdownReadWrite");
	}
}

//get ip info from addr and save it in buf
void sockets::toIp(char *buf, size_t size,
                   const struct sockaddr *addr)
{
	if (addr->sa_family == AF_INET) {
		assert(size >= INET_ADDRSTRLEN);
		const struct sockaddr_in *addr4 = (const struct sockaddr_in *)(addr);//sockaddr_in_cast(addr);
		::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
	} else if (addr->sa_family == AF_INET6) {
		//assert(size >= INET6_ADDRSTRLEN);
		//const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
		//::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
		printf("not support ipv6 yet\r\n");
	}
}

//get ip and port info from addr and save it in buf
void sockets::toIpPort(char* buf, size_t size,
                       const struct sockaddr* addr)
{
	toIp(buf,size, addr);
	size_t end = ::strlen(buf);
	const struct sockaddr_in* addr4 = (const struct sockaddr_in *)(addr);//sockaddr_in_cast(addr);
	uint16_t port = sockets::networkToHost16(addr4->sin_port);
	assert(size > end);
	snprintf(buf+end, size-end, ":%u", port);
}

void sockets::fromIpPort(const char* ip, uint16_t port,
                         struct sockaddr_in* addr)
{
	addr->sin_family = AF_INET;
	addr->sin_port = hostToNetwork16(port);
	if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
		//LOG_SYSERR << "sockets::fromIpPort";
		perror("sockets::fromIpPort");
	}
}					   

void sockets::fromIpPort(const char* ip, uint16_t port,
                         struct sockaddr_in6* addr)
{
	addr->sin6_family = AF_INET6;
	addr->sin6_port = hostToNetwork16(port);
	if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
		perror("sockets::fromIpPort(IPV6)");
	}
}

struct sockaddr_in sockets::getLocalAddr(int sockfd)
{
	struct sockaddr_in localaddr;
	bzero(&localaddr, sizeof localaddr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
	
	if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
		perror("sockets::getLocalAddr");
	}
	return localaddr;
}

