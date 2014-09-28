#ifndef __HUSERTIMER_H__
#define __HUSERTIMER_H__

#include "HTimer.h"
#include "HUserItem.h"

enum{
	H_USER_TIMER = 'HUST'
};

class HUserTimer :public HTimer {
public:
				HUserTimer(uint32 interval
							,uint32 sock
							,BLooper *taget);
virtual			~HUserTimer();
virtual void	Timer();

protected:
	BLooper*	fTarget;
	uint32		fSock;
};
#endif