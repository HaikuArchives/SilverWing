#include <string>
#include <iostream>
#include <ClassInfo.h>
#include <stdlib.h>
#include <stdio.h>
#include <View.h>

#include "HFileItem.h"
#include "HApp.h"
#include "HPrefs.h"
#include "Colors.h"
#include "TextUtils.h"
//#include "Paper.h"
#include "PrefilledBitmap.h"
#include "folder.h"
#include "unknown.h"
#include "text.h"
#include "partitial.h"
#include "binary.h"
#include "archive.h"
#include "sit.h"
#include "mp3.h"
#include "mov.h"
#include "HTbm.h"
#include "image.h"
#include "rohd.h"

const rgb_color kBorderColor = ui_color(B_PANEL_BACKGROUND_COLOR);


/***********************************************************
 * Constructor
 ***********************************************************/
HFileItem::HFileItem(const char* name,
					const char* type,
					const char* creator,
					uint32 size,
					uint32 index,
					uint32 modified,
					bool isSuper)
	:CLVEasyItem(0,isSuper,false,20.0)
	,fTime(modified)
{
	fItemIndex = index;
	fIsFolder = false;
	BString str = type;
	int i  = str.FindFirst("fldr");
	if(i >=0 )
		fIsFolder = true;

	BString size_str;
	/************* bitmap ***************/
	PrefilledBitmap *bitmap;
	if(strcmp("fldr",str.String())==0)
		bitmap = new PrefilledBitmap(BRect(0,0,15,15),B_COLOR_8_BIT,kFolderBits);
	else if(strcmp("TEXT",str.String()) ==0)
		bitmap = new PrefilledBitmap(BRect(0,0,15,15),B_COLOR_8_BIT,kTextBits);
	else if(strcmp("HTft",str.String()) == 0)
		bitmap = new PrefilledBitmap(BRect(0,0,15,15),B_COLOR_8_BIT,kPartitialBits);
	else if(strcmp("DEXE",str.String()) == 0 || strcmp("APPL",str.String()) == 0)
		bitmap = new PrefilledBitmap(BRect(0,0,15,15),B_COLOR_8_BIT,kBinaryBits);
	else if(strncmp("ZIP",str.String(),3) == 0)
		bitmap = new PrefilledBitmap(BRect(0,0,15,15),B_COLOR_8_BIT,kArchiveBits);
	else if(strcmp("SITD",str.String()) == 0 )
		bitmap = new PrefilledBitmap(BRect(0,0,15,15),B_COLOR_8_BIT,kSitBits);
	else if(strcmp("MooV",str.String()) == 0 || strcmp("MPEG",str.String()) == 0)
		bitmap = new PrefilledBitmap(BRect(0,0,15,15),B_COLOR_8_BIT,kMovBits);
	else if(strcmp("PNGf",str.String()) == 0 || strncmp("BMP",str.String(),3) == 0||strcmp("GIFf",str.String())== 0||strcmp("PICT",str.String())==0)
		bitmap = new PrefilledBitmap(BRect(0,0,15,15),B_COLOR_8_BIT,kImageBits);
	else if(strncmp("MP3",str.String(),3) == 0||strcmp("WAVE",str.String()) ==0 )
		bitmap = new PrefilledBitmap(BRect(0,0,15,15),B_COLOR_8_BIT,kMp3Bits);
	else if(strcmp("rohd",str.String()) == 0)
		bitmap = new PrefilledBitmap(BRect(0,0,15,15),B_COLOR_8_BIT,kRohdBits);
	else if(strcmp("HTbm",str.String()) == 0)
		bitmap = new PrefilledBitmap(BRect(0,0,15,15),B_COLOR_8_BIT,kHTbmBits);
	else
		bitmap = new PrefilledBitmap(BRect(0,0,15,15),B_COLOR_8_BIT,kUnknownBits);
	/***************************************************/
	if( fIsFolder)
	{	
		size_str << "( " << size << " items" << " )";
		this->fSize = 0;
	}else{
		char *text = new char[1024];
		::memset(text,0,1024);
		this->fSize = size;
		double d = size;
		if( d < 1024)
		{
			::sprintf(text,"%10.0fBytes",d);
		}else if( d >= 1024 && d < 1048576.0){
			d = d/1024L;
			::sprintf(text,"%8.2fKB",d);
		}else if(d >= 1048576.0){
			d = d/1024.0;
			d = d/1024.0;
			::sprintf(text,"%8.2fMB",d);
		} 
		size_str = text;
		delete [] text;
	}
	char *tmpname = new char[strlen(name)+1];
	::memset(tmpname,0,strlen(name)+1);
	
	fName = name;
	::strcpy(tmpname,name);
	int32 encoding;
	TextUtils tUtils;
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
	if(encoding)
		tUtils.ConvertToUTF8(&tmpname,encoding-1);

	this->SetColumnContent(1,bitmap);
	delete bitmap;
	BString encoded_name = tmpname;
	this->SetColumnContent(2,encoded_name.String());
	this->SetColumnContent(3,size_str.String());
	if(modified)
	{
		if(fIsFolder)
		{
			this->SetColumnContent(4,"");
		}else{
			char *mod = new char[64];
			char tmp[64];
			
			struct tm* time = localtime((time_t*)&modified);
			strftime(tmp, 64,"%m/%d/%y", time);
			
			sprintf(mod,"%s %.2d:%.2d:%.2d",tmp,time->tm_hour,time->tm_min, time->tm_sec);
		
			this->SetColumnContent(4,mod);
			delete[] mod;
		}
	}
	//this->SetHeight(20);
	delete[] tmpname;
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HFileItem::~HFileItem()
{
}

/***********************************************************
 * Return Decoded file name.
 ***********************************************************/
const char*
HFileItem::DecodedName()
{
	return GetColumnContentText(2);
}

/***********************************************************
 * Return item bitmap
 ***********************************************************/
const BBitmap*
HFileItem::Bitmap()
{
	return GetColumnContentBitmap(1);
}

/***********************************************************
 * Sort function.
 ***********************************************************/
int 
HFileItem::CompareItems(const CLVListItem *a_Item1, const CLVListItem *a_Item2, int32 KeyColumn)
{
	int result;
	const CLVEasyItem* Item1 = cast_as(a_Item1,const CLVEasyItem);
	const CLVEasyItem* Item2 = cast_as(a_Item2,const CLVEasyItem);
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
	
	/******** In size column *******/
	if( KeyColumn == 3 )
	{
		int32 col1 = ((HFileItem*)Item1)->Size();
		int32 col2 = ((HFileItem*)Item2)->Size();
		if( col1 == col2 )
			result = 0;
		else
			result = (col1 < col2 )? -1:1;
	}else if(KeyColumn == 4){
		int32 time1 = ((HFileItem*)Item1)->Time();
		int32 time2 = ((HFileItem*)Item2)->Time();
		
		if(time1 == time2)
			result = 0;
		else
			result = (time1 < time2) ? -1:1;
	}else{
		result = strcasecmp(text1,text2);
	}
	
	return result;
}

/***********************************************************
 * Draw item 
 ***********************************************************/
void
HFileItem::DrawItemColumn(BView* owner, BRect item_column_rect, int32 column_index, bool complete)
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