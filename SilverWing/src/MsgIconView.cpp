#include "MsgIconView.h"
#include "HApp.h"

MsgIconView::MsgIconView(BRect rect,const char* nick,BBitmap *bitmap)
	:BView(rect,nick,B_FOLLOW_TOP|B_FOLLOW_LEFT_RIGHT,B_WILL_DRAW)
	,fIcon(bitmap)
	,fNick(nick)
{
	fBad = ((HApp*)be_app)->IsBadmoon();
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));		
}

MsgIconView::~MsgIconView()
{
	delete fIcon;
}

void
MsgIconView::Draw(BRect updateRect)
{
	BRect bounds = Bounds();
	BFont newfont(be_bold_font),oldfont;
	this->GetFont(&oldfont);
	newfont.SetSize(12);
	drawing_mode mode = DrawingMode();

	if(fIcon)
	{
		if(!fBad)
			SetDrawingMode(B_OP_ALPHA);
		DrawBitmap(fIcon,BPoint(bounds.left,bounds.top) );
	}
	if(fBad)
		SetDrawingMode(B_OP_ALPHA);

	SetFont(&newfont);
	SetHighColor(0,0,0,255);
	MovePenTo(bounds.left+38, bounds.bottom-7);
	DrawString(fNick.String());
	SetFont(&oldfont);
	SetDrawingMode(mode);
}

void
MsgIconView::SetIcon(BBitmap *bitmap)
{
	delete fIcon;
	if(bitmap)
		fIcon = new BBitmap(bitmap);
	delete bitmap;
}