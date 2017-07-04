#include <View.h>

#include "HAccountItem.h"
#include "people.h"
#include <santa/Colors.h>

/**************************************************************
 * Constructor.
 **************************************************************/
HAccountItem::HAccountItem(const char* name)
	:BListItem()
{
	fName = name;

	fBitmap = new BBitmap(BRect(0,0,15,15),B_COLOR_8_BIT);
	uint32 length;
	BRect bounds = fBitmap->Bounds();
	length = ((int32(bounds.right-bounds.left+1)+3)&0xFFFFFFFC)*int32(bounds.bottom-bounds.top+1);
	fBitmap->SetBits(kPeopleBits, length, 0, B_COLOR_8_BIT);
}

/**************************************************************
 * Destructor.
 **************************************************************/
HAccountItem::~HAccountItem()
{
	delete fBitmap;
}

/**************************************************************
 * Draw item content.
 **************************************************************/
void
HAccountItem::DrawItem(BView *owner, BRect frame, bool complete)
{
	rgb_color color;
	const rgb_color selectcolor = {184,194, 255,180};
	bool selected = IsSelected();
	// まず、Viewカラーで塗りつぶす
	owner->SetDrawingMode(B_OP_COPY);
	//if (complete)
	//{
		color = owner->ViewColor();
		owner->SetHighColor(color);
		owner->FillRect(frame);
	//}
	BFont newfont = be_plain_font;
	newfont.SetSize(12);
	BFont oldfont;
	owner->GetFont(&oldfont);

	drawing_mode mode = owner->DrawingMode();
	owner->SetDrawingMode(B_OP_ALPHA);
	if(fBitmap != NULL)
		owner->DrawBitmap(fBitmap,BPoint(frame.left,frame.top) );
	owner->SetFont(&newfont);
	owner->SetHighColor(Black);
	owner->MovePenTo(frame.left+20, frame.bottom-5);
	owner->DrawString(fName.String());
	owner->SetFont(&oldfont);

	if(selected){
		owner->SetHighColor(selectcolor);
		owner->FillRect(frame);
	}
	owner->SetDrawingMode(mode);
}

/**************************************************************
 * Set item height.
 **************************************************************/
void
HAccountItem::Update(BView *list ,const BFont *font)
{
	SetHeight(18.0);
}
