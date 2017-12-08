/*
 *TcpClient.cpp
 *
 *  Created on: Aug 01, 2017
 *      Author: honeywell
 */
 
#include "TcpClien.h"

TcpClient::TcpClient()
{
	pTcpSocket = new TcpClientSocket(0);
}

TcpClient::~TcpClient()
{
	delete pTcpSocket;
}

int TcpClient::Run()
{

}

