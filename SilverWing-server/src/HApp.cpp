#include <Autolock.h>
#include "HApp.h"
#include "HAboutWindow.h"
#include "HWindow.h"
#include "HPrefs.h"

#define APP_SIG "application/x-vnd.takamatsu-silverwing-server"

/**************************************************************
 * Contructor.
 **************************************************************/
HApp::HApp():BApplication(APP_SIG)
{
	fPrefs = new HPrefs("SilverWing-server-prefs");
	fPrefs->LoadPrefs();
	BRect rect;
	fPrefs->GetData("window_rect",&rect);
	fWindow = new HWindow(rect,"SilverWing-Server");
	fWindow->Show();
	
}

/**************************************************************
 * Destructor.
 **************************************************************/
HApp::~HApp()
{
	delete fPrefs;
}


/**************************************************************
 * MessageReceived.
 **************************************************************/
void
HApp::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	default:
		BApplication::MessageReceived(message);
	}
}

/**************************************************************
 * AboutRequested.
 **************************************************************/
void
HApp::AboutRequested()
{
	(new HAboutWindow("SilverWing server",
			__DATE__,
"Created by Atsushi Takamatsu @ Sapporo,Japan.",
"http://hp.vector.co.jp/authors/VA013465/garden_e.html",
"E-Mail: atsushi@io.ocn.ne.jp"))->Show();
}

/**************************************************************
 * QuitRequested.
 **************************************************************/
bool
HApp::QuitRequested()
{
	fPrefs->StorePrefs();
	return BApplication::QuitRequested();
}