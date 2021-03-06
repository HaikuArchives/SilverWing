// HOtherColor.cpp
// Generated by Interface Elements (Window v2.0) on Apr 5 1998
// This is a user written class and will not be overwritten.

#include "HOtherColor.h"
#include "HWindow.h"
#include "HApp.h"
#include <ColorControl.h>
#include <Button.h>
#include <Application.h>
#define M_SLIDER_MSG	'MSLD'
#define M_TEXT_MSG 	 	'MTEM'


/***********************************************************
 * Constructor.
 ***********************************************************/
HOtherColor::HOtherColor(BRect frame)
	:BWindow(frame,_("Color Selector"),B_FLOATING_WINDOW_LOOK,B_MODAL_APP_WINDOW_FEEL,B_ASYNCHRONOUS_CONTROLS|B_NOT_CLOSABLE|B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
{
	Lock();
	BView *view = new BView(Bounds(),"",B_FOLLOW_ALL,B_WILL_DRAW);

	view->SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(view);
	BPoint point;
	point.Set(10,10);
	palette = new BColorControl(point, B_CELLS_32x8, 3,"colorMap",new BMessage(IE_HOtherColor_NEXTBTN));
    view->AddChild(palette);
    BRect rect;
    rect = Bounds();
    rect.bottom -= 15;
    rect.top = rect.bottom -35;
    rect.left += 30;
    rect.right = rect.left + 50;
    sample = new BView(rect,"sample",B_FOLLOW_ALL,B_WILL_DRAW);
  	sample->SetViewColor(palette->ValueAsColor());
    view->AddChild(sample);
    rect = Bounds();
    rect.bottom -= 25;
    rect.top = rect.bottom - 10;
    rect.right -= 10;
    rect.left = rect.right - 80;
    BButton *theBtn = new BButton(rect,"",APPLY_MSG,new BMessage(IE_HOtherColor_FINDBTN));
	theBtn->MakeDefault(true);
	view->AddChild(theBtn);
	rect.right -= 90;
    rect.left = rect.right - 80;
	theBtn = new BButton(rect,"",CLOSE_MSG,new BMessage(B_QUIT_REQUESTED));
	//heBtn->MakeDefault(true);
	

	view->AddChild(theBtn);
	Unlock();
	fTarget = NULL;
	fHandler = NULL;
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HOtherColor::~HOtherColor(void)
{
}

/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
HOtherColor::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case IE_HOtherColor_FINDBTN:
	{
		BMessage msg(M_OTHER_COLOR_SELECT);
		rgb_color color = palette->ValueAsColor();
		uint32 indexColor = color.red << 16|color.green << 8|color.blue;
		msg.AddInt32("color",indexColor);
		fTarget->PostMessage(&msg,fHandler);
		this->PostMessage(B_QUIT_REQUESTED);
		break;
	}
	case IE_HOtherColor_NEXTBTN:
	{
		sample->SetViewColor(palette->ValueAsColor());
		sample->SetHighColor(palette->ValueAsColor());
		sample->FillRect(sample->Bounds());
		sample->Sync();
		break;
	}
	case M_TEXT_MSG:
		
		break;
	default:
		BWindow::MessageReceived(message);
		break;
	}

}

/***********************************************************
 * Set color.
 ***********************************************************/
void
HOtherColor::SetColor(rgb_color color)
{
	palette->SetValue(color);
	sample->SetViewColor(color);
	sample->SetHighColor(color);
}

