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
#define LOG_LEVEL LOG_PRINT //directly print in console
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

EventLoop::EventLoop():
quit_(false)
{

}

EventLoop::~EventLoop()
{

}

void EventLoop::loop()
{

}

