#include <ClassInfo.h>
#include <Autolock.h>
#include <Application.h>
#include <Debug.h>

#include "HCategoryList.h"
#include "HCategoryItem.h"
#include "HWindow.h"
#include "HNews15Window.h"
#include "HotlineClient.h"
#include "HFileCaption.h"
#include "HApp.h"

/***********************************************************
 *
 ***********************************************************/
HCategoryList::HCategoryList(BRect rect,BetterScrollView **scroll,const char* title)
		:ColumnListView(rect,(CLVContainerView**)scroll,title,B_FOLLOW_ALL,B_WILL_DRAW,B_SINGLE_SELECTION_LIST,true)
{
	this->AddColumn(new CLVColumn(NULL,20.0,CLV_EXPANDER|CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE));
	this->AddColumn(new CLVColumn(NULL,20,CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE|
		CLV_NOT_RESIZABLE|CLV_PUSH_PASS|CLV_MERGE_WITH_RIGHT));
	this->AddColumn(new CLVColumn(_("Categories"),200,CLV_NOT_MOVABLE|CLV_TELL_ITEMS_WIDTH));
	fPointerList.MakeEmpty();
	rgb_color selection_col = {184,194, 255,255};
	SetItemSelectColor(true, selection_col);
	SetViewColor(tint_color( ui_color(B_PANEL_BACKGROUND_COLOR),B_LIGHTEN_2_TINT));
}

/***********************************************************
 *
 ***********************************************************/
HCategoryList::~HCategoryList()
{
	this->RemoveAll();
}

/***********************************************************
 *
 ***********************************************************/
void
HCategoryList::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	default:
		ColumnListView::MessageReceived(message);
	}
}

/***********************************************************
 *
 ***********************************************************/
void
HCategoryList::AddCategory(const char* name,int16 type,int16 posted,uint32 index)
{
	BAutolock lock(Window());
	if( lock.IsLocked())	
	{
		if(index == 0)
		{
			HCategoryItem *item;
			this->AddItem(item = new HCategoryItem(name,type,posted,this->FullListCountItems()+1,type == 2 ? true:false));
			item->SetPath(name);
			fPointerList.AddItem(item);
		}else{
			HCategoryItem **items = (HCategoryItem**)this->Items();
			for(register int i = 0;i < this->FullListCountItems();i++)
			{
				if( (*(items+i))->Index() == index )
				{
					HCategoryItem *item;
					this->AddUnder(item = new HCategoryItem(name,type,posted,this->FullListCountItems()+1,type == 2 ? true:false),*(items+i));
					BString path=(*(items+i))->Path();
					path << "/";
					path << name;
					item->SetPath(path.String());
					fPointerList.AddItem(item);
					break;
				}
			}
		}
	}	
}

/***********************************************************
 *
 ***********************************************************/
void
HCategoryList::SelectionChanged()
{
	Window()->PostMessage(H_CATEGORY_SEL_CHANGED);
	int32 sel = this->CurrentSelection();
	if(sel >=0)
	{
		HCategoryItem *item = cast_as(this->ItemAt(sel),HCategoryItem);
		if(item == NULL)
			return;
		if(item->Type() == 3)
		{
			BMessage msg(H_NEWS_SEND_GET_ARTICLELIST);
			msg.AddString("path",item->Path());
			be_app->PostMessage(&msg);
			StartBarberPole(_("Getting Article List…"));
		}
	}
}

/***********************************************************
 *
 ***********************************************************/
void
HCategoryList::Expand(CLVListItem* item)
{
	BAutolock lock(Window());
	if(FullListNumberOfSubitems(item) == 0)
	{
		/*int32 index = this->FullListIndexOf(item);
		int32 num = this->FullListNumberOfSubitems(item);
		for(int i = 0;i< num;i++)
		{
			HCategoryItem *del = (HCategoryItem*)this->RemoveItem((long)index+1);
			delete del;
		}*/
		
		HCategoryItem *cate = cast_as( item ,HCategoryItem );
		BMessage msg(MWIN_NEWS_GET_CATEGORY);
		msg.AddString("path",cate->Path());
		msg.AddInt32("index",cate->Index());
		be_app->PostMessage(&msg);
		
		StartBarberPole(_("Getting News Categories…"));
	}
	ColumnListView::Expand(item);
}

/***********************************************************
 *
 ***********************************************************/
void
HCategoryList::Collapse(CLVListItem* item)
{
	BAutolock lock(Window());

	RemoveChildItems(item);
	ColumnListView::Collapse(item);	
}

/***********************************************************
 *
 ***********************************************************/
void
HCategoryList::RemoveAll()
{
	BAutolock lock(Window());
	
	if(lock.IsLocked())
	{
		register int32 count = fPointerList.CountItems();
		
		this->MakeEmpty();	
		while(count >0)
		{
			delete (HCategoryItem*)fPointerList.RemoveItem(--count);
		}
	}
}

/***********************************************************
 *
 ***********************************************************/
void
HCategoryList::MouseDown(BPoint pos)
{
	this->MakeFocus(true);
	ColumnListView::MouseDown(pos);
}

/***********************************************************
 *
 ***********************************************************/
int32
HCategoryList::RemoveChildItems(CLVListItem* item)
{
	BAutolock lock(Window());
	int32 index = this->FullListIndexOf(item);
	int32 num = this->FullListNumberOfSubitems(item);
	
	for(register int i = 0;i< num;i++)
	{
		HCategoryItem *del = cast_as(FullListItemAt(index+1),HCategoryItem);
		if(del == NULL)
			continue;
		if(del->IsSuperItem())
			i += RemoveChildItems(del);
		RemoveCategoryItem(del);					
	}
	return num;
}

/***********************************************************
 *
 ***********************************************************/
void
HCategoryList::RemoveCategoryItem(HCategoryItem* item)
{
	RemoveItem(item);
	fPointerList.RemoveItem(item);
}

/***********************************************************
 *
 ***********************************************************/
void
HCategoryList::StartBarberPole(const char *label)
{
	HNews15Window *win = (HNews15Window*)Window();
	win->Caption()->SetLabel(label);
	win->Caption()->StartBarberPole();
}