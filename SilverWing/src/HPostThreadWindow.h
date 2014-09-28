#ifndef __HPOSTTHREAD_WINDOW_H__
#define __HPOSTTHREAD_WINDOW_H__

#include <Window.h>
#include <String.h>

enum{
	POST_MESSAGE = 'POME',
	POST_THREAD_MESSAGE = 'POTM'
};

class HPostThreadWindow :public BWindow {
public:
				HPostThreadWindow(BRect rect,const char* name,const char* category,
							const char* subject="",uint16 reply =0,uint16 parent = 0);
virtual			~HPostThreadWindow();

protected:
virtual void	MessageReceived(BMessage *message);
		void	InitGUI();

private:
		uint16 fReply;
		uint16 fParent;
		BString fCategory;
};
#endif