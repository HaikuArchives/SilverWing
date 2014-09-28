#include "HUserList.h"
#include "HUserItem.h"
#include "HWindow.h"
#include "ResourceUtils.h"
#include "IconMenuItem.h"
#include "HUserItem.h"
#include "HApp.h"

#include <PopUpMenu.h>
#include <Autolock.h>
#include <ClassInfo.h>

/***********************************************************
 * Constructor.
 ***********************************************************/
HUserList::HUserList(BRect frame,const char* title)
		:BListView(frame,title,B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL,B_WILL_DRAW)
{
	fPointerList.MakeEmpty();
	this->MakeEmpty();
	SetTarget(Window());
	
	SetViewColor(tint_color( ui_color(B_PANEL_BACKGROUND_COLOR),B_LIGHTEN_2_TINT));
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HUserList::~HUserList()
{
	this->RemoveAll();
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HUserList::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	
	default:
			this->BListView::MessageReceived(message);
	}
}

/***********************************************************
 * InitiateDrag
 ***********************************************************/
bool
HUserList::InitiateDrag (
	BPoint  point , int32 index, bool wasSelected)
{
	if (wasSelected) 
	{
		BMessage msg(USERLIST_USER_DRAG_MSG);
		HUserItem *item = cast_as(ItemAt(index),HUserItem);
		if(item == NULL)
			return false;
		BRect	theRect = this->ItemFrame(index);
		msg.AddInt32("sock",(int32)item->Sock());
		msg.AddPointer("old_window",Window());
		msg.AddString("nick",item->Nick());
		theRect.OffsetTo(B_ORIGIN);
		BBitmap *bitmap = new BBitmap(theRect,B_RGBA32,true);
		BView *view = new BView(theRect,"",B_FOLLOW_NONE,0);
		bitmap->AddChild(view);
		bitmap->Lock();
		view->SetHighColor(0,0,0,0);
		view->FillRect(view->Bounds());
		
		view->SetDrawingMode(B_OP_ALPHA);
		view->SetHighColor(0,0,0,128);
		view->SetBlendingMode(B_CONSTANT_ALPHA,B_ALPHA_COMPOSITE);
		if(item->Bitmap())
			view->DrawBitmap(item->Bitmap());
		
		BFont newfont = be_bold_font;
		newfont.SetSize(12);
		
		view->SetFont(&newfont);
		view->MovePenTo(theRect.left+38, theRect.bottom-3);
		switch(item->Color())
		{
		// Normal User
		case 0: // normal
		case 4: // refuse message
		case 8: // refuse prvchat
		case 12: // refuse both
		{
			view->SetHighColor(0,0,0,128);
			break;
		}	
		// Away normal user
		case 1: // normal
		case 5: // refuse message
		case 9: // refuse prvchat
		case 13: // refuse both
		{
			view->SetHighColor(152,152,152,128);;
			break;
		}	
		// admin 
		case 2: // normal
		case 6: // refuse message
		case 10: // refuse prvchat
		case 14: // refuse both
		{
			view->SetHighColor(255,0,0,128);
			break;
		}	
		// admin away
		case 3: // normal
		case 7: // refuse message
		case 11: // refuse prvchat
		case 15: // refuse both
		{
			view->SetHighColor(255,150,150,128);
			break;
		}	
		default:
			view->SetHighColor(0,0,0,128);
		}
		if(item->Nick())
			view->DrawString(item->Nick());
		bitmap->Unlock();
		//point.PrintToStream();

		DragMessage(&msg, bitmap, B_OP_ALPHA,
				BPoint(bitmap->Bounds().Width()/2,bitmap->Bounds().Height()/2));
		//delete bitmap;
	}	
	return (wasSelected);
}

/***********************************************************
 * RemoveAll
 ***********************************************************/
void
HUserList::RemoveAll()
{	
	register int32 count = fPointerList.CountItems();
	while(count>0)
	{
		delete (HUserItem*)fPointerList.RemoveItem(--count);
	}
	this->MakeEmpty();
}

/***********************************************************
 * Delete user from pointer list.
 ***********************************************************/
HUserItem*
HUserList::DeleteItem(int32 index)
{
	BAutolock lock(Window());
	
	fPointerList.RemoveItem(index);
	return (HUserItem*)RemoveItem(index);
	
}

