#include <ClassInfo.h>
#include <ScrollView.h>
#include <Autolock.h>
#include <Path.h>
#include <Beep.h>

#include "HApp.h"
#include "HPrefs.h"
#include "HTaskWindow.h"
#include "HTaskView.h"
#include "HToolbar.h"
#include "HToolbarButton.h"
#include "ResourceUtils.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HTaskWindow::HTaskWindow(BRect frame)
	:BWindow(frame,_("Task"),B_TITLED_WINDOW,B_NOT_CLOSABLE|B_NOT_ZOOMABLE|B_NOT_H_RESIZABLE)
{
	BRect viewrect = Bounds();
	
	BRect toolrect = this->Bounds();
	toolrect.bottom = toolrect.top + 30;
	toolrect.right += 2;
	toolrect.left -= 2;
	 
	ResourceUtils utils;
	HToolbar *toolbox = new HToolbar(toolrect,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	toolbox->AddButton("removebtn",utils.GetBitmapResource('BBMP',"BMP:DISCONNECT")
				,new BMessage(M_REMOVE_TASK_FROM_BUTTON),_("Remove task"));
	this->AddChild(toolbox);
	viewrect.top += 30;
	viewrect.right -= B_V_SCROLL_BAR_WIDTH;
	
	BView *mainview = new BView(viewrect,"mainview",B_FOLLOW_ALL,B_WILL_DRAW|B_FRAME_EVENTS);
	mainview->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	BRect rect = Bounds();
	rect.left  = rect.right - B_V_SCROLL_BAR_WIDTH+1;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT;
	BScrollView *scrollview = new BScrollView("bscroll",mainview,B_FOLLOW_TOP_BOTTOM,0,false,true);

	this->AddChild(scrollview);	
	BScrollBar *scroll = scrollview->ScrollBar(B_VERTICAL);
	scroll->SetValue(0.0);
	//scroll->SetSteps(10,20);
	scroll->SetRange(0,0);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HTaskWindow::~HTaskWindow()
{
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HTaskWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case M_ADD_TASK:
	{
		ShowHide(true);
		const char* name = message->FindString("label");
		uint32 task = message->FindInt32("task");
		uint32 type = message->FindInt32("type");
		BRect rect = this->Bounds();
		rect.right -= B_V_SCROLL_BAR_WIDTH;
		rect.bottom = rect.top + VIEW_HEIGHT;
		HTaskView *view = new HTaskView(rect,name,task,type);
		this->AddView(view);
		break;
	}
	case M_UPDATE_TASK:
	{	
		uint32 task = message->FindInt32("task");
		uint32 update = message->FindInt32("update");
		HTaskView *view = LookupTaskView(task);
		if(view != NULL)
			view->Update(update - view->CurrentValue());
		break;
	}
	case M_REMOVE_TASK:
	{
		uint32 task = message->FindInt32("task");
		HTaskView *view = LookupTaskView(task);
		if(view != NULL)
		{
			this->RemoveView(view);
			ShowHide(false);
		}
		break;
	}
	case M_REMOVE_ALL_TASK:
	{
		RemoveAllView();
		break;
	}
	case M_SET_MAX_TASK:
	{
		uint32 max = message->FindInt32("max_value");
		uint32 task = message->FindInt32("task");
		
		HTaskView *view = LookupTaskView(task);
		if(view != NULL)
		{
			view->SetMaxValue(max);
		}
		break;
	}
	case M_DOWNLOAD_TASK:
	{
		ShowHide(true);
		uint32 task = message->FindInt32("task");
		
		uint32 ref = message->FindInt32("ref");
		uint32 size = message->FindInt32("size");
		const char* localpath = message->FindString("localpath");
		const char* remotepath = message->FindString("remotepath");
		const char* address = message->FindString("address");
		uint16 port = message->FindInt16("port");
		uint32 server_queue = message->FindInt32("queue");
		
		BRect rect = this->Bounds();
		rect.right -= B_V_SCROLL_BAR_WIDTH;
		rect.bottom = rect.top + VIEW_HEIGHT;
		BPath path(localpath);
		HTaskView *view = new HTaskView(rect,path.Leaf(),task,T_FILE_TRANS_TYPE);
		view->SetServer(address,port);
		view->SetFiles(localpath,remotepath);
		view->SetRefAndSize(ref,size);
		view->SetDownload(true);
		view->SetQueue(server_queue,true);
		this->AddView(view);
		
		bool queue;
		((HApp*)be_app)->Prefs()->GetData("queue_download",&queue);
		
		if(queue == false)
			view->Start();
		else if(queue == true && this->CountFileTrans() <= 1)
			view->Start();
		else{
			BString label = _("waiting queue");
			label += "…";
			view->SetTrailingText(label.String());
		}
		break;
	}
	case M_UPLOAD_TASK:
	{
		ShowHide(true);
		uint32 task = message->FindInt32("task");
		
		uint32 ref = message->FindInt32("ref");
		uint32 size = message->FindInt32("size");
		const char* localpath = message->FindString("localpath");
		const char* remotepath = message->FindString("remotepath");
		const char* address = message->FindString("address");
		uint16 port = message->FindInt16("port");
		uint32 server_queue = message->FindInt32("queue");
		
		BRect rect = this->Bounds();
		rect.right -= B_V_SCROLL_BAR_WIDTH;
		rect.bottom = rect.top + VIEW_HEIGHT;
		
		BPath path(localpath);
		HTaskView *view = new HTaskView(rect,path.Leaf(),task,T_FILE_TRANS_TYPE);
		view->SetServer(address,port);
		view->SetFiles(localpath,remotepath);
		view->SetRefAndSize(ref,size);
		view->SetDownload(false);
		view->SetQueue(server_queue,true);
		this->AddView(view);
		
		view->Start();
		break;
	}
	case M_REMOVE_TASK_FROM_BUTTON:
	{
		BView* main = FindView("mainview");
		int32 count = main->CountChildren();
		if(count > 0)
		{
			BView *view = this->CurrentFocus();
			if(is_kind_of(view,HTaskView))
			{
				HTaskView *task_view = cast_as(view,HTaskView);
				if(task_view->Type() == T_FILE_TRANS_TYPE)
				{
					task_view->Cancel();
					// start next file transfer when queued.
					bool queue;
					((HApp*)be_app)->Prefs()->GetData("queue_download",&queue);
		
					if(queue)
					{
						HTaskView *nexttask = this->NextFileTrans(task_view);
						if(nexttask)
							nexttask->Start();
					}
				}
			}
		}
		break;
	}
	case M_TRANS_COMPLETE:
	{
		BAutolock lock(this);
		void *pointer;
		message->FindPointer("pointer",&pointer);
		HTaskView *view = static_cast<HTaskView*>(pointer);
		// start next file transfer when queued.
		/*bool queue;
		((HApp*)be_app)->Prefs()->GetData("queue_download",&queue);
		
		if(queue)
		{
			HTaskView *nexttask = this->NextFileTrans(view);
			if(nexttask)
			{
				::snooze(1000000*1);
				nexttask->Start();
			}
		}*/
		this->RemoveView(view);
		ShowHide(false);
		break;
	}
	// Update queue
	case M_RECEIVE_UPDATE_QUEUE:
	{
		uint32 queue = message->FindInt32("queue");
		uint32 ref = message->FindInt32("ref");
		HTaskView* view = FindFileTrans(ref);
		if(view)
			view->SetQueue(queue);
		break;
	}
	// Update toolbar
	case M_UPDATE_TOOLBUTTON:
	{
		void *pointer;
		message->FindPointer("pointer",&pointer);
		HToolbarButton *btn = static_cast<HToolbarButton*>(pointer);
		BView* main = FindView("mainview");
		int32 count = main->CountChildren();
		if(count > 0)
		{
			BView *view = this->CurrentFocus();
			if(is_kind_of(view,HTaskView))
			{
				HTaskView *task_view = cast_as(view,HTaskView);
				if(task_view->Type() == T_FILE_TRANS_TYPE)
				{
					btn->SetEnabled(true);
				}
			}
		}else{
			btn->SetEnabled(false);
		}
		break;
	}
	default:
		BWindow::MessageReceived(message);
	}
}

/***********************************************************
 * QuitRequested.
 ***********************************************************/
bool
HTaskWindow::QuitRequested()
{
	((HApp*)be_app)->SaveRect("task_rect",this->Frame());
	return BWindow::QuitRequested();
}

/***********************************************************
 * Add task view.
 ***********************************************************/
void
HTaskWindow::AddView(HTaskView *view)
{
	BView *main = FindView("mainview");
	int count = main->CountChildren();
	view->MoveBy(0,VIEW_HEIGHT*count);
	main->ResizeBy(0,VIEW_HEIGHT);
	main->AddChild(view);
	UpdateScrollBar();
}

/***********************************************************
 * Remove view.
 ***********************************************************/
void
HTaskWindow::RemoveView(HTaskView* view)
{
	BView *nextview  = view->NextSibling();
	while(nextview != NULL)
	{
		nextview->MoveBy(0,-VIEW_HEIGHT);
		nextview = nextview->NextSibling();
	}
	BView *main = FindView("mainview");
	main->RemoveChild(view);
	delete view;
	UpdateScrollBar();
}

/***********************************************************
 * Remove all views.
 ***********************************************************/
void
HTaskWindow::RemoveAllView()
{
	BView *main = FindView("mainview");
	int32 count = main->CountChildren();
	for(register int32 i = 0;i < count;i++)
	{
		BView *view = main->ChildAt(0);
		main->RemoveChild(view);
		delete view;
	}
}

/***********************************************************
 * Count file transfers.
 ***********************************************************/
int32
HTaskWindow::CountFileTrans()
{
	int32 count = 0;
	
	BView *main = FindView("mainview");
	int32 all = main->CountChildren();
	for(register int32 i = 0;i < all;i++)
	{
		HTaskView *view = cast_as(main->ChildAt(i),HTaskView);
		if(view->Type() == T_FILE_TRANS_TYPE)
			count++;
	}
	return count;
}

/***********************************************************
 * FindFileTrans
 ***********************************************************/
HTaskView*
HTaskWindow::FindFileTrans(uint32 ref)
{
	BView *main = FindView("mainview");
	int32 all = main->CountChildren();
	HTaskView *result = NULL;
	for(register int32 i = 0;i < all;i++)
	{
		HTaskView *view = cast_as(main->ChildAt(i),HTaskView);
		if(view->Type() == T_FILE_TRANS_TYPE)
		{
			if(view->Ref() == ref)
			{
				result = view;
				break;			
			}
		}
	}
	return result;
}

/***********************************************************
 * Find next file transfer.
 ***********************************************************/
HTaskView*
HTaskWindow::NextFileTrans(HTaskView *view)
{
	HTaskView *nextview  = cast_as(view->NextSibling(),HTaskView);
	bool found = false;
	while(nextview != NULL)
	{
		if(nextview->Type() == T_FILE_TRANS_TYPE)
		{
			found = true;
			break;
		}
		nextview = cast_as(nextview->NextSibling(),HTaskView);
	}
	if(!found)
		nextview = NULL;
	return nextview;
}

/***********************************************************
 * Pick up Task view.
 ***********************************************************/
HTaskView*
HTaskWindow::LookupTaskView(uint32 task)
{
	BView *main = FindView("mainview");
	HTaskView *view = NULL;
	int count = main->CountChildren();
	bool found = false;
	for(register int i =0;i<count;i++)
	{
		view = cast_as(main->ChildAt(i),HTaskView);
		if(view->Task() == task)
		{
			found = true;
			break;
		}
	}
	if(!found)
		view = NULL;
	return view;
}

/***********************************************************
 * ScrollViewをUpdateする
 ***********************************************************/
void
HTaskWindow::UpdateScrollBar()
{
	BView *main = FindView("mainview");
	
	int count = main->CountChildren();
	float w = main->Frame().Height();
	w = VIEW_HEIGHT*count - w;
	if(w < 0) w = 0.0;
	//((BScrollBar*)Window()->FindView("downloadScroller"))->SetRange(0,w);
	BScrollBar *scroll = ((BScrollView*)FindView("bscroll"))->ScrollBar(B_VERTICAL);
	scroll->SetRange(0,w);
}

/***********************************************************
 * Show or Hide window
 ***********************************************************/
void
HTaskWindow::ShowHide(bool show)
{
	BAutolock lock(this);
	bool iconfy;
	((HApp*)be_app)->Prefs()->GetData("taskiconfy",&iconfy);
	if(iconfy)
	{
		if(show)
		{
			if(IsHidden())
				Show();
		}else{
			BView *main = FindView("mainview");
			int count = main->CountChildren();
			if(!IsHidden() && count == 0)
				Hide();
		}
	}
}