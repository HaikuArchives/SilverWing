#include "HMsgWindow.h"
#include "RectUtils.h"
#include <santa/Colors.h>
#include "HSendMsgWindow.h"
#include "TextUtils.h"
#include "HApp.h"
#include "HMsgWindow.h"
#include "HPrefs.h"
#include "MovieView.h"
#include "MsgIconView.h"

#include <ScrollView.h>
#include <Button.h>
#include <stdio.h>
#include <StringView.h>
#include <Autolock.h>

/***********************************************************
 * Constructor
 ***********************************************************/
HMsgWindow::HMsgWindow(BRect rect
					,const char *nick
					,uint32 sock
					,uint32 icon
					,const char* text)
	:BWindow(rect,"",B_DOCUMENT_WINDOW,B_NOT_CLOSABLE|B_ASYNCHRONOUS_CONTROLS)
	,fSock(sock)
	,fMovieView(NULL)
{
	BString title = _("Private message from");
	title += ": ";
	title += nick;
	SetTitle(title.String());
	InitGUI(text,nick,icon);
	this->AddShortcut(B_ESCAPE,0,new BMessage(B_QUIT_REQUESTED));
	this->AddShortcut('W',0,new BMessage(B_QUIT_REQUESTED));
	this->AddShortcut('R',0,new BMessage(MESSAGE_REPLY_MSG));
	this->AddShortcut(B_RETURN,0,new BMessage(MESSAGE_REPLY_MSG));
	this->AddShortcut('/',0,new BMessage(B_ZOOM));
	this->AddShortcut('M',0,new BMessage(MESSAGE_CHAT_MSG));
	float min_width,min_height,max_width,max_height;
	GetSizeLimits(&min_width,&max_width,&min_height,&max_height);
	min_width = 200;
	min_height = 100;
	SetSizeLimits(min_width,max_width,min_height,max_height);
	if(fMovieView)
		fMovieView->Play();
}

/***********************************************************
 * Constructor.
 ***********************************************************/
HMsgWindow::~HMsgWindow()
{
}

/***********************************************************
 * Destructor.
 ***********************************************************/
void
HMsgWindow::InitGUI(const char* text
					,const char* nick
					,uint32 icon)
{
	BRect rect = Bounds();
	BView *bg = new BView(rect,"bg",B_FOLLOW_ALL,0);
	bg->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	/*
	BRect movieRect = Bounds();
	movieRect.top += 5;
	movieRect.left += 5;
	movieRect.right -= 5;
	movieRect.bottom = movieRect.top;
	BPath path("");
	if( ((HApp*)be_app)->GetMovie("SilverWing Message Movie",path) == B_OK)
	{
		fMovieView = new MovieView(movieRect
								,"movie"
								,path.Path()
								,B_FOLLOW_TOP|B_FOLLOW_LEFT
								,B_WILL_DRAW);
		fMovieView->ResizeToPreferred();
		fMovieView->Loop(true);
		if(Bounds().Width() < fMovieView->Bounds().Width()+10 )
		{
			bg->ResizeBy(Bounds().Width()+48 - fMovieView->Bounds().Width(),0 );
			ResizeBy(Bounds().Width()+48 - fMovieView->Bounds().Width(),0 );
		}
		bg->ResizeBy(0,fMovieView->Bounds().Height());
		ResizeBy(0,fMovieView->Bounds().Height());
		bg->AddChild(fMovieView);
	}
	*/
	rect = Bounds();
	rect.top += 5;
	rect.left += 5;
	rect.right -= 5;
	rect.bottom = rect.top + 20;
	BBitmap *bitmap = ((HApp*)be_app)->GetIcon(icon);
	MsgIconView *iconView = new MsgIconView(rect,nick,bitmap);
	bg->AddChild(iconView);
	rect = Bounds();
	rect.left += 5;
	rect.right -= 5+B_V_SCROLL_BAR_WIDTH;
	rect.top += 25;//fMovieView->Bounds().Height()+10;//(fMovieView == NULL)?5:fMovieView->Bounds().Height()+5;
	rect.bottom = Bounds().bottom - 50;

	textview = new URLTextView(rect,"textview",B_FOLLOW_ALL,B_WILL_DRAW|B_NAVIGABLE);
	textview->MakeEditable(false);
	textview->SetWordWrap(true);
	BScrollView *scrollView = new BScrollView("scrollview",textview,B_FOLLOW_ALL,
													B_WILL_DRAW|B_FRAME_EVENTS,false,true,B_FANCY_BORDER);
	bg->AddChild(scrollView);
	rect.top = rect.bottom + 10;
	rect.bottom = rect.top + 20;
	rect.right += B_V_SCROLL_BAR_WIDTH;
	rect.left = rect.right - 80;
	BButton *btn;
	btn = new BButton(rect,"send",_("Reply"),new BMessage(MESSAGE_REPLY_MSG),B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT);
	bg->AddChild(btn);
	AddChild(bg);
	rect.OffsetBy(-90,0);
	btn = new BButton(rect,"ok",_("Close"),new BMessage(B_QUIT_REQUESTED),B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT);
	bg->AddChild(btn);
	rect.left = Bounds().left + 5;
	rect.right = rect.left + 80;
	btn = new BButton(rect,"chat",_("Message chat"),new BMessage(MESSAGE_CHAT_MSG),B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT);
	bg->AddChild(btn);

	InsertMessage(text);
	rect.OffsetBy(0,20);
	rect.left = Bounds().left + 5;
	rect.right = rect.left + 100;
	BStringView *stringview = new BStringView(rect,"time","",B_FOLLOW_BOTTOM|B_FOLLOW_LEFT_RIGHT);
	bg->AddChild(stringview);
	be_app->PostMessage(SOUND_SRVMSG_SND);
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HMsgWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case MESSAGE_REPLY_MSG:
		{
			BRect rect = RectUtils().CenterRect(SEND_MESSAGE_WIDTH,SEND_MESSAGE_HEIGHT);
			HSendMsgWindow *win = new HSendMsgWindow(rect,_("Send Message"),fSock);
			win->SetParentMessage(textview->Text());
			win->Show();
			break;
		}
	case MESSAGE_CHAT_MSG:
		{
			message->AddInt32("sock",fSock);
			message->AddString("text",textview->Text());
			be_app->PostMessage(message);

			this->PostMessage(B_QUIT_REQUESTED);
			break;
		}
	default:
		BWindow::MessageReceived(message);
	}
}

/***********************************************************
 * Insert message.
 ***********************************************************/
void
HMsgWindow::InsertMessage(const char* text)
{
	TextUtils utils;
	char *t = new char[strlen(text)+1];
	::memset(t,0,strlen(text)+1);
	::sprintf(t,"%s",text);
	int32 encoding;
	utils.ConvertReturnsToLF(t);
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
	if(encoding)
		utils.ConvertToUTF8(&t,encoding-1);
	BAutolock lock(this);
	if(lock.IsLocked())
	{
		textview->Insert(t);
	}
	delete[] t;
}

/***********************************************************
 * Set time
 ***********************************************************/
void
HMsgWindow::SetTime()
{
	BString title = "Time";
	title += ": ";
	time_t t = time(NULL);
	struct tm *stm = localtime(&t);
	char *buf = new char[1024];
	::memset(buf,0,1024);
	::sprintf(buf,"%.2d:%.2d:%.2d",stm->tm_hour,stm->tm_min,stm->tm_sec);
	title << buf;
	delete[] buf;
	BStringView *stringview = (BStringView*)FindView("time");
	stringview->SetText(title.String());
}
