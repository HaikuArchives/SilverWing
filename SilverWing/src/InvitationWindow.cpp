#include "InvitationWindow.h"
#include "HotlineClient.h"
#include <View.h>
#include <Button.h>
#include <StringView.h>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include "HBitmapView.h"
#include "ResourceUtils.h"
#include "HApp.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
InvitationWindow::InvitationWindow(BRect frame,
							const char* name,
							const char* nick,
							uint32 pcref,
							BLooper *target)
			:BWindow(frame,name,B_FLOATING_WINDOW_LOOK,
							B_NORMAL_WINDOW_FEEL,B_NOT_CLOSABLE|B_NOT_ZOOMABLE)
			,fPcref(pcref)
			,fTarget(target)
{
	AddShortcut(B_RETURN,0,new BMessage(M_JOIN_CHAT));
	AddShortcut(B_ESCAPE,0,new BMessage(M_REFUSE_CHAT));
	
	// set up GUI
	BView *view = new BView(Bounds(),"bgview",B_FOLLOW_ALL,B_WILL_DRAW);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect bitmapRect = Bounds();
	bitmapRect.top += 5;
	bitmapRect.left += 5;
	bitmapRect.right = bitmapRect.left + 35;
	bitmapRect.bottom = bitmapRect.top + 35;
	
	BBitmap *bitmap = ResourceUtils().GetBitmapResource('BBMP',"BMP:INVITATION");
	
	HBitmapView *bitmapView = new HBitmapView(bitmapRect,
									"bitmap",
									B_FOLLOW_NONE,
									bitmap);
	view->AddChild(bitmapView);
	
	BRect rect = Bounds();
	rect.top += 5;
	rect.left += 40;
	rect.right -= 5;
	rect.bottom = rect.top + 20;	

	BString title = _("Private chat invitation from");
	title << " " << nick;

	BStringView *stringview;
	stringview = new BStringView(rect,"title",title.String(),B_FOLLOW_ALL);
	view->AddChild(stringview);
	rect.OffsetBy(0,20);
	
	time_t timet = time(NULL);
	struct tm* t = localtime(&timet);
	title = _("Time:");
	title += " ";
	char *tmp = new char[1024];
	::sprintf(tmp,"%.2d:%.2d:%.2d",t->tm_hour,t->tm_min,t->tm_sec);
	title << tmp;
	delete[] tmp;
	stringview = new BStringView(rect,"time",title.String(),B_FOLLOW_ALL);
	view->AddChild(stringview);
	rect.OffsetBy(0,25);
	rect.left = rect.right - 80;
	
	BButton *button = new BButton(rect,"ok",_("Join"),new BMessage(M_JOIN_CHAT));
	view->AddChild(button);
	rect.OffsetBy(-90,0);
	button = new BButton(rect,"cancel",_("Decline"),new BMessage(M_REFUSE_CHAT));
	view->AddChild(button);
	AddChild(view);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
InvitationWindow::~InvitationWindow()
{

}
/***********************************************************
 * InitGUI.
 ***********************************************************/
void
InvitationWindow::InitGUI()
{
	
}

/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
InvitationWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case M_JOIN_CHAT:
		{
			BMessage msg(H_JOIN_CHAT);
			msg.AddInt32("pcref",fPcref);
			fTarget->PostMessage(&msg);
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
	case M_REFUSE_CHAT:
		{
			BMessage msg(H_REFUSE_CHAT);
			msg.AddInt32("pcref",fPcref);
			fTarget->PostMessage(&msg);
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
	default:
		BWindow::MessageReceived(message);
	}
}