#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <MenuField.h>
#include <Button.h>
#include <fs_attr.h>
#include <Roster.h>
#include <Path.h>
#include <Application.h>
#include <Directory.h>
#include <MenuItem.h>
#include <NodeInfo.h>
#include <ClassInfo.h>
#include <String.h>

#include "NumberControl.h"
#include "HConnectWindow.h"
#include <santa/Colors.h>
#include "HApp.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HConnectWindow::HConnectWindow(BRect rect,const char *name,bool edit)
	:BWindow(rect,name,B_TITLED_WINDOW_LOOK,B_MODAL_APP_WINDOW_FEEL,B_ASYNCHRONOUS_CONTROLS|B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
{
	this->InitGUI(edit);
	BTextControl *control = cast_as(FindView("address"),BTextControl);
	control->MakeFocus(true);
	this->AddShortcut('W',0,new BMessage(B_QUIT_REQUESTED));
	if(!edit)
	this->AddShortcut(B_RETURN,0,new BMessage(CONNECT_CONNECT_REQUESTED));
	else
	this->AddShortcut(B_RETURN,0,new BMessage(CONNECT_ADD_REQUESTED));
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HConnectWindow::~HConnectWindow()
{
}

/***********************************************************
 * InitGUI
 ***********************************************************/
void
HConnectWindow::InitGUI(bool edit)
{
	BRect rect = Bounds();
	rect.OffsetTo(B_ORIGIN);
	BView *bg = new BView(rect,"bg",B_FOLLOW_ALL,B_WILL_DRAW);
	bg->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	BTextControl *control;
	rect.top += 20;
	rect.left += 20;
	rect.right -= 32;
	rect.bottom = rect.top + 30;
	if(edit)
	{
		control = new BTextControl(rect,"name",_("Name:"),"",NULL,B_FOLLOW_ALL);
		control->SetDivider(50);
		bg->AddChild(control);
		rect.OffsetBy(0,30);
		bg->ResizeBy(0,30);
		this->ResizeBy(0,30);
	}
	const int divider = (int)bg->StringWidth(_("Password:")) +2;
	control = new BTextControl(rect,"address",_("Address:"),"",NULL,B_FOLLOW_ALL);
	control->SetDivider(divider);
	bg->AddChild(control);
	BRect pop = rect;
	pop.left = pop.right+2;
	pop.right += 10;
	BMenu *menu = InitBookmarks();
	BMenuField *bookmarks = new BMenuField(pop,"Bookmarks","",menu);
	bg->AddChild(bookmarks);
	rect.OffsetBy(0,30);
	NumberControl* numControl = new NumberControl(rect,"port",_("Port:"),5500,NULL,B_FOLLOW_ALL);
	numControl->SetDivider(divider);
	bg->AddChild(numControl);
	rect.OffsetBy(0,30);
	control = new BTextControl(rect,"login",_("Login:"),"",NULL,B_FOLLOW_ALL);
	control->SetDivider(divider);
	bg->AddChild(control);
	rect.OffsetBy(0,30);
	control = new BTextControl(rect,"password",_("Password:"),"",NULL);
	control->SetDivider(divider);
	control->TextView()->HideTyping(true);
	bg->AddChild(control);
	rect.OffsetBy(0,30);
	rect.left = rect.right -80;
	rect.bottom = rect.top + 23;
	BButton *btn;
	if(edit == false)
		btn = new BButton(rect,"connect",_("Connect"),new BMessage(CONNECT_CONNECT_REQUESTED));
	else
		btn = new BButton(rect,"add",_("Save"),new BMessage(CONNECT_ADD_REQUESTED));
	bg->AddChild(btn);
	rect.OffsetBy(-90,0);
	btn = new BButton(rect,"cancel",_("Cancel"),new BMessage(B_QUIT_REQUESTED));
	bg->AddChild(btn);
	AddChild(bg);
}

/***********************************************************
 * InitBookmarks
 ***********************************************************/
BMenu*
HConnectWindow::InitBookmarks()
{
	BMenu *menu = new BMenu("");
	BMenuItem *item;
	BMessage *msg;
	// Bookmarksディレクトリを探す
 	status_t 	err = B_NO_ERROR;
	app_info info;
    BPath pt;
    be_app->GetAppInfo(&info);
    BEntry entry(&info.ref);
    entry.GetPath(&pt);
    pt.GetParent(&pt);
	pt.Append("Bookmarks");
	BDirectory dir( pt.Path() );

   	// Serverファイルを読み込む
	while( err == B_NO_ERROR ){
		err = dir.GetNextEntry( (BEntry*)&entry, true );
		if( entry.InitCheck() != B_NO_ERROR ){
			break;
		}
		if( entry.GetPath(&pt) != B_NO_ERROR ){
			::create_directory(pt.Path(),0777);
		}
		if( entry.IsFile() )
		{
			BFile *file = new BFile(&entry,B_READ_WRITE);
			char name[B_FILE_NAME_LENGTH+1];
			entry.GetName(name);
			msg = new BMessage(CONNECT_BOOKMARK_MSG);
			msg->AddString("name",name);
			item = new BMenuItem(name,msg);
			menu->AddItem(item);
			delete file;
		}
	}

	return menu;
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HConnectWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case CONNECT_ADD_REQUESTED:
	{
		const char* address = ((BTextControl*)FindView("address"))->Text();
		const char* login = ((BTextControl*)FindView("login"))->Text();
		const char* name = ((BTextControl*)FindView("name"))->Text();
		const char* password = ((BTextControl*)FindView("password"))->Text();
		const char* port = ((BTextControl*)FindView("port"))->Text();
		int16 iport = atoi(port);
		this->SaveServer(name,address,login,password,iport);
		PostMessage(B_QUIT_REQUESTED);
		break;
	}
	case CONNECT_CONNECT_REQUESTED:
	{
		const char* address = ((BTextControl*)FindView("address"))->Text();
		const char* login = ((BTextControl*)FindView("login"))->Text();
		const char* password = ((BTextControl*)FindView("password"))->Text();
		const char* port = ((BTextControl*)FindView("port"))->Text();
		int16 iport = atol(port);
		message->AddString("address",address);
		message->AddString("login",login);
		message->AddString("password",password);
		message->AddInt16("port",(int16)iport);
		be_app->PostMessage(message);
		PostMessage(B_QUIT_REQUESTED);
		break;
	}
	case CONNECT_BOOKMARK_MSG:
	{
		const char* name = message->FindString("name");
		this->LoadServer(name);
		break;
	}
	default:
		BWindow::MessageReceived(message);
	}
}

/***********************************************************
 * Save new server.
 ***********************************************************/
void
HConnectWindow::SaveServer(const char* name
							,const char* address
							,const char* login
							,const char* password
							,int16 port)
{
	// Bookmarksディレクトリを探す
	app_info info;
    BPath pt;
    be_app->GetAppInfo(&info);
    BEntry entry(&info.ref);
    entry.GetPath(&pt);
    pt.GetParent(&pt);
	pt.Append("Bookmarks");
	pt.Append(name);
	BDirectory dir( pt.Path() );
	BFile *file = new BFile(pt.Path(),B_READ_WRITE|B_CREATE_FILE);
	dir.CreateFile(pt.Path(),file);
	if(file->InitCheck() == B_OK)
	{
		BMessage msg;
		msg.AddString("address",address);
		msg.AddString("login",login);
		msg.AddString("password",password);
		msg.AddInt16("port",port);
		msg.Flatten(file);
		BNodeInfo ninfo(file);
		ninfo.SetType("text/hotline-bookmarks");
		ninfo.SetPreferredApp("application/x-vnd.takamatsu-silverwing");
	}else{
		pt.GetParent(&pt);

		create_directory(pt.Path(),0777);
		this->SaveServer(name,address,login,password,port);
	}
	delete file;
}

/***********************************************************
 * Load servers.
 ***********************************************************/
void
HConnectWindow::LoadServer(const char* name)
{
	// Serversディレクトリを探す
	app_info ainfo;
    BPath pt;
    be_app->GetAppInfo(&ainfo);
    BEntry entry(&ainfo.ref);
    entry.GetPath(&pt);
    pt.GetParent(&pt);
	pt.Append("Bookmarks");
	pt.Append(name);

	BFile *file = new BFile(pt.Path(),B_READ_ONLY);
	if(file->InitCheck() == B_OK)
	{
		BString host = "",login = "",pass = "";
		BString port= "";
		int16 iport;
		BMessage msg;
		msg.Unflatten(file);
		host = msg.FindString("address");
		login = msg.FindString("login");
		pass = msg.FindString("password");
		iport = msg.FindInt16("port");
		int32 d = iport;
		port << d;
		((BTextControl*)FindView("address"))->SetText(host.String());
		((BTextControl*)FindView("login"))->SetText(login.String());
		((BTextControl*)FindView("password"))->SetText(pass.String());
		((BTextControl*)FindView("port"))->SetText(port.String());
	}
	delete file;
}

/***********************************************************
 * QuitRequested.
 ***********************************************************/
bool
HConnectWindow::QuitRequested()
{
	return true;
}

/***********************************************************
 * Set login.
 ***********************************************************/
void
HConnectWindow::SetLogin(const char* login)
{
	BTextControl *control;
	control = (BTextControl*)FindView("login");
	control->SetText(login);
}

/***********************************************************
 * Set password.
 ***********************************************************/
void
HConnectWindow::SetPassword(const char* password)
{
	BTextControl *control;
	control = cast_as(FindView("password"),BTextControl);
	control->SetText(password);
}

/***********************************************************
 * Set address.
 ***********************************************************/
void
HConnectWindow::SetAddress(const char* address)
{
	BTextControl *control;
	control = cast_as(FindView("address"),BTextControl);
	control->SetText(address);
}

/***********************************************************
 * Set port.
 ***********************************************************/
void
HConnectWindow::SetPort(const char* port)
{
	BTextControl *control;
	control = cast_as(FindView("port"),BTextControl);
	control->SetText(port);
}

/***********************************************************
 * Set name
 ***********************************************************/
void
HConnectWindow::SetName(const char* name)
{
	BTextControl *control;
	control = cast_as(FindView("name"),BTextControl);
	control->SetText(name);
}
