#ifndef __HOTHERSETTING_VIEW_H__
#define __HOTHERSETTING_VIEW_H__

#include <View.h>

enum{
	M_KEEP_ALIVE_CHANGED = 'MKAC',
	M_CONFIG_SOUND = 'MSIG',
	M_SELECT_FOLDER = 'MBOT'
};

class FolderPanel;

class HOtherSettingView: public BView {
public:	
				HOtherSettingView(BRect rect);
	virtual		~HOtherSettingView();
		void	InitGUI();
		bool	LogLogin();
		bool	Preload();
		bool	RefuseChat();
		bool	EnableSound();
		bool	QueueDownload();
		bool	TimeStamp();
		bool	TaskIconfy();
		bool	KeepAlive();
		bool	SingleWindow();
		int32	Interval();
		bool	MessageChat();
	const char*	DownloadPath();
protected:
	virtual void	MessageReceived(BMessage *message);
private:
	FolderPanel 	*fFilePanel;
};
#endif