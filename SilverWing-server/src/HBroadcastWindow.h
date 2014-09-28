#ifndef __HBROADCAST_WINDOW_H__
#define __HBROADCAST_WINDOW_H__

#include <Window.h>
#include "CTextView.h"

enum{
	M_BROADCAST = 'MBRO',
	OK_MSG = 'MOKM'
};

class HBroadcastWindow :public BWindow {
public:
				HBroadcastWindow(BRect rect,const char* name,BLooper *target);
virtual			~HBroadcastWindow();
		void	InitGUI();	
virtual	void	MessageReceived(BMessage *message);
protected:
BLooper			*fTarget;
CTextView		*fTextView;
};
#endif