/*
 *	Channel.cpp
 *
 *  Created on: Feb 09, 2017
 *      Author: Daniel yuan
 */

#include <algorithm>
#include "logging.h"
#include "Channel.h"

//-------------------------FdMamager class---------------------------------
//-------------------------------------------------------------------------
#define LOG_TAG "Channel"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

Channel::Channel(int fd__)
:fd_(fd__),events_(0),revents_(0)
{
}

Channel::~Channel()
{
}

void Channel::handleEvent()
{
	/*if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
		if (logHup_) {
			LOGGING("Channel::handle_event() POLLHUP\r\n");
		}
		if (closeCallback_) closeCallback_();
	}

	if (revents_ & POLLNVAL) {
		LOGGING("Channel::handle_event() POLLNVAL\r\n");
	}

	if (revents_ & (POLLERR | POLLNVAL)) {
		if (errorCallback_) errorCallback_();
	}
	if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
		if (readCallback_) readCallback_();
	}
	if (revents_ & POLLOUT) {
		if (writeCallback_) writeCallback_();
	}*/

}


