#include "ColorView.h"
#include <stdio.h>
#include <Window.h>
#include <Message.h>
#include "HOtherColor.h"
#include "HWindow.h"
#include "RectUtils.h"
#include <santa/Colors.h>

/***********************************************************
 * Constructor.
 ***********************************************************/
ColorView::ColorView(const char *name,BRect rect,rgb_color color,BHandler *target)
:BView(rect,name,B_FOLLOW_LEFT|B_FOLLOW_BOTTOM,B_WILL_DRAW)
{
	fTarget = target;
	this->SetViewColor(color);
	this->SetFlags(B_WILL_DRAW);
	Invalidate();
	SetColor(color);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
ColorView::~ColorView()
{

}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
ColorView::MouseDown(BPoint)
{
	HOtherColor *theColor = new HOtherColor( RectUtils().CenterRect(300,120));
	theColor->SetTargetWindow(this->Window(),this);
	theColor->SetColor(color);
	theColor->Show();

}

/***********************************************************
 * Draw
 ***********************************************************/
void
ColorView::Draw(BRect rect)
{
	BView::Draw(rect);
	SetHighColor(color);
	FillRect(Bounds());
	SetHighColor(Black);
	StrokeRect(Bounds());
	SetHighColor(color);
	Sync();
}

/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
ColorView::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		case M_OTHER_COLOR_SELECT:
		{
			int8 index;
			int32 indexColor;
			rgb_color color;
			indexColor = msg->FindInt32("color");
			index = indexColor;
			color.blue = index;
			index = indexColor >> 8;
			color.green = index;
			index = indexColor >> 16;
			color.red = index;
			this->SetHighColor(color);
			this->Draw(Bounds());
			this->Invalidate();
			this->SetColor(color);
			break;
		}

		default:
			BView::MessageReceived(msg);
	}
}
