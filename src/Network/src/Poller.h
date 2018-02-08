/*
 * Poller.h
 *
 *  Created on: Dec 26, 2017
 *      Author: Daniel Yuan
 */
#ifndef SRC_POLLER_H_
#define SRC_POLLER_H_

#include "socket.h"
#include <thread>
#include <map>
#include <mutex>
#include <vector>
//#include "ScanDriver.h"
//#include "Ipc.h"


class Poller:public cNonCopyable
{
public:
	Poller();
	~Poller();
	int Poll(int timeoutMs);

private:
  int epollfd_ = -1;
	
protected:
};
#endif
