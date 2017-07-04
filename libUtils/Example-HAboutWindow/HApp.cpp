#include "HApp.h"
#include "HWindow.h"
#include "HAboutWindow.h"

/***********************************************************
 *
 ***********************************************************/
HApp::HApp() :BApplication("application/x-vnd.takamatsu-aboutwin")
{
	HWindow *win = new HWindow(BRect(40,40,200,200),"About Example");
	win->Show();	
}

/***********************************************************
 *
 ***********************************************************/
HApp::~HApp()
{
}

/***********************************************************
 * About
 ***********************************************************/
void
HApp::AboutRequested()
{
	HAboutWindow *win = new HAboutWindow("Example"
				,__DATE__
				,"About window example"
				,"http://hp.vector.co.jp/authors/VA013465/garden_e.html"
				,"atsushi@io.ocn.ne.jp");
	win->Show();	
									
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