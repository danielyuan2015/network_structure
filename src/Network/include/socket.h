/*
 * cSocket.h
 *
 *  Created on: Dec 17, 2017
 *      Author: Daniel Yuan
 */

#ifndef SRC_SOCKET_H_
#define SRC_SOCKET_H_
#include <sys/epoll.h>
#include <sys/socket.h>
#include<netinet/in.h>   // sockaddr_in

class cSocket 
{
public:
	cSocket();
	virtual ~cSocket();
	virtual int Open() = 0;
	virtual void Close() = 0;
	virtual void Shutdown(int) = 0;
	virtual int SocketRead(char *,int) = 0;
	virtual int SocketWrite(const char*,int) = 0;
	
protected:
	struct sockaddr_in socketAddr;
	int socketFd = -1;
};

class cNonCopyable
{
private:
    cNonCopyable(const cNonCopyable&);
    const cNonCopyable& operator=(const cNonCopyable&);
 
protected:
    cNonCopyable() { }
    ~cNonCopyable() { }
};

//a new socket class
class Socket
{
public:
	explicit Socket(int sockfd):sockfd_(sockfd){};
	 ~Socket();

	/*void Bind();
	void Listen();
	int Accept(InetAddress* peeraddr);
	void Shutdown();
	void ShutdownWrite();*/
	
	int fd() const {return sockfd_;}

private:
	const int sockfd_;

};

void modify_event(int epollfd,int fd,int state);
void add_event(int epollfd,int fd,int state);
void delete_event(int epollfd,int fd,int state);

#endif /* SRC_SOCKET_H_ */

