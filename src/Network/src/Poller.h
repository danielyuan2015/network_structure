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
#include "EventLoop.h"
#include <thread>
#include <map>
//#include <mutex>
#include <vector>

#define MAX_EVENTS 100

class Channel;

class Poller:public cNonCopyable
{
public:
	typedef std::vector<struct epoll_event> Eventlist;
	typedef std::vector<Channel*> ChannelList;
	typedef std::map<int, Channel*> ChannelMap;
	/*typedef enum PollEventType{

	}PollEventType_t;*/
	typedef enum ChannelIndex {
		cNew = 0,
		cAdded,
		cDeleted
	}ChannelIndex_t;

	Poller();
	~Poller();
	
	void updateChannel(Channel*);
	void removeChannel(Channel*);
	void update(int,Channel*);
	
	int poll(int,ChannelList*);
	void fillActiveChannels(int,ChannelList*) const;
	
	void assertInLoopThread()
	{
	  //ownerLoop_->assertInLoopThread();
	}

private:
	static const int kInitEventListSize = 16;
	int epollfd_;
	//int events_;
	//int revents_;
	Eventlist events_;
	ChannelMap channels_;
	//EventLoop* ownerLoop_;
	//ChannelList
protected:
};

#endif
