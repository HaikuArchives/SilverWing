/*******************************************************
*   Toolbar©
*
*   Toolbar is a usefull UI component.
*
*   @author  Atsushi Takamatsu (tak_atsu@tau.bekkoame.ne.jp)
*   @version 1.1.1
*   @date    Dec. 10 1999
*******************************************************/

#include "HToolbar.h"
#include "HToolbarButton.h"
#include "HToolbarSpace.h"
#include "space.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HToolbar::HToolbar(BRect rect,uint32 resize)
		:BBox(rect,NULL,resize)
		,fUseLabel(false)
{
	fButtons = 0;
	fSpaces = 0;
	
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HToolbar::~HToolbar()
{

}

/***********************************************************
 * Add button to toolbar
 ***********************************************************/
void
HToolbar::AddButton(const char* name,const void* icon,color_space space,BMessage *msg,const char *tips)
{
	// build bitmap 
	int32 length;
	BBitmap bitmap(BRect(0,0,15,15),space);
	BRect bounds(0,0,15,15);

	BView offview(BRect(0,0,15,15),"",0,0);
	if(space == B_CMAP8)
		length = ((int32(bounds.right-bounds.left+1)+3)&0xFFFFFFFC)*int32(bounds.bottom-bounds.top+1);
	else //if(space == B_RGB32)
		length = int32(bounds.right-bounds.left+1)*int32(bounds.bottom-bounds.top+1)*3;
		
	bitmap.SetBits(icon, length, 0, space);
	
	BRect rect = ButtonRect();
	rect.OffsetBy(fButtons * rect.Width() + fSpaces* 8,0);

	HToolbarButton *button = new HToolbarButton(rect,name,&bitmap,msg,tips);
	this->AddChild(button);
	button->InitPictures();
	
	fButtons++;
}

/***********************************************************
 * Add button to toolbar.
 ***********************************************************/
void
HToolbar::AddButton(const char* name,BBitmap* bitmap,BMessage *msg,const char *tips)
{	
	BRect rect = ButtonRect();
	rect.OffsetBy(fButtons * rect.Width() + fSpaces* 8,0);
	
	HToolbarButton *button = new HToolbarButton(rect,name,bitmap,msg,tips);
	this->AddChild(button);
	button->InitPictures();
	
	fButtons++;
	delete bitmap;
}


/***********************************************************
 * Add separator item.
 ***********************************************************/
void
HToolbar::AddSpace()
{
	BRect buttonRect = ButtonRect();
	BRect rect(6,2,6+3,6+buttonRect.Height());
	rect.OffsetBy(fButtons * buttonRect.Width() + fSpaces* 8,0);	
	HToolbarSpace *space = new HToolbarSpace(rect);

	this->AddChild(space);
	fSpaces++;
}

/***********************************************************
 * ButtonRect
 ***********************************************************/
BRect
HToolbar::ButtonRect()
{
	BRect rect;
	const int32 kWidth = (fUseLabel)?WITH_LABEL_WIDTH:NORMAL_WIDTH;
	
	rect.Set(4,4,4+kWidth,4+kWidth);
	return rect;
}