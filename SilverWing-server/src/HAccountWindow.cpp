#include "HAccountWindow.h"
#include "MenuUtils.h"
#include "RectUtils.h"
#include "AppUtils.h"
#include "HDialog.h"
#include "HAccountItem.h"
#include "HApp.h"
#include "TServer.h"
#include "HWindow.h"

#include <NodeMonitor.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <ScrollView.h>
#include <Alert.h>
#include <Directory.h>

/**************************************************************
 * Constructor.
 **************************************************************/
HAccountWindow::HAccountWindow(BRect rect,const char* name)
	:BWindow(rect,name,B_TITLED_WINDOW_LOOK,B_MODAL_APP_WINDOW_FEEL,B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
{
	InitMenu();
	InitGUI();
	AddShortcut('W',0,new BMessage(B_QUIT_REQUESTED));
	
	fChangedAccount.MakeEmpty();
	fChangedAccount.what = T_RESET_ACCOUNT;
	
	LoadAccounts();
	
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Accounts");
	BEntry entry(path.Path());
	node_ref nref;
	entry.GetNodeRef(&nref);
	::watch_node(&nref,B_WATCH_NAME|B_WATCH_DIRECTORY,this);
}

/**************************************************************
 * Destructor.
 **************************************************************/
HAccountWindow::~HAccountWindow()
{
	::stop_watching(this);
	this->RemoveAll();
}

/**************************************************************
 * Set up menus.
 **************************************************************/
void
HAccountWindow::InitMenu()
{
	BRect frame;
	frame = Bounds();
	frame.bottom = frame.top + 15;

	BMenuBar *menuBar = new BMenuBar(frame,"MENUBAR");
	MenuUtils utils;
	BMenu *menu;
	menu = new BMenu("Account");
	utils.AddMenuItem(menu,"Create new account",M_CREATE_ACCOUNT,this,this,'N',0);
	utils.AddMenuItem(menu,"Delete",M_DELETE_ACCOUNT,this,this);
	menuBar->AddItem(menu);
	this->AddChild(menuBar);
}

/**************************************************************
 * Set up GUI.
 **************************************************************/
void
HAccountWindow::InitGUI()
{
	BRect rect = Bounds();
	
	rect.top += (KeyMenuBar())->Bounds().Height();
	rect.right = rect.left + 140 - B_V_SCROLL_BAR_WIDTH;
	fListView = new HAccountList(rect,"listview");
	BScrollView *scrollview = new BScrollView("scrollview",fListView,
			B_FOLLOW_LEFT|B_FOLLOW_TOP,0,false,true,B_PLAIN_BORDER);
	
	this->AddChild(scrollview);
	
	rect.right = Bounds().right;
	rect.left = Bounds().left + 140;
	fAccountView = new HAccountView(rect,"accountview");
	this->AddChild(fAccountView);
}

/**************************************************************
 * MessageReceived.
 **************************************************************/
void
HAccountWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case M_CREATE_ACCOUNT:
	{
		HDialog *dlg = new HDialog(RectUtils().CenterRect(200,80),"Account","Account name:","Create");
		dlg->SetParent(this);
		dlg->Show();
		break;
	}
	case M_LIST_SELECTION_CHANGED:
	{
		const char* text;
		// check changed
		if(fAccountView->IsDirty())
		{
			const char *account = fAccountView->Account();
			if(!FindChangedAccount(account))
			{
				AddChangedAccount(account);
			}
		}
		//
		if(message->FindString("text",&text) == B_OK)
			fAccountView->SetAccount(text);
		break;
	}
	case B_NODE_MONITOR:
	{
		const char* name;
		int32 opcode;
		if (message->FindInt32("opcode", &opcode) == B_OK)
		{
			switch(opcode)
			{
				case B_ENTRY_MOVED:
				{
					message->FindString("name", &name);
					BStringItem **item = (BStringItem**)fListView->Items();
					int32 count = fListView->CountItems();
					for(register int32 i = 0;i < count;i++)
					{
						if(strcmp(name, (*(item+i))->Text() ) == 0)
						{
							BStringItem *it = (BStringItem*)fListView->RemoveItem( i );
							delete it;
						}
					}
					break;
				}
				case B_ENTRY_CREATED:
				{
					message->FindString("name", &name);
					fListView->AddItem(new HAccountItem(name));
					break;
				}
			}
		}
		break;
	}
	case M_DELETE_ACCOUNT:
	{
		int32 sel = fListView->CurrentSelection();
		if(sel >= 0)
		{
			HAccountItem *item = (HAccountItem*)fListView->RemoveItem(sel);
			BPath path = AppUtils().GetAppDirPath(be_app);
			path.Append("Accounts");
			path.Append(item->Text());
		
			BEntry entry(path.Path());
			entry_ref ref;
			entry.GetRef(&ref);
			// Send move to trash message to Tracker
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
    		delete item; 
		}
		break;
	}
	case OK_MESSAGE:
	{
		const char* name = message->FindString("text");
		this->CreateAccount(name);
		break;
	}
	case M_DIRTY_MSG:
	{
		PostMessage(message,fAccountView);
		break;
	}
	default:
		BWindow::MessageReceived(message);
	}
}

