/*
 *Ppoller.cpp
 *
 *  Created on: Dec 26, 2017
 *      Author: Daniel yuan
 */
 
#include <sys/socket.h>
#include <sys/prctl.h>
#include <string.h>
#include <poll.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>  
#include <arpa/inet.h>
#include <sys/wait.h>
//#include <algorithm>

#include "realtime.h"
#include "logging.h"
#include "Poller.h"

#define LOG_TAG "Poller"
#define LOG_LEVEL LOG_PRINT
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

static void epoll_modify_event(int epollfd,int fd,int state)
{
     struct epoll_event ev;
     ev.events = state;
     ev.data.fd = fd;
     epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
}

static void epoll_add_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
}

static void epoll_delete_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
}

Poller::Poller():epollfd_(-1)
{
	epollfd_ = epoll_create(MAX_EVENTS);

}

Poller::~Poller()
{
	if(epollfd_ > 0)
		close(epollfd_);
}

int Poller::poll(int timeoutMs,ChannelList* activeChannels)
{
	int numEvents = epoll_wait(epollfd_,&(*events_.begin()),MAX_EVENTS,timeoutMs);
	if(numEvents > 0) {
		fillActiveChannels(numEvents,activeChannels);
		if (numEvents == events_.size()) {
			events_.resize(events_.size()*2);
		}
	}
}

void Poller::fillActiveChannels(int numEvents,ChannelList* activeChannels) const
{
	//assert(static_cast<size_t>(numEvents) <= events_.size());

	for (int i = 0; i < numEvents; ++i) {
		Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
		channel->set_revents(events_[i].events); //assign epoll events
		activeChannels->push_back(channel);
	}
}

//operation:EPOLL_CTL_DEL,EPOLL_CTL_DEL,EPOLL_CTL_MOD
void Poller::update(int operation, Channel* channel)
{
	struct epoll_event event;
	bzero(&event, sizeof event);
	event.events = channel->events();
	event.data.ptr = channel;
	int fd = channel->fd();

	if (epoll_ctl(epollfd_, operation, fd, &event) < 0) {
		//if (operation == EPOLL_CTL_DEL) {
			LOGGING("epoll_ctl op=%d,fd=%d\r\n",operation,fd);
		//} else {
			//LOGGING("epoll_ctl op=%d,fd=%d\r\n",operation,fd);
		//}
	}
}

void Poller::updateChannel(Channel* channel)
{
	Poller::assertInLoopThread();
	const int index = channel->index();
	if (index == cNew||index == cDeleted) {
	// a new one, add with EPOLL_CTL_ADD
		int fd =channel->fd();
		if(index == cNew) {
			if(channels_.find(fd) == channels_.end()) { //not find
				channels_[fd] = channel;	
			}		
		} else {// index == cDeleted
			if(channels_.find(fd) != channels_.end()) {//find
				if(channels_[fd] == channel) {
					return;
				}
			}
		}
	    channel->set_index(cAdded);
	    update(EPOLL_CTL_ADD, channel);
	} else {
	// update existing one with EPOLL_CTL_MOD/DEL
	    int fd = channel->fd();
		if(channels_.find(fd) != channels_.end()) { //find
			if(channels_[fd] == channel) {
				if(index == cAdded) {
					if (channel->isNoneEvent()) {
						update(EPOLL_CTL_DEL,channel);
						channel->set_index(cDeleted);
					} else {
						update(EPOLL_CTL_MOD,channel);
					}
				}
			}
		}
	}
}

void Poller::removeChannel(Channel* channel)
{
	Poller::assertInLoopThread();
	int fd = channel->fd();
	if(channels_.find(fd) != channels_.end()) { //find
		if(channels_[fd] == channel) {
			if(channel->isNoneEvent()) {
  				int index = channel->index();
				if(index == cAdded||index == cDeleted) {
					channels_.erase(fd);
					if (index == cAdded) {
						update(EPOLL_CTL_DEL, channel);
					}
					channel->set_index(cNew);
				}
			}	
		}
	}
}

