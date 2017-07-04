#include "HToolbarButton.h"
#include "HToolbar.h"

#include <ClassInfo.h>
#include <stdio.h>
#include <Beep.h>

const rgb_color bgcolor = 				{216,216,216,	255};
const rgb_color White =					{255,255,255,	255};
const rgb_color BeDarkShadow =			{108,108,108,	255};
const rgb_color Black =					{0,  0,  0,		255};
const rgb_color BeShadow =				{152,152,152,	255};
const rgb_color BeLightShadow =			{194,194,194,	255};

/***********************************************************
 * Constructor.
 ***********************************************************/
HToolbarButton::HToolbarButton(BRect rect,
							const char* name,
							BBitmap *bitmap,
							BMessage *msg,
							const char *tips)
	:BView(rect,name,B_FOLLOW_NONE,B_WILL_DRAW|B_PULSE_NEEDED)
	,fOutsidePicture(NULL)
	,fInsidePicture(NULL)
	,fDownPicture(NULL)
	,fDisablePicture(NULL)
	,fPicture(NULL)
	,fTipsWindow(NULL)
	,fUpdate(true)
	,fEnabled(true)
	,fName(name)
	,fState(0)
	,fMsg(msg)
	,fBitmap(bitmap)
{
	this->SetViewColor(bgcolor);
	if(tips != NULL)
	{
		fTips = tips;
	}else{
		fTips = "";
	}
}

/***********************************************************
 * Init
 ***********************************************************/
void
HToolbarButton::InitPictures()
{
	fOutsidePicture = MakeOutsidePicture(fBitmap);
	fInsidePicture = MakeInsidePicture(fBitmap);
	fDownPicture = MakeDownPicture(fBitmap);
	fDisablePicture = MakeDisablePicture(fBitmap);	
	fPicture = fOutsidePicture;
	fUpdate = true;
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HToolbarButton::~HToolbarButton()
{
	delete fOutsidePicture;
	delete fInsidePicture;
	delete fDownPicture;
	delete fDisablePicture;
	delete fMsg;
}

/***********************************************************
 * Mouse moved
 ***********************************************************/
void
HToolbarButton::MouseMoved(BPoint point, uint32 transit, const BMessage *message)
{
//=========== Change button's pictures ============//
	if(fEnabled == true)
	{
		if(transit == B_ENTERED_VIEW && this->Window()->IsActive())
		{ 
			if(fState != 0)
			{
				fPicture = fInsidePicture;
				fState = 0;
				fUpdate = true;	
				Invalidate();
			}
		}else if(transit == B_INSIDE_VIEW&& this->Window()->IsActive()){
			if(fState != 1)
			{
				fPicture = fInsidePicture; 
				
				fState = 1;
				fUpdate = true;
				Invalidate();
			}
		}else if(transit == B_EXITED_VIEW)
		{
			if(fState != 2)
			{
				fPicture = fOutsidePicture; 
				fUpdate = true;
				Invalidate();
				fState = 2;
			}		
		}
	}
	if(fEnabled == false && fState != 4)
	{
		fPicture = fDisablePicture;
		fState = 4;
		fUpdate = true;	
	}
//========== Show or Hide tooltips window ==============//	
	if(transit == B_INSIDE_VIEW&& this->Window()->IsActive())
	{
		if(fTipsWindow == NULL && fTips != NULL && this->Window()->IsActive() )
		{
			BPoint where = ConvertToScreen(point);
			fTipsWindow = new HToolTipsWindow(this,where,fTips.String());
		}
	}else if(transit == B_EXITED_VIEW) {
		if(fTipsWindow != NULL )
		{
			fTipsWindow->PostMessage(B_QUIT_REQUESTED);
			fTipsWindow = NULL;
		}
	}
}

/***********************************************************
 * Draw bitmap button.
 ***********************************************************/
void
HToolbarButton::Draw(BRect rect)
{
	BView::Draw(rect);
	if(fPicture)
		this->DrawPicture(fPicture,BPoint(1,1)); 	
	if(fUpdate)
	{
		this->Invalidate();
		fUpdate = false;
	}

}

/***********************************************************
 * When mouse is down, close fTipsWindow.
 ***********************************************************/
void
HToolbarButton::MouseDown(const BPoint point)
{
	if(fTipsWindow != NULL )
	{
		fTipsWindow->PostMessage(B_QUIT_REQUESTED);
		fTipsWindow = NULL;
	}
	if(fEnabled == true)
	{
		fPicture = fDownPicture;
		fState = 3;
		fUpdate = true;
		Invalidate();
	}
}

/***********************************************************
 * When mouse is up, send message to parent window.
 ***********************************************************/
void
HToolbarButton::MouseUp(const BPoint point)
{
	if(fEnabled == true)
	{
		fPicture = fInsidePicture;
		fState = 1;
		fUpdate = true;
		Invalidate();
		if(fMsg != NULL)
			Window()->PostMessage(fMsg);
	}
}


/***********************************************************
 * Make outside state picture.
 ***********************************************************/
BPicture*
HToolbarButton::MakeOutsidePicture(BBitmap *in)
{
	HToolbar *toolbar = cast_as(Parent(),HToolbar);
	BRect buttonRect = toolbar->ButtonRect();
	BView *view = new BView(BRect(0,0,buttonRect.Width(),buttonRect.Height())
							,"offview",0,0);
	BBitmap *bitmap = new BBitmap(BRect(0,0,15,15), in->ColorSpace(), true);

	BPicture *pict;
	bitmap->AddChild(view);
	bitmap->Lock();
	view->SetDrawingMode(B_OP_ALPHA); 
	view->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);	
	view->BeginPicture(new BPicture); 
	
	DrawBitmap(view,in);
	DrawString(view,fName.String());
	
	pict = view->EndPicture();
	bitmap->Unlock();
	
	delete bitmap;
	return pict;
}

