#ifndef __HTIMER_H__
#define __HTIMER_H__

#include <Looper.h>
#include <Locker.h>
#include <time.h>

class HTimer {
public:
				HTimer(uint32 interval,uint32 sleep_time = 1000);
virtual			~HTimer();
		void	StartTimer();
		void	EndTimer();
		void	ResetTimer();
virtual void	Timer()=0;
protected:
static	int32	ThreadFunc(void* data);
private:
	thread_id fThread;
	bool	  fCancel;
	BLooper	  *fLooper;
	time_t 	  fStartTime;
	time_t	  fEndTime;
	uint32	  fInterval;
	BLocker	  fLocker;
	uint32	  fSleepTime;
};
#endif