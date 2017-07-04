#include "HConnectWindow.h"
#include <stdio.h>
#include <stdlib.h>
#include <ClassInfo.h>
#include <TextControl.h>
#include <Button.h>
#include <MenuField.h>
#include <Roster.h>
#include <Path.h>
#include <Application.h>
#include <Directory.h>
#include <File.h>
#include <fs_attr.h>
#include <MenuItem.h>
#include <String.h>
#include <CheckBox.h>
#include "HApp.h"

/***********************************************************
 * Constructor
 ***********************************************************/
HConnectWindow::HConnectWindow(BRect rect,const char *name,bool edit)
	:BWindow(rect,name,B_TITLED_WINDOW,B_NOT_RESIZABLE|B_ASYNCHRONOUS_CONTROLS)
{
	AddShortcut(B_RETURN,0,new BMessage(CONNECT_CONNECT_REQUESTED));
	this->InitGUI(edit);
	BTextControl *control = cast_as(FindView("address"),BTextControl);
	control->MakeFocus(true);
}

/***********************************************************
 * Destructor
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
	control = new BTextControl(rect,"address",_("Address:"),"",NULL,B_FOLLOW_ALL);
	control->SetDivider(50);
	bg->AddChild(control);
	BRect pop = rect;
	pop.left = pop.right+2;
	pop.right += 10;
	BMenu *menu = InitBookmarks();
	BMenuField *bookmarks = new BMenuField(pop,"Bookmarks","",menu);
	bg->AddChild(bookmarks);
	rect.OffsetBy(0,30);
	control = new BTextControl(rect,"port",_("Port:"),"5500",NULL,B_FOLLOW_ALL);
	control->SetDivider(50);
	bg->AddChild(control);
	rect.OffsetBy(0,30);
	control = new BTextControl(rect,"login",_("Login:"),"",NULL,B_FOLLOW_ALL);
	control->SetDivider(50);
	bg->AddChild(control);
	rect.OffsetBy(0,30);
	control = new BTextControl(rect,"password",_("Password:"),"",NULL,B_FOLLOW_ALL);
	control->SetDivider(50);
	bg->AddChild(control);
	/*rect.OffsetBy(0,30);
	BCheckBox *checkBox = new BCheckBox(rect,"new_window","Connect with new window",NULL);
	checkBox->SetValue(true);
	bg->AddChild(checkBox);
	*/
	rect.OffsetBy(0,30);
	rect.left = rect.right -80;
	rect.bottom = rect.top + 23;
	BButton *btn;
	if(edit == false)
		btn = new BButton(rect,"connect",_("Connect"),new BMessage(CONNECT_CONNECT_REQUESTED));
	else
		btn = new BButton(rect,"add",_("Add"),new BMessage(CONNECT_ADD_REQUESTED));
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
	// Get Bookmarks directory
 	status_t 	err = B_NO_ERROR;
	app_info info;
    BPath pt;
    be_app->GetAppInfo(&info);
    BEntry entry(&info.ref);
    entry.GetPath(&pt);
    pt.GetParent(&pt);
	pt.Append("Bookmarks");
	::create_directory(pt.Path(),0777);
	BDirectory dir( pt.Path() );

   	// Read all server file
	while( err == B_NO_ERROR ){
		err = dir.GetNextEntry( (BEntry*)&entry, true );
		if( entry.InitCheck() != B_NO_ERROR ){
			break;
		}
		char name[B_FILE_NAME_LENGTH+1];
		entry.GetName(name);
		msg = new BMessage(CONNECT_BOOKMARK_MSG);
		msg->AddString("name",name);
		item = new BMenuItem(name,msg);
		menu->AddItem(item);
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
		const char* password = ((BTextControl*)FindView("password"))->Text();
		const char* port = ((BTextControl*)FindView("port"))->Text();
		int16 iport = atoi(port);
		this->SaveServer(address,login,password,iport);
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
		/*BCheckBox *checkBox = cast_as(FindView("new_window"),BCheckBox);
		message->AddBool("new_window"checkBox->Value());
//		cout << iport << endl;
		*/
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
 * SaveServer
 ***********************************************************/
void
HConnectWindow::SaveServer(const char* address,const char* login ,const char* password,int16 port)
{
	app_info info;
    BPath pt;
    be_app->GetAppInfo(&info);
    BEntry entry(&info.ref);
    entry.GetPath(&pt);
    pt.GetParent(&pt);
	pt.Append("Bookmarks");
	pt.Append(address);
	BDirectory dir( pt.Path() );
	BFile *file = new BFile(pt.Path(),B_WRITE_ONLY|B_CREATE_FILE);
	dir.CreateFile(pt.Path(),file);
	if(file->InitCheck() == B_OK)
	{
		BMessage msg;
		msg.AddString("address",address);
		msg.AddString("login",login);
		msg.AddString("password",password);
		msg.AddInt16("port",port);
		msg.Flatten(file);
	}else{
		pt.GetParent(&pt);
		::create_directory(pt.Path(),0777);
		this->SaveServer(address,login,password,port);
	}
	delete file;
}

/***********************************************************
 * LoadServer
 ***********************************************************/
void
HConnectWindow::LoadServer(const char* name)
{
	app_info ainfo;
    BPath pt;
    be_app->GetAppInfo(&ainfo);
    BEntry entry(&ainfo.ref);
    entry.GetPath(&pt);
    pt.GetParent(&pt);
	pt.Append("Bookmarks");
	pt.Append(name);

	BFile file(pt.Path(),B_READ_ONLY);
	if(file.InitCheck() == B_OK)
	{
		BString port= "";
		int16 iport;
		BMessage msg;
		msg.Unflatten(&file);
		const char* host = msg.FindString("address");
		const char* login = msg.FindString("login");
		const char* pass = msg.FindString("password");
		iport = msg.FindInt16("port");
		int32 d = iport;
		port << d;
		((BTextControl*)FindView("address"))->SetText(host);
		((BTextControl*)FindView("login"))->SetText(login);
		((BTextControl*)FindView("password"))->SetText(pass);
		((BTextControl*)FindView("port"))->SetText(port.String());
	}
}

/***********************************************************
 * QuitRequested
 ***********************************************************/
bool
HConnectWindow::QuitRequested()
{
	return BWindow::QuitRequested();
}

/***********************************************************
 * SetLogin
 ***********************************************************/
void
HConnectWindow::SetLogin(const char* login)
{
	BTextControl *control;
	control = cast_as(FindView("login"),BTextControl);
	control->SetText(login);
}

/***********************************************************
 * SetPassword
 ***********************************************************/
void
HConnectWindow::SetPassword(const char* password)
{
	BTextControl *control;
	control = cast_as(FindView("password"),BTextControl);
	control->SetText(password);
}

/***********************************************************
 * SetAddress
 ***********************************************************/
void
HConnectWindow::SetAddress(const char* address)
{
	BTextControl *control;
	control = cast_as(FindView("address"),BTextControl);
	control->SetText(address);
}

/***********************************************************
 * SetPort
 ***********************************************************/
void
HConnectWindow::SetPort(const char* port)
{
	BTextControl *control;
	control = cast_as(FindView("port"),BTextControl);
	control->SetText(port);
}
