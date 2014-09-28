#include "HBroadcastWindow.h"
#include <ScrollView.h>
#include <Button.h>

/**************************************************************
 * Contructor.
 **************************************************************/
HBroadcastWindow::HBroadcastWindow(BRect rect
								,const char* name
								,BLooper* target)
	:BWindow(rect,name,B_TITLED_WINDOW_LOOK,B_MODAL_APP_WINDOW_FEEL,B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
	,fTarget(target)
{
	this->InitGUI();	
	this->AddShortcut(B_RETURN,0,new BMessage(OK_MSG));
}

/**************************************************************
 * Destroctor.
 **************************************************************/
HBroadcastWindow::~HBroadcastWindow()
{
}

/**************************************************************
 * Setup GUIs.
 **************************************************************/
void
HBroadcastWindow::InitGUI()
{
	BRect rect = Bounds();
	BView *view = new BView(rect,"bgview",B_FOLLOW_ALL,B_WILL_DRAW);
	view->SetViewColor(216,216,216,0);
	rect.top += 5;
	rect.left += 5;
	rect.right -= 5;
	rect.bottom -= 40;

	fTextView = new CTextView(rect,"textview",B_FOLLOW_ALL,B_WILL_DRAW|B_NAVIGABLE);
	BScrollView *scrollview = new BScrollView("scrollview",fTextView,B_FOLLOW_ALL,B_WILL_DRAW);
	
	view->AddChild(scrollview);

	rect.top = rect.bottom + 5;
	rect.left = rect.right - 70;
	BButton *button = new BButton(rect,"ok","Broadcast",new BMessage(OK_MSG));
	view->AddChild(button);
	this->AddChild(view);
}

/**************************************************************
 * MessageReceived.
 **************************************************************/
void
HBroadcastWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case OK_MSG:
	{
		BMessage msg(M_BROADCAST);
		msg.AddString("text",fTextView->Text());
		fTarget->PostMessage(&msg);
		this->PostMessage(B_QUIT_REQUESTED);
		break;
	}
	default:
		BWindow::MessageReceived(message);
	}
}