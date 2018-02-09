/*
 * Poller.h
 *
 *  Created on: Dec 26, 2017
 *      Author: Daniel Yuan
 */
#ifndef SRC_POLLER_H_
#define SRC_POLLER_H_

#include "socket.h"
#include "Channel.h"
#include <thread>
#include <map>
#include <mutex>
#include <vector>

#define MAX_EVENTS 100

class Poller:public cNonCopyable
{
public:
	typedef std::vector<struct epoll_event> Eventlist;
	typedef std::vector<Channel*> ChannelList;
	typedef std::map<int, Channel*> ChannelMap;
	typedef enum PollEventType{

	}PollEventType_t;
	Poller();
	~Poller();
	void update(int,Channel*);
	int Poll(int,ChannelList*);
	void fillActiveChannels(int,ChannelList*) const;

private:
	int epollfd_;
	//int events_;
	//int revents_;
	Eventlist events_;
	ChannelMap channels_;
	//ChannelList
protected:
};

#endif
