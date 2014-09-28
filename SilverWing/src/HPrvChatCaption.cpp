#include <stdio.h>
#include <Window.h>
#include <Autolock.h>
#include <String.h>

#include "HPrvChatCaption.h"
#include "HotlineClient.h"
#include "RectUtils.h"
#include "HDialog.h"
#include "HApp.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HPrvChatCaption::HPrvChatCaption(BRect rect
								,const char* name
								,BListView *target)
			:BView(rect
					,name
					,B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM
					,B_WILL_DRAW|B_PULSE_NEEDED)
			,fTopic(_("None"))
{
	this->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fTarget = target;	
	BRect rect1 = rect;
	rect1.OffsetTo(B_ORIGIN);
	//
	fTime = 0;
	rect1.top += 1;
	//rect1.bottom -=1;
	rect1.right = rect.right -3;
	rect1.left = rect.left + 1;
	BString title = _("Topic:");
	title << " ";
	title << fTopic << "       ";
	title << _("Last chat") << ": 00:00:00";
	title << "   ";
	title << "0 " << _("users");
	view = new BStringView(rect1,"",title.String(),B_FOLLOW_ALL);
	view->SetAlignment(B_ALIGN_RIGHT);
	this->AddChild(view);

	fOld = -1;
	
	BFont font;
	view->GetFont(&font);
	font.SetSize(10);
	view->SetFont(&font);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HPrvChatCaption::~HPrvChatCaption()
{
}

/***********************************************************
 * Pulse
 *		Set new number if new number is not same as old one.
 ***********************************************************/
void
HPrvChatCaption::Pulse()
{
	if(fTarget)
	{
		int32 num = fTarget->CountItems();
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
HPrvChatCaption::SetCaption(int32 num,time_t time)
{
	BAutolock lock(Window());

	BString str = _("Topic:");
	str << " ";
	str << fTopic << "       ";  
	str << _("Last chat:") << " ";
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
HPrvChatCaption::SetTime(time_t time)
{
	fTime = time;
	this->SetCaption(fOld,fTime);
}

/***********************************************************
 * Set topic
 ***********************************************************/
void
HPrvChatCaption::SetTopic(const char* topic)
{
	if(fTopic.Compare(topic) != 0)
	{
		fTopic = topic;
		SetCaption(fOld,fTime);
	}
}