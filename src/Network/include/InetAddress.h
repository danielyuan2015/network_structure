/*
 * InetAddress.h
 *
 *  Created on: Feb 27, 2018
 *      Author: Daniel Yuan
 */

#ifndef _INETADDRESS_H_
#define _INETADDRESS_H_

#include<netinet/in.h>   // sockaddr_in

class InetAddress
{
public:
	explicit InetAddress(int port);

private:
	struct sockaddr_in addr_;
};

#endif

