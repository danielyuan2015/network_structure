/*
 *EventLoop.cpp
 *
 *  Created on: Feb 11, 2018
 *      Author: Daniel yuan
 */
#include <sys/eventfd.h> //eventfd
#include <unistd.h>  //close
#include <algorithm>

#include "realtime.h"
#include "logging.h"
#include "EventLoop.h"

#define LOG_TAG "EventLoop"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)
#define PRINTFUNC LOGGING("%s\r\n",__func__);

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
    eventHandling_(false),
    callingPendingFunctors_(false),
	threadId_(std::this_thread::get_id()),
	poller_(new Poller()),
	wakeupFd_(createEventfd()),
	wakeupChannel_(new Channel(this, wakeupFd_)),
	currentActiveChannel_(NULL)
{
	LOGGING("current thread id:%d,eventLoop id:%d\r\n",std::this_thread::get_id(),threadId_);
	//LOGGING("wakeupFd_ = %d\r\n",wakeupFd_);
	wakeupChannel_= new Channel(this, wakeupFd_);
	wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
	// we are always reading the wakeupfd
	wakeupChannel_->enableReading();

}

EventLoop::~EventLoop()
{
	//need test
	//delete poller_;
	//delete wakeupChannel_;
	wakeupChannel_->disableAll();
	wakeupChannel_->remove();
	::close(wakeupFd_);

}

bool EventLoop::isInLoopThread() const 
{
	return threadId_ == std::this_thread::get_id(); 
}

void EventLoop::loop()
{
	thread_ = std::thread(std::bind(& EventLoop::loopthread, this));
	//thread_ = std::move(t);
}

void EventLoop::loopthread()
{
	//assert(!looping_);
	assertInLoopThread();
	//looping_ = true;
	quit_ = false;	// FIXME: what if someone calls quit() before loop() ?
	LOGGING("start looping\r\n");

	while (!quit_) {
		activeChannels_.clear();
		/* pollReturnTime_ = */
		//LOGGING("begin epoll 1 event\r\n");
		poller_->poll(kPollTimeMs, &activeChannels_);
		//LOGGING("end epoll 1 event\r\n");
		//++iteration_;
		/* if (Logger::logLevel() <= Logger::TRACE)
		{
			printActiveChannels();
		}*/
		printActiveChannels();
		// TODO sort channel by priority
		eventHandling_ = true;
		for (ChannelList::iterator it = activeChannels_.begin();
			it != activeChannels_.end(); ++it) {
			currentActiveChannel_ = *it;
			currentActiveChannel_->handleEvent();
		}
		currentActiveChannel_ = NULL;
		eventHandling_ = false;
		doPendingFunctors();
	}
	LOGGING("stop looping\r\n");
	//looping_ = false;
}

void EventLoop::quitLoop()
{
	quit_ = true;
	// There is a chance that loop() just executes while(!quit_) and exits,
	// then EventLoop destructs, then we are accessing an invalid object.
	// Can be fixed using mutex_ in both places.
	if (!isInLoopThread()) {
		wakeup();
	}
}

void EventLoop::runInLoop(const Functor& cb)
{
	if (isInLoopThread()) {
		cb();
	} else {
		queueInLoop(cb);
	}

}

void EventLoop::queueInLoop(const Functor& cb)
{
	{
		//MutexLockGuard lock(mutex_);
		std::lock_guard<std::mutex> lock(mutex_);
		pendingFunctors_.push_back(cb);
	}

	if (!isInLoopThread() || callingPendingFunctors_) {
		wakeup();
	}
}

void EventLoop::updateChannel(Channel* channel)
{
	//assert(channel->ownerLoop() == this);
	assertInLoopThread();
	poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
	//assert(channel->ownerLoop() == this);
	assertInLoopThread();
	if (eventHandling_) {
		if(currentActiveChannel_ == channel ||
			std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end()) {
			LOGGING("removeChannel\r\n");
		} else {
			LOGGING("removeChannel channel not find!\r\n");
		}
		/*assert(currentActiveChannel_ == channel ||
			std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());*/
	}
	poller_->removeChannel(channel);
}

void EventLoop::assertInLoopThread()
{
	if (!isInLoopThread()) {
		abortNotInLoopThread();
	}
}

void EventLoop::abortNotInLoopThread()
{
	/* LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
	        << " was created in threadId_ = " << threadId_
	        << ", current thread id = " <<  CurrentThread::tid();*/
	LOGGING("EventLoop::abortNotInLoopThread - EventLoop was created in threadId_ = %d , current thread id = %d\r\n",
		threadId_,std::this_thread::get_id());
}

void EventLoop::wakeup()
{
	PRINTFUNC;
	uint64_t one = 1;
	ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
	if (n != sizeof one) {
		//LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
		printf("ERROR EventLoop::wakeup() writes %ld bytes instead of 8\r\n",n);
	}
}

void EventLoop::handleRead()
{
	uint64_t one = 1;
	ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
	if (n != sizeof one) {
		printf("ERROR EventLoop::handleRead() reads %ld bytes instead of 8\r\n",n);
		//LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
	}
}

void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;

	{
		//MutexLockGuard lock(mutex_);
		std::lock_guard<std::mutex> lock(mutex_);
		functors.swap(pendingFunctors_);
	}

	for (size_t i = 0; i < functors.size(); ++i) {
		functors[i]();
	}
	callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const
{
	int cnt = 0;

	for (ChannelList::const_iterator it = activeChannels_.begin();
		it != activeChannels_.end(); ++it) {
		cnt++;
		const Channel* ch = *it;
		//LOG_TRACE << "{" << ch->reventsToString() << "} ";
		printf("[%d]:{%s}\r\n",cnt,ch->reventsToString().c_str());
	}
}

