#include "HToolTipsWindow.h"
#include "HToolTipsView.h"

#define kDelayTime 2000
#define kHoldTime 7000

/***********************************************************
 * Constructor.
 ***********************************************************/
HToolTipsWindow::HToolTipsWindow(BView *owner,BPoint where,const char* tips)
:BWindow(BRect(where.x-20,where.y+10,where.x+50,where.y+30),"",B_BORDERED_WINDOW_LOOK,B_FLOATING_SUBSET_WINDOW_FEEL,B_AVOID_FOCUS|B_AVOID_FRONT)
,fOwner(owner)
{
	this->SetPulseRate(1000);
	fCurrentDelay = 0;
	BRect bounds=Bounds();
	bounds.InsetBy(3,3);
	HToolTipsView *bg=new HToolTipsView(owner->Window(),Bounds(),"",bounds,B_FOLLOW_ALL,B_WILL_DRAW|B_PULSE_NEEDED);
	AddChild(bg);
	BFont font;
	bg->GetFont(&font);
	font.SetSize(10);
	font_height height;
	font.GetHeight(&height);
	ResizeBy(font.StringWidth(tips)-60,height.ascent+height.descent-15);
	bg->SetText(tips);
	// ---
	Show();
	Hide();	
	//Minimize(true);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HToolTipsWindow::~HToolTipsWindow()
{
		
}

/***********************************************************
 * DispatchMessage.
 ***********************************************************/
void
HToolTipsWindow::DispatchMessage(BMessage *message,BHandler *target)
{
	if(message->what == B_PULSE) // get B_PULSE message
	{
		fCurrentDelay += 1000;
		if(fCurrentDelay == kDelayTime)
		{
			AddToSubset(fOwner->Window());
		}
		if(fCurrentDelay >= kHoldTime)
			this->Close();
	}
	BWindow::DispatchMessage(message,target);
}