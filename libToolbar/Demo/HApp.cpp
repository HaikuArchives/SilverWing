#include "HApp.h"
#include "HWindow.h"

#define APP_SIG "application/x-vnd.takamatsu.toolbardemo"

HApp::HApp() :BApplication(APP_SIG)
{
	HWindow *win = new HWindow(BRect(50,50,350,350),"Demo toolbar");
	win->Show();
}	

HApp::~HApp()
{

}

