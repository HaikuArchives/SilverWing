#include <stdio.h>
#include <Window.h>
#include <Autolock.h>
#include <String.h>

#include "HApp.h"
#include "HCaption.h"
#include <santa/Colors.h>

/***********************************************************
 * Constructor.
 ***********************************************************/
HCaption::HCaption(BRect rect,const char* name,BListView *target)
			:BView(rect,name,B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM,B_WILL_DRAW|B_PULSE_NEEDED)
			,fTarget(target)
			,fOld(-1)
			,fTime(0)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	BRect rect1 = rect;
	rect1.OffsetTo(B_ORIGIN);
	//
	rect1.top += 1;
	//rect1.bottom -=1;
	rect1.right = rect.right -3;
	rect1.left = rect.left + 1;
	BString title = _("Last chat");
	title += ": 00:00:00";
	title +=  "   ";
	title += "0 ";
	title += _("users");
	view = new BStringView(rect1,"",title.String(),B_FOLLOW_ALL);
	view->SetAlignment(B_ALIGN_RIGHT);
	this->AddChild(view);
	//this->Draw(this->Bounds());
	BFont font;
	view->GetFont(&font);
	font.SetSize(10);
	view->SetFont(&font);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HCaption::~HCaption()
{
}

/***********************************************************
 * Pulse
 *		Set new number if new number is not same as old one.
 ***********************************************************/
void
HCaption::Pulse()
{
	if(fTarget!= NULL)
	{
		int32 num = fTarget->CountItems();
		if(num != fOld ){
			this->SetCaption(num,fTime);
			fOld = num;
		}
	}else{// for message chat
		int32 num = 2;
		if(num != fOld ){
			this->SetCaption(num,fTime);
			fOld = num;
		}
	}
}

/***********************************************************
 * Set number.
 ***********************************************************/
void
HCaption::SetCaption(int32 num,time_t time)
{
	BAutolock lock(Window());

	BString str  = _("Last chat");
	str += ": ";
	if(time != 0)
	{
		struct tm* t = localtime(&time);

		char *tmp = new char[1024];
		::sprintf(tmp,"%.2d:%.2d:%.2d",t->tm_hour,t->tm_min,t->tm_sec);
		str << tmp;
		delete[] tmp;
	}else
		str << "00:00:00";

	if(num == 1)
		str << "   " << num << " " << _("user");
	else
		str << "   " << num << " " << _("users");
	if( lock.IsLocked())
	{
		view->SetText(str.String());
	}
}

/***********************************************************
 * Set time
 ***********************************************************/
void
HCaption::SetTime(time_t time)
{
	fTime = time;
	this->SetCaption(fOld,fTime);
}
