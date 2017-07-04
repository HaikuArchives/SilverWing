#include <ClassInfo.h>
#include <Debug.h>

#include "HNews15Window.h"
#include "HArticleList.h"
#include "HArticleItem.h"
#include "HCategoryList.h"
#include "HCategoryItem.h"
#include "HApp.h"
#include "HToolbar.h"
#include "CTextView.h"
#include <santa/Colors.h>
#include "HArticleView.h"
#include "HToolbar.h"
#include "HotlineClient.h"
#include "HPostThreadWindow.h"
#include "ResourceUtils.h"
#include "HApp.h"
#include "HDialog.h"
#include "MAlert.h"
#include "SplitPane.h"
#include "HPrefs.h"
#include "HWindow.h"
#include "HFileCaption.h"
#include "RectUtils.h"
#include "TextUtils.h"

#define kBarThickness 5

/***********************************************************
 * Constructor.
 ***********************************************************/
HNews15Window::HNews15Window(BRect rect,const char* title)
				:BWindow(rect,title,B_DOCUMENT_WINDOW,B_ASYNCHRONOUS_CONTROLS)
{
	InitGUI();
	float min_width,min_height,max_width,max_height;
	GetSizeLimits(&min_width,&max_width,&min_height,&max_height);
	min_width = 350;
	min_height = 350;
	SetSizeLimits(min_width,max_width,min_height,max_height);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HNews15Window::~HNews15Window()
{
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HNews15Window::InitGUI()
{
	BView *bg = new BView(Bounds(),"bgview",B_FOLLOW_ALL,B_WILL_DRAW);
	bg->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect rightrect = this->Bounds();
	rightrect.top += 30;
	rightrect.bottom -= B_H_SCROLL_BAR_HEIGHT+1;
	BetterScrollView *scroller1;
	new HArticleList(Bounds(),&scroller1,"artlist");
	HArticleView *artview = new HArticleView(Bounds(),"articleview");
	fHSplitter = new SplitPane(rightrect,scroller1,artview,B_FOLLOW_ALL);
	fHSplitter->SetAlignment(B_HORIZONTAL);
	fHSplitter->SetViewInsetBy(BPoint(0,0));
	fHSplitter->SetBarAlignmentLocked(true);
	int32 pos;
	((HApp*)be_app)->Prefs()->GetData("news_hbar_pos",&pos);
	fHSplitter->SetBarPosition(BPoint(0,pos));
	fHSplitter->SetBarThickness(BPoint(0,kBarThickness));
	BetterScrollView *scroller;
	new HCategoryList(Bounds(),&scroller,"catelist");

	fVSplitter = new SplitPane(rightrect,scroller,fHSplitter,B_FOLLOW_ALL);
	fVSplitter->SetAlignment(B_VERTICAL);
	fVSplitter->SetViewInsetBy(BPoint(0,0));
	((HApp*)be_app)->Prefs()->GetData("news_vbar_pos",&pos);
	fVSplitter->SetBarPosition(BPoint(pos,0));
	fVSplitter->SetBarAlignmentLocked(true);
	fVSplitter->SetBarThickness(BPoint(kBarThickness,0));
	bg->AddChild(fVSplitter);
	this->AddChild(bg);
	BRect toolrect = Bounds();

	ResourceUtils utils;
	toolrect.bottom = toolrect.top + 30;
	toolrect.left -= 1;
	toolrect.right += 1;
	HToolbar *toolbar = new HToolbar(toolrect,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	toolbar->AddButton("refreshbtn",utils.GetBitmapResource('BBMP',"BMP:REFRESH"),new BMessage(NEWS15_REFRESH_MESSAGE),"Refresh Groups");
	toolbar->AddSpace();
	toolbar->AddButton("bundlebtn",utils.GetBitmapResource('BBMP',"BMP:BUNDLE"),new BMessage(NEWS15_CREATE_GROUP),"Create new folder");
	toolbar->AddButton("categorybtn",utils.GetBitmapResource('BBMP',"BMP:FOLDER"),new BMessage(NEWS15_CREATE_CATEGORY),"Create new category");
	toolbar->AddSpace();
	toolbar->AddButton("articlebtn",utils.GetBitmapResource('BBMP',"BMP:POST"),new BMessage(NEWS15_POST_THREAD_MESSAGE),"Post new article");
	toolbar->AddButton("replybtn",utils.GetBitmapResource('BBMP',"BMP:REPLY"),new BMessage(NEWS15_REPLY_MESSAGE),"Reply");
	toolbar->AddSpace();
	toolbar->AddButton("trashbtn",utils.GetBitmapResource('BBMP',"BMP:TRASH"),new BMessage(NEWS15_DELETE_MESSAGE),"Delete");
	bg->AddChild(toolbar);

	/********** Captionの追加 ***********/
	BRect captionframe = Bounds();
	captionframe.bottom+=2;
	captionframe.top = captionframe.bottom - B_H_SCROLL_BAR_HEIGHT-1;
	captionframe.right = Bounds().Width() - B_V_SCROLL_BAR_WIDTH +2;
	captionframe.left-=2;
	BBox *bbox = new BBox(captionframe,NULL,B_FOLLOW_BOTTOM);
	captionframe.OffsetTo(B_ORIGIN);
	captionframe.top += 2;
	captionframe.bottom -= 2;
	captionframe.right -= 2;
	captionframe.left += 2;
	fCaption = new HFileCaption(captionframe,"caption",NULL);
	fCaption->SetLabel(_("Getting News Categories…"));
	bbox->AddChild(fCaption);
	AddChild(bbox);
}


/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
HNews15Window::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	/*************** Recv News15 Category ****************/
	case H_NEWS_RECEIVE_FOLDER:
	{
		int32 count;
		type_code type;
		HCategoryList *list = cast_as(FindView("catelist"),HCategoryList);
		message->GetInfo("name",&type,&count);
		for(register int i = 0;i< count;i++)
		{
			const char* name = message->FindString("name",i);
			int16 type = message->FindInt16("type",i);
			int16 posted = message->FindInt16("posted",i);
			uint32 index = message->FindInt32("index",i);
			list->AddCategory(name,type,posted,index);
		}
		fCaption->SetLabel("");
		fCaption->StopBarberPole();

		break;
	}
	/******* Article List Recieve *************/
	case H_NEWS_RECEIVE_ARTICLELIST:
	{
		fCaption->SetLabel("");
		fCaption->StopBarberPole();

		int32 count;
		type_code type;
		HArticleList *list = cast_as(FindView("artlist"),HArticleList);
		list->RemoveAll();
		const char* category = message->FindString("category");
		list->SetCategory(category );
		message->GetInfo("subject",&type,&count);
		for(register int i = 0;i< count;i++)
		{
			const char* subject = message->FindString("subject",i);
			const char* sender = message->FindString("sender",i);
			uint32 time = message->FindInt32("time",i);
			uint32 parent_id = message->FindInt32("parent_id",i);
			uint16 index = message->FindInt16("index",i);
			list->AddArticle(subject,sender,time,parent_id,index);
		}
		list->SortItems();
		break;
	}
	/******* Recv Article **************/
	case H_RECEIVE_THREAD:
	{
		const char* subject = message->FindString("subject");
		const char* sender = message->FindString("sender");
		const char* date = message->FindString("date");
		const char* content = message->FindString("content");
		HArticleView *artview = cast_as(this->FindView("articleview"),HArticleView);
		artview->SetSender(sender);
		artview->SetSubject(subject);
		artview->SetDate(date);
		artview->SetArticle(content);
		fCaption->StopBarberPole();
		fCaption->SetLabel("");
		break;
	}
	/*
	 * Change category list selection.
	 */
	case H_CATEGORY_SEL_CHANGED:
	{
		HArticleList *list = cast_as(FindView("artlist"),HArticleList);
		list->RemoveAll();
		break;
	}
	/*
	 * Post news thread.
	 */
	case NEWS15_POST_THREAD_MESSAGE:
	{
		HArticleList *list = cast_as(FindView("artlist"),HArticleList);
		HCategoryList *catlist = cast_as(FindView("catelist"),HCategoryList);
		int32 sel = catlist->CurrentSelection();
		if(sel < 0)
			break;
		HCategoryItem *item = cast_as(catlist->ItemAt(sel),HCategoryItem);
		if(item->Type() == 3)
		{
			HPostThreadWindow *win = new HPostThreadWindow(RectUtils().CenterRect(300,300),_("Post article"),list->Category());
			win->Show();
		}
		break;
	}
	/*
	 * Post reply news thread.
	 */
	case NEWS15_REPLY_MESSAGE:
	{
		HArticleList *list = cast_as(FindView("artlist"),HArticleList);
		int32 sel = list->CurrentSelection();
		if(sel >= 0)
		{
			HArticleItem *item = cast_as(list->ItemAt(sel),HArticleItem);
			BString tmp = item->Subject();
			BString subject = "";
			//if(tmp.find("Re:") != 0)
			subject = "Re:";
			subject << item->Subject();
			char* sub = new char[subject.Length()+1];
			::strcpy(sub,subject.String());

			int32 encoding;
			((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
			if(encoding)
				TextUtils().ConvertToUTF8(&sub,encoding-1);
			subject = sub;
			delete[] sub;
			HPostThreadWindow *win = new HPostThreadWindow(RectUtils().CenterRect(300,300),_("Post article"),list->Category(),subject.String(),item->Index(),item->ParentID());
			win->Show();
		}
		break;
	}
	/*
	 * Refresh list.
	 */
	case NEWS15_REFRESH_MESSAGE:
	{
		HArticleList *alist = cast_as(this->FindView("artlist"),HArticleList);
		alist->RemoveAll();
		HCategoryList *clist = cast_as(this->FindView("catelist"),HCategoryList);
		clist->RemoveAll();
		HArticleView *artview = cast_as(this->FindView("articleview"),HArticleView);
		artview->ResetAll();
		BMessage msg(MWIN_NEWS_GET_CATEGORY);
		msg.AddString("path","/");
		msg.AddInt32("index",0);
		be_app->PostMessage(&msg);

		fCaption->SetLabel(_("Getting News Categories…"));
		fCaption->StartBarberPole();

		break;
	}
	/*
	 * Create group.
	 */
	case NEWS15_CREATE_GROUP:
	{
		HCategoryList *clist = cast_as(FindView("catelist"),HCategoryList);
		int32 sel = clist->CurrentSelection();
		BMessage *msg = new BMessage(H_CREATE_FOLDER);
		if(sel >=0)
		{
			HCategoryItem *item = cast_as(clist->ItemAt(sel),HCategoryItem);
			if(item->Type() == 3)
				return;
			msg->AddString("path",item->Path());
		}else{
			msg->AddString("path","");
		}
		HDialog *dlg = new HDialog(RectUtils().CenterRect(250,80),_("Create folder"),msg,_("Folder name:"),_("Create"));
		dlg->Show();
		/// NOTE: msg was destroyed in HDialog!
		break;
	}
	/*
	 * Create category.
	 */
	case NEWS15_CREATE_CATEGORY:
	{
		HCategoryList *clist = cast_as(FindView("catelist"),HCategoryList);
		int32 sel = clist->CurrentSelection();
		BMessage *msg = new BMessage(H_CREATE_CATEGORY);
		if(sel >=0)
		{
			HCategoryItem *item = cast_as(clist->ItemAt(sel),HCategoryItem);
			if(item->Type() == 3)
				return;
			msg->AddString("path",item->Path());
		}else{
			msg->AddString("path","");
		}
		HDialog *dlg = new HDialog(RectUtils().CenterRect(250,80),_("Create folder"),msg,_("Folder name:"),_("Create"));
		dlg->Show();

		break;
	}
	/*
	 * Delete message.
	 */
	case NEWS15_DELETE_MESSAGE:
	{
		BView* view = this->CurrentFocus();
		if(view == NULL)
		{
			PRINT(( "NULL VIEW" ));
			return;
		}
		if( is_kind_of(view,HCategoryList) )
		{
			HCategoryList *list = cast_as(view,HCategoryList);
			int32 sel = list->CurrentSelection();
			if(sel >= 0)
			{
				HCategoryItem *item = cast_as(list->ItemAt(sel),HCategoryItem);
				if( (new MAlert(_("Caution"),_("Would you like to delete it?"),_("OK"),_("Cancel"),NULL,B_STOP_ALERT) )->Go() == 0)
				{
					BMessage msg(H_DELETE_CATEGORY);
					msg.AddString("path",item->Path());
					((HApp*)be_app)->Client()->PostMessage(&msg);
				}
			}
		}else if( is_kind_of(view,HArticleList) ){
			HArticleList *list = cast_as(view,HArticleList);
			int32 sel = list->CurrentSelection();
			if(sel >= 0)
			{
				if( (new MAlert(_("Caution"),_("Would you like to delete it?"),_("OK"),_("Cancel"),NULL,B_STOP_ALERT) )->Go() == 0)
				{
					HArticleItem *item = cast_as(list->ItemAt(sel),HArticleItem);
					BMessage msg(H_DELETE_THREAD);
					msg.AddString("path",list->Category());
					msg.AddInt16("thread",item->Index());
					((HApp*)be_app)->Client()->PostMessage(&msg);
				}
			}
		}
		break;
	}
	/*
	 * Update toolbar buttons.
	 */
	case M_UPDATE_TOOLBUTTON:
	{
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
HNews15Window::QuitRequested()
{
	HArticleList *alist = cast_as(FindView("artlist"),HArticleList);
	alist->RemoveAll();
	HCategoryList *clist = cast_as(FindView("catelist"),HCategoryList);
	clist->RemoveAll();

	BPoint pos = fHSplitter->GetBarPosition();
	int32 p = static_cast<int32>(pos.y);
	((HApp*)be_app)->Prefs()->SetData("news_hbar_pos",p);
	pos = fVSplitter->GetBarPosition();
	p = static_cast<int32>(pos.x);
	((HApp*)be_app)->Prefs()->SetData("news_vbar_pos",p);
	((HApp*)be_app)->SaveRect("news15_rect",this->Frame());
	fTarget->PostMessage(NEWS15_CLOSE_WINDOW);
	return BWindow::QuitRequested();
}
