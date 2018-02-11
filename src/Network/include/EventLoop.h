/*
 * EventLoop.h
 *
 *  Created on: Feb 11, 2018
 *      Author: Daniel Yuan
 */
#ifndef SRC_EVENTLOOP_H_
#define SRC_EVENTLOOP_H_

#include <vector>
#include "socket.h"
#include "Channel.h"

///
/// Reactor, at most one per thread.
///
/// This is an interface class, so don't expose too much details.
class EventLoop:public cNonCopyable
{
public:
	typedef std::function<void()> Functor;
	typedef std::vector<Channel*> ChannelList;

	EventLoop();
	~EventLoop();	// force out-line dtor, for scoped_ptr members.

	///
	/// Loops forever.
	///
	/// Must be called in the same thread as creation of the object.
	///
	void loop();
private:
	bool quit_;
	ChannelList activeChannels_;
protected:
};

#endif

