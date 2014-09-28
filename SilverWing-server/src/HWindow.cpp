#include "HWindow.h"
#include "HApp.h"
#include "MenuUtils.h"
#include "RectUtils.h"
#include "HPrefWindow.h"
#include "HAccountWindow.h"
#include "HBroadcastWindow.h"
#include "HView.h"
#include "CTextView.h"
#include "HStatusView.h"
#include "TServer.h"
#include "HTrackerConnection.h"
#include "AppUtils.h"
#include "HPrefs.h"

#include <MenuBar.h>
#include <MenuItem.h>
#include <ScrollView.h>
#include <Beep.h>
#include <Autolock.h>

/**************************************************************
 * Contructor.
 **************************************************************/
HWindow::HWindow(BRect rect,const char* name)
	:BWindow(rect,name,B_DOCUMENT_WINDOW,B_NOT_V_RESIZABLE)
	,fServer(NULL)
	,fLogFile(NULL)
{
	this->InitMenu();
	this->InitGUI();
	this->InitInfomations();
}

/**************************************************************
 * Destructor.
 **************************************************************/
HWindow::~HWindow()
{
	delete fLogFile;
}

/**************************************************************
 * Set up all menu.
 **************************************************************/
void
HWindow::InitMenu()
{
	BRect frame;
	frame = Bounds();
	frame.bottom = frame.top + 15;

	BMenuBar *menuBar = new BMenuBar(frame,"MENUBAR");
	MenuUtils utils;
	BMenu *menu;
	/********* FILE MENU ***********/
	menu = new BMenu("File");
	utils.AddMenuItem(menu,"About...",B_ABOUT_REQUESTED,be_app,be_app);
	menuBar->AddItem(menu);
	menu->AddSeparatorItem();
	utils.AddMenuItem(menu,"Quit",B_QUIT_REQUESTED,this);
	/********* EDIT MENU ***********/
	menu = new BMenu("Edit");
	utils.AddMenuItem(menu,"Run",M_START_SERVER,this,this,'R',0);
	utils.AddMenuItem(menu,"Stop",M_STOP_SERVER,this,this);
	menu->AddSeparatorItem();
	utils.AddMenuItem(menu,"Broadcast message...",M_BROADCAST_MSG,this,this,'B',0);
	menu->AddSeparatorItem();
	utils.AddMenuItem(menu,"Account Manager...",M_ACCOUNT_MSG,this,this,'A',0);
	utils.AddMenuItem(menu,"Server preferences...",M_SETTING_MSG,this,this,'P',0);
	menuBar->AddItem(menu);
	this->AddChild(menuBar);
}

/**************************************************************
 * Initialize GUIs.
 **************************************************************/
void
HWindow::InitGUI()
{
	BRect rect = this->Bounds();
	/******** LOG VIEW  **********/
	rect.top += (this->KeyMenuBar()->Bounds()).Height()+1;
	rect.left += 150;
	rect.right -= B_V_SCROLL_BAR_WIDTH;
	fLogView = new CTextView(rect,"logview",B_FOLLOW_ALL,B_WILL_DRAW);
	fLogView->MakeEditable(false);
	BScrollView *scrollview = new BScrollView("scrollview",fLogView,B_FOLLOW_ALL,B_WILL_DRAW,false,true);
	this->AddChild(scrollview);
	/******** INFOMATION VIEW *******/
	rect.right = rect.left -1;
	rect.left = 0;
	HView *view = new HView(rect,"infoview");
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	rect = view->Bounds();
	rect.top += 10;
	rect.left += 10;
	rect.right -= 10;
	rect.bottom = rect.top + 15;
	
	fEditUsers = new HStatusView(rect,"users","Current Users:");
	view->AddChild(fEditUsers);
	rect.OffsetBy(0,20);
	fEditMaxUsers = new HStatusView(rect,"count","Total connections:");
	view->AddChild(fEditMaxUsers);
	rect.OffsetBy(0,20);
	fEditDownloads = new HStatusView(rect,"download","Total downloads:");
	view->AddChild(fEditDownloads);
	rect.OffsetBy(0,20);
	fEditUploads = new HStatusView(rect,"users","Total uploads:");
	view->AddChild(fEditUploads);
	this->AddChild(view);
}

/**************************************************************
 * Message Received.
 **************************************************************/
