#include "HUserTimer.h"
#include <Message.h>

HUserTimer::HUserTimer(uint32 interval,uint32 sock,BLooper *target)
	:HTimer(interval,60*1000)
	,fTarget(target)
	,fSock(sock)
{
}

HUserTimer::~HUserTimer()
{
}

void
HUserTimer::Timer()
{
	BMessage msg(H_USER_TIMER);
	msg.AddInt32("sock",fSock);
	fTarget->PostMessage(&msg);
}