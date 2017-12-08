/*
 * Timer.h
 *
 *  Created on: Aug 28, 2017
 *      Author: honeywell
 */

#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_
//#include <stdint.h>

class Timer
{
#define TMR_PATH "/dev/epit"
#define TMR_IOCTL_GET_CNT           0x6B0B
public:
    Timer();
    virtual ~Timer();
    void InstallTimerModule();
    void Start();
    void Stop();
    int GetCount();

private:
    bool isTimerValid;
    int timerFd;
protected:
};

void EpitTimerInit(void);
void EpitTimerDeInit(void);
int  GetEpitTimerCount(void);
//uint64_t get_time_ms(void);

#endif /* SRC_TIMER_H_ */


