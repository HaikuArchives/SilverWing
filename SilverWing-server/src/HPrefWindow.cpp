#include <TabView.h>
#include <Button.h>

#include "HPrefWindow.h"
#include "HApp.h"
#include "HPrefs.h"

/**************************************************************
 * Contructor.
 **************************************************************/
HPrefWindow::HPrefWindow(BRect rect,const char* name)
	:BWindow(rect,name,B_TITLED_WINDOW_LOOK,B_MODAL_APP_WINDOW_FEEL,B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
{
	InitGUI();
	AddShortcut('W',0,new BMessage(B_QUIT_REQUESTED));
	AddShortcut(B_RETURN,0,new BMessage(M_OK_MSG));
}

/**************************************************************
 * Destructor.
 **************************************************************/
HPrefWindow::~HPrefWindow()
{

}

/**************************************************************
 * Setup GUI.
 **************************************************************/
void
HPrefWindow::InitGUI()
{
	BRect rect = Bounds();
	rect.bottom -= 30;
	BTabView *tabview = new BTabView(rect,"tabview");
	tabview->SetViewColor(216,216,216,0);
	BTab *tab;
	BRect frame = tabview->Bounds();
//*********** Server Setting ******************/
	tab = new BTab();
	tabview->AddTab(fServerSetting = new HServerSetting(frame,""),tab);
	tab->SetLabel("Server");
//*********** Name Setting ****************//
	tab = new BTab();
	tabview->AddTab(fNameSetting = new HNameSetting(frame,""),tab);
	tab->SetLabel("Name/Description");	
//*********** Tracker Setting ******************/
	tab = new BTab();
	tabview->AddTab(fTrackerSetting = new HTrackerSetting(frame,""),tab);
	tab->SetLabel("Tracker");
	this->AddChild(tabview);
	
//*********** Apply Button ***********//	
	BRect bgrect = Bounds();
	bgrect.top = bgrect.bottom - 30;
	BView *bgview = new BView(bgrect,"bgview",B_FOLLOW_ALL,B_WILL_DRAW);
	bgview->SetViewColor(216,216,216,0);

	bgrect.OffsetTo(B_ORIGIN);
	bgrect.top += 5;
	bgrect.right -= 10;
	bgrect.left = bgrect.right - 80;
	bgrect.bottom -= 5;

	BButton *button = new BButton(bgrect,"apply","Apply",new BMessage(M_OK_MSG));
	bgview->AddChild(button);
	this->AddChild(bgview);
}

/**************************************************************
 * MessageReceived.
 **************************************************************/
void
HPrefWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	/************ save settings and close window ************/
	case M_OK_MSG:
	{
		((HApp*)be_app)->Prefs()->SetData("address",fServerSetting->Address());
		((HApp*)be_app)->Prefs()->SetData("port",(int32)fServerSetting->Port());
		((HApp*)be_app)->Prefs()->SetData("max_users",(int32)fServerSetting->MaxUser());
		((HApp*)be_app)->Prefs()->SetData("sim_download",(int32)fServerSetting->SimDownloads());
		((HApp*)be_app)->Prefs()->SetData("sim_upload",(int32)fServerSetting->SimUploads());
		((HApp*)be_app)->Prefs()->SetData("server_name",fNameSetting->Name());
		((HApp*)be_app)->Prefs()->SetData("server_desc",fNameSetting->Desc());
		((HApp*)be_app)->Prefs()->SetData("encoding",fNameSetting->Encoding());
		((HApp*)be_app)->Prefs()->SetData("save_log",fServerSetting->SaveLog());
		((HApp*)be_app)->Prefs()->SetData("sound",fServerSetting->Sound());
		fTrackerSetting->SaveTrackers();
		((HApp*)be_app)->Prefs()->StorePrefs();
		this->PostMessage(B_QUIT_REQUESTED);
		break;
	}
	/************ send message to tracker setting view. **********/
	case M_ADD_MSG:
	case M_DEL_MSG:
	case M_SEL_CHANGE_MSG:
	{
		this->PostMessage(message,fTrackerSetting);
		break;
	}
	default:
		BWindow::MessageReceived(message);
	}
}
