#ifndef __NEWS15WINDOW_H__
#define __NEWS15WINDOW_H__

#include <Window.h>

class SplitPane;
class HFileCaption;

enum{
	NEWS15_CLOSE_WINDOW = 'N5CW',
	NEWS15_REFRESH_MESSAGE = 'NERF',
	NEWS15_POST_THREAD_MESSAGE= 'NPOT',
	NEWS15_DELETE_MESSAGE = 'NDEL',
	NEWS15_REPLY_MESSAGE = 'NREP',
	NEWS15_CREATE_CATEGORY = 'NCRC',
	NEWS15_CREATE_GROUP = 'NCRG',
};

class HNews15Window :public BWindow {
public:
					HNews15Window(BRect rect,const char* title);
		virtual		~HNews15Window();

			void	SetTarget(BLooper* looper) {fTarget = looper;}
	HFileCaption*	Caption() {return fCaption;}
protected:
	virtual void	MessageReceived(BMessage *message);
			void	InitGUI();
	virtual bool	QuitRequested();
private:
			BLooper *fTarget;
			SplitPane *fVSplitter;
			SplitPane *fHSplitter;
			HFileCaption *fCaption;
};
#endif