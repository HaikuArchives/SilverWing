#include "HWindow.h"
#include <santa/BetterScrollView.h>
#include "RectUtils.h"
#include "HToolbar.h"
#include "HListView.h"
#include "HTrackerItem.h"
#include "hl_magic.h"
#include "AppUtils.h"
#include "HAddTrackerWindow.h"
#include "MenuUtils.h"
#include "HConnectWindow.h"
#include "HFindWindow.h"
#include "ResourceUtils.h"
#include "HToolbarButton.h"
#include "MAlert.h"
#include "HApp.h"

#include <ClassInfo.h>
#include <MenuBar.h>
#include <Roster.h>
#include <TextControl.h>
#include <Beep.h>
#include <Directory.h>
#include <Debug.h>

/***********************************************************
 * Constructor.
 ***********************************************************/
HWindow::HWindow(BRect rect,const char* name)
			:BWindow(rect,name,B_DOCUMENT_WINDOW,B_ASYNCHRONOUS_CONTROLS)
{
	SetPulseRate(1.5*1000000);
	InitMenu();
	InitGUI();
	fSearchThread = -1;
	fOldServerCount = 0;
	fStatusString = _("idle");
	fSearchText = "";
	fSearchIndex = 0;
	fAlive = false;
	AddShortcut('/',0,new BMessage(B_ZOOM));

	float min_width,min_height,max_width,max_height;
	GetSizeLimits(&min_width,&max_width,&min_height,&max_height);
	min_width = 200;
	min_height = 150;
	SetSizeLimits(min_width,max_width,min_height,max_height);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HWindow::~HWindow()
{
	HListView *listview = cast_as(FindView("listview"),HListView);
	listview->SetInvocationMessage(NULL);
}


/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HWindow::InitGUI()
{
	/******* Toolbarの追加 *********/
	BRect rect = this->Bounds();
	rect.OffsetTo(B_ORIGIN);
	rect.top += (KeyMenuBar()->Bounds()).Height();
	rect.bottom = rect.top + 33;
	rect.left -= 1;
	rect.right += 1;
	BResources *rsrc = BApplication::AppResources();
	ResourceUtils utils(rsrc);
	HToolbar* toolbar = new HToolbar(rect,B_FOLLOW_LEFT_RIGHT);
	toolbar->AddButton("connectbtn",utils.GetBitmapResource('BBMP',"BMP:CONNECT"),new BMessage(M_CONNECT_MSG),_("Connect"));
	toolbar->AddButton("trackerbtn",utils.GetBitmapResource('BBMP',"BMP:ADDTRACKER"),new BMessage(M_ADD_NEW_TRACKER),_("Add New Tracker"));
	toolbar->AddButton("refreshbtn",utils.GetBitmapResource('BBMP',"BMP:REFRESH"),new BMessage(M_REFRESH_MSG),_("Refresh"));
	toolbar->AddSpace();
	toolbar->AddButton("delete",utils.GetBitmapResource('BBMP',"BMP:TRASH"),new BMessage(M_DEL_TRACKER),_("Delete Tracker"));
	toolbar->AddSpace();
	toolbar->AddButton("search",utils.GetBitmapResource('BBMP',"BMP:SEARCH"),new BMessage(M_FIND_MSG),_("Search Server"));
	this->AddChild(toolbar);
	/********* Filter **************/
	rect.Set(0,0,150,20);
	rect.OffsetBy(150,6);
	BTextControl *control = new BTextControl(rect,"filter",_("Filter:"),"",new BMessage(M_REFRESH_FILTER));
	control->SetDivider(40);
	toolbar->AddChild(control);
	/********* ListViewの追加 *********/
	rect = this->Bounds();
	rect.top += (KeyMenuBar()->Bounds()).Height();
	rect.top += 31;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT*2;
	rect.right -= B_V_SCROLL_BAR_WIDTH;
	BetterScrollView *scroller;
	HListView *listview = new HListView(rect,(CLVContainerView**)&scroller,"listview");
	listview->SetInvocationMessage(new BMessage(M_CLICK_LIST));
//	BScrollBar *scrollbar = scrollview->ScrollBar(B_VERTICAL);
	//scrollbar->ResizeBy(0,B_V_SCROLL_BAR_WIDTH);
	this->AddChild(scroller);
	scroller->SetResizingMode(B_FOLLOW_ALL);
	/******** Statusの追加 *********/
	rect = this->Bounds();
	rect.bottom += 2;
	rect.left -=1;
	rect.top = rect.bottom -15;
	rect.right -= 12;
	BBox *box = new BBox(rect,"",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	rect.OffsetTo(B_ORIGIN);
	rect.top += 1;
	rect.bottom -= 1;
	rect.left += 5;
	rect.right -= 2;
	BString label = "0 ";
	label << _("servers") << "  ( " << _("idle") << " )";
	BStringView *stringview = new BStringView(rect,"status",label.String());
	box->AddChild(stringview);
	this->AddChild(box);
	this->FindTrackers();
}


/***********************************************************
 * Set up menus
 ***********************************************************/
void
HWindow::InitMenu()
{
	BRect frame;
	frame = Bounds();
	frame.bottom = frame.top + 10;
	frame.OffsetTo(B_ORIGIN);
	BMenuBar *menuBar = new BMenuBar(frame,"MENUBAR");
	BMenu* aMenu;
    MenuUtils utils;
    BResources *rsrc = BApplication::AppResources();
    ResourceUtils rutils(rsrc);
	//------------------------ File Menu
    aMenu = new BMenu(_("File"));
    	utils.AddMenuItem(aMenu,_("Add New Tracker…"),M_ADD_NEW_TRACKER,this,this,'A',0,
    			rutils.GetBitmapResource('BBMP',"BMP:ADDTRACKER"));
    	aMenu->AddSeparatorItem();
    	utils.AddMenuItem(aMenu,_("Connect"),M_CLICK_LIST,this,this,0,0,
    			rutils.GetBitmapResource('BBMP',"BMP:CONNECT"));
    	utils.AddMenuItem(aMenu,_("Connect with New Window"),M_CONNECT_NEW_WINDOW,this,this,'N',0);
  		utils.AddMenuItem(aMenu,_("Refresh All"),M_REFRESH_MSG,this,this,0,0,
  			rutils.GetBitmapResource('BBMP',"BMP:REFRESH"));
  		aMenu->AddSeparatorItem();
  		utils.AddMenuItem(aMenu,_("Delete Tracker"),M_DEL_TRACKER,this,this,0,0,
  			rutils.GetBitmapResource('BBMP',"BMP:TRASH"));

  	 	aMenu->AddSeparatorItem();
        utils.AddMenuItem(aMenu,_("About…"),B_ABOUT_REQUESTED,be_app,be_app);
  		aMenu->AddSeparatorItem();
    	utils.AddMenuItem(aMenu,_("Quit"),B_QUIT_REQUESTED,this,this,'Q',0);

    menuBar->AddItem( aMenu );
    aMenu = new BMenu(_("Edit"));
    	utils.AddMenuItem(aMenu,_("Find…"),M_FIND_MSG,this,this,'F',0,
    		rutils.GetBitmapResource('BBMP',"BMP:SEARCH"));
    	utils.AddMenuItem(aMenu,_("Find Next"),M_FIND_NEXT_MSG,this,this,'G',0);
    menuBar->AddItem( aMenu );
    this->AddChild(menuBar);
}

/***********************************************************
 * MenusBeginning
 ***********************************************************/
void
HWindow::MenusBeginning(void)
{
	HListView *view = cast_as(FindView("listview"),HListView);
	int32 sel = view->CurrentSelection();
	bool enable = false;
	if(sel >= 0)
	{
		HTrackerItem *tItem = cast_as(view->ItemAt(sel),HTrackerItem);
		if(!tItem->isTracker())
			enable = true;
	}
	BMenu *menu = this->KeyMenuBar()->SubmenuAt(0);
	BMenuItem *item = menu->FindItem(M_CLICK_LIST);
	item->SetEnabled(enable);
	item = menu->FindItem(M_CONNECT_NEW_WINDOW);
	item->SetEnabled(enable);
	item = menu->FindItem(M_DEL_TRACKER);
	if(sel < 0)
		item->SetEnabled(false);
	else
		item->SetEnabled(!enable);

	if(fSearchText.Length() == 0)
	{
		KeyMenuBar()->FindItem(M_FIND_NEXT_MSG)->SetEnabled(false);
	}else{
		KeyMenuBar()->FindItem(M_FIND_NEXT_MSG)->SetEnabled(true);
	}
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	/****** 新しいトラッカーの追加 ***********/
	case M_ADD_NEW_TRACKER:
	{
		HAddTrackerWindow* win = new HAddTrackerWindow( RectUtils().CenterRect(250,100),"Add Tracker");
		win->Show();
		break;
	}
	/****** トラッカーの削除 *********/
	case M_DEL_TRACKER:
	{
		HListView *view = cast_as(FindView("listview"),HListView);
		int32 sel = view->CurrentSelection();
		if(sel >= 0)
		{
			HTrackerItem *item = cast_as(view->ItemAt(sel),HTrackerItem);
			if( item->isTracker() )
			{
				BString title;
				title << _("Would you like to delete this tracker") << " "<< item->Name() <<".";
				int32 n = (new MAlert(_("Caution"),title.String(),_("OK"),_("Cancel"),NULL,B_IDEA_ALERT))->Go();
				if(n == 0)
				{
					this->DeleteTracker(item->Name());
					int32 index = view->FullListIndexOf(item);
					int32 num = view->FullListNumberOfSubitems(item);
					for(int32 i = 0;i< num;i++)
					{
						//HTrackerItem *del = cast_as(view->RemoveItem((long)index+1),HTrackerItem);
						//delete del;
						HTrackerItem *del = cast_as(view->ItemAt((long)index+1),HTrackerItem);
						view->RemoveServer(del);
					}
					view->RemoveServer(item);
				}
			}
		}
		break;
	}
	/******* Serverの受信 *********/
	case H_RECEIVE_SERVER:
	{
		const char* name = message->FindString("name");
		const char* address = message->FindString("address");
		const char* desc = message->FindString("desc");
		uint16 users = message->FindInt16("users");
		uint16 port = message->FindInt16("port");
		HListView *view = cast_as(FindView("listview"),HListView);
		HTrackerItem *item;
		PRINT(( "Name: %s\n", name ));
		message->FindPointer("parent",(void**)&item);
		if(view)
		//	view->AddServerUnder(new HTrackerItem(name,address,port,users,desc),item);
			view->Add(new HTrackerItem(name,address,port,users,desc,false,item));
		break;
	}
	case H_END_SEARCH:
	{
		HListView *view = cast_as(FindView("listview"),HListView);
		if(view != NULL)
			view->SortItems();
		break;
	}
	/******* 検索の開始 *********/
	case M_REFRESH_MSG:
	{
		HListView *view = cast_as(FindView("listview"),HListView);
		if(view != NULL)
		{
			HTrackerItem **items = (HTrackerItem**)view->Items();
			for(register int32 i = 0;i < view->CountItems();i++)
			{
				if( (*(items+i))->isTracker() )
				{
					view->Collapse( *(items+i) );
					view->Expand( *(items+i) );
				}
			}
		}
		break;
	}
	/******* FilterのRefresh *********/
	case M_REFRESH_FILTER:
	{
		HListView* view = cast_as(FindView("listview"),HListView);
		view->Draw(view->Bounds());
		BTextControl *control = cast_as(FindView("filter"),BTextControl);
		if(strcmp( control->Text(),view->Keyword()) != 0)
		{
			view->StartWatching(false);
			view->SetKeyword(control->Text());
			view->EmptyQueue();
			view->RemoveAllChildren();
			view->StartWatching(true);
		}
		break;
	}
	/******* Set Status *********/
	case M_SET_STATUS:
	{
		const char* text = message->FindString("text");

		fStatusString = text;
		break;
	}
	/******** ListView's double click *********/
	case M_CLICK_LIST:
	{
		HListView *view = cast_as(FindView("listview"),HListView);
		int32 selection = view->CurrentSelection();
		if(selection >= 0)
		{
			HTrackerItem *item = cast_as(view->ItemAt(selection),HTrackerItem);
			if(item->isTracker() == false)
			{
			BMessage msg(B_REFS_RECEIVED);
			msg.AddString("login","");
			msg.AddString("password","");
			msg.AddString("address",item->Address());
			msg.AddInt16("port",(int16)atoi(item->Port()));
			entry_ref app;
			if(be_roster->IsRunning("application/x-vnd.takamatsu-silverwing"))
			{
				team_id id = be_roster->TeamFor("application/x-vnd.takamatsu-silverwing");
				BMessenger messenger("application/x-vnd.takamatsu-silverwing",id);
				messenger.SendMessage(&msg);
			}else{
				be_roster->Launch("application/x-vnd.takamatsu-silverwing",&msg);
			}
			}
		}
		break;
	}
	/*********** Connect with new window *************/
	case M_CONNECT_NEW_WINDOW:
	{
		HListView *view = cast_as(FindView("listview"),HListView);
		int32 selection = view->CurrentSelection();
		if(selection >= 0)
		{
			HTrackerItem *item = cast_as(view->ItemAt(selection),HTrackerItem);
			if(item->isTracker() == false)
			{
			BMessage msg(B_REFS_RECEIVED);
			msg.AddString("login","");
			msg.AddString("password","");
			msg.AddString("address",item->Address());
			msg.AddInt16("port",(int16)atoi(item->Port()));

			be_roster->Launch("application/x-vnd.takamatsu-silverwing",&msg);

			}
		}
		break;
	}
	/********* Connect **********/
	case M_CONNECT_MSG:
	{
		HListView *view = cast_as(FindView("listview"),HListView);
		int32 selection = view->CurrentSelection();
		if(selection >= 0)
		{
			HTrackerItem *item = cast_as(view->ItemAt(selection),HTrackerItem);
			if(item->isTracker() == true)
				break;
			HConnectWindow* win = new HConnectWindow((new RectUtils())->CenterRect(300,180),
									_("Connect"),false);
			win->SetAddress(item->Address());
			win->SetPort(item->Port());
			win->Show();
		}
		break;
	}
	/********* New Tracker ***********/
	case M_NEW_TRACKER:
	{
		const char* address = message->FindString("address");
		const char* name = message->FindString("name");
		HListView *view = (HListView*)FindView("listview");
		if(view != NULL)
			view->AddServer(new HTrackerItem(name,address,HTRK_TCPPORT,0,"",true));
		break;
	}
	/********* Find message **********/
	case M_FIND_MSG:
	{
		RectUtils utils;
		HFindWindow *findWin = new HFindWindow( utils.CenterRect(250,70));
		findWin->SetTarget(this);
		if(fSearchText.Length() != 0)
			findWin->SetSearchText( fSearchText.String() );
		findWin->Show();
		break;
	}
	/********* Find **************/
	case M_SEARCH_MSG:
	{
		const char* text = message->FindString("text");
		fSearchText = text;
		if(strlen(text) != 0)
			this->Search(text,0);
		break;
	}
	/******** Find next ***********/
	case M_FIND_NEXT_MSG:
	{
		this->Search(fSearchText.String(),fSearchIndex);
		break;
	}
	case 'MNOT':
	{
		BPoint drop_point = message->FindPoint("_old_drop_point_");
		BRect rect = this->Frame();
		if(rect.Contains(drop_point) == true)
			break;
		BMessage msg,reply;
		//message->PrintToStream();
		msg.what = B_GET_PROPERTY;
		msg.AddSpecifier("Path");
		if(message->SendReply(&msg,&reply) != B_OK)
			break;
		entry_ref ref;
		if(reply.FindRef("result",&ref) == B_OK)
		{
			BEntry entry(&ref);
			BPath path(&entry);
			HListView *listview = cast_as(FindView("listview"),HListView);
			HTrackerItem **item = (HTrackerItem**)listview->Items();
			for(register int i =0;i<listview->CountItems();i++)
			{
				if(listview->IsItemSelected(i) == true)
				{
					if( (*(item+i))->isTracker() == false)
					{
						BString name = (*(item+i))->Name();

						path.Append(name.String());

						BFile file(path.Path(),B_READ_WRITE|B_CREATE_FILE);
						if(file.InitCheck() == B_OK)
						{
							BMessage msg;
							msg.AddString("address",(*(item+i))->Address());
							msg.AddString("login","");
							msg.AddString("password","");
							msg.AddInt16("port",atoi( (*(item+i))->Port()));
							msg.Flatten(&file);
							BNodeInfo ninfo(&file);
							ninfo.SetType("text/hotline-bookmarks");
						}
						path.GetParent(&path);
					}
				}
			}
		}// end if
		break;
	}
	/******* ToolbarのUpdate ********/
	case M_UPDATE_TOOLBUTTON:
	{
		const char* name = message->FindString("name");
		void *data;
		message->FindPointer("pointer",&data);
		HToolbarButton *btn = static_cast<HToolbarButton*>(data);
		if( ::strcmp(name,"delete") == 0)
		{
			HListView *view = cast_as(FindView("listview"),HListView);
			int32 sel = view->CurrentSelection();
			if(sel >= 0)
			{
				HTrackerItem *item = cast_as(view->ItemAt(sel),HTrackerItem);
				if( item->isTracker() )
					btn->SetEnabled(true);
				else
					btn->SetEnabled(false);
			}else
				btn->SetEnabled(false);
		}else if(::strcmp(name,"connectbtn") == 0){
			HListView *view = cast_as(FindView("listview"),HListView);
			int32 sel = view->CurrentSelection();
			if(sel >= 0)
			{
				HTrackerItem *item = cast_as(view->ItemAt(sel),HTrackerItem);
				if( item->isTracker() )
					btn->SetEnabled(false);
				else
					btn->SetEnabled(true);
			}else
				btn->SetEnabled(false);
		}
		break;
	}
	default:
		BWindow::MessageReceived(message);
	}
}

/***********************************************************
 * Return tracker address
 ***********************************************************/
const char*
HWindow::LookupAddress(const char* name)
{
	AppUtils utils;
	BPath path = utils.GetAppDirPath(be_app);
	path.Append("Trackers");
	path.Append(name);
	BMessage msg;

	BFile file(path.Path(),B_READ_ONLY);
	if(file.InitCheck() == B_OK)
	{
		msg.Unflatten(&file);
	}
	return msg.FindString("address");
}

/***********************************************************
 * DispatchMessage
 ***********************************************************/
void
HWindow::DispatchMessage(BMessage *message,BHandler *handler)
{
	switch(message->what)
	{
	case B_PULSE:
		{
			this->Pulse();
			break;
		}
	}
	BWindow::DispatchMessage(message,handler);
}


/***********************************************************
 * Load trackers.
 ***********************************************************/
void
HWindow::FindTrackers()
{
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Trackers");
	BDirectory dir( path.Path() );
	//(new BAlert("",path.Path(),"OK"))->Go();
	BEntry entry;
	status_t err = B_NO_ERROR;
   	// Fileをすべてロードする
	while( err == B_NO_ERROR )
	{
		err = dir.GetNextEntry( (BEntry*)&entry, TRUE );

		if( entry.InitCheck() != B_NO_ERROR )
			break;

		char name[B_FILE_NAME_LENGTH+1];
		entry.GetName(name);


		BFile file(&entry,B_READ_ONLY);
		if(file.InitCheck() == B_OK)
		{
			BMessage msg;
			msg.Unflatten(&file);

			const char* address = msg.FindString("address");
			HListView *view = cast_as(FindView("listview"),HListView);
			if(view != NULL)
				view->AddServer(new HTrackerItem(name,address,HTRK_TCPPORT,0,"",true));
		}
	}
}

/***********************************************************
 * Search servers.
 ***********************************************************/
void
HWindow::Search(const char* text,uint32 start)
{
	HListView* view = cast_as(FindView("listview"),HListView);
	int32 count = view->CountItems();
	bool found = false;
	HTrackerItem **items = (HTrackerItem**)view->Items();
	for(register int i = start+1;i < count;i++)
	{
		const char* name = (*(items+i))->Name();
		const char* desc = (*(items+i))->Description();

		if(name != NULL)
		{
			BString tmp = name;
			// Hit
			int j = tmp.IFindFirst(text);
			if(j >=0)
			{
				fSearchIndex = i;
				view->Select(i);
				view->ScrollToSelection();
				found = true;
				break;
			}
		}
		if(desc != NULL)
		{
			BString tmp = desc;
			// Hit
			int j = tmp.IFindFirst(text);
			if(j >=0)
			{
				fSearchIndex = i;
				view->Select(i);
				view->ScrollToSelection();
				found = true;
				break;
			}
		}
	}
	if(!found)
		beep();
	return;
}

/***********************************************************
 * Delete trackers.
 ***********************************************************/
void
HWindow::DeleteTracker(const char* name)
{
	BPath path = (new AppUtils())->GetAppDirPath(be_app);
	path.Append("Trackers");
	path.Append(name);

	BEntry entry;
    entry_ref ref;
    entry.SetTo(path.Path());
    entry.GetRef(&ref);
    BMessenger tracker("application/x-vnd.Be-TRAK" );
    BMessage msg( B_DELETE_PROPERTY ) ;
    BMessage specifier( 'sref' ) ;
    specifier.AddRef( "refs", &ref ) ;
    specifier.AddString( "property", "Entry" ) ;
    msg.AddSpecifier( &specifier ) ;

	msg.AddSpecifier( "Poses" ) ;
    msg.AddSpecifier( "Window", 1 ) ;

    BMessage reply ;
    tracker.SendMessage( &msg, &reply );
}



/*
 * Pulse
 */
void
HWindow::Pulse()
{
	HListView* view = cast_as(FindView("listview"),HListView);
	BStringView *stringView = cast_as(FindView("status"),BStringView);
	int32 items = view->CountItems();
	if(items != fOldServerCount || fStatusString.Compare(stringView->Text()))
	{
		BString title;
		if(items == 1)
			title << items << " " << _("item") << "  ( " << fStatusString << " )";
		else
			title << items << " " << _("items") << "  ( " << fStatusString << " )";
		stringView->SetText(title.String());
	}
}

/***********************************************************
 * Return key word.
 ***********************************************************/
const char*
HWindow::Keyword()
{
	BTextControl *control = cast_as(FindView("filter"),BTextControl);
	return control->Text();
}

/***********************************************************
 * QuitRequested.
 ***********************************************************/
bool
HWindow::QuitRequested()
{
	/**** Window Rectのセーブ *******/
	RectUtils utils;
	HListView* view = cast_as(FindView("listview"),HListView);
	view->RemoveAll();
	utils.SaveRectToApp("servwinrect",this->Frame());
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}
