#include "HToolTipsView.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HToolTipsView::HToolTipsView(BWindow *owner,BRect rect,const char* name,BRect textrect,uint32 resize,uint32 flag)
			:BTextView(rect,name,textrect,resize,flag)
{
	// set view color light yellow.
	this->SetViewColor(255,255,150);
	
	this->MakeResizable(true);
	this->MakeEditable(false);
	this->MakeSelectable(false);
	
	BFont font;
	uint32 propa;
	this->GetFontAndColor(&font,&propa);
	font.SetSize(10);
	this->SetFontAndColor(&font,propa);
	fShown = false;
	fOwnerWindow = owner;
}

/***********************************************************
 * Pulse
 ***********************************************************/
void
HToolTipsView::Pulse()
{
	if(fShown == false)
	{
		if(Window()->IsHidden() && fOwnerWindow->IsActive())
		{
			Window()->Show();
			fShown = true;	
		}
	}
}