/*
 * Make inside state picture.
 */
BPicture*
HToolbarButton::MakeInsidePicture(BBitmap *in)
{
	HToolbar *toolbar = cast_as(Parent(),HToolbar);
	BRect buttonRect = toolbar->ButtonRect();
	BView *view = new BView(BRect(0,0,buttonRect.Width(),buttonRect.Height())
							,"offview",0,0);
	BBitmap *bitmap = new BBitmap(view->Bounds(), in->ColorSpace(), true);
	BPicture *pict;
	bitmap->AddChild(view);
	bitmap->Lock();
	view->SetDrawingMode(B_OP_ALPHA); 
	view->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	view->BeginPicture(new BPicture); 
	
	DrawBitmap(view,in);
	DrawString(view,fName.String());
	
	//view->SetHighColor(White);
	//view->FillRect(BRect(0,0,0,22));
	//view->FillRect(BRect(0,0,22,0));
	//view->SetHighColor(BeShadow);
	//view->FillRect(BRect(21,0,21,21));
	//view->FillRect(BRect(0,21,21,21));
	BRect rect(Bounds());
	view->SetDrawingMode(B_OP_OVER); 
	rect.InsetBy(1,1);
	view->BeginLineArray(5);
	view->AddLine(rect.LeftTop(), rect.LeftBottom(),
			tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_MAX_TINT));
	view->AddLine(rect.LeftTop(), rect.RightTop(),
			tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_MAX_TINT));
	view->AddLine(rect.LeftBottom(), rect.RightBottom(),
			tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_3_TINT));
	rect.bottom--;
	view->AddLine(rect.LeftBottom(), rect.RightBottom(),
			tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_1_TINT));
	view->AddLine(rect.RightTop(), rect.RightBottom(),
			tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_3_TINT));
	view->EndLineArray();
	
	view->SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_3_TINT));
	view->StrokeRect(Bounds());
	pict = view->EndPicture();
	bitmap->Unlock();
	delete bitmap;
	return pict;
}

/***********************************************************
 * Make down state picture.
 ***********************************************************/
BPicture*
HToolbarButton::MakeDownPicture(BBitmap *in)
{
	HToolbar *toolbar = cast_as(Parent(),HToolbar);
	BRect buttonRect = toolbar->ButtonRect();
	BView *view = new BView(BRect(0,0,buttonRect.Width(),buttonRect.Height())
							,"offview",0,0);
	BBitmap *bitmap = new BBitmap(view->Bounds(), in->ColorSpace(), true);
	BPicture *pict;
	bitmap->AddChild(view);
	bitmap->Lock();
	
	view->SetHighColor(BeLightShadow);
	view->FillRect(view->Bounds());
	
	view->BeginPicture(new BPicture); 
	
	DrawString(view,fName.String(),true);
	view->SetDrawingMode(B_OP_MIN); 
	DrawBitmap(view,in,true);
	
	const float height = view->Bounds().Height();
	view->SetDrawingMode(B_OP_OVER);
	view->SetHighColor(BeShadow);
	view->FillRect(BRect(0,0,0,height));
	view->FillRect(BRect(0,0,height,0));
	view->SetHighColor(White);
	view->FillRect(BRect(height-1,0,height-1,height-1));
	view->FillRect(BRect(0,height-1,height-1,height-1));
	
	pict = view->EndPicture();
	bitmap->Unlock();
	delete bitmap;
	return pict;
}

