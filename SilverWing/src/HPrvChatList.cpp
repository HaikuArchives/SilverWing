#include <Application.h>
#include <PopUpMenu.h>

#include "HPrvChatList.h"
#include "HUserItem.h"
#include "HPrvChatWindow.h"
#include "HUserList.h"
#include "ClassInfo.h"
#include "ResourceUtils.h"
#include "IconMenuItem.h"
#include "HApp.h"

/***********************************************************
 * Constructor
 ***********************************************************/
HPrvChatList::HPrvChatList(BRect frame,const char* title)
		:BListView(frame,title,B_SINGLE_SELECTION_LIST,
							B_FOLLOW_ALL,B_WILL_DRAW)
{
	this->MakeEmpty();
	SetTarget(Window());
}

/***********************************************************
 * Destructor
 ***********************************************************/
HPrvChatList::~HPrvChatList()
{
	int count = this->CountItems();
	for(int i = 0;i < count;i++)
	{
		HUserItem *item = cast_as(this->RemoveItem((long)0),HUserItem);
		delete item;
	}
}

/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
HPrvChatList:: MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	default:
		if(message->WasDropped())
		{
			this->WhenDropped(message);
		}else
		{
			this->BListView::MessageReceived(message);
		}
	}
}

/***********************************************************
 * InititateDrag
 ***********************************************************/
bool
HPrvChatList::InitiateDrag (
	BPoint /* point */, int32 index, bool wasSelected)
{
	if (wasSelected) {
		BMessage msg(USERLIST_USER_DRAG_MSG);
		HUserItem *item = cast_as(ItemAt(index),HUserItem);
		BRect	theRect = this->ItemFrame(index);
		msg.AddInt32("sock",(int32)item->Sock());
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

		DragMessage(&msg, bitmap, B_OP_ALPHA,
				BPoint(bitmap->Bounds().Width()/2,bitmap->Bounds().Height()/2));
		DragMessage(&msg, theRect);
	}	
	return (wasSelected);
}

/***********************************************************
 * When dropped
 ***********************************************************/
void
HPrvChatList::WhenDropped(BMessage *message)
{		
	switch(message->what)
	{
		case USERLIST_USER_DRAG_MSG:
		{
			uint32 sock;
			message->FindInt32("sock",(int32*)&sock);
			BMessage msg(PRVLIST_CHAR_INVITE);
			msg.AddInt32("sock",(int32)sock);
			msg.AddInt32("pcref",(int32)((HPrvChatWindow*)Window())->Pcref());
			be_app->PostMessage(&msg);
			break;
		}	
	}
}

/***********************************************************
 * MouseMoved
 ***********************************************************/
void
HPrvChatList::MouseMoved(BPoint point,uint32 transit,const BMessage *message)
{
	// if message is not NULL, maybe dragged.
	if(message != NULL && transit == B_INSIDE_VIEW)
	{
		void *data;
		if( message->FindPointer("old_window",&data) == B_OK)
		{
			BWindow *old = static_cast<BWindow*>(data);
			if(old->IsActive())
			{
				old->Activate(false);
				old->SendBehind(Window());
			}
			if(!Window()->IsActive())
				Window()->Activate(true);
			this->MakeFocus(true);
		}
	}else if(message != NULL && transit == B_OUTSIDE_VIEW){
		this->MakeFocus(false);
	}
	BListView::MouseMoved(point,transit,message);
}

/***********************************************************
 * MouseDown
 ***********************************************************/
void
HPrvChatList::MouseDown(BPoint pos)
{
	int32 buttons = 0; 
	ResourceUtils utils;
	BPoint point = pos;
	
    Window()->CurrentMessage()->FindInt32("buttons", &buttons); 
    this->MakeFocus(true);
	BListView::MouseDown(point);
    // 右クリックのハンドリング 
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
    	 				,new BMessage(PRVCHAT_SEND_MESSAGE)
    	 				,0,0
    	 				,utils.GetBitmapResource('BBMP',"BMP:MESSAGE"));
         aItem->SetEnabled( (sel >= 0)?true:false);
         theMenu->AddItem(aItem);
       
         aItem = new IconMenuItem(_("Get User Info")
         					,new BMessage(PRVCHAT_GET_INFO)
         					,'I',0
         					,utils.GetBitmapResource('BBMP',"BMP:INFO"));
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
	 }	
}