void
HWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	/********* Receive logs **********/
	case T_LOG_MESSAGE:
	{
		uint32 type;
		if(message->HasInt32("type"))
		{
			type = message->FindInt32("type");
			this->SetInfomations(type);
		}
		const char* log = message->FindString("log");
		this->InsertLog(log);
		break;
 	}
 	/********* Start serving　*********/
 	case M_START_SERVER:
	{
		const char* address;
		uint32 port;
		uint32 max_user;
		((HApp*)be_app)->Prefs()->GetData("address",&address);
		((HApp*)be_app)->Prefs()->GetData("port",(int32*)&port);
		((HApp*)be_app)->Prefs()->GetData("max_users",(int32*)&max_user);
		fServer = new TServer(address,port,max_user,this);
		fServer->Run();
		fServer->PostMessage(T_START_SERVER);
		this->PostMessage(M_TIMER_MSG);
		this->InitTracker();
		break;
	}
	/********** Regist to tracker ***********/
	case M_TIMER_MSG:
	{
		int32 count = fTrackerList.CountItems();
		for(int32 i = 0; i < count ;i++)
		{
			HTrackerConnection *con = (HTrackerConnection*)fTrackerList.ItemAt(i);
			con->SetUsers(fUsers);
		}
		this->WriteToTracker();
		break;
	}	
	/*********　Stop serving ********/
	case M_STOP_SERVER:
	{
		fServer->PostMessage(T_STOP_SERVER);
		fServer = NULL;
		this->ClearTracker();
		break;
	}
	/********* Edit settings **********/
	case M_SETTING_MSG:
	{
		HPrefWindow *window = new HPrefWindow( RectUtils().CenterRect(400,250),"Preferences");
		window->Show();
		break;
	}
	/********* Edit accounts ********/
	case M_ACCOUNT_MSG:
	{
		HAccountWindow* window = new HAccountWindow( RectUtils().CenterRect(400,400),"Account Manager");
		window->Show();
		break;
	}
	/********* Broadcast message *********/
	case M_BROADCAST_MSG:
	{
		HBroadcastWindow* window = new HBroadcastWindow( RectUtils().CenterRect(300,200),"Broadcast message",this);
		window->Show();
		break;
	}
	/******** Send broadcast message to socket **********/
	case M_BROADCAST:
	{
		const char* text = message->FindString("text");
		BMessage msg(T_BROADCAST_MSG);
		msg.AddString("text",text);
		if(fServer)
			fServer->PostMessage(&msg);
		break;
	}
	default:
		BWindow::MessageReceived(message);
	}
}

/**************************************************************
 * Set infomation to captions.
 **************************************************************/
void
HWindow::SetInfomations(uint32 type)
{
	switch(type)
	{
	case T_ERROR_TYPE:
	{
		beep();
		break;
	}
	case T_UPLOAD_TYPE:
	{
		fUploads++;
		fEditUploads->SetNumber(fUploads);
		break;
	}
	case T_DOWNLOAD_TYPE:
	{
		fDownloads++;
		fEditDownloads->SetNumber(fDownloads);
		break;
	}
	case T_DOWNLOAD_END_TYPE:
	{
		//fDownloads--;
		//fEditDownloads->SetNumber(fDownloads);
		break;
	}
	case T_UPLOAD_END_TYPE:
	{
		//fUploads--;
		//fEditUploads->SetNumber(fUploads);
		break;
	}
	case T_LOGIN_TYPE:
	{
		fUsers++;
		fTotalUsers++;
		fEditUsers->SetNumber(fUsers);
		fEditMaxUsers->SetNumber(fTotalUsers);
		PlaySound(T_LOGIN_TYPE);
		break;
	}
	case T_LOGOUT_TYPE:
	{
		fUsers--;
		fEditUsers->SetNumber(fUsers);
		PlaySound(T_LOGOUT_TYPE);
		break;
	}
	}
}

/****************************************************************
 * Send message to TrackerConnection.
 ****************************************************************/
void
HWindow::WriteToTracker()
{
	register int32 count = fTrackerList.CountItems();
	while(count>0)
	{
		HTrackerConnection *con = (HTrackerConnection*)fTrackerList.ItemAt(--count);
		con->PostMessage(M_START_WRITE);
	}
}

/*******************************************************************
 * Read trackers and add to fTrackerList.
 *******************************************************************/
void
HWindow::InitTracker()
{
	this->ClearTracker();
		if(fServer != NULL)
	{
		const char* address;
		uint32 port;

		((HApp*)be_app)->Prefs()->GetData("address",&address);
		((HApp*)be_app)->Prefs()->GetData("port",(int32*)&port);
		const char* name;
		((HApp*)be_app)->Prefs()->GetData("server_name",&name);
		const char* desc;
		((HApp*)be_app)->Prefs()->GetData("server_desc",&desc);
		
		BMessage msg;
		BPath path = AppUtils().GetAppDirPath(be_app);
		path.Append("Trackers");
		path.Append("tracker.dat");
		
		BFile file(path.Path(),B_READ_ONLY);
		if(file.InitCheck() != B_OK)
			return;
		msg.Unflatten(&file);
		int32 count;
		type_code type;
		msg.GetInfo("tracker",&type,&count);
		for(int32 i = 0;i < count;i++)
		{
			const char* tracker = msg.FindString("tracker",i);
			const char* login = NULL;
			if(msg.HasString("login",i))
				login = msg.FindString("login",i);
			const char* password =  NULL;
			if(msg.HasString("password",i))
				password = msg.FindString("password",i);
			HTrackerConnection *con = new HTrackerConnection(tracker,5499,address,port,fServer->Users(),name,desc);
			con->SetLoginInfo(atol(login),password);
			con->Run();
			fTrackerList.AddItem(con);
		}
	}
}

