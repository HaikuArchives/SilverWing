#include "HToolbarSpace.h"
#include "space.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HToolbarSpace::HToolbarSpace(BRect rect):BView(rect,"",B_FOLLOW_NONE,B_WILL_DRAW)
{
	Init();
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HToolbarSpace::~HToolbarSpace()
{
	delete fBitmap;
}

/***********************************************************
 * Initialize.
 ***********************************************************/
void
HToolbarSpace::Init()
{
	int32 length;
	fBitmap = new BBitmap(BRect(0,0,3,27),B_COLOR_8_BIT);
	BRect bounds(0,0,3,27);

	length = ((int32(bounds.right-bounds.left+1)+3)&0xFFFFFFFC)*int32(bounds.bottom-bounds.top+1);
	
	fBitmap->SetBits(spaceBits, length, 0, B_COLOR_8_BIT);
	//this->SetViewBitmap(&bitmap);
}

/***********************************************************
 * Draw
 ***********************************************************/
void
HToolbarSpace::Draw(BRect rect)
{
	BView::Draw(rect);
	DrawBitmap(fBitmap,rect);
}	