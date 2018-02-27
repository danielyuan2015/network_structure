/*
 *InetAddress.cpp
 *
 *  Created on: Feb 27, 2018
 *      Author: Daniel yuan
 */
#include "InetAddress.h"
#include "logging.h"
#include <string.h>

#define LOG_TAG "InetAddress"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

InetAddress::InetAddress(int port)
{
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = htonl(INADDR_ANY);//sockets::hostToNetwork32(kInaddrAny);
  addr_.sin_port = htons(port);//sockets::hostToNetwork16(port);
}