/***********************************************************
 * Add new user to pointer list and list view.
 ***********************************************************/
void
HUserList::AddUserItem(HUserItem* item)
{
	BAutolock lock(Window());
	
	fPointerList.AddItem(item);
	this->AddItem(item);
}

/***********************************************************
 * AddUserList
 ***********************************************************/
void
HUserList::AddUserList(BList *list)
{
	fPointerList.AddList(list);
	this->AddList(list);
}

/***********************************************************
 * MouseMoved
 ***********************************************************/
void
HUserList::MouseMoved(BPoint point,uint32 transit,const BMessage *message)
{
	// if message is not NULL, maybe dragged.
	/*if(message != NULL && transit == B_OUTSIDE_VIEW)
	{
		//message->PrintToStream();
		if(Window()->IsActive())
			Window()->Activate(false);
	}*/
	if(message != NULL && transit == B_OUTSIDE_VIEW){
		this->MakeFocus(false);
	}
	BListView::MouseMoved(point,transit,message);
}

/***********************************************************
 * Find user icon
 ***********************************************************/
uint16
HUserList::FindUserIcon(uint32 sock) const
{
	BAutolock lock(Window());
	register int32 count = fPointerList.CountItems();
	uint16 icon = 0;
	while(count > 0)
	{
		HUserItem *item = static_cast<HUserItem*>(fPointerList.ItemAt(--count));
		if(item->Sock() == sock)
		{
			icon = item->Icon();
			break;
		}
	}
	return icon;
}

/***********************************************************
 * MouseDown
 ***********************************************************/
void
HUserList::MouseDown(BPoint pos)
{
	int32 buttons = 0; 
	ResourceUtils utils;
	BPoint point = pos;
	
    Window()->CurrentMessage()->FindInt32("buttons", &buttons); 
    this->MakeFocus(true);
	
    // Right click
    if(buttons == B_SECONDARY_MOUSE_BUTTON)
    {
    	 int32 sel = IndexOf(pos);
    	 if(sel >= 0)
    	 	Select(sel);
    	 else
    	 	DeselectAll();
    	 sel = CurrentSelection();
    	 BPopUpMenu *theMenu = new BPopUpMenu("RIGHT_CLICK",false,false);
    	 BFont font(be_plain_font);
    	 font.SetSize(10);
    	 theMenu->SetFont(&font);
    	 IconMenuItem *aItem = new IconMenuItem(_("Send Message")
    	 				,new BMessage(MWIN_SEND_MESSAGE)
    	 				,0,0
    	 				,utils.GetBitmapResource('BBMP',"BMP:MESSAGE"));
         aItem->SetEnabled( (sel >= 0)?true:false);
         theMenu->AddItem(aItem);
         aItem = new IconMenuItem(_("Create Private Chat")
         					,new BMessage(MWIN_PRV_CREATE_MESSAGE)
         					,0,0
         					,utils.GetBitmapResource('BBMP',"BMP:PRVCHAT"));
         aItem->SetEnabled( (sel >= 0)?true:false);
         theMenu->AddItem(aItem);
         
         aItem = new IconMenuItem(_("Get User Info")
         					,new BMessage(MWIN_USER_INFO_MESSAGE)
         					,'I',0
         					,utils.GetBitmapResource('BBMP',"BMP:INFO"));
         aItem->SetEnabled( (sel >= 0)?true:false);
         theMenu->AddItem(aItem);
         theMenu->AddSeparatorItem();
         aItem = new IconMenuItem(_("Kick User")
         					,new BMessage(MWIN_KICK_USER)
         					,0,0
         					,utils.GetBitmapResource('BBMP',"BMP:KICK"));
         aItem->SetEnabled( (sel >= 0)?true:false);
         theMenu->AddItem(aItem);
         BRect r;
         ConvertToScreen(&pos);
         r.top = pos.y - 5;
         r.bottom = pos.y + 5;
         r.left = pos.x - 5;
         r.right = pos.x + 5;
         
    	BMenuItem *theItem = theMenu->Go(pos, false,true,r);  
    	if(theItem)
    	{
    	 	BMessage*	aMessage = theItem->Message();
			if(aMessage)
				this->Window()->PostMessage(aMessage);
	 	} 
	 	delete theMenu;
	 }else
	 	BListView::MouseDown(point);
}
