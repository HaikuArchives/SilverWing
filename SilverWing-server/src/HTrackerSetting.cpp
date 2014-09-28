#include <TextControl.h>
#include <Button.h>
#include <ScrollView.h>
#include <ListView.h>
#include <ClassInfo.h>
#include "HListView.h"
#include "HTrackerItem.h"
#include "HTrackerSetting.h"
#include "AppUtils.h"
/**************************************************************
 * Constructor.
 **************************************************************/
HTrackerSetting::HTrackerSetting(BRect rect,const char* name)
	:BView(rect,name,B_FOLLOW_ALL,B_WILL_DRAW)
{
	InitGUI();
}

/**************************************************************
 * Destructor.
 **************************************************************/
HTrackerSetting::~HTrackerSetting()
{
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HTrackerSetting::InitGUI()
{
	BRect rect = Bounds();
	this->SetViewColor(216,216,216,0);
	
	rect.top += 10;
	rect.left += 10;
	rect.right = rect.left + 200;
	rect.bottom = rect.top + 15;
	const int32 kDevider = static_cast<int32>(this->StringWidth("Tracker address:"));
	fAddress = new BTextControl(rect,"tracker","Tracker address:","",NULL);
	fAddress->SetDivider( kDevider + 4 );
	this->AddChild(fAddress);
	
	rect.OffsetBy(0,20);
	fLogin = new BTextControl(rect,"login","Login:","",NULL);
	fLogin->SetDivider( kDevider + 4 );
	this->AddChild(fLogin);
	
	rect.OffsetBy(0,20);
	fPassword = new BTextControl(rect,"password","Password:","",NULL);
	fPassword->SetDivider( kDevider + 4 );
	this->AddChild(fPassword);
	
	BRect btnRect = rect;
	btnRect.right = btnRect.left + 70;
	btnRect.OffsetBy(215,0);
	BButton *AddBtn = new BButton(btnRect,"add","Add",new BMessage(M_ADD_MSG));
	AddBtn->SetTarget(this);
	this->AddChild(AddBtn);
	
	btnRect.OffsetBy(75,0);
	AddBtn = new BButton(btnRect,"del","Delete",new BMessage(M_DEL_MSG));
	AddBtn->SetTarget(this);
	this->AddChild(AddBtn);
	
	
	rect.top += 40;
	rect.bottom = Bounds().bottom - 50;
	rect.right = Bounds().right - 10;
	rect.right -= B_V_SCROLL_BAR_WIDTH;
	fListView = new HListView(rect,"listview",B_SINGLE_SELECTION_LIST,M_SEL_CHANGE_MSG);
	BScrollView *scrollview = new BScrollView("scroll",fListView,B_FOLLOW_ALL,B_WILL_DRAW,false,true);
	this->AddChild(scrollview);
	
	this->LoadTrackers();
}

/**************************************************************
 * MessageReceived.
 **************************************************************/
void
HTrackerSetting::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case M_ADD_MSG:
	{
		const char* tracker = fAddress->Text();
		const char* login = fLogin->Text();
		const char* password = fPassword->Text();
		HTrackerItem *item;
		if( FindItem(tracker,&item) )
		{
			item->SetAddress(tracker);
			item->SetLogin(login);
			item->SetPassword(password);		
		}else
			fListView->AddItem( new HTrackerItem(tracker,login,password) );
		break;
	}
	case M_SEL_CHANGE_MSG:
	{
		int32 sel = fListView->CurrentSelection();
		if(sel < 0)
			break;
		HTrackerItem *item = cast_as(fListView->ItemAt(sel),HTrackerItem);
		if(item == NULL)
			break;
		fAddress->SetText(item->Address());
		fLogin->SetText(item->Login());
		fPassword->SetText(item->Password());
		break;
	}
	case M_DEL_MSG:
	{
		int32 sel = fListView->CurrentSelection();
		if(sel >= 0)
		{
			HTrackerItem *item = cast_as(fListView->RemoveItem(sel),HTrackerItem);
			delete item;
		}
		break;
	}
	default:
		BView::MessageReceived(message);
	}
}

/**************************************************************
 * Load trackers.
 **************************************************************/
void
HTrackerSetting::LoadTrackers()
{
	BMessage msg;
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Trackers");
	path.Append("tracker.dat");
	
	BFile file(path.Path(),B_READ_ONLY);
	if(file.InitCheck() != B_OK)
		return;
	msg.Unflatten(&file);
	
	int32 count;
	type_code type;
	
	msg.GetInfo("tracker",&type,&count);
	for(register int32 i = 0;i < count;i++)
	{
		const char* tracker = msg.FindString("tracker",i);
		
		BString login = "";
		BString password = "";
		//To keep compatibility for old preferences.
		if(msg.HasString("address"))
			login = msg.FindString("address",i);
		if(msg.HasString("password"))
			password = msg.FindString("password",i);	

		fListView->AddItem( new HTrackerItem(tracker,login.String(),password.String()));
	}
}

/**************************************************************
 * Save trackers.
 **************************************************************/
void
HTrackerSetting::SaveTrackers()
{
	BMessage msg;
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Trackers");
	path.Append("tracker.dat");
	
	BFile file(path.Path(),B_WRITE_ONLY|B_CREATE_FILE|B_ERASE_FILE);
	if(file.InitCheck() != B_OK)
		return;
	
	int32 count = fListView->CountItems();
	for(register int32 i = 0;i < count;i++)
	{
		HTrackerItem *item = cast_as(fListView->ItemAt(i),HTrackerItem);
		if(item == NULL)
			break;
		const char* tracker = item->Address();
		const char* login = item->Login();
		const char* password = item->Password();
		msg.AddString("tracker",tracker);
		msg.AddString("login",login);
		msg.AddString("password",password);
	}
	msg.Flatten(&file);
	this->RemoveAll();
}

/**************************************************************
 * MessageReceived.
 **************************************************************/
bool
HTrackerSetting::FindItem(const char* name,HTrackerItem **out)
{
	int32 count = fListView->CountItems();
	bool rc = false;
	for(register int32 i = 0;i < count;i++)
	{
		HTrackerItem *item = cast_as(fListView->ItemAt(i),HTrackerItem);
		if( ::strcmp(item->Address(),name) == 0)
		{
			(*out) = item;
			rc = true;	
		}
	}
	return rc;
}

/**************************************************************
 * Remove all list item and free them.
 **************************************************************/
void
HTrackerSetting::RemoveAll()
{
	int32 count = fListView->CountItems();
	for(register int32 i = 0;i < count;i++)
		delete fListView->ItemAt(i);
	fListView->MakeEmpty();
}