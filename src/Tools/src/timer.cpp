/*
 *Timer.cpp
 *
 *  Created on: Aug 28, 2017
 *      Author: honeywell
 */

#include "timer.h"
#include <fcntl.h>
#include <stdio.h>
//#include <string.h>
#include <sstream>
#include <unistd.h>
#include <sys/ioctl.h>
//#include <sys/select.h>
//#include <signal.h>
//#include <errno.h>
//#include <stdio.h>

static Timer *pTimer = NULL;

Timer::Timer():isTimerValid(false),timerFd(-1)
{
    timerFd = open(TMR_PATH,O_RDWR);
    if(timerFd < 0) {
        printf("open failed!\n");
        return;
    } else {
        printf("open timer driver successessfully, timerFd=%d\n",timerFd);
        isTimerValid = true;
    }
}

Timer::~Timer()
{
    if(timerFd > 0)
        close(timerFd);
}

void Timer::InstallTimerModule()
{
    //use_udmabuf = false; // Will be set to true if all conditions are met.
    //int bufferSize = GetSensorHeight() * GetSensorWidth();
    //if (bufferSize == 0) return;

    std::ostringstream modprobeCommand;
    modprobeCommand << "modprobe epit";
    if(system(modprobeCommand.str().c_str()) == -1) {
        printf("ERROR: Failed to load epit kernel object\n");
    }
    //else use_udmabuf = true;
}

void Timer::Start()
{
    printf("Tcp Server Start\r\n");
}

void Timer::Stop()
{
    printf("Tcp Server Stop\r\n");
}

int Timer::GetCount()
{
    if(isTimerValid) {
        return ioctl(timerFd,TMR_IOCTL_GET_CNT,0);//tick step is 1 ms
    } else
        return -1;
}

void EpitTimerInit(void)
{
    if(NULL == pTimer)
        pTimer = new Timer();
}
void EpitTimerDeInit(void)
{
    if(NULL == pTimer)
        return;
    delete pTimer;
}

int  GetEpitTimerCount(void)
{
    return pTimer->GetCount();
}

/*uint64_t get_time_ms(void)
{
	return (uint64_t)(pTimer->GetCount());
}*/