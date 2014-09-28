#include <ClassInfo.h>
#include <TextControl.h>
#include <StringView.h>
#include <Button.h>
#include <ScrollView.h>

#include "HUserSettingView.h"
#include "HUserList.h"
#include "HUserItem.h"
#include "HotlineClient.h"
#include "AppUtils.h"
#include "RectUtils.h"
#include "HIconView.h"
#include "HProgressWindow.h"

#include "HApp.h"
#include "hldat.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HUserSettingView::HUserSettingView(BRect rect)
	:BView(rect,"usersetting",B_FOLLOW_ALL,B_WILL_DRAW)
{
	this->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	InitGUI();
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HUserSettingView::~HUserSettingView()
{
	RemoveAll();
	be_app->PostMessage(M_RESET_RESOURCE);
	fUserList->SetSelectionMessage(NULL);
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HUserSettingView::InitGUI()
{
	BRect rect = Bounds();
	rect.top += 10;
	rect.left += 10;
	rect.right -= 10;
	rect.bottom = rect.top + 15;
	BRect textRect =rect;
	textRect.right= textRect.left + 200;
	BTextControl *control = new BTextControl(textRect,"nick",_("Nickname:"),"",NULL);
	control->SetDivider(this->StringWidth(_("Nickname:"))+2);
	this->AddChild(control);
	rect.OffsetBy(0,30);
	//control = new BTextControl(rect,"icon","Icon:","",NULL);
	//control->SetDivider(this->StringWidth("Nickname:")+2);
	BRect labelRect = rect;
	labelRect.right = labelRect.left + this->StringWidth(_("Nickname:"))+4;
	BStringView *stringview = new BStringView(labelRect,"label",_("Icon:"));
	stringview->SetAlignment(B_ALIGN_RIGHT);
	this->AddChild(stringview);
	rect.left += this->StringWidth(_("Nickname:"))+4+4;
	HIconView *iconview = new HIconView(rect,((HApp*)be_app)->Icon());
	this->AddChild(iconview);
	BRect btnrect = Bounds();
	btnrect.left += 10;
	btnrect.right -= 10;
	btnrect.top += rect.bottom + 8;
	btnrect.bottom = btnrect.top + 3;
	BButton *gatherBtn = new BButton(btnrect,"gatherbtn",_("Show all icons"),new BMessage(M_GATHER_ICON_MSG));
	gatherBtn->SetTarget(this);
	this->AddChild(gatherBtn);
	BRect listrect = Bounds();
	listrect.left += 10;
	listrect.right -= 10 + B_V_SCROLL_BAR_WIDTH;
	listrect.bottom -= 30;
	listrect.top = rect.bottom + 38;
	fUserList = new HUserList(listrect,"userlist");
	BScrollView *scrollView = new BScrollView("scrollview",fUserList,B_FOLLOW_LEFT|B_FOLLOW_BOTTOM,
													B_WILL_DRAW|B_FRAME_EVENTS,false,true,B_FANCY_BORDER);
	fUserList->SetSelectionMessage(new BMessage(M_LIST_CLICKED));
	this->AddChild(scrollView);
}

/***********************************************************
 * Set nick name.
 ***********************************************************/
void
HUserSettingView::SetNick(const char* nick)
{
	BTextControl *control = cast_as(FindView("nick"),BTextControl);
	control->SetText(nick);
}

/***********************************************************
 * Gather all icons ( call from thread)
 ***********************************************************/
void
HUserSettingView::GatherAllIcons()
{
	fProgressWin = new HProgressWindow( RectUtils().CenterRect(300,40),_("Wait…"));
	fProgressWin->Show();
	if( !((HApp*)be_app)->IsBadmoon() )
	{
		size_t outsize;
		BResources *res = ((HApp*)be_app)->IconResource();
		const void* da = res->LoadResource(B_INT32_TYPE,"numicons",&outsize);
		if(da == NULL)
			return;
		int32 max_value;
		::memcpy(&max_value,da,4);
		BMessage msg(M_SET_MAX_VALUE);
		msg.AddInt32("value",max_value);
		fProgressWin->PostMessage(&msg);
		
	
		for(register int i = 0;i < max_value;i++)
		{
			int32 id;
			size_t length;
				const char* name;
			
			if(res->GetResourceInfo(B_RAW_TYPE,i,&id,&name,&length))
			{
				BMessage addmsg(M_ADD_ICON_ITEM);
				addmsg.AddInt16("icon",id);
				Window()->PostMessage(&addmsg,this);
			}else
				break;
		}
		
	}else{
		int32 count = ((HApp*)be_app)->HotlineDat()->GetItemCount();
		
		BMessage msg(M_SET_MAX_VALUE);
		msg.AddInt32("value",count);
		fProgressWin->PostMessage(&msg);
		
		
		int32 i = 0;
		int32 k = 0;
		for(;;)
		{
			struct Icon *ic;
			ic = ((HApp*)be_app)->HotlineDat()->get_icon(i);
			if( ic )
			{
				//if(ic->width >= 50)
				//{
					BMessage addmsg(M_ADD_ICON_ITEM);
					addmsg.AddInt16("icon",ic->number);
					Window()->PostMessage(&addmsg,this);
				//}
				k++;		
			}
			i++;
			if(count <= k)
				break;		
		}
	}	
	//Window()->PostMessage(M_END_SEARCH,this);
	fProgressWin->PostMessage(B_QUIT_REQUESTED);
}

/***********************************************************
 * Thread function.
 ***********************************************************/
int32
HUserSettingView::ThreadFunc(void* data)
{
	HUserSettingView* view = (HUserSettingView*)data;
	view->GatherAllIcons();
	return 0;
}

/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
HUserSettingView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case M_GATHER_ICON_MSG:
	{
		this->RemoveAll();
		thread_id thread = ::spawn_thread(ThreadFunc,"IconGatherThread",B_NORMAL_PRIORITY,this);
		::resume_thread(thread);
		break;
	}
	case M_LIST_CLICKED:
	{
		int32 sel = fUserList->CurrentSelection();
		if(sel >= 0)
		{
			HUserItem *item = cast_as(fUserList->ItemAt(sel),HUserItem);
			this->SetIcon(item->Icon());
		}
		break;
	}
	case M_ADD_ICON_ITEM:
	{
		int32 count;
		type_code type;
		message->GetInfo("icon",&type,&count);
		BList list;
		//BMessage msg(M_RESET_MSG);
		//msg.AddString("label","Creating list...");
		//fProgressWin->PostMessage(&msg);
		//msg.MakeEmpty(); 
		//msg.what = M_SET_MAX_VALUE;
		//msg.AddInt32("value",count);
		//fProgressWin->PostMessage(&msg);
		
		for(register int i=0;i<count;i++)
		{
			uint16 icon;
			message->FindInt16("icon",i,(int16*)&icon);
			//list.AddItem(new HUserItem(0,icon,0,""));
			fUserList->AddUserItem(new HUserItem(0,icon,0,""));
			fProgressWin->PostMessage(M_UPDATE_MSG);
		}
		//fProgressWin->PostMessage(B_QUIT_REQUESTED);
		break;
	}	
	case M_END_SEARCH:
	{
		fProgressWin->PostMessage(B_QUIT_REQUESTED);
		break;
	}
	case M_UPDATE_MSG:
	case M_SET_MAX_VALUE:
	{
		break;
	}
	default:
		BView::MessageReceived(message);
	}
}

/***********************************************************
 * Set user icon.
 ***********************************************************/
void
HUserSettingView::SetIcon(int32 icon)
{
	HIconView* view = cast_as(FindView("iconview"),HIconView);
	view->SetIcon(icon);
}

/***********************************************************
 * Return nick.
 ***********************************************************/
const char*
HUserSettingView::Nick()
{
	BTextControl *control = cast_as(FindView("nick"),BTextControl);
	return control->Text();
}

/***********************************************************
 * Remove all icon list items.
 ***********************************************************/
void
HUserSettingView::RemoveAll()
{
	fUserList->RemoveAll();
}

/***********************************************************
 * Return user icon.
 ***********************************************************/
uint32
HUserSettingView::Icon()
{
	HIconView *iconview = cast_as(FindView("iconview"),HIconView);
	return iconview->Icon();
}