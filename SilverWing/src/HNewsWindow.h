#ifndef __HNEWSWINDOW_H__
#define __HNEWSWINDOW_H__

#include <Window.h>
#include <string>
#include <iostream>

class URLTextView;
class HFileCaption;

enum{
	NEWS_POST_MESSAGE = 'NPOM',
	NEWS_POSTED = 'NPOD',
	NEWS_SET_NEWS = 'NSET',
	NEWS_CLOSE_WINDOW = 'NCLO',

};

class HNewsWindow: public BWindow {
public:
					HNewsWindow(BRect rect,const char* name);
	virtual			~HNewsWindow();
	virtual void	MessageReceived(BMessage *message);
			void	InitGUI();
			void	SetNews(const char* text);
			void	AddPosted(const char* text);
	virtual	bool	QuitRequested();
			void	SetTarget(BLooper* target) {fTarget = target;}
protected:
			BLooper *fTarget;
			HFileCaption *fCaption;
};
#endif