#include "HApp.h"
#include <iostream>
#include <Roster.h>
#include "RectUtils.h"
#include "MAlert.h"
#include "HAboutWindow.h"
#include "HAddTrackerWindow.h"
#include "HConnectWindow.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HApp::HApp():BApplication("application/x-vnd.takamatsu-hltrackerreader")
{
	BRect rect;
	RectUtils utils;
	if(utils.LoadRectFromApp("servwinrect",&rect) == false)
	{
		rect.Set(40,40,500,300);
	}
	fWindow = new HWindow(rect,_("Tracker"));
	fWindow->Show();
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HApp::~HApp()
{
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HApp::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case H_END_SEARCH:
	{
		fWindow->PostMessage(message);
		break;
	}
	case M_SET_STATUS:
	case H_RECEIVE_SERVER:
	{
		fWindow->PostMessage(message);
		break;
	}
	case M_NEW_TRACKER:
	{
		fWindow->PostMessage(message);
		break;
	}
	case CONNECT_CONNECT_REQUESTED:
	{
		const char* login = message->FindString("login");
		const char* password = message->FindString("password");
		const char* address = message->FindString("address");
		int16 port;
		message->FindInt16("port",(int16*)&port);
		BMessage msg(B_REFS_RECEIVED);
		msg.AddString("address",address);
		msg.AddInt16("port",port);
		msg.AddString("password",password);
		msg.AddString("login",login);
		be_roster->Launch("application/x-vnd.takamatsu-silverwing",&msg);
		break;
	}
	default:
		BApplication::MessageReceived(message);
	}
}

/***********************************************************
 * AboutRequested
 ***********************************************************/
void
HApp::AboutRequested()
{	
	(new HAboutWindow("HLTrackerReader",__DATE__,
			"Created by Atsushi Takamatsu @ Sapporo,Japan."))->Show();
}

/***********************************************************
 * main
 ***********************************************************/
int main()
{
	HApp app;
	app.Run();
	return 0;
}