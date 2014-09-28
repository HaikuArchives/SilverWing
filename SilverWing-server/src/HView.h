#ifndef __HVIEW_H__
#define __HVIEW_H__
#include <View.h>
#include <time.h>

enum{
	M_TIMER_MSG = 'MTIM'
};	

class HView :public BView {
public:	
				HView(BRect rect,const char* name);
virtual 		~HView();
		
protected:
virtual void	Pulse();
		void	SendTrackerUpdate();
private:
	time_t	fStartTime,fEndTime;
};
#endif