/***********************************************************
 * Make disable state picture.
 ***********************************************************/
BPicture*
HToolbarButton::MakeDisablePicture(BBitmap *in)
{
	HToolbar *toolbar = cast_as(Parent(),HToolbar);
	BRect buttonRect = toolbar->ButtonRect();
	
	BView *view = new BView(BRect(0,0,buttonRect.Width(),buttonRect.Height())
							,"offview",0,0);
	BBitmap *bitmap = new BBitmap(view->Bounds(), in->ColorSpace(), true);
	BPicture *pict;
	bitmap->AddChild(view);
	bitmap->Lock();

	view->BeginPicture(new BPicture); 
	view->SetHighColor(bgcolor);
	view->FillRect(view->Bounds());
	
	DrawString(view,fName.String(),false,false);
	
	view->SetDrawingMode(B_OP_BLEND); 
	
	DrawBitmap(view,in);
	
	pict = view->EndPicture();
	bitmap->Unlock();
	delete bitmap;
	return pict;
}

/***********************************************************
 * DrawBitmap
 ***********************************************************/
void
HToolbarButton::DrawBitmap(BView *view,BBitmap *bitmap,bool downState)
{
	int32 x = 3,y = 3;
	if(WITH_LABEL_WIDTH == view->Bounds().Width())
	{
		y = 5;
		x = 11;
	}
	if(downState)
	{
		x++;
		y++;
	}
	view->DrawBitmap(bitmap,BPoint(x,y));
}

/***********************************************************
 * DrawString
 ***********************************************************/
void
HToolbarButton::DrawString(BView *view ,const char* label,bool downState,bool enabled)
{
	BRect bounds = view->Bounds();
	if(NORMAL_WIDTH == bounds.Width())
		return;
	int32 x,y;
	
	BFont font;
	font.SetFamilyAndStyle("Swis721 BT","Roman");
	font.SetSize(10);
	font.SetSpacing(B_BITMAP_SPACING);
	font_height height;
	font.GetHeight(&height);
	
	float h = height.ascent + height.descent;
	y = (int32)(bounds.bottom - h + 4);
	
	float w = view->StringWidth(label);
	if(w > bounds.Width())
		x = 1;
	else{
		x = (int32)(bounds.Width() - w)/2;
	}
	
	if(downState)
	{
		x ++;
		y ++;
	}
	view->SetLowColor(bgcolor);
	/*
	if(enabled && !downState)
	{
		view->SetHighColor(237,237,237);
	
		view->SetDrawingMode(B_OP_COPY);
		font.SetFace(B_BOLD_FACE);
		view->SetFont(&font);
		view->DrawString(label,BPoint(x+1,y+1));
	}
	*/
	if(enabled)
	{
		if(!downState)	
			view->SetHighColor(Black);
		else
			view->SetHighColor(255,0,0);
	}else
		view->SetHighColor(BeDarkShadow);
	font.SetFace(B_REGULAR_FACE);
	view->SetFont(&font);
	view->DrawString(label,BPoint(x,y));
}

/***********************************************************
 * Send update message to parent window.
 ***********************************************************/
void
HToolbarButton::Pulse()
{
	BMessage msg(M_UPDATE_TOOLBUTTON);
	msg.AddPointer("pointer",this);
	msg.AddString("name",fName.String());
	Window()->PostMessage(&msg);
}


/***********************************************************
 * Enable or disable picture.
 ***********************************************************/
void
HToolbarButton::SetEnabled(bool which)
{
	if(fEnabled != which)
	{
		fEnabled = which;
		if(fEnabled ==false)
		{
			fPicture = fDisablePicture;
			fState = 4;
		}else{
			fPicture = fOutsidePicture;
			fState = 0;
		}
		Invalidate();		
	}
}