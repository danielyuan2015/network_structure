/*
 * NetworkConfig.h
 *
 *  Created on: Dec 17, 2017
 *      Author: Daniel Yuan
 */

#ifndef NETWORKCONFIG_H_
#define NETWORKCONFIG_H_

#include <sys/socket.h>
#include<netinet/in.h>   // sockaddr_in

#define IP_SIZE     16
#define MAC_SIZE    18
static const char *ethName = "eth0";

class NetworkConfig 
{
public:
	NetworkConfig();
	virtual ~NetworkConfig();
	int GetIpAddr(const char *eth_name = ethName);
	int GetMAC(const char *eth_name = ethName);

private:
	char ip_[IP_SIZE];
	char mac_[MAC_SIZE];

protected:

};

#endif

