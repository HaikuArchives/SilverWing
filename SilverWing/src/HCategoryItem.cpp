#include "HCategoryItem.h"
#include "TextUtils.h"
#include "ResourceUtils.h"
#include "HApp.h"
#include "HPrefs.h"

#include <View.h>

const rgb_color kBorderColor = ui_color(B_PANEL_BACKGROUND_COLOR);

/***********************************************************
 * Constructor.
 ***********************************************************/
HCategoryItem::HCategoryItem(const char* title,uint16 type,uint16 posted,uint32 index,bool superitem)
			:CLVEasyItem(0,superitem,false,16.0)
{
	fTitle = title;
	fType = type;
	fPosted = posted;
	int32 p = posted;
	BString str;
	str << title << "   (" << p << ")";
	char *t = new char[str.Length()+1];
	strcpy(t,str.String());
	int32 encoding;
	TextUtils tUtils;
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
	if(encoding)
		tUtils.ConvertToUTF8(&t,encoding-1);
	str = t;
	ResourceUtils utils;
	BBitmap *bitmap = NULL;
	if(type == 2)
		bitmap = utils.GetBitmapResource('BBMP',"BMP:BUNDLE");
	else
		bitmap = utils.GetBitmapResource('BBMP',"BMP:NEWS");
	SetColumnContent(1,bitmap);
	delete bitmap;
	SetColumnContent(2,str.String());
	fIndex = index;
	delete[] t;
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HCategoryItem::~HCategoryItem()
{
}

/***********************************************************
 * Draw item 
 ***********************************************************/
void
HCategoryItem::DrawItemColumn(BView* owner, BRect item_column_rect, int32 column_index, bool complete)
{
	CLVEasyItem::DrawItemColumn(owner,item_column_rect,column_index,complete);
	// Stroke line
	rgb_color old_col = owner->HighColor();
	
	owner->SetHighColor(kBorderColor);
	
	BPoint start,end;
	start.y = end.y = item_column_rect.bottom;
	start.x = 0;
	end.x = owner->Bounds().right;
	
	owner->StrokeLine(start,end);
	owner->SetHighColor(old_col);
}	