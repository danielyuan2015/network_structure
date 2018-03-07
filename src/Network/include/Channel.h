/*
 * Channel.h
 *
 *  Created on: Feb 09, 2018
 *      Author: Daniel Yuan
 */
#ifndef SRC_CHANNEL_H_
#define SRC_CHANNEL_H_

#include "socket.h"
#include <functional>
#include <iostream>
#include <memory>
#include "EventLoop.h"

using namespace std;
class EventLoop;

class Channel:public cNonCopyable
{
public:
	typedef std::function<void()> EventCallback;

	Channel(EventLoop* loop,int fd);
	~Channel();
	
	/// Tie this channel to the owner object managed by shared_ptr,
	/// prevent the owner object being destroyed in handleEvent.
	void tie(const std::shared_ptr<void>&);	
	void handleEvent(/*Timestamp receiveTime*/);

	// for Poller
	int index() { return index_; }
	void set_index(int idx) { index_ = idx; }
	
	void setReadCallback(const EventCallback& cb)
	{ readCallback_ = cb; }
	void setWriteCallback(const EventCallback& cb)
	{ writeCallback_ = cb; }
	void setCloseCallback(const EventCallback& cb)
	{ closeCallback_ = cb; }
	void setErrorCallback(const EventCallback& cb)
	{ errorCallback_ = cb; }
	
	void enableReading() { events_ |= kReadEvent; update(); }
	void disableReading() { events_ &= ~kReadEvent; update(); }
	void enableWriting() { events_ |= kWriteEvent; update(); }
	void disableWriting() { events_ &= ~kWriteEvent; update(); }
  	void disableAll() { events_ = kNoneEvent; update(); }

	int fd() const { return fd_; }

	int events() const { return events_; }
	void set_revents(int revt) { revents_ = revt; } // used by pollers
	bool isNoneEvent() const { return events_ == kNoneEvent; }
	
	void remove();


	// for debug
	std::string reventsToString() const;
	std::string eventsToString() const;

private:
	void update();
	void handleEventWithGuard(/*Timestamp receiveTime*/);
	static std::string eventsToString(int fd, int ev);

	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;

	EventLoop* loop_;
	const int  fd_;
	int 	   events_;
	int 	   revents_;
	int 	   index_; // used by Poller.
	bool	   logHup_;

	std::weak_ptr<void> tie_;
	bool tied_;
	bool eventHandling_;
	bool addedToLoop_;

	//ReadEventCallback readCallback_;
	EventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback closeCallback_;
	EventCallback errorCallback_;

protected:
};

#endif
