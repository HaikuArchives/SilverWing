#include "HStatusView.h"
#include <Window.h>
#include <Autolock.h>

/**************************************************************
 * Contructor.
 **************************************************************/
HStatusView::HStatusView(BRect rect,const char*name,const char* title)
:BStringView(rect,name,title,B_FOLLOW_ALL,B_WILL_DRAW|B_PULSE_NEEDED)
{
	fNumbers = 1;
	fTitle = title;
}

/**************************************************************
 * Destructor.
 **************************************************************/
HStatusView::~HStatusView()
{

}
/*
void
HStatusView::Pulse()
{
	
	BAutolock lock(Window());
	if(lock.IsLocked())
		this->SetText(str.String());
}
*/

/**************************************************************
 * Set number.
 **************************************************************/
void
HStatusView::SetNumber(uint32 num)
{
	if(num != fNumbers)
	{
		fNumbers = num;
		BString str;
		str = fTitle;
		str << "  ";
		str << fNumbers;
		BAutolock lock(Window());
		if(lock.IsLocked())
			this->SetText(str.String());
	}
}