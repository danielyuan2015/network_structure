//---------------------------------------------------------------------------
//  Copyright (C) Honeywell International, Inc. 2017
//---------------------------------------------------------------------------

#ifndef TIMEOUT_H_F3E1EF42_35D6_11D2_93BE_00805F2987D9
#define TIMEOUT_H_F3E1EF42_35D6_11D2_93BE_00805F2987D9

#include <stdint.h>
#include <time.h>
#include <thread>
#include <atomic>

/*-------------------------------------------------------------------------------
   CTimeout provides a mechanism for setting a time duration and periodically
   checking the timeout object to see if the duration has expired.  </p>
*/
//-------------------------------------------------------------------------------
class CTimeout
{
public:
	CTimeout()
	{
		start_ns = 0;
		duration_ns = 0;
	}
	CTimeout( uint32_t uSec )
	{
		SetExpiration( uSec );
	}

	void	Reset(void)
	{
		start_ns = GetTimeNs();
	}

	void	ms( uint32_t mSec )
	{
		SetExpiration(mSec*1000);
	}

	void	us( uint32_t uSec )
	{
		SetExpiration(uSec);
	}

	void	SetExpiration( uint32_t uSec )
	{
		start_ns = GetTimeNs();
		duration_ns = uSec*1000;
	}

	uint32_t GetExpiration(void)
	{
		return duration_ns/1000;
	}

	bool	HasExpired()
	{
		uint64_t nDiff = GetTimeNs() - start_ns;
		return (nDiff >= duration_ns);  // overflow is automatically handled by binary complement. See below.
	}

	bool	NotExpired()
	{
		return !HasExpired();
	}

	uint32_t GetRestTime(void)
	{
		int64_t temp = (int64_t)start_ns+(int64_t)duration_ns-(int64_t)GetTimeNs();
		uint32_t time = temp < 0 ? 0 : (uint32_t)(temp/1000);

		return time;
	}

private:
	static uint64_t GetTimeNs(void)
	{
		struct timespec spec;
		clock_gettime(CLOCK_MONOTONIC, &spec);
		uint64_t ns = spec.tv_sec * 1000000000 + spec.tv_nsec;
		return ns;
	}

	uint64_t start_ns;
	uint64_t duration_ns;
};


class	CTimeoutmS
{
public:
	CTimeoutmS()
	{
		start_ns = 0;
		duration_ns = 0;
	}
	CTimeoutmS( uint32_t mSec )
	{
		SetExpiration( mSec );
	}

	void Reset(void)
	{
		start_ns = GetTimeNs();
	}


	void SetExpiration( uint32_t mSec )
	{
		start_ns = GetTimeNs();
		duration_ns = (uint64_t)mSec*1000000;
	}

	bool HasExpired()
	{
		uint64_t nDiff = GetTimeNs() - start_ns;
		return (nDiff >= duration_ns);  // overflow is automatically handled by binary complement. See below.
	}

//	bool NotExpired()
//	{
//		return !HasExpired();
//	}
	uint32_t GetExpiration(void)
	{
		return duration_ns/1000000;
	}

	uint32_t GetRestTime(void)
	{
		int64_t temp = (int64_t)start_ns+(int64_t)duration_ns-(int64_t)GetTimeNs();
		uint32_t time = temp < 0 ? 0 : (uint32_t)(temp/1000000);

		return time;
	}

	uint32_t GetElapsed(void)
	{
		return GetTimeNs() - start_ns;

	}
private:
	static uint64_t GetTimeNs(void)
	{
		struct timespec spec;
		clock_gettime(CLOCK_MONOTONIC, &spec);
		uint64_t ns = spec.tv_sec * 1000000000ULL + spec.tv_nsec;
		return ns;
	}

	uint64_t start_ns;
	uint64_t duration_ns;
};

class TimerCallback
{
public:
	TimerCallback() {nThread = 0;};