/**************************************************************
 * Contructor.
 **************************************************************/
void
HWindow::ClearTracker()
{
	register int32 count = fTrackerList.CountItems();
	while(count >0)
	{
		HTrackerConnection *con = static_cast<HTrackerConnection*>(fTrackerList.RemoveItem(--count));
		con->PostMessage(B_QUIT_REQUESTED);
	}
}

/**************************************************************
 * Contructor.
 **************************************************************/
void
HWindow::InitInfomations()
{
	fUploads = 0;
	fDownloads = 0;
	fUsers = 0;
	fTotalUsers = 0;
	fEditUsers->SetNumber(0);
	fEditMaxUsers->SetNumber(0);
	fEditDownloads->SetNumber(0);
	fEditUploads->SetNumber(0);
}

/**************************************************************
 * Contructor.
 **************************************************************/
void
HWindow::InsertLog(const char* text)
{
	BAutolock lock(this);
	if(lock.IsLocked())
	{
		fLogView->Insert( ::strlen( fLogView->Text() ),text,::strlen(text));
		float min,max;
		(fLogView->ScrollBar(B_VERTICAL))->GetRange(&min,&max);
		(fLogView->ScrollBar(B_VERTICAL))->SetValue(max);
	}
	this->SaveLog(text);
}

/*****************************************************************
 * MenusBeginging
 *****************************************************************/
void
HWindow::MenusBeginning()
{
	BMenuItem *item = KeyMenuBar()->FindItem("Run");
	
	if(fServer != NULL)
		item->SetEnabled(false);
	else
		item->SetEnabled(true);

	if(fServer != NULL)
	{
		(KeyMenuBar())->FindItem("Stop")->SetEnabled(true);
		(KeyMenuBar())->FindItem("Broadcast message...")->SetEnabled(true);
	}else{
		(KeyMenuBar())->FindItem("Stop")->SetEnabled(false);
		(KeyMenuBar())->FindItem("Broadcast message...")->SetEnabled(false);
	}
}


/*********************************************************************
 * Save log as a file.
 *********************************************************************/
void
HWindow::SaveLog(const char* text)
{
	bool enabled;
	((HApp*)be_app)->Prefs()->GetData("save_log",&enabled);
	if(enabled)
	{
		if(fLogFile == NULL)
		{
			BPath path = AppUtils().GetAppDirPath(be_app);
			path.Append("Log.txt");
			fLogFile = new BFile(path.Path(),B_WRITE_ONLY|B_CREATE_FILE|B_OPEN_AT_END);
			if(fLogFile->InitCheck() != B_OK)
				return;
		}
		fLogFile->Write(text,::strlen(text));
	}
}

/***********************************************************
 * PlaySound
 ***********************************************************/
void
HWindow::PlaySound(uint32 type)
{
	bool enabled;
	((HApp*)be_app)->Prefs()->GetData("sound",&enabled);
	if(!enabled)
		return;
		
	thread_id play;
	fCurrentEffect = type;
	
	play = ::spawn_thread(PlayThread,"LoginSound",B_LOW_PRIORITY,this);
	::resume_thread(play);
}

/***********************************************************
 * PlayThread
 ***********************************************************/
int32
HWindow::PlayThread(void *data)
{
	BPath path = AppUtils().GetAppDirPath();
	path.Append("Sounds");
	HWindow *win = (HWindow*)data;
	switch(win->fCurrentEffect)
	{
	case T_LOGIN_TYPE:
		path.Append("login.wav");
		break;
	case T_LOGOUT_TYPE:
		path.Append("logout.wav");
		break;
	}
	
	BFileGameSound sound(path.Path(),false);
	sound.StartPlaying();
	// wait for end of playing
	while(sound.IsPlaying());
	sound.StopPlaying();
	return 0;
}


/************************************************************************
 * QuitRequested.
 * If server is running ,avoid to quit.
 ************************************************************************/
bool	
HWindow::QuitRequested()
{
	if(fServer != NULL){
		if(fServer->IsServing())
		{
			(new BAlert("","Server is running.","OK"))->Go();
			return false;
		}
	}
	((HApp*)be_app)->Prefs()->SetData("window_rect",this->Frame());
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}