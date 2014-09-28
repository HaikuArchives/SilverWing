#ifndef __HADDTRACKER_WINDOW_H__
#define __HADDTRACKER_WINDOW_H__

#include <Window.h>

enum{
	M_OK_MSG = 'MOKM',
	M_REFRESH_TRACKER_ITEM = 'MREI',
	M_END_NAME = 'MENN',
	M_NEW_TRACKER = 'MNET'
};

class HAddTrackerWindow: public BWindow {
public:
				HAddTrackerWindow(BRect rect,const char* name);
				~HAddTrackerWindow();
	
			void SaveTracker(const char* name,const char* address);
protected:
	virtual void MessageReceived(BMessage* message);
			void InitGUI();
};
#endif