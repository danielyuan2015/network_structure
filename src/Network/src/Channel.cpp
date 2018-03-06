/*
 *	Channel.cpp
 *
 *  Created on: Feb 09, 2017
 *      Author: Daniel yuan
 */

#include <algorithm>
#include "logging.h"
#include "Channel.h"

#if 0
BOOST_STATIC_ASSERT(EPOLLIN == POLLIN);
BOOST_STATIC_ASSERT(EPOLLPRI == POLLPRI);
BOOST_STATIC_ASSERT(EPOLLOUT == POLLOUT);
BOOST_STATIC_ASSERT(EPOLLRDHUP == POLLRDHUP);
BOOST_STATIC_ASSERT(EPOLLERR == POLLERR);
BOOST_STATIC_ASSERT(EPOLLHUP == POLLHUP);
#endif
//-------------------------FdMamager class---------------------------------
//-------------------------------------------------------------------------
#define LOG_TAG "Channel"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop,int fd)
	:loop_(loop),
	fd_(fd),
	events_(0),
	revents_(0),
	index_(-1)
{
}

Channel::~Channel()
{
}

void Channel::update()
{
  //addedToLoop_ = true;
  loop_->updateChannel(this);
}

void Channel::handleEvent()
{
	LOGGING("handle_event(),revents_:%d\r\n",revents_);

	if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
		if (logHup_) {
			LOGGING("Channel::handle_event() POLLHUP\r\n");
		}
		if (closeCallback_) closeCallback_();
	}

	/*if (revents_ & POLLNVAL) {
		LOGGING("Channel::handle_event() POLLNVAL\r\n");
	}*/

	if (revents_ & (EPOLLERR/* | POLLNVAL*/)) {
		if (errorCallback_) errorCallback_();
	}
	if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
		if (readCallback_) readCallback_();
	}
	if (revents_ & EPOLLOUT) {
		if (writeCallback_) writeCallback_();
	}

}


