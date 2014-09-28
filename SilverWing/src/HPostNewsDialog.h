#ifndef __HPOSTNEWSDIALOG_H__
#define __HPOSTNEWSDIALOG_H__

#include <Window.h>

enum{
	POST_NEWS_MSG = 'PONS',
	POST_NEWS_POST= 'POSN'
};

class HPostNewsDialog :public BWindow {
public:
				HPostNewsDialog(BRect rect,const char* name);
virtual			~HPostNewsDialog();
virtual void	MessageReceived(BMessage *message);
		void	InitGUI();

};
#endif