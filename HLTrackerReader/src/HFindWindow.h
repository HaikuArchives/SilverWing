#ifndef __HFINDWINDOW_H__
#define __HFINDWINDOW_H__

#include <Window.h>

enum{
M_SEARCH_MSG = 'MSRC',
};

class HFindWindow :public BWindow {
public:
					HFindWindow(BRect rect,const char* name = "Find");
		virtual		~HFindWindow();
			void	SetTarget(BWindow *win) {fTarget = win;}
			void	SetSearchText(const char* text);
	const char*		SearchText();

protected:
	virtual void	MessageReceived(BMessage *message);
			void	InitGUI();
private:
			BWindow *fTarget;
};
#endif