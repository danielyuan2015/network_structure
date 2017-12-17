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
//static const char *ethName = "eth0";

class NetworkConfig 
{
public:
	NetworkConfig(const char *pName);
	virtual ~NetworkConfig();
	int GetIpAddr(char *ipBuf);
	int GetMAC(char *macBuf);

	enum NetworkMode{
		Nw_Server_Master = 0,
		Nw_Client_Slave,
	};
private:
	char ifcName[10];
	char ip_[IP_SIZE];
	char mac_[MAC_SIZE];
	bool isDHCPEnabled = false;
	int mode_ = -1;

protected:

};

#endif

