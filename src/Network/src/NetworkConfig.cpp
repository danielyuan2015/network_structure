/*
 * NetworkConfig.cpp
 *
 *  Created on: Dec 17, 2017
 *      Author: Daniel Yuan
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include "NetworkConfig.h"
#include "logging.h"
//#include <sys/epoll.h>

#define LOG_TAG "NwkConfig"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)


NetworkConfig::NetworkConfig(const char *pName)
{
	if(NULL != pName) {
		memcpy(ifcName,pName, strlen(pName));
		LOGGING("ifcName:[%s] strlen:[%d]  [%d]\r\n",ifcName,strlen(pName),sizeof(pName));		
	}
	mode_  =  Nw_Server_Master;
}

NetworkConfig::~NetworkConfig()
{
}

int NetworkConfig::GetIpAddr(char *ipBuf)
{
	int fd = -1;
	struct sockaddr_in sin;
	struct ifreq ifr;

	if(NULL == ipBuf) {
		return -1;
	}
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == fd) {
		printf("socket error: %s\n", strerror(errno));
		return -1;
	}

	strncpy(ifr.ifr_name, ifcName, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
		printf("ioctl error: %s\n", strerror(errno));
		close(fd);
		return -1;  
	}
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));

	//convert to ip
	snprintf(ip_, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));
	LOGGING("ip:[%s]\r\n",ip_);
	memcpy(ipBuf,ip_, strlen(ip_));
	close(fd);

	return 0;
}

int NetworkConfig::GetMAC(char *macBuf)
{
	struct ifreq ifr;
	int fd;
    
	if(NULL == macBuf) {
		return -1;
	}
    bzero(&ifr, sizeof(struct ifreq));
    if( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("get %s mac address socket creat error\n", ifcName);
        return -1;
    }

    strncpy(ifr.ifr_name, ifcName, sizeof(ifr.ifr_name) - 1);
  
    if(ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
        printf("get %s mac address error\n", ifcName);
        close(fd);
        return -1;
    }
  
    snprintf(mac_, MAC_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
        (unsigned char)ifr.ifr_hwaddr.sa_data[0],
        (unsigned char)ifr.ifr_hwaddr.sa_data[1],
        (unsigned char)ifr.ifr_hwaddr.sa_data[2],
        (unsigned char)ifr.ifr_hwaddr.sa_data[3],
        (unsigned char)ifr.ifr_hwaddr.sa_data[4],
        (unsigned char)ifr.ifr_hwaddr.sa_data[5]);

	memcpy(macBuf,mac_, strlen(mac_));
	LOGGING("mac:[%s]\r\n",mac_);
    close(fd);
      
    return 0;
}
