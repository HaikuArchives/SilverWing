#include <ClassInfo.h>
#include <Message.h>
#include <Application.h>
#include <ScrollView.h>
#include <Button.h>
#include <String.h>

#include "HPostNewsDialog.h"
#include "CTextView.h"
#include "TextUtils.h"
#include "HApp.h"
#include "HPrefs.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HPostNewsDialog::HPostNewsDialog(BRect rect,const char* name)
		:BWindow(rect,name,B_FLOATING_WINDOW_LOOK,
				B_MODAL_APP_WINDOW_FEEL,
				B_ASYNCHRONOUS_CONTROLS|B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
{
	InitGUI();
	CTextView *view = cast_as(FindView("textview"),CTextView);
	view->MakeFocus(true);
	this->AddShortcut(B_RETURN,0,new BMessage(POST_NEWS_MSG));	
	this->AddShortcut(B_ESCAPE,0,new BMessage(B_QUIT_REQUESTED));	
	this->AddShortcut('W',0,new BMessage(B_QUIT_REQUESTED));
	float min_width,min_height,max_width,max_height;
	GetSizeLimits(&min_width,&max_width,&min_height,&max_height);
	min_width = 150;
	min_height = 150;
	SetSizeLimits(min_width,max_width,min_height,max_height);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HPostNewsDialog::~HPostNewsDialog()
{

}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HPostNewsDialog::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	// News posting
	case POST_NEWS_MSG:
	{
		BMessage msg(POST_NEWS_POST);
		CTextView *view = cast_as(FindView("textview"),CTextView);
		if(view->TextLength() > 0)
		{
			BString text = view->Text();
			TextUtils utils;
			int32 encoding;
			((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
			if(encoding)
				utils.ConvertFromUTF8(text,encoding-1);
			utils.ConvertReturnCode(text,K_CR);
			msg.AddString("text",text);
			be_app->PostMessage(&msg);
			this->PostMessage(B_QUIT_REQUESTED);
		}
		break;
	}
	default:
		BWindow::MessageReceived(message);
	}
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HPostNewsDialog::InitGUI()
{
	BView *bgview = new BView(Bounds(),"bgview",0,0);
	bgview->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	BRect rect = Bounds();
	rect.top += 10;
	rect.left += 10;
	rect.right -= 10;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT+30;
	rect.right -= B_V_SCROLL_BAR_WIDTH;
	CTextView *textview = new CTextView(rect,"textview",B_FOLLOW_ALL,B_WILL_DRAW);
	BScrollView *scrollview = new BScrollView("scrollview",textview,B_FOLLOW_ALL,B_WILL_DRAW,false,true);
	bgview->AddChild(scrollview);
	
	rect.left  = rect.right - 70;
	rect.top = rect.bottom + 10;
	rect.bottom = rect.top + 20;
	
	BButton *btn = new BButton(rect,"ok",_("Post"),new BMessage(POST_NEWS_MSG));
	bgview->AddChild(btn);
	this->AddChild(bgview);
}
