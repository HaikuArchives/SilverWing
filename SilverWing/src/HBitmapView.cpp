#include "HBitmapView.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HBitmapView::HBitmapView(BRect rect,const char* name,uint32 resizing_mode,BBitmap *bitmap)
	:BView(rect,name,resizing_mode,B_WILL_DRAW)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fBitmap = bitmap;
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HBitmapView::~HBitmapView()
{
	delete fBitmap;
}

/***********************************************************
 * Draw.
 ***********************************************************/
void
HBitmapView::Draw(BRect updateRect)
{
	this->SetDrawingMode(B_OP_ALPHA);
	this->DrawBitmap(fBitmap,BPoint(0,0));
}