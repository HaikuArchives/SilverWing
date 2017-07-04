#define private public
#include <santa/CLVEasyItem.h>
#undef private
enum
{
	CLVColNone =				0x00000000,
	CLVColStaticText = 			0x00000001,
	CLVColTruncateText =		0x00000002,
	CLVColBitmap = 				0x00000003,
	CLVColUserText = 			0x00000004,
	CLVColTruncateUserText =	0x00000005,

	CLVColTypesMask =			0x00000007,

	CLVColFlagBitmapIsCopy =	0x00000008,
	CLVColFlagNeedsTruncation =	0x00000010,
	CLVColFlagRightJustify =	0x00000020
};


#include "HArticleItem.h"
#include "ResourceUtils.h"
#include "TextUtils.h"
#include "HApp.h"
#include "HPrefs.h"
#include <ClassInfo.h>
#include <View.h>

const rgb_color kBorderColor = ui_color(B_PANEL_BACKGROUND_COLOR);


/***********************************************************
 * Constructor.
 ***********************************************************/
HArticleItem::HArticleItem(const char* subject,const char* sender,
						uint32 in_time,uint32 parent_id,uint16 index)
			:CLVEasyItem(0,false,false,16.0)
{
	fSubject = subject;
	fParent_id = parent_id;
	fIndex = index;
	fTime = in_time;
	BString str;
	str << subject << "   ( by " << sender << " )";

	ResourceUtils utils;
	BBitmap *bitmap = utils.GetBitmapResource('BBMP',"BMP:POST");

	this->SetColumnContent(1,bitmap);
	delete bitmap;
	char *text = new char[str.Length()+1];
	strcpy(text,str.String());
	int32 encoding;
	TextUtils tUtils;
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
	if(encoding)
		tUtils.ConvertToUTF8(&text,encoding-1);
	str = text;
	time_t timet = in_time;
	struct tm *time = localtime(&timet);
	char *date = new char[1024];
	::sprintf(date,"%.2d/%.2d %.2d:%.2d:%.2d %d",\
		(time->tm_mon)+1,time->tm_mday,time->tm_hour,time->tm_min,time->tm_sec,\
		(time->tm_year) + 1900);
	this->SetColumnContent(2,subject);
	this->SetColumnContent(3,sender);
	this->SetColumnContent(4,date);
	delete[] text;
	delete[] date;
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HArticleItem::~HArticleItem()
{

}

/***********************************************************
 * List items compare function.
 ***********************************************************/
int
HArticleItem::CompareItems(const CLVListItem *a_Item1, const CLVListItem *a_Item2, int32 KeyColumn)
{
	int result;
	const HArticleItem* Item1 = cast_as(a_Item1,const HArticleItem);
	const HArticleItem* Item2 = cast_as(a_Item2,const HArticleItem);
	if(Item1 == NULL || Item2 == NULL || Item1->m_column_types.CountItems() <= KeyColumn ||
		Item2->m_column_types.CountItems() <= KeyColumn)
		return 0;

	int32 type1 = ((int32)Item1->m_column_types.ItemAt(KeyColumn)) & CLVColTypesMask;
	int32 type2 = ((int32)Item2->m_column_types.ItemAt(KeyColumn)) & CLVColTypesMask;

	if(!((type1 == CLVColStaticText || type1 == CLVColTruncateText || type1 == CLVColTruncateUserText ||
		type1 == CLVColUserText) && (type2 == CLVColStaticText || type2 == CLVColTruncateText ||
		type2 == CLVColTruncateUserText || type2 == CLVColUserText)))
		return 0;

	const char* text1 = NULL;
	const char* text2 = NULL;

	if(type1 == CLVColStaticText || type1 == CLVColTruncateText)
		text1 = (const char*)Item1->m_column_content.ItemAt(KeyColumn);
	else if(type1 == CLVColTruncateUserText || type1 == CLVColUserText)
		text1 = Item1->GetUserText(KeyColumn,-1);

	if(type2 == CLVColStaticText || type2 == CLVColTruncateText)
		text2 = (const char*)Item2->m_column_content.ItemAt(KeyColumn);
	else if(type2 == CLVColTruncateUserText || type2 == CLVColUserText)
		text2 = Item2->GetUserText(KeyColumn,-1);

	// date columns
	if( KeyColumn == 4 )
	{
		int diff =((HArticleItem*)Item1)->Time() - ((HArticleItem*)Item2)->Time();
		if(diff > 0)
			result = -1;
		else if(diff == 0)
			result = 0;
		else
			result = 1;
	}else{
		result = strcasecmp(text1,text2);
	}

	return result;
}

/***********************************************************
 * Draw item
 ***********************************************************/
void
HArticleItem::DrawItemColumn(BView* owner, BRect item_column_rect, int32 column_index, bool complete)
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
