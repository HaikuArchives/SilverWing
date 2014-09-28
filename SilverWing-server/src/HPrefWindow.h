#ifndef __HPREFWINDOW_H__
#define __HPREFWINDOW_H__

#include <Window.h>

#include "HServerSetting.h"
#include "HTrackerSetting.h"
#include "HNameSetting.h"

enum{
	M_OK_MSG = 'MOKM'
};

class HPrefWindow :public BWindow {
public:
				HPrefWindow(BRect rect,const char* name);
virtual			~HPrefWindow();
		void	InitGUI();
virtual void	MessageReceived(BMessage *message);
protected:
HServerSetting*	fServerSetting;
HTrackerSetting* fTrackerSetting;
HNameSetting*	fNameSetting;
};
#endif