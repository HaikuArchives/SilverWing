#include "HListView.h"
#include "HTrackerItem.h"
#include "HWindow.h"
#include <Autolock.h>
#include <ClassInfo.h>
#include <Message.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <stdlib.h>
#include <Mime.h>
#include "HTrackerItem.h"
#include "LockingList.h"
#include "IconMenuItem.h"
#include "ResourceUtils.h"
#include "HApp.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HListView::HListView(BRect rect
					,CLVContainerView** ContainerView
					,const char* name)
		:ColumnListView(rect
					,ContainerView
					,name
					,B_FOLLOW_ALL
					,B_WILL_DRAW|B_PULSE_NEEDED
					,B_SINGLE_SELECTION_LIST
					,true)
{
	this->AddColumn(new CLVColumn(NULL,20.0,CLV_EXPANDER|CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE));
	this->AddColumn(new CLVColumn(NULL,20,CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE|
		CLV_NOT_RESIZABLE|CLV_PUSH_PASS|CLV_MERGE_WITH_RIGHT));
	this->AddColumn(new CLVColumn(_("Name"),200,CLV_SORT_KEYABLE|CLV_TELL_ITEMS_WIDTH));
	this->AddColumn(new CLVColumn(_("Address"),100,CLV_SORT_KEYABLE|CLV_TELL_ITEMS_WIDTH));
	this->AddColumn(new CLVColumn(_("Port"),50,CLV_SORT_KEYABLE|CLV_TELL_ITEMS_WIDTH));	
	this->AddColumn(new CLVColumn(_("Users"),50,CLV_SORT_KEYABLE|CLV_TELL_ITEMS_WIDTH));
	this->AddColumn(new CLVColumn(_("Description"),600,CLV_SORT_KEYABLE|CLV_TELL_ITEMS_WIDTH));
	this->SetSortFunction(HTrackerItem::CompareItems);
	this->SetSortKey(2);
	//this->SetViewColor(BeBackgroundGrey);
	fItemList.MakeEmpty();
	fQueueList.MakeEmpty();
	fAllList.MakeEmpty();
	fKeyword ="";
	StartWatching(true);
	
	rgb_color selection_col = {184,194, 255,255};
	SetItemSelectColor(true, selection_col);
	SetViewColor(tint_color( ui_color(B_PANEL_BACKGROUND_COLOR),B_LIGHTEN_2_TINT));
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HListView::~HListView()
{
	
	fKeepWatching = false;
	status_t err = B_OK;
	::wait_for_thread(fWatcher,&err);
}

/***********************************************************
 * Remove all items.
 ***********************************************************/
void
HListView::RemoveAll()
{
	BAutolock lock(Window());

	register int32 count = fAllList.CountItems();
	//cout << "ITEM:" << count << endl;
	this->MakeEmpty();	
	while(count >0)
	{
		HTrackerItem *item = static_cast<HTrackerItem*>(fAllList.RemoveItem(--count));
		if(item == NULL)
			continue;
		item->StopSearch();
		delete item;
	}
}

/***********************************************************
 * Expand.
 ***********************************************************/
void
HListView::Expand(CLVListItem* item)
{
	int32 index = this->FullListIndexOf(item);
	int32 num = this->FullListNumberOfSubitems(item);
	for(register int i = 0;i< num;i++)
	{
		HTrackerItem *del = cast_as(this->RemoveItem((long)index+1),HTrackerItem);
		fItemList.RemoveItem(del);
		fAllList.RemoveItem(del);
		delete del;
	}	
	ColumnListView::Expand(item);
	fQueueList.MakeEmpty();
	HTrackerItem *tracker = cast_as(item,HTrackerItem);
	if(tracker != NULL)
		tracker->Search();
}

/***********************************************************
 * Add server to list.
 ***********************************************************/
void
HListView::AddServer(HTrackerItem* item)
{
	this->AddItem(item);
	fItemList.AddItem(item);
	fAllList.AddItem(item);
}

/***********************************************************
 * Collapse.
 ***********************************************************/
void
HListView::Collapse(CLVListItem* item)
{
	BAutolock lock(Window());
	//int32 index = this->FullListIndexOf(item);
	//int32 num = this->FullListNumberOfSubitems(item);
	
	HTrackerItem *tracker = cast_as(item,HTrackerItem);
	if(tracker != NULL)
		tracker->StopSearch();
	
	ColumnListView::Collapse(item);	
}

/***********************************************************
 * InitiateDrag
 ***********************************************************/
bool
HListView::InitiateDrag (
	BPoint  point , int32 index, bool wasSelected)
{
	if (wasSelected) 
	{
		BMessage		theMessage(B_SIMPLE_DATA);
		BRect			theRect = this->ItemFrame(index);
		HTrackerItem *item = (HTrackerItem*)this->ItemAt(index);
		if(!item->isTracker())
		{
			theMessage.AddString("be:types", B_FILE_MIME_TYPE);
			theMessage.AddString("address",item->Address());
			theMessage.AddInt16("port",atoi(item->Port()));
			BString mime = "application/octet-stream";
			theMessage.AddString("be:types", mime.String());
			theMessage.AddString("be:filetypes", mime.String());
			DragMessage(&theMessage, theRect);
		}
	}
	return (wasSelected);
}

/***********************************************************
 * MouseDown
 ***********************************************************/
void
HListView::MouseDown(BPoint pos)
{
	int32 buttons = 0; 
	
	BPoint point = pos;
	bool enable=false;
    Window()->CurrentMessage()->FindInt32("buttons", &buttons); 
    this->MakeFocus(true);
    // 右クリックのハンドリング 
    if(buttons == B_SECONDARY_MOUSE_BUTTON)
    {
    	
    	 BPopUpMenu *theMenu = new BPopUpMenu("RIGHT_CLICK",false,false);
 
    	 BFont font(be_plain_font);
    	 font.SetSize(10);
    	 theMenu->SetFont(&font);
    	 IconMenuItem *aItem;
        
         int32 sel = this->CurrentSelection();
		 if(sel >= 0)
		 {
		 	HTrackerItem *item = cast_as(ItemAt(sel),HTrackerItem);
		 	if(!item->isTracker())
         		enable =true;
         	else
         		enable = false;		
         }
         ResourceUtils utils;
         BBitmap *bitmap = utils.GetBitmapResource('BBMP',"BMP:CONNECT");
         aItem = new IconMenuItem(_("Connect"),new BMessage(M_CLICK_LIST),0,0,bitmap);
         aItem->SetEnabled(enable);
         theMenu->AddItem(aItem);
         aItem = new IconMenuItem(_("Connect with New Window"),new BMessage(M_CONNECT_NEW_WINDOW),'N',0,NULL);
         aItem->SetEnabled(enable);
         theMenu->AddItem(aItem);
         BRect r;
         ConvertToScreen(&pos);
		 r.top = pos.y-5;
		 r.bottom = pos.y+5;
		 r.left = pos.x-5;
		 r.right = pos.x+5;
    	 BMenuItem* BItem = theMenu->Go(pos, false,true,r);  
    	 if(BItem != NULL)
    	 {
    	 	BMessage*	aMessage = BItem->Message();
			if(aMessage != NULL)
				this->Window()->PostMessage(aMessage);
	 	} 
	 	delete theMenu;
	 }else{
     	ColumnListView::MouseDown(point);
     }
}

/***********************************************************
 * Add server under
 ***********************************************************/
void
HListView::AddServerUnder(HTrackerItem* newItem ,HTrackerItem* parent)
{
	if(newItem)
	{
	this->AddUnder(newItem,parent);
	fItemList.AddItem(newItem);
	}
}

/***********************************************************
 * Remove server
 ***********************************************************/
void
HListView::RemoveServer(HTrackerItem* item)
{
	this->RemoveItem(item);
	fItemList.RemoveItem(item);
}	

/***********************************************************
 * Remove all children
 *	Just remove children (dont connect to tracker)
 ***********************************************************/
void
HListView::RemoveAllChildren()
{
	register int32 count = CountItems();
	while(count > 0)
	{
		HTrackerItem *item = cast_as(ItemAt(--count),HTrackerItem);
		if(item->isTracker())
			RemoveChildren(item);
	}
}

/***********************************************************
 * RemoveChildren
 *   Romove child items , dont free items.
 ***********************************************************/
int32
HListView::RemoveChildren(HTrackerItem *item)
{
	BAutolock lock(Window());
	int32 index = this->FullListIndexOf(item);
	int32 num = this->FullListNumberOfSubitems(item);
	
	for(register int i = 0;i< num;i++)
	{
		HTrackerItem *del = cast_as(FullListItemAt(index+1),HTrackerItem);
		//cout << "Del" << del->Name() << endl;
		if(del == NULL)
			continue;
		if(del->isTracker())
			i += RemoveChildren(del);
		RemoveItem(del);					
		fItemList.RemoveItem(del);
		del->SetShow(false);
	}
	return num;
}

/***********************************************************
 * Pulse
 ***********************************************************/
void
HListView::Pulse()
{
	BAutolock lock(fQueueList);
	int32 count = fQueueList.CountItems();
	//cout << "Count: " << count << endl;
	
	while(count > 0)
	{
		HTrackerItem* item = static_cast<HTrackerItem*>(fQueueList.RemoveItem(--count));	
		this->AddServerUnder(item,item->ParentItem());
	}
}

/***********************************************************
 * Add
 ***********************************************************/
void
HListView::Add(HTrackerItem* item)
{
	BAutolock lock(fAllList);
	fAllList.AddItem(item);
}
/***********************************************************
 * Watcher thread
 ***********************************************************/
int32
HListView::Watcher(void* data)
{
	HListView* view = (HListView*)data;
	view->fKeepWatching = true;
	
	while(view->fKeepWatching)
	{
		BString keyword = view->fKeyword;
		view->fAllList.Lock();
		int32 count = view->fAllList.CountItems();
		view->fAllList.Unlock();
		for(register int32 i = 0;i < count;i++)
		{
			view->fAllList.Lock();
			HTrackerItem *item = static_cast<HTrackerItem*>(view->fAllList.ItemAt(i));
			view->fAllList.Unlock();
			if(!item)
				continue;
			if(item->isTracker())
				continue;
			// not specified keyword
			if(keyword.Length()== 0)
			{
				if(!item->IsShown())
					view->AddQueue(item);
				::snooze(100);
				continue;
			}
			//
			const char* name = item->Name();
			const char* desc = item->Description();
		
			if(name != NULL)
			{
				BString sname =name;
				int32 index = sname.IFindFirst(keyword);
				if( index != B_ERROR &&item->IsShown() == false)
					view->AddQueue(item);
			}
			if(desc != NULL)
			{
				BString sdesc =desc;
				int32 index = sdesc.IFindFirst(keyword);
				if( index != B_ERROR&&item->IsShown() == false )
					view->AddQueue(item);
			}
		}
		::snooze(100);	
	}
	return B_OK;
}

/***********************************************************
 * Start or stop watching thread 
 ***********************************************************/
void
HListView::StartWatching(bool start)
{
	if(start)
	{
		fWatcher = ::spawn_thread(Watcher,"watcher_thread",B_LOW_PRIORITY,this);
		::resume_thread(fWatcher);
	}else{
		fKeepWatching = false;
		status_t err;
		::wait_for_thread(fWatcher,&err);
	}
}

/***********************************************************
 * Empty queue list
 ***********************************************************/
void
HListView::EmptyQueue()
{
	BAutolock lock(fQueueList);
	fQueueList.MakeEmpty();
}

/***********************************************************
 * Add Queue
 ***********************************************************/
void
HListView::AddQueue(HTrackerItem *item)
{
	BAutolock lock(fQueueList);
	item->SetShow(true);
	fQueueList.AddItem(item);
}