	template <class callable, class ... arguments>
	void Start(int after, callable&& f, arguments&&... args) {
		std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));

		//disable previous timer if exist
		Stop();

		std::atomic<bool> *enable = new std::atomic<bool>(true);
		std::atomic<unsigned int> *thread = &nThread;
		lastEnable = enable;
		nThread++;

		std::thread([after, task, enable, thread]() {
			std::this_thread::sleep_for(std::chrono::milliseconds(after));
			if(*enable) task();
			delete enable;
			(*thread)--;
		}).detach();
	}

	void Stop() {
		if(lastEnable != NULL && nThread > 0)
			*lastEnable = false;
	};

private:
	std::atomic<bool> *lastEnable = NULL;
	std::atomic<unsigned int> nThread;
};

//vector<int> &ve

/*
 
Test program to proof the algorithm of the HasExpired() function:
----------------------------------------------------------------
// We use UCHAR to as a model for better understanding:
void test1(void)
{
	UCHAR start = 250;
	UCHAR duration = 20;
	UCHAR counter = start;
	UCHAR temp;
	signed char Diff;
	signed int nDiff;
	bool elapsed;
	for(UCHAR i=0; i<255; i++)
	{
		Diff = counter-start;
		nDiff = counter-start;
		elapsed = (((UCHAR)Diff) >= duration);
		temp = ((UCHAR)Diff);
		printf("start=%3i, counter=%4i, diff=%4i(%03X) - start=%02X, counter=%02X, diff=%03X, temp=%03X - %i\n",
			(int)start, (int)counter, (int)nDiff, (int)nDiff&0xfff, (UINT)start, (UINT)counter, (UINT)0xfff & Diff, (UINT) temp, elapsed);
		counter++;
		counter &=0xff;
	}
}
output:
start=250, counter= 250, diff=   0(000) - start=FA, counter=FA, diff=000, temp=000 - 0
start=250, counter= 251, diff=   1(001) - start=FA, counter=FB, diff=001, temp=001 - 0
start=250, counter= 252, diff=   2(002) - start=FA, counter=FC, diff=002, temp=002 - 0
start=250, counter= 253, diff=   3(003) - start=FA, counter=FD, diff=003, temp=003 - 0
start=250, counter= 254, diff=   4(004) - start=FA, counter=FE, diff=004, temp=004 - 0
start=250, counter= 255, diff=   5(005) - start=FA, counter=FF, diff=005, temp=005 - 0
start=250, counter=   0, diff=-250(F06) - start=FA, counter=00, diff=006, temp=006 - 0
start=250, counter=   1, diff=-249(F07) - start=FA, counter=01, diff=007, temp=007 - 0
start=250, counter=   2, diff=-248(F08) - start=FA, counter=02, diff=008, temp=008 - 0
start=250, counter=   3, diff=-247(F09) - start=FA, counter=03, diff=009, temp=009 - 0
start=250, counter=   4, diff=-246(F0A) - start=FA, counter=04, diff=00A, temp=00A - 0
start=250, counter=   5, diff=-245(F0B) - start=FA, counter=05, diff=00B, temp=00B - 0
start=250, counter=   6, diff=-244(F0C) - start=FA, counter=06, diff=00C, temp=00C - 0
start=250, counter=   7, diff=-243(F0D) - start=FA, counter=07, diff=00D, temp=00D - 0
start=250, counter=   8, diff=-242(F0E) - start=FA, counter=08, diff=00E, temp=00E - 0
start=250, counter=   9, diff=-241(F0F) - start=FA, counter=09, diff=00F, temp=00F - 0
start=250, counter=  10, diff=-240(F10) - start=FA, counter=0A, diff=010, temp=010 - 0
start=250, counter=  11, diff=-239(F11) - start=FA, counter=0B, diff=011, temp=011 - 0
start=250, counter=  12, diff=-238(F12) - start=FA, counter=0C, diff=012, temp=012 - 0
start=250, counter=  13, diff=-237(F13) - start=FA, counter=0D, diff=013, temp=013 - 0
start=250, counter=  14, diff=-236(F14) - start=FA, counter=0E, diff=014, temp=014 - 1
start=250, counter=  15, diff=-235(F15) - start=FA, counter=0F, diff=015, temp=015 - 1
start=250, counter=  16, diff=-234(F16) - start=FA, counter=10, diff=016, temp=016 - 1
start=250, counter=  17, diff=-233(F17) - start=FA, counter=11, diff=017, temp=017 - 1
 
// and here with ULONG, just to be sure
void test2(void)
{
	ULONG start = ULONG_MAX-4;
	ULONG counter = start;
	ULONG duration = 20;
	signed int nDiff;
	bool elapsed;
	for(int i=0; i<100; i++)
	{
		nDiff = counter-start;
		elapsed = (((ULONG)nDiff) >= duration);
		printf("start=%u, counter=%10u, diff=%4i - start=%08X, counter=%08X, diff=%08X - %i\n",
			start, counter, nDiff, start, counter, nDiff, elapsed);
		counter++;
	}
}
output:
start=4294967291, counter=4294967291, diff=   0 - start=FFFFFFFB, counter=FFFFFFFB, diff=00000000 - 0
start=4294967291, counter=4294967292, diff=   1 - start=FFFFFFFB, counter=FFFFFFFC, diff=00000001 - 0
start=4294967291, counter=4294967293, diff=   2 - start=FFFFFFFB, counter=FFFFFFFD, diff=00000002 - 0
start=4294967291, counter=4294967294, diff=   3 - start=FFFFFFFB, counter=FFFFFFFE, diff=00000003 - 0
start=4294967291, counter=4294967295, diff=   4 - start=FFFFFFFB, counter=FFFFFFFF, diff=00000004 - 0
start=4294967291, counter=         0, diff=   5 - start=FFFFFFFB, counter=00000000, diff=00000005 - 0
start=4294967291, counter=         1, diff=   6 - start=FFFFFFFB, counter=00000001, diff=00000006 - 0
start=4294967291, counter=         2, diff=   7 - start=FFFFFFFB, counter=00000002, diff=00000007 - 0
start=4294967291, counter=         3, diff=   8 - start=FFFFFFFB, counter=00000003, diff=00000008 - 0
start=4294967291, counter=         4, diff=   9 - start=FFFFFFFB, counter=00000004, diff=00000009 - 0
start=4294967291, counter=         5, diff=  10 - start=FFFFFFFB, counter=00000005, diff=0000000A - 0
start=4294967291, counter=         6, diff=  11 - start=FFFFFFFB, counter=00000006, diff=0000000B - 0
start=4294967291, counter=         7, diff=  12 - start=FFFFFFFB, counter=00000007, diff=0000000C - 0
start=4294967291, counter=         8, diff=  13 - start=FFFFFFFB, counter=00000008, diff=0000000D - 0
start=4294967291, counter=         9, diff=  14 - start=FFFFFFFB, counter=00000009, diff=0000000E - 0
start=4294967291, counter=        10, diff=  15 - start=FFFFFFFB, counter=0000000A, diff=0000000F - 0
start=4294967291, counter=        11, diff=  16 - start=FFFFFFFB, counter=0000000B, diff=00000010 - 0
start=4294967291, counter=        12, diff=  17 - start=FFFFFFFB, counter=0000000C, diff=00000011 - 0
start=4294967291, counter=        13, diff=  18 - start=FFFFFFFB, counter=0000000D, diff=00000012 - 0
start=4294967291, counter=        14, diff=  19 - start=FFFFFFFB, counter=0000000E, diff=00000013 - 0
start=4294967291, counter=        15, diff=  20 - start=FFFFFFFB, counter=0000000F, diff=00000014 - 1
start=4294967291, counter=        16, diff=  21 - start=FFFFFFFB, counter=00000010, diff=00000015 - 1
start=4294967291, counter=        17, diff=  22 - start=FFFFFFFB, counter=00000011, diff=00000016 - 1
start=4294967291, counter=        18, diff=  23 - start=FFFFFFFB, counter=00000012, diff=00000017 - 1
start=4294967291, counter=        19, diff=  24 - start=FFFFFFFB, counter=00000013, diff=00000018 - 1
start=4294967291, counter=        20, diff=  25 - start=FFFFFFFB, counter=00000014, diff=00000019 - 1
 
 */

#endif
