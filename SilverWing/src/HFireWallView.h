#ifndef __HFIREWALL_H__
#define __HFIREWALL_H__

#include <View.h>
#include <String.h>
#include <iostream>

enum{
	M_USE_SOCK5 = 'MSO5',
	M_AUTH_REQ = 'AURQ'
};

class HFireWallView :public BView {
public:
					HFireWallView(BRect rect);
		virtual		~HFireWallView();
			bool	UseFirewall() {return fUseSock5;}
			bool	Auth() {return fAuth;}
	const char*		Server();
			uint32  Port();
	const char*		Username();
	const char*		Password();

protected:
	virtual void	MessageReceived(BMessage *message);
			void	InitGUI();
private:
			BString fServer;
			uint32 fPort;
			BString fUser;
			BString fPassword;
			bool	fUseSock5;
			bool	fAuth;
};
#endif