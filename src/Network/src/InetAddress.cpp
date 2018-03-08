/*
 *InetAddress.cpp
 *
 *  Created on: Feb 27, 2018
 *      Author: Daniel yuan
 */
 
//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

#include "InetAddress.h"
#include "logging.h"
#include <string.h>
#include "SocketOps.h"

#define LOG_TAG "InetAddress"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

using namespace sockets;

InetAddress::InetAddress(uint16_t port)
{
	bzero(&addr_, sizeof addr_);
	addr_.sin_family = AF_INET;
	addr_.sin_addr.s_addr = htonl(INADDR_ANY);//sockets::hostToNetwork32(kInaddrAny);
	addr_.sin_port = sockets::hostToNetwork16(port);//htons(port);
}

/// Constructs an endpoint with given ip and port.
/// @c ip should be "1.2.3.4"
InetAddress::InetAddress(string ip, uint16_t port)
{
	//bzero(&addr_, sizeof addr_);
	//sockets::fromIpPort(ip.c_str(),port,&addr_);
	InetAddress(ip.c_str(),port);
}

/// Constructs an endpoint with given ip and port.
/// @c ip should be "1.2.3.4"
InetAddress::InetAddress(const char *ip, uint16_t port)
{
	bzero(&addr_, sizeof addr_);
	sockets::fromIpPort(ip,port,&addr_);
}

/// Constructs an endpoint with given struct @c sockaddr_in
/// Mostly used when accepting new connections
InetAddress::InetAddress(const struct sockaddr_in& addr)
	:addr_(addr)
{
}

const struct sockaddr* InetAddress::getSockAddr() const
{ 
	return sockets::sockaddr_cast(&addr_); 
}

//return ip adn port string
//format: x.x.x.x:port
string InetAddress::toIpPort() const
{
	char buf[32];
	sockets::toIpPort(buf, sizeof buf, (struct sockaddr *)&addr_);
	//LOGGING("toipPort:%s\r\n",buf);
	return buf;
}

