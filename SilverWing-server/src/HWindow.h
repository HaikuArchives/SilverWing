#ifndef __HWINDOW_H__
#define __HWINDOW_H__

/******* SYSTEM HEADER ********/
#include <Window.h>
#include <string>
#include <iostream>
#include <File.h>
#include <FileGameSound.h>
#include <SimpleGameSound.h>

enum{
	M_LOG_MESSAGE = 'MWLO',
	M_START_SERVER = 'MRUN',
	M_STOP_SERVER = 'MSTP',
	M_ACCOUNT_MSG = 'MACT',
	M_SETTING_MSG = 'MSEM',
	M_BROADCAST_MSG = 'MBCM'
};

class CTextView;
class HStatusView;
class TServer;
class HTrackerConnection;

class HWindow :public BWindow {
public:
					HWindow(BRect rect,const char* name);
	virtual 		~HWindow();
	virtual void	MessageReceived(BMessage *message);
	virtual bool	QuitRequested();

			void	InitGUI();
			void	InitMenu();
			void	InsertLog(const char* text);
			void	SetInfomations(uint32 type);
			void	InitInfomations();
			void	WriteToTracker();
	virtual void	MenusBeginning();
		TServer*	Server()const {return fServer;}
protected:
			void	InitTracker();
			void	ClearTracker();
			void	SaveLog(const char* text);
			void	PlaySound(uint32 type);
	static	int32	PlayThread(void* data);
private:
			TServer*	fServer;
			CTextView* fLogView;
			HStatusView *fEditUsers;
			HStatusView *fEditMaxUsers;
			HStatusView *fEditDownloads;
			HStatusView *fEditUploads;
			BFile		*fLogFile;
			uint32		fUploads;
			uint32 		fDownloads;
			uint32		fUsers;
			uint32		fTotalUsers;
			BList		fTrackerList;
			uint32		fCurrentEffect;
};
#endif