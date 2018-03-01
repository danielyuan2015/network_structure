/*
 *EventLoop.cpp
 *
 *  Created on: Feb 11, 2018
 *      Author: Daniel yuan
 */
#include "realtime.h"
#include "logging.h"
#include "EventLoop.h"

#define LOG_TAG "EventLoop"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

const int kPollTimeMs = 10000;

EventLoop::EventLoop():
quit_(false),poller_(new Poller())
{

}

EventLoop::~EventLoop()
{

}

void EventLoop::loop()
{
	while (!quit_) {
		activeChannels_.clear();
		/* pollReturnTime_ = */
		LOGGING("begin epoll 1 event\r\n");
		poller_->poll(kPollTimeMs, &activeChannels_);
		LOGGING("end epoll 1 event\r\n");
	  //++iteration_;
	 /* if (Logger::logLevel() <= Logger::TRACE)
	  {
		printActiveChannels();
	  }*/
	  // TODO sort channel by priority
	  //eventHandling_ = true;
	for (ChannelList::iterator it = activeChannels_.begin();
		it != activeChannels_.end(); ++it) {
		currentActiveChannel_ = *it;
		currentActiveChannel_->handleEvent();
	}
	  currentActiveChannel_ = NULL;
	  //eventHandling_ = false;
	  //doPendingFunctors();
	}

}

void EventLoop::runInLoop(const Functor& cb)
{
	cb(); 
}

void EventLoop::updateChannel(Channel* channel)
{
	//assert(channel->ownerLoop() == this);
	//assertInLoopThread();
	poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
	//assert(channel->ownerLoop() == this);
	//assertInLoopThread();
	/*if (eventHandling_) {
		assert(currentActiveChannel_ == channel ||
		std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
	}*/
	poller_->removeChannel(channel);
}

