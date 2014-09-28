#ifndef __HConnectWindow_H__
#define __HConnectWindow_H__

#include <Window.h>

enum{
CONNECT_CONNECT_MSG = 'CCMG',
CONNECT_OPTION_MSG = 'COMG',
CONNECT_BOOKMARK_MSG = 'CBKM',
CONNECT_CONNECT_REQUESTED = 'CCRM',
CONNECT_ADD_REQUESTED = 'CADR'
};

class HConnectWindow :public BWindow {
public:

							HConnectWindow(BRect rect,const char* name,bool edit);
					virtual	~HConnectWindow();
				virtual void MessageReceived(BMessage *msg);
				virtual bool QuitRequested();
						void SaveServer(const char* address,const char* login ,const char* password,int16 port);
						void InitGUI(bool edit);
						void LoadServer(const char* name);
						void SetAddress(const char* address);
						void SetPort(const char* port);
						void SetLogin(const char* login);
						void SetPassword(const char* password);
					
					BMenu*	 InitBookmarks();
protected:
};
#endif		
					