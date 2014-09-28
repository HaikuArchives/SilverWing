#ifndef __HWINDOW_H__
#define __HWINDOW_H__
#include <Window.h>
#include <String.h>
#include <iostream>
#include "HTrackerConnection.h"
enum{
	M_ADD_NEW_TRACKER = 'ADNT',
	M_DEL_TRACKER = 'DELT',
	M_REFRESH_FILTER = 'MRFL',
	M_REFRESH_MSG = 'MREM',
	M_SET_STATUS = 'MSES',
	M_CLICK_LIST = 'MCLI',
	M_CONNECT_MSG = 'MCON',
	M_FIND_MSG = 'MFIN',
	M_FIND_NEXT_MSG = 'FNET'
};

class HWindow :public BWindow {
public:
						HWindow(BRect rect,const char* name);
		virtual			~HWindow();
			void		InitGUI();
			void		InitMenu();
			void		FindTrackers();
		const char*		LookupAddress(const char* name);
		const char*		Keyword();
			void		DeleteTracker(const char* text);
			void		Search(const char* text,uint32 start);

protected:
	virtual void		MessageReceived(BMessage *message);
	virtual bool		QuitRequested();
	virtual	void		DispatchMessage(BMessage *message,BHandler *hander);
	virtual void 		MenusBeginning(void);
	virtual	void		Pulse();

private:
			bool		fRefresh;
			thread_id	fSearchThread;
			int32		fOldServerCount;
			BString 	fStatusString;
			BString		fSearchText;
			bool		fAlive;
			uint32		fSearchIndex;
};
#endif