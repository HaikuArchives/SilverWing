#include <Autolock.h>
#include <ClassInfo.h>
#include <Debug.h>
#include <PopUpMenu.h>

#include "HFileWindow.h"
#include "HFileList.h"
#include "HFileItem.h"
#include "IconMenuItem.h"
#include "ResourceUtils.h"
#include "HApp.h"
#include "HPrefs.h"

/***********************************************************
 *
 ***********************************************************/
HFileList::HFileList(BRect rect, BetterScrollView **scroll
					,const char* title
					,bool IsSilverWing)
		:ColumnListView(rect,(CLVContainerView**)scroll,title,B_FOLLOW_ALL,
		B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE,
		B_SINGLE_SELECTION_LIST,true)
{
	
	float width;
	((HApp*)be_app)->Prefs()->GetData("file_column_width",&width);
	AddColumn(new CLVColumn(NULL,20.0,CLV_EXPANDER|CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE));
	AddColumn(new CLVColumn(NULL,20,CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE|
		CLV_NOT_RESIZABLE|CLV_PUSH_PASS|CLV_MERGE_WITH_RIGHT));
	AddColumn(new CLVColumn(_("Name"),width,CLV_SORT_KEYABLE|CLV_TELL_ITEMS_WIDTH));
	
	if( IsSilverWing ){
		AddColumn(new CLVColumn(_("Size"),70,CLV_SORT_KEYABLE|CLV_TELL_ITEMS_WIDTH));
		AddColumn(new CLVColumn(_("Modified"),130,CLV_SORT_KEYABLE|CLV_TELL_ITEMS_WIDTH));
	}else
		AddColumn(new CLVColumn(_("Size"),150,CLV_SORT_KEYABLE|CLV_TELL_ITEMS_WIDTH));
	SetSortFunction(HFileItem::CompareItems);
	SetSortKey(2);
	MakeFocus(true);
	fDraggedItemIndex = -1;
	fPointerList.MakeEmpty();
	rgb_color selection_col = {184,194, 255,255};
	SetItemSelectColor(true, selection_col);
	SetViewColor(tint_color( ui_color(B_PANEL_BACKGROUND_COLOR),B_LIGHTEN_2_TINT));
}

/***********************************************************
 *
 ***********************************************************/
HFileList::~HFileList()
{
}

/***********************************************************
 *
 ***********************************************************/
void
HFileList::MessageReceived(BMessage *message)
{
	_inherited::MessageReceived(message);
}

