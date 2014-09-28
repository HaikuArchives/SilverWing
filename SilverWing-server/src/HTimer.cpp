#include "HTimer.h"

#include <Autolock.h>

/***********************************************************
 * Constructor
 ***********************************************************/
HTimer::HTimer(uint32 interval,uint32 sleep_time)
{
	fCancel = true;
	fInterval = interval;
	fSleepTime = sleep_time;
}

/***********************************************************
 * Destructor
 ***********************************************************/
HTimer::~HTimer()
{
	if(fCancel == false)
		EndTimer();
}

/***********************************************************
 * StartTimer
 ***********************************************************/
void
HTimer::StartTimer()
{
	fCancel = false;
	fStartTime = time(NULL);
	fThread = ::spawn_thread(ThreadFunc,"timer",B_LOW_PRIORITY,this);
	::resume_thread(fThread);
}

/***********************************************************
 * TimerThread
 ***********************************************************/
int32
HTimer::ThreadFunc(void* data)
{
	HTimer *timer = (HTimer*)data;
	while(!timer->fCancel)
	{
		timer->fEndTime = time(NULL);
		double diff = difftime(timer->fEndTime,timer->fStartTime);
		
		if(diff >= timer->fInterval)
		{
			timer->Timer();
			timer->fStartTime = time(NULL);
			::snooze(timer->fInterval);
		}
		::snooze(timer->fSleepTime);
	}
	return 0;	
}

/***********************************************************
 * EndTimer
 ***********************************************************/
void
HTimer::EndTimer()
{
	fCancel = true;
	
	::kill_thread(fThread);
}

/***********************************************************
 * ResetTimer
 ***********************************************************/
void
HTimer::ResetTimer()
{
	BAutolock lock(fLocker);
	fStartTime = time(NULL);
}