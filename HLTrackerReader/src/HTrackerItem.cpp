#include "HTrackerItem.h"
#include "HWindow.h"
#include "Colors.h"
#include "ResourceUtils.h"
#include <String.h>
#include <View.h>
#include <ClassInfo.h>
#include <Autolock.h>
#include <stdlib.h>

const rgb_color kBorderColor = ui_color(B_PANEL_BACKGROUND_COLOR);

/***********************************************************
 * Constructor.
 ***********************************************************/
HTrackerItem::HTrackerItem(const char* name 
						,const char* address
						,uint16 port
						,uint16 users
						,const char* desc
						,bool isTracker
						,HTrackerItem *parent)
	:CLVEasyItem(0, isTracker,0,16.0)
	,fParentItem(parent)
{
	BBitmap *bitmap = NULL;
	fConnection = NULL;
	fIsShown = false;
	fIsTracker = isTracker;
	BResources *rsrc = BApplication::AppResources();
	ResourceUtils utils(rsrc);
	if(isTracker == false)
		utils.GetBitmapResource('BBMP',"BMP:SERVER",&bitmap);
	else
		utils.GetBitmapResource('BBMP',"BMP:ADDTRACKER",&bitmap);
		
	this->SetColumnContent(1,bitmap);
	delete bitmap;
	this->SetColumnContent(2,name);
	this->SetColumnContent(3,address);
	BString Users;
	uint32 port32 = port;
	Users << port32;
	this->SetColumnContent(4,Users.String());
	Users = "";
	if(isTracker == false)
	{
		port32 = users;
		Users << port32;
	}else
		Users = "";
	this->SetColumnContent(5,Users.String());
	this->SetColumnContent(6,desc);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HTrackerItem::~HTrackerItem()
{
	delete fConnection;
}

/***********************************************************
 * Draw item 
 ***********************************************************/
void
HTrackerItem::DrawItemColumn(BView* owner, BRect item_column_rect, int32 column_index, bool complete)
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

/***********************************************************
 * Search servers.
 ***********************************************************/
void
HTrackerItem::Search()
{
	delete fConnection;
	fConnection = new HTrackerConnection( this->Address(),atoi(this->Port()),this);
	fConnection->Start();
}

/***********************************************************
 * Thread function.
 ***********************************************************/
int32
HTrackerItem::ThreadFunc(void* data)
{
	HTrackerItem *item = (HTrackerItem*)data;
	item->fConnection = new HTrackerConnection( item->Address(),atoi(item->Port()),item);
	item->fConnection->Main();
	return B_OK;
}

/***********************************************************
 * Cancel search.
 ***********************************************************/
void
HTrackerItem::StopSearch()
{
	if(fConnection	!= NULL)
	{
		fConnection->Cancel();
		fConnection = NULL;
	}
}

/***********************************************************
 * List items compare function.
 ***********************************************************/
int 
HTrackerItem::CompareItems(const CLVListItem *a_Item1, const CLVListItem *a_Item2, int32 KeyColumn)
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
	
	// port and users columns
	if( KeyColumn == 5 )
	{
		int col1 = atoi(text1);
		int col2 = atoi(text2);
		if( col1 == col2 )
			result = 0;
		else
			result = (col1 < col2 )? -1:1;
	}else{
		result = strcasecmp(text1,text2);
	}
	
	return result;
}