/***** For key navigation *********/
void
HFileList::MouseDown(BPoint pos)
{
	int32 buttons = 0;
	ResourceUtils utils;
	
	this->MakeFocus(true);
	Window()->CurrentMessage()->FindInt32("buttons", &buttons); 
	_inherited::MouseDown(pos);
	fDraggedItemIndex = CurrentSelection();	
	 // 右クリックのハンドリング 
    if(buttons == B_SECONDARY_MOUSE_BUTTON)
    {
    	 int32 sel = IndexOf(pos);
    	 if(sel >= 0)
    	 	Select(sel);
    	 else
    	 	DeselectAll();
    	 sel = CurrentSelection();
    	 fDraggedItemIndex = CurrentSelection();
    	 
    	 BPopUpMenu *theMenu = new BPopUpMenu("RIGHT_CLICK",false,false);
    	 BFont font(be_plain_font);
    	 font.SetSize(10);
    	 theMenu->SetFont(&font);
    	 IconMenuItem *aItem;
    	 bool enabled = false;
    	 if(sel >= 0){
    	 	HFileItem *item = cast_as(ItemAt(sel),HFileItem);
    	 	if(item->isFolder())
    	 		enabled = true;
    	 	else
    	 		enabled = false;
    	 }else
    	 	enabled = true;
    	 aItem = new IconMenuItem(_("Make folder")
    	 				,new BMessage(FILE_MAKE_FOLDER)
    	 				,0,0
    	 				,utils.GetBitmapResource('BBMP',"BMP:FOLDER"));
         aItem->SetEnabled( enabled );
         theMenu->AddItem(aItem);
         
         theMenu->AddSeparatorItem();
         aItem = new IconMenuItem(_("Upload")
    	 				,new BMessage(FILE_PUT_FILE_MSG)
    	 				,0,0
    	 				,utils.GetBitmapResource('BBMP',"BMP:UPLOAD"));
         aItem->SetEnabled( enabled );
         theMenu->AddItem(aItem);
    	 
    	 aItem = new IconMenuItem(_("Download")
    	 				,new BMessage(FILE_ITEM_CLICK_MSG)
    	 				,0,0
    	 				,utils.GetBitmapResource('BBMP',"BMP:DOWNLOAD"));
    	 if(sel > 0)
    	 {
         	aItem->SetEnabled(!enabled);
         }else
         	aItem->SetEnabled(false);
         theMenu->AddItem(aItem);
       
         aItem = new IconMenuItem(_("Get file Info")
         					,new BMessage(FILE_INFO_MESSAGE)
         					,0,0
         					,utils.GetBitmapResource('BBMP',"BMP:INFO"));
          if(sel > 0)
    	 {
         	aItem->SetEnabled(!enabled);
         }else
         	aItem->SetEnabled(false);
         theMenu->AddItem(aItem);
        
         theMenu->AddSeparatorItem();
         aItem = new IconMenuItem(_("Remove")
         					,new BMessage(FILE_DELETE_FILE)
         					,0,0
         					,utils.GetBitmapResource('BBMP',"BMP:TRASH"));
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

/***********************************************************
 *
 ***********************************************************/
void
HFileList::MouseMoved(BPoint point,uint32 transit,const BMessage *message)
{
	// if message is not NULL, maybe dragged.
	if(message != NULL && transit == B_INSIDE_VIEW)
	{
		//message->PrintToStream();
		entry_ref ref;
		if(message->FindRef("refs",&ref) == B_OK) // item is from tracker.
			FindItem(point);
		else{ // move files.
			FindItem(point);
		}
	}else{
		if(fDraggedItemIndex >= 0)
		Select(fDraggedItemIndex);
	}
	_inherited::MouseMoved(point,transit,message);
}

/***********************************************************
 *
 ***********************************************************/
bool
HFileList::InitiateDrag (
	BPoint  point , int32 index, bool wasSelected)
{	
	fDraggedItemIndex = index;
	if (wasSelected) 
	{
		BMessage		theMessage(B_SIMPLE_DATA);
		BRect			theRect = this->ItemFrame(index);

		HFileItem* item = cast_as(this->ItemAt(index),HFileItem);
		/////// For Tracker --------------------
		if(item == NULL)
			return false;
		theMessage.AddString("be:types", B_FILE_MIME_TYPE);
		BString mime = "application/octet-stream";
		theMessage.AddString("be:types", mime.String());
		theMessage.AddString("be:filetypes", mime.String());
		theMessage.AddInt32("dragged_item",item->ItemIndex());
		theMessage.AddInt32("list_index",index);
	
		theRect.left += 5;
		theRect.right = theRect.left + StringWidth(item->DecodedName()) + 30;
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
			view->DrawBitmap(item->Bitmap(),BPoint(0,2));
		
		font_height FontAttributes;
		BFont font;
		GetFont(&font);
		font.GetHeight(&FontAttributes);
		float FontHeight = ceil(FontAttributes.ascent) + ceil(FontAttributes.descent);
		
		view->MovePenTo(theRect.left+20, theRect.top+FontHeight);
		if(item->DecodedName())
		view->DrawString(item->DecodedName());
		bitmap->Unlock();
		
		DragMessage(&theMessage, bitmap, B_OP_ALPHA
				,BPoint(bitmap->Bounds().Width()/2,bitmap->Bounds().Height()/2));
	}
	return (wasSelected);
}

/***********************************************************
 *
 ***********************************************************/
void
HFileList::RemoveAll()
{
	BAutolock lock(Window());
	
	this->SetFlags(0);
	if(lock.IsLocked())
	{
		register int32 count = fPointerList.CountItems();
		
		this->MakeEmpty();	
		while(count >0)
		{
			delete (HFileItem*)fPointerList.RemoveItem(--count);
		}
	}
	this->SetFlags(B_WILL_DRAW);
}

/***********************************************************
 * AddFileItem
 ***********************************************************/
void
HFileList::AddFileItem(HFileItem* item)
{
	fPointerList.AddItem(item);
	this->AddItem(item);
}

/***********************************************************
 *
 ***********************************************************/
void
HFileList::AddFileItemUnder(HFileItem *item ,HFileItem *parent)
{
	fPointerList.AddItem(item);
	this->AddUnder(item,parent);
}

/***********************************************************
 *
 ***********************************************************/
void
HFileList::RemoveFileItem(HFileItem *item)
{
	fPointerList.RemoveItem(item);
	RemoveItem(item);
	delete item;
}

/***********************************************************
 *
 ***********************************************************/
void
HFileList::Expand(CLVListItem* item)
{
	RemoveChildItems(item);
	//int32 index = FullListIndexOf(item);
	HFileItem *file_item = cast_as( item,HFileItem );
	if(file_item == NULL)
		return;
	((HFileWindow*)Window())->GetSubItems(file_item);
	_inherited::Expand(item);	
	DeselectAll();
	fDraggedItemIndex = -1;
}

/***********************************************************
 *
 ***********************************************************/
void
HFileList::Collapse(CLVListItem* item)
{
	_inherited::Collapse(item);
	DeselectAll();
	fDraggedItemIndex = -1;
}

/***********************************************************
 *
 ***********************************************************/
int32
HFileList::RemoveChildItems(CLVListItem* item)
{
	BAutolock lock(Window());
	int32 index = this->FullListIndexOf(item);
	int32 num = this->FullListNumberOfSubitems(item);
	
	for(register int i = 0;i< num;i++)
	{
		HFileItem *del = cast_as(FullListItemAt(index+1),HFileItem);
		PRINT(( "Del:%s\n", del->Name() ));
		if(del == NULL)
			continue;
		if(del->isFolder())
			i += RemoveChildItems(del);
		RemoveFileItem(del);					
	}
	return num;
}

/***********************************************************
 *
 ***********************************************************/
HFileItem*
HFileList::FindItem(uint32 index)
{
	BAutolock lock(Window());
	HFileItem *result = NULL;
	
	int32 count = this->FullListCountItems();
	for(register int32 i = 0;i < count;i++)
	{
		HFileItem* item = cast_as( FullListItemAt(i),HFileItem );
		if(item == NULL)
			continue;
		if(item->ItemIndex() == index)
		{
			result = item;
			break;
		}
	}
	return result;
}

/***********************************************************
 *
 ***********************************************************/
HFileItem*
HFileList::FindItem(const BPoint point)
{
	BAutolock lock(Window());
	int32 count = FullListCountItems();
	HFileItem *result = NULL;
	for(register int32 i = 0;i < count;i++)
	{
		BRect rect = ItemFrame(i);
		//rect.PrintToStream();
		//Bounds().PrintToStream();
		if( rect.Contains(point) )
		{
			HFileItem *item = cast_as( FullListItemAt(i), HFileItem);
			if(item != NULL)
			{
				result = item;
				if(item->isFolder())
					Select(i);
				else{
					if(i == fDraggedItemIndex)
						Select(i);
					else
						DeselectAll();
					
				}
			}
			break;
		}
	}

	return result;
}

/***********************************************************
 * 
 ***********************************************************/
void
HFileList::GetItemPath(HFileItem *item,BString &path)
{
	BAutolock lock(Window());
	HFileItem *parent = cast_as(Superitem(item),HFileItem);
	path = item->Name();
	
	while(parent != NULL)
	{
		path.Insert(dir_char,1,0);
		path.Insert(parent->Name(),0);
		HFileItem *parent2 = cast_as(Superitem(parent),HFileItem);
		parent = parent2;
	}
}

/***********************************************************
 *
 ***********************************************************/
bool
HFileList::FindSameItem(const char* name,HFileItem *parent)
{
	bool rc = false;
	BAutolock lock(Window());
	BString tmp_name = name;
	tmp_name += ".hpf";

	if(parent != NULL)
	{
		int32 index = FullListIndexOf(parent);
		int32 num = FullListNumberOfSubitems(parent);

		for(register int32 i = index+1;i< index+num+1;i++)
		{
			HFileItem* item = cast_as( ItemAt(i),HFileItem );
			if(item == NULL)
				continue;
			if( strcmp(item->DecodedName(),name) ==0 || strcmp(item->DecodedName(),tmp_name.String()) ==0 )
			{
				rc = true;
				break;
			}
		}
	}else{// root folder.
		int32 count = FullListCountItems();	
		for(register int32 i = 0;i < count;i++)
		{
			HFileItem* item = cast_as( FullListItemAt(i),HFileItem );
			if(item == NULL)
				continue;
			if(item->OutlineLevel() == 0)
			{
				if( strcmp(item->DecodedName(),name) ==0 || strcmp(item->DecodedName(),tmp_name.String()) ==0 )
				{
					rc = true;
					break;
				}
			}
		}
	}
	return rc;
}

/***********************************************************
 * FindSameItem
 ***********************************************************/
bool
HFileList::FindSameItem(const char* name)
{
	int32 count = CountItems();
	bool result = false;
	
	for(register int32 i = 0; i < count;i++)
	{
		HFileItem *item = cast_as(ItemAt(i),HFileItem);
		if(item)
		{
			if(::strcmp(name,item->DecodedName()) == 0)
			{
				result = true;
				break;
			}
		}
	}
	return result;
}
