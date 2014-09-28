#include <Message.h>

#include "HProgressWindow.h"
#include "Colors.h"
#include "HApp.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HProgressWindow::HProgressWindow(BRect rect,const char* title)
	:BWindow(rect,title,B_FLOATING_WINDOW_LOOK,B_MODAL_APP_WINDOW_FEEL,B_NOT_CLOSABLE|B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
{
	InitGUI();
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HProgressWindow::~HProgressWindow()
{
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HProgressWindow::InitGUI()
{
	BView *bgview = new BView(Bounds(),"bg",B_FOLLOW_ALL,B_WILL_DRAW);
	bgview->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect rect = Bounds();
	fStatusBar = new BStatusBar(rect,"status",_("Gathering all icons…"));
	bgview->AddChild(fStatusBar);
	this->AddChild(bgview);
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HProgressWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case M_UPDATE_MSG:
	{
		fStatusBar->Update(1);
		break;
	}
	case M_SET_MAX_VALUE:
	{
		uint32 max = message->FindInt32("value");
		fStatusBar->SetMaxValue(max);
		break;
	}
	case M_RESET_MSG:
	{
		const char* label = message->FindString("label");
		fStatusBar->Reset(label);
		break;
	}	
	default:
		BWindow::MessageReceived(message);
	}
}