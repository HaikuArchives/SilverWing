#include <stdio.h>
#include <Window.h>
#include <Autolock.h>
#include <String.h>

#include "HCaption.h"
#include "HApp.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HCaption::HCaption(BRect rect,const char* name,const char* cpu,BListView *target)
			:BView(rect,name,B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM,B_WILL_DRAW|B_PULSE_NEEDED)
{
	this->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fTarget = target;	
	BRect rect1 = rect;
	rect1.OffsetTo(B_ORIGIN);
	fCpu = cpu;
	//
	rect1.top += 1;
	//rect1.bottom -=1;
	rect1.right = rect.right -3;
	rect1.left = rect.left + 1;
	BString title = fCpu;
	title << "  ";
	title << "0 " << _("users");
	view = new BStringView(rect1,"",title.String(),B_FOLLOW_ALL);
	view->SetAlignment(B_ALIGN_RIGHT);
	this->AddChild(view);
	//this->Draw(this->Bounds());
	fOld = 0;
	
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
	delete view;	
	RemoveSelf();
}

/***********************************************************
 * Pulse
 ***********************************************************/
void
HCaption::Pulse()
{
	if(fTarget!= NULL)
	{
		int32 num = fTarget->CountItems();
		if(num != fOld){
			this->SetNumber(num);
			fOld = num;
		}
	}
}

/***********************************************************
 * Set number.
 ***********************************************************/
void 
HCaption::SetNumber(int32 num)
{
	BAutolock lock(Window());

	BString str  = fCpu;
	str << "  " << num << " " << _("users");
	if( lock.IsLocked())
	{
		view->SetText(str.String());
	}
}
