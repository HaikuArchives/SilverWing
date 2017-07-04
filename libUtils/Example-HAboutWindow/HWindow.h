#ifndef __HWINDOW_H__
#define __HWINDOW_H__

#include <Be.h>

class HWindow :public BWindow {
public:
					HWindow(BRect rect ,const char* name);
	virtual			~HWindow();

protected:
	virtual	bool	QuitRequested();
			void	InitGUI();
			void	InitMenu();
};
#endif