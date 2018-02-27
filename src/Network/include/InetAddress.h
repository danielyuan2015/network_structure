/*
 * InetAddress.h
 *
 *  Created on: Feb 27, 2018
 *      Author: Daniel Yuan
 */

#ifndef _INETADDRESS_H_
#define _INETADDRESS_H_

#include <netinet/in.h>   // sockaddr_in

//TODO:suppoot ipv6
class InetAddress
{
public:
	explicit InetAddress(int port);
	const struct sockaddr* getSockAddr() const;
	void setSockAddr(const struct sockaddr_in& addr) { addr_ = addr; }

private:
	struct sockaddr_in addr_;
};

#endif