/**************************************************************
 * Create accounts.
 **************************************************************/
void
HAccountWindow::CreateAccount(const char* name)
{
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Accounts");
	path.Append(name);
	
	BFile file(path.Path(),B_CREATE_FILE|B_WRITE_ONLY);
	if(file.InitCheck() != B_OK)
		return;
	BMessage msg;
	
	msg.AddString("login",name);
	msg.AddString("password","");
	msg.AddBool("download",false);
	msg.AddBool("upload",false);
	msg.AddBool("uploaduploads",true);
	msg.AddBool("viewfile",true);
	msg.AddBool("delete",false);
	msg.AddBool("rename",false);
	msg.AddBool("move",false);
	msg.AddBool("createfolder",false);
	msg.AddBool("getinfo",false);
	msg.AddBool("kick",false);
	msg.AddBool("ban",false);
	msg.AddBool("readnews",false);
	msg.AddBool("postnews",false);
	msg.AddBool("prvchat",false);
	
	msg.Flatten(&file);
	

}

/**************************************************************
 * Load all accounts.
 **************************************************************/
void
HAccountWindow::LoadAccounts()
{
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Accounts");
	BDirectory dir( path.Path() );
   	status_t err = B_NO_ERROR;
   	BEntry entry;

	while( err == B_NO_ERROR )
	{
		err = dir.GetNextEntry( &entry, true );			
		if( entry.InitCheck() != B_NO_ERROR )
			break;
		BPath filepath;
		if(entry.GetPath(&filepath) != B_OK)
		{
			break;
		}
		fListView->AddItem(new HAccountItem(filepath.Leaf()));
	}
		
}

/**************************************************************
 * Remove all listitem and free them.
 **************************************************************/
void
HAccountWindow::RemoveAll()
{
	int32 count = fListView->CountItems();
	
	for(register int32 i = 0;i < count ;i++)
	{
		HAccountItem *item = (HAccountItem*)fListView->RemoveItem(i);
		delete item;
	}
}

/***********************************************************
 * Add changed account
 ***********************************************************/
void
HAccountWindow::AddChangedAccount(const char* name)
{
	fChangedAccount.AddString("account_name",name);
}

/***********************************************************
 * Find changed account
 ***********************************************************/
bool
HAccountWindow::FindChangedAccount(const char* name) const
{
	int32 count;
	type_code type;
	fChangedAccount.GetInfo("account_name",&type,&count);
	bool found = false;
	
	for(register int32 i = 0;i < count;i++)
	{
		const char* account;
		if(fChangedAccount.FindString("account_name",i,&account) == B_OK)
		{
			if(::strcmp(account,name) == 0)
			{
				found = true;
				break;
			}	
		}
	}
	
	return found;
}

/***********************************************************
 * QuitRequested
 ***********************************************************/
bool
HAccountWindow::QuitRequested()
{
	// check changed
	if(fAccountView->IsDirty())
	{
		const char *account = fAccountView->Account();
		if(!FindChangedAccount(account))
		{
			AddChangedAccount(account);
		}
	}
	//
	if(!fChangedAccount.IsEmpty())
	{
		((HApp*)be_app)->Window()->Server()->PostMessage(&fChangedAccount);
	}
	return BWindow::QuitRequested();
}