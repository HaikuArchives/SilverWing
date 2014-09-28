#ifndef __HAPP_H__
#define __HAPP_H__

/********** SYSTEM HEADER ***********/
#include <Window.h>
#include <string>
#include <Application.h>
#include <iostream>


class HWindow;
class HPrefs;

#define APP_VERSION "alpha1"

class HApp:public BApplication {
public:
					HApp();
			virtual ~HApp();
	virtual void	MessageReceived(BMessage *message);
	virtual bool	QuitRequested();
	virtual void	AboutRequested();
	HWindow*		Window() {return fWindow;}
	HPrefs*			Prefs() {return fPrefs;}
protected:
	HWindow*		fWindow;
	HPrefs*			fPrefs;
};
#endif
		