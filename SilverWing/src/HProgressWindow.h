#ifndef __HPROGRESS_WIN_H__
#define __HPROGRESS_WIN_H__

#include <Window.h>
#include <StatusBar.h>

enum{
	M_UPDATE_MSG = 'UPRO',
	M_SET_MAX_VALUE = 'MSEM',
	M_RESET_MSG = 'MRET'
};

class HProgressWindow :public BWindow {
public:
					HProgressWindow(BRect rect, const char* title);
		virtual		~HProgressWindow();
		BStatusBar* Status() {return fStatusBar;}

protected:
			void	InitGUI();
virtual		void	MessageReceived(BMessage *message);

private:
		BStatusBar*	fStatusBar;
};
#endif