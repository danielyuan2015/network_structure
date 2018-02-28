/*
 * InetAddress.h
 *
 *  Created on: Feb 27, 2018
 *      Author: Daniel Yuan
 */

#ifndef _INETADDRESS_H_
#define _INETADDRESS_H_

#include <netinet/in.h>   // sockaddr_in
#include <string>

using namespace std;
//TODO:suppoot ipv6
class InetAddress
{
public:
	explicit InetAddress(uint16_t port);
	InetAddress(const char *ip, uint16_t port);
	InetAddress(string ip, uint16_t port);

	const struct sockaddr* getSockAddr() const;
	void setSockAddr(const struct sockaddr_in& addr) { addr_ = addr; }
	
	string toIpPort() const;

private:
	struct sockaddr_in addr_;
};

#endif

