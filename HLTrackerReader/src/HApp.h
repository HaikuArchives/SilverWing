#ifndef __HAPP_H__
#define __HAPP_H__
#include <Application.h>
#include <NetworkKit.h>
#include "HWindow.h"

#define _(String) ((HApp*)be_app)->reserved(String)

class HApp: public BApplication {
public:
					HApp();
		virtual		~HApp();
	const char*		reserved(const char* text) {return text;}

protected:
	virtual void	MessageReceived(BMessage *message);
	virtual void	AboutRequested();
	
private:
		HWindow *fWindow;
};
#endif