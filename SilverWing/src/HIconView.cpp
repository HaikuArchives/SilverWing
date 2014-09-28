#include <String.h>
#include <Resources.h>
#include <ClassInfo.h>
#include <Beep.h>

#include "HIconView.h"
#include "AppUtils.h"
#include "HApp.h"
#include "Colors.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HIconView::HIconView(BRect rect,uint32 icon)
	:BView(rect,"iconview",B_FOLLOW_ALL,B_WILL_DRAW)
	,fIcon(icon)
	,fBitmap(NULL)
{	
	fBad = ((HApp*)be_app)->IsBadmoon();
	LoadUserIcon();	
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HIconView::~HIconView()
{
	delete fBitmap;
}

/***********************************************************
 * Draw icon.
 ***********************************************************/
void
HIconView::Draw(BRect updateRect)
{
	const rgb_color old = this->HighColor();
	this->SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->FillRect(Bounds());
	this->SetHighColor(old);
	if(fBitmap != NULL)
	{
		drawing_mode mode = this->DrawingMode();
		if(!fBad)
			this->SetDrawingMode(B_OP_ALPHA);

		this->DrawBitmap(fBitmap,BPoint(Bounds().left,Bounds().top));
		this->SetDrawingMode(mode);
	}
}

/***********************************************************
 * Load user icons.
 ***********************************************************/
void
HIconView::LoadUserIcon()
{
	delete fBitmap;
	BBitmap *bitmap = ((HApp*)be_app)->GetIcon(fIcon);
	if(bitmap)
		fBitmap = new BBitmap(bitmap);
	delete bitmap;
}

/***********************************************************
 * Set icon
 ***********************************************************/
void
HIconView::SetIcon(uint32 icon)
{
	fIcon = icon;
	LoadUserIcon();
	this->Draw(Bounds());
}