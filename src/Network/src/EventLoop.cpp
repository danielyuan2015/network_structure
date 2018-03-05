/*
 *EventLoop.cpp
 *
 *  Created on: Feb 11, 2018
 *      Author: Daniel yuan
 */
#include <sys/eventfd.h> //eventfd
#include <unistd.h>  //close

#include "realtime.h"
#include "logging.h"
#include "EventLoop.h"

#define LOG_TAG "EventLoop"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

const int kPollTimeMs = 10000;

//we use eventfd here
static int createEventfd()
{
	int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0) {
		perror("Failed in eventfd");
		abort();
	}
	return evtfd;
}

EventLoop::EventLoop():
	quit_(false),
	poller_(new Poller()),
	wakeupFd_(createEventfd()),
	wakeupChannel_(new Channel(this, wakeupFd_))
{
	LOGGING("current thread id:%d,eventLoop id:%d\r\n",std::this_thread::get_id(),threadId_);
}

EventLoop::~EventLoop()
{
	//need test
	//delete poller_;
	//delete wakeupChannel_;
	//wakeupChannel_->disableAll();
	//wakeupChannel_->remove();
	::close(wakeupFd_);

}

bool EventLoop::isInLoopThread() const 
{
	return threadId_ == std::this_thread::get_id(); 
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

void EventLoop::quitLoop()
{
	quit_ = true;
	// There is a chance that loop() just executes while(!quit_) and exits,
	// then EventLoop destructs, then we are accessing an invalid object.
	// Can be fixed using mutex_ in both places.
	/*if (!isInLoopThread()) {
		wakeup();
	}*/
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

void EventLoop::assertInLoopThread()
{
  /*if (!isInLoopThread())
  {
	abortNotInLoopThread();
  }*/
}

