#include "HArticleList.h"
#include "HArticleItem.h"
#include "HNews15Window.h"
#include "HFileCaption.h"
#include "HApp.h"

#include <ClassInfo.h>
#include <Autolock.h>
#include <Window.h>
#include <Application.h>

/***********************************************************
 * Constructor.
 ***********************************************************/
HArticleList::HArticleList(BRect rect,BetterScrollView **scroll,const char* title)
		:ColumnListView(rect,(CLVContainerView**)scroll,title,B_FOLLOW_ALL,B_WILL_DRAW,B_SINGLE_SELECTION_LIST,true)
{
	this->AddColumn(new CLVColumn(NULL,20.0,CLV_EXPANDER|CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE));
	this->AddColumn(new CLVColumn(NULL,20,CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE|
		CLV_NOT_RESIZABLE|CLV_PUSH_PASS|CLV_MERGE_WITH_RIGHT));
	this->AddColumn(new CLVColumn(_("Articles"),150,CLV_NOT_MOVABLE|CLV_SORT_KEYABLE|CLV_TELL_ITEMS_WIDTH));
	this->AddColumn(new CLVColumn(_("Sender"),100,CLV_NOT_MOVABLE|CLV_SORT_KEYABLE|CLV_TELL_ITEMS_WIDTH));
	this->AddColumn(new CLVColumn(_("Date"),110,CLV_NOT_MOVABLE|CLV_SORT_KEYABLE|CLV_TELL_ITEMS_WIDTH));
	fPointerList.MakeEmpty();
	this->SetSortFunction(HArticleItem::CompareItems);
	this->SetSortKey(3);
	rgb_color selection_col = {184,194, 255,255};
	SetItemSelectColor(true, selection_col);
	SetViewColor(tint_color( ui_color(B_PANEL_BACKGROUND_COLOR),B_LIGHTEN_2_TINT));
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HArticleList::~HArticleList()
{
}

/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
HArticleList::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	default:
		ColumnListView::MessageReceived(message);
	}
}

/***********************************************************
 * Add article to the list.
 ***********************************************************/
void
HArticleList::AddArticle(const char* subject,const char*sender,uint32 time,uint32 parent_id,uint16 index)
{
	BAutolock lock(Window());
	if( lock.IsLocked())
	{
	if(parent_id == 0)
	{
		HArticleItem *item = new HArticleItem(subject,sender,time,parent_id,index);
		this->AddItem( item );
		fPointerList.AddItem( item );
	}else{
		HArticleItem *item = cast_as(this->FullListLastItem(),HArticleItem);
		if(item->ParentID() == 0)
		{
			HArticleItem *target = new HArticleItem(subject,sender,time,parent_id,index);
			item->SetSuperItem(true);
			this->AddUnder(target,item);
			fPointerList.AddItem( target );
		}else{
			HArticleItem *target = new HArticleItem(subject,sender,time,parent_id,index);
			HArticleItem* parent = cast_as(this->Superitem(item),HArticleItem);
			this->AddUnder(target,parent);
			fPointerList.AddItem( target );
		}
	}
	}
}

/***********************************************************
 * Remove all articles from list and pointer list.
 ***********************************************************/
void
HArticleList::RemoveAll()
{
	BAutolock lock(Window());

	if(lock.IsLocked())
	{
		register int32 count = fPointerList.CountItems();
		
		this->MakeEmpty();	
		while(count >0)
		{
			delete (HArticleItem*)fPointerList.RemoveItem(--count);
		}
	}

}

/***********************************************************
 * SelectionChanged.
 ***********************************************************/
void
HArticleList::SelectionChanged()
{
	int32 sel = this->CurrentSelection();
	if(sel >=0)
	{
		HArticleItem *item = cast_as(this->ItemAt(sel),HArticleItem);
		if(item == NULL)
			return;
		BMessage msg(ARTICLE_REQUESTED);
		msg.AddString("category",fCategory.String());
		msg.AddInt16("thread",item->Index());
		be_app->PostMessage(&msg);
		StartBarberPole();
	}
	
}

/***********************************************************
 * MouseDown.
 ***********************************************************/
void
HArticleList::MouseDown(BPoint pos)
{
	this->MakeFocus(true);
	ColumnListView::MouseDown(pos);
}

/***********************************************************
 *
 ***********************************************************/
void
HArticleList::StartBarberPole()
{
	HNews15Window *win = (HNews15Window*)Window();
	win->Caption()->SetLabel(_("Getting Article…"));
	win->Caption()->StartBarberPole();
}