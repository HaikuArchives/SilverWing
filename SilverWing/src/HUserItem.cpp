#include <iostream>
#include <Autolock.h>
#include <View.h>

#include "TextUtils.h"
#include "HApp.h"
#include "HotlineClient.h"
#include "HUserItem.h"
#include "Colors.h"
#include "HPrefs.h"
#include <stdlib.h>
#include <Debug.h>

/***********************************************************
 * Constructor.
 ***********************************************************/
HUserItem::HUserItem(uint16 sock,uint16 icon,uint16 color,const char* nick)
	:BListItem()
	,fNick(NULL)
	,fBitmap(NULL)
	,fSock(sock)
	,fColor(color)
	,fIcon(0)
{
	fBad = ((HApp*)be_app)->IsBadmoon();
	this->ChangeUser(sock,icon,color,nick);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HUserItem::~HUserItem()
{
	delete fBitmap;
	fBitmap = NULL;
	delete[] fNick;
}

/***********************************************************
 * Draw item
 ***********************************************************/
void
HUserItem::DrawItem(BView *owner, BRect frame, bool complete)
{
	rgb_color color;
	const rgb_color selectcolor = {184,194, 255,180};
	bool selected = IsSelected();
	
	owner->SetDrawingMode(B_OP_COPY);
	//if (complete) 
	//{
		color = owner->ViewColor();
		owner->SetHighColor(color);
		owner->FillRect(frame);
	//}	
	BFont newfont = be_bold_font;
	newfont.SetSize(12);
	BFont oldfont;
	owner->GetFont(&oldfont);
	int32 usr_color = fColor%4;

	switch(usr_color)
	{
	// Normal User
	case 0: // normal
		color = Black;
		break;	
	// Away normal user
	case 1: // normal
		color = BeShadow;
		break;
	// admin 
	case 2: // normal
		color = Red;
		break;
	// admin away
	case 3: // normal
		color = GrayRed;
		break;
	default:
		color = Black;
	}
	drawing_mode mode = owner->DrawingMode();
	
	if(fBitmap != NULL)
	{
		if(!fBad)
		owner->SetDrawingMode(B_OP_ALPHA);

		owner->DrawBitmap(fBitmap,BPoint(frame.left,frame.top) );	
	}
	if(fBad)
		owner->SetDrawingMode(B_OP_ALPHA);

	owner->SetFont(&newfont);
	owner->SetHighColor(color);
	owner->MovePenTo(frame.left+38, frame.bottom-4);
	owner->SetDrawingMode(B_OP_ALPHA);
	if(fNick)
		owner->DrawString(fNick);
	owner->SetFont(&oldfont);
	
	if(selected){
		owner->SetHighColor(selectcolor);
		owner->FillRect(frame);
	}
	owner->SetDrawingMode(mode);
}

/***********************************************************
 * Load user bitmap
 ***********************************************************/
void
HUserItem::LoadUserBitmap(int16 icon)
{
	delete fBitmap;
	BBitmap *bitmap = ((HApp*)be_app)->GetIcon(icon);
	if(bitmap)
		fBitmap = new BBitmap(bitmap);
	delete bitmap;
	//delete fBitmap;
	//fBitmap = NULL;
	//((HApp*)be_app)->CloseResource();
	//fBitmap = ((HApp*)be_app)->GetIcon(icon);
}

/***********************************************************
 * Change user item
 ***********************************************************/
void
HUserItem::ChangeUser(uint16 sock ,uint16 icon,uint16 color,const char* nick)
{
	if(strlen(nick) != 0)
	{
		delete[] fNick;
		fNick = new char[strlen(nick)+1];
		::strcpy(fNick,nick);
		fNick[strlen(nick)] = '\0';
		int32 encoding;
		((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
		if(encoding)
			TextUtils().ConvertToUTF8(&fNick,encoding-1);
	}else{
		delete[] fNick;
		fNick = NULL;
	}
		
	if(fIcon != icon)
		this->LoadUserBitmap(icon);
	fIcon = icon;
	fColor = color;
	fSock = sock;
}

/***********************************************************
 * Return user nick
 ***********************************************************/
const char*
HUserItem::Nick() const
{
	return fNick;
}	

/***********************************************************
 * Update
 *		Set item height
 ***********************************************************/
void	
HUserItem::Update(BView *list ,const BFont *font)
{
	SetHeight(17.0);
}