#include <ClassInfo.h>
#include <TextControl.h>
#include <Button.h>

#include <santa/Colors.h>

#include "HAddTrackerWindow.h"
#include "AppUtils.h"
#include "MAlert.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HAddTrackerWindow::HAddTrackerWindow(BRect rect,const char* name)
		:BWindow(rect,name,B_TITLED_WINDOW_LOOK,B_MODAL_APP_WINDOW_FEEL,B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_ASYNCHRONOUS_CONTROLS)
{
	InitGUI();
	BTextControl *control = cast_as(FindView("name"),BTextControl);
	control->MakeFocus(true);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HAddTrackerWindow::~HAddTrackerWindow()
{

}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HAddTrackerWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case M_OK_MSG:
	{
		BTextControl *control1 = cast_as(FindView("name"),BTextControl);
		BTextControl *control2 = cast_as(FindView("address"),BTextControl);
		const char* name = control1->Text();
		const char* address = control2->Text();
		this->SaveTracker(name,address);
		this->PostMessage(B_QUIT_REQUESTED);
		break;
	}
	case M_END_NAME:
	{
		BTextControl *control1 = cast_as(FindView("name"),BTextControl);
		BTextControl *control2 = cast_as(FindView("address"),BTextControl);

		control2->SetText(control1->Text());
		control2->TextView()->SelectAll();
		break;
	}
	default:
		BWindow::MessageReceived(message);
	}
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HAddTrackerWindow::InitGUI()
{
	BView *view = new BView(Bounds(),"bgview",0,0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect rect = Bounds();
	rect.left += 20;
	rect.right -= 20;
	rect.top += 10;
	rect.bottom = rect.top + 20;
	BTextControl *control = new BTextControl(rect,"name","Name:","",new BMessage(M_END_NAME));
	control->SetDivider(50);
	view->AddChild(control);
	rect.OffsetBy(0,30);
	control = new BTextControl(rect,"address","Address:","",NULL);
	control->SetDivider(50);
	view->AddChild(control);
	rect.OffsetBy(0,30);
	rect.left = rect.right - 70;
	BButton *btn = new BButton(rect,"ok","OK",new BMessage(M_OK_MSG));
	view->AddChild(btn);
	rect.OffsetBy(-80,0);
	btn = new BButton(rect,"cancel","Cancel",new BMessage(B_QUIT_REQUESTED));
	view->AddChild(btn);
	this->AddChild(view);
}

/***********************************************************
 * Save trackers.
 ***********************************************************/
void
HAddTrackerWindow::SaveTracker(const char* name,const char* address)
{
	AppUtils utils;
	BPath path = utils.GetAppDirPath(be_app);
	path.Append("Trackers");
	path.Append(name);

	BFile file(path.Path(),B_WRITE_ONLY|B_CREATE_FILE);
	if(file.InitCheck() == B_OK)
	{
		BMessage msg;
		msg.AddString("address",address);
		msg.Flatten(&file);

		BMessage msg2(M_NEW_TRACKER);
		msg2.AddString("address",address);
		msg2.AddString("name",name);
		msg2.AddInt16("port",5948);
		be_app->PostMessage(&msg2);
	}else{
		( new MAlert("Cannot save a tracker","file initialize failed","OK"))->Go();
	}
}
