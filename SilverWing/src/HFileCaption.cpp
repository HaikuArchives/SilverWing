#include "HFileCaption.h"
#include <Autolock.h>
#include <Window.h>
#include <String.h>
#include <Region.h>
#include <stdio.h>

#include "ResourceUtils.h"
#include "HApp.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HFileCaption::HFileCaption(BRect rect,const char* name,BListView *target)
	:BView(rect,name,B_FOLLOW_LEFT|B_FOLLOW_BOTTOM,B_WILL_DRAW|B_PULSE_NEEDED)
	,fTarget(target)
	,fLastBarberPoleOffset(0)
	,fShowingBarberPole(false)
	,fBarberPoleBits(NULL)
{
	this->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	BRect rect1 = rect;
	rect1.OffsetTo(B_ORIGIN);
	
	//
	rect1.top += 1;
	rect1.bottom = rect1.top + 10;
	rect1.left = rect.left + 1;
	rect1.right = Bounds().right - BarberPoleOuterRect().Width() - 5;
	
	BString label = "0 ";
	label += _("items");
	view = new BStringView(rect1,"",label.String());
	if(!target)
		view->SetText("");
	view->SetAlignment(B_ALIGN_RIGHT);
	this->AddChild(view);
	fOld = 0;
	
	BFont font;
	view->GetFont(&font);
	font.SetSize(10);
	view->SetFont(&font);
	StartBarberPole();
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HFileCaption::~HFileCaption()
{
	delete fBarberPoleBits;
}

const rgb_color kLightGray = {150, 150, 150, 255};
const rgb_color kGray = {100, 100, 100, 255};
const rgb_color kBlack = {0,0,0,255};
const rgb_color kWhite = {255,255,255,255};

/***********************************************************
 * Draw
 ***********************************************************/
void
HFileCaption::Draw(BRect updateRect)
{
	BView::Draw(updateRect);
	// show barber pole
	if(fShowingBarberPole)
	{
		BRect barberPoleRect = BarberPoleOuterRect();
		
		
		BeginLineArray(4);
		AddLine(barberPoleRect.LeftTop(), barberPoleRect.RightTop(), kLightGray);
		AddLine(barberPoleRect.LeftTop(), barberPoleRect.LeftBottom(), kLightGray);
		AddLine(barberPoleRect.LeftBottom(), barberPoleRect.RightBottom(), kWhite);
		AddLine(barberPoleRect.RightBottom(), barberPoleRect.RightTop(), kWhite);
		EndLineArray();
		
		barberPoleRect.InsetBy(1, 1);
	
		if(!fBarberPoleBits)
			fBarberPoleBits= ResourceUtils().GetBitmapResource('BBMP',"BMP:BARBERPOLE");
		BRect destRect(fBarberPoleBits ? fBarberPoleBits->Bounds() : BRect(0, 0, 0, 0));
		destRect.OffsetTo(barberPoleRect.LeftTop() - BPoint(0, fLastBarberPoleOffset));
		fLastBarberPoleOffset -= 1;
		if (fLastBarberPoleOffset < 0)
			fLastBarberPoleOffset = 5;
		BRegion region;
		region.Set(BarberPoleInnerRect());
		ConstrainClippingRegion(&region);	

		if (fBarberPoleBits)
			DrawBitmap(fBarberPoleBits, destRect);
	}
}

/***********************************************************
 * Pulse
 ***********************************************************/
void
HFileCaption::Pulse()
{
	if(fTarget!= NULL)
	{
		int32 num = fTarget->CountItems();
		if(num != fOld){
			this->SetNumber(num);
			fOld = num;
		}
	}
	if (!fShowingBarberPole)
		return;
	Invalidate(BarberPoleOuterRect());
}

/***********************************************************
 * Set number.
 ***********************************************************/
void
HFileCaption::SetNumber(int32 num)
{
	BAutolock lock(Window());

	BString str;
	if(num == 1)
		str << num << " " << _("item");
	else
		str << num << " " << _("items");
	if( lock.IsLocked())
	{
		view->SetText(str.String());
	}
}

/***********************************************************
 * Start BarberPole Animation
 ***********************************************************/
void
HFileCaption::StartBarberPole()
{
	BAutolock lock(Window());
	fShowingBarberPole = true;
	Invalidate();
}

/***********************************************************
 * Stop BarberPole Animation
 ***********************************************************/
void
HFileCaption::StopBarberPole()
{
	BAutolock lock(Window());
	fShowingBarberPole = false;
	Invalidate();
}

/***********************************************************
 * Return barber pole inner rect
 ***********************************************************/
BRect 
HFileCaption::BarberPoleInnerRect() const
{
	BRect result = Bounds();
	result.InsetBy(3, 2);
	result.left = result.right - 7;
	result.bottom = result.top + 6;
	return result;
}

/***********************************************************
 * Return barber pole outer rect
 ***********************************************************/
BRect 
HFileCaption::BarberPoleOuterRect() const
{
	BRect result(BarberPoleInnerRect());
	result.InsetBy(-1, -1);
	return result;
}

/***********************************************************
 *
 ***********************************************************/
void
HFileCaption::SetLabel(const char* text)
{
	view->SetText(text);
}