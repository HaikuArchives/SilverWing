#include "HView.h"
#include <Window.h>

/**************************************************************
 * Contructor.
 **************************************************************/
HView::HView(BRect rect,const char* name)
	:BView(rect,name,B_FOLLOW_TOP_BOTTOM|B_FOLLOW_LEFT,B_PULSE_NEEDED|B_WILL_DRAW)
{
	fStartTime = time(NULL);
}

/**************************************************************
 * Destructor.
 **************************************************************/
HView::~HView()
{
}

/**************************************************************
 * Pulse.
 *	Send M_TIMER_MSG to parent window.
 **************************************************************/
void
HView::Pulse()
{
	fEndTime = time(NULL);
	float diff = difftime(fEndTime,fStartTime);
	if(diff >= 180)
	{
		Window()->PostMessage(M_TIMER_MSG);
		fStartTime = time(NULL);
	}
}