#include "HWindow.h"

HWindow::HWindow(BRect rect ,const char* name)
	:BWindow(rect,name,B_DOCUMENT_WINDOW,0)
{
	InitMenu();
	InitGUI();
}

HWindow::~HWindow()
{

}

void
HWindow::InitGUI()
{
}

void
HWindow::InitMenu()
{
	BMenuBar *menuBar = new BMenuBar(Bounds(),"MENUBAR");
	BMenu *menu;
	BMenuItem *item;
	
	menu = new BMenu("File");
	item = new BMenuItem("About...",new BMessage(B_ABOUT_REQUESTED),0,0);
	item->SetTarget(be_app);
	menu->AddItem(item);
	menuBar->AddItem(menu);
	this->AddChild(menuBar);
}


bool
HWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}