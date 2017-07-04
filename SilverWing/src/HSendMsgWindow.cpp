#include "HSendMsgWindow.h"
#include "RectUtils.h"
#include <santa/Colors.h>
#include "TextUtils.h"
#include "HApp.h"
#include "HPrefs.h"
#include "ResourceUtils.h"
#include "HBitmapView.h"
#include "HMsgWindow.h"

#include <Autolock.h>
#include <StringView.h>
#include <ScrollView.h>
#include <Button.h>
#include <stdio.h>

/***********************************************************
 * Constructor.
 ***********************************************************/
HSendMsgWindow::HSendMsgWindow(BRect rect,const char *name,uint32 sock)
	:BWindow(rect,name,B_DOCUMENT_WINDOW,B_NOT_CLOSABLE|B_ASYNCHRONOUS_CONTROLS)
//	,fToolTip(NULL)
{
	//fToolTip = new ToolTip(this, "ToolTip");
	InitGUI();
	fSock = sock;
	this->AddShortcut(B_RETURN,0,new BMessage(SENDMSG_SEND_MSG));
	this->AddShortcut(B_ESCAPE,0,new BMessage(B_QUIT_REQUESTED));
	this->AddShortcut('W',0,new BMessage(B_QUIT_REQUESTED));
	this->AddShortcut('M',0,new BMessage(MESSAGE_CHAT_MSG));

	float min_width,min_height,max_width,max_height;
	GetSizeLimits(&min_width,&max_width,&min_height,&max_height);
	min_width = 300;
	min_height = 150;
	SetSizeLimits(min_width,max_width,min_height,max_height);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HSendMsgWindow::~HSendMsgWindow()
{
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HSendMsgWindow::InitGUI()
{
	BRect rect = Bounds();
	rect.OffsetTo(B_ORIGIN);
	BView *bg = new BView(rect,"bg",B_FOLLOW_ALL,0);
	bg->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	/******* Bitmap **********/
	BRect bitmapRect = rect;
	bitmapRect.top += 5;
	bitmapRect.left += 5;
	bitmapRect.bottom = bitmapRect.top + 32;
	bitmapRect.right = bitmapRect.left + 32;
	BBitmap *bitmap = ResourceUtils().GetBitmapResource('BBMP',"BMP:SENDMESSAGE");
	HBitmapView *bitmapView = new HBitmapView(bitmapRect,"bitmap",B_FOLLOW_TOP|B_FOLLOW_LEFT,bitmap);
	bg->AddChild(bitmapView);
	/********* StringView ***********/
	BRect stringRect = bitmapRect;
	stringRect.left = stringRect.right + 5;
	stringRect.right = Bounds().right - 5;
	const char* label = "Type a message and click the \'Send\' button to send it privately.";
	BStringView *stringView = new BStringView(stringRect,"stringview",label,B_FOLLOW_TOP|B_FOLLOW_LEFT_RIGHT);
	stringView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	stringView->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(stringView);

	rect.left += 5;
	rect.right -= 5+B_V_SCROLL_BAR_WIDTH;
	rect.top  = bitmapRect.bottom + 5;
	rect.bottom -= 50;

	textview = new CTextView(rect,"textview",B_FOLLOW_ALL,B_WILL_DRAW|B_NAVIGABLE);
	textview->MakeEditable(true);
	textview->SetWordWrap(true);
	BScrollView *scrollView = new BScrollView("scrollview",textview,B_FOLLOW_ALL,
													B_WILL_DRAW|B_FRAME_EVENTS,false,true,B_FANCY_BORDER);
	bg->AddChild(scrollView);
	rect.top = rect.bottom + 10;
	rect.bottom = rect.top + 20;
	rect.left = rect.right - 80;
	BButton *btn;

	btn = new BButton(rect,"send",_("Send"),new BMessage(SENDMSG_SEND_MSG),B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT);
	//fToolTip->SetText(btn, "Send message privately. [ALT+RETURN]");
	bg->AddChild(btn);
	rect.OffsetBy(-90,0);
	btn = new BButton(rect,"ok",_("Close"),new BMessage(B_QUIT_REQUESTED),B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT);
	//fToolTip->SetText(btn, "Close window. [ALT+W]");
	bg->AddChild(btn);

	rect.left = Bounds().left + 5;
	rect.right = rect.left + 80;
	btn = new BButton(rect,"chat",_("Message chat"),new BMessage(MESSAGE_CHAT_MSG),B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT);
	bg->AddChild(btn);
	AddChild(bg);
	textview->MakeFocus(true);
}

/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
HSendMsgWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case SENDMSG_SEND_MSG:
	{
		BString text = textview->Text();
		if(text.Length() > 0)
		{
			TextUtils utils;
			char* t = new char[text.Length()+1];
			::memset(t,0,text.Length()+1);
			::sprintf(t,"%s",text.String());

			int32 encoding;
			((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
			if(encoding)
				utils.ConvertFromUTF8(&t,encoding-1);
			utils.ConvertReturnsToCR(t);
			message->AddString("text",t);
			message->AddInt32("sock",(int32)fSock);
			be_app->PostMessage(message);
			this->PostMessage(B_QUIT_REQUESTED);
			delete[] t;
		}
		break;
	}
	case MESSAGE_CHAT_MSG:
	{
		message->AddInt32("sock",fSock);
		be_app->PostMessage(message);
		this->PostMessage(B_QUIT_REQUESTED);
		break;
	}
	default:
		BWindow::MessageReceived(message);
	}
}
/***********************************************************
 *
 ***********************************************************/
void
HSendMsgWindow::SetParentMessage(const char* text)
{
	BString str = ">";
	str << text;
	str.ReplaceAll("\n","\n>");
	InsertMessage(str.String());
}


/***********************************************************
 * Insert message.
 ***********************************************************/
void
HSendMsgWindow::InsertMessage(const char* text)
{
	BAutolock lock(this);
	if(lock.IsLocked())
	{
		textview->Insert(text);
		textview->Insert("\n");
	}
}
