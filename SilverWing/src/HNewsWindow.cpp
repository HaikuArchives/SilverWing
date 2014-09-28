#include <Autolock.h>
#include <ScrollView.h>
#include <ClassInfo.h>

#include "HNewsWindow.h"
#include "HToolbar.h"
#include "CTextView.h"
#include "HPostNewsDialog.h"
#include "RectUtils.h"
#include "ResourceUtils.h"
#include "HApp.h"
#include "URLTextView.h"
#include "HotlineClient.h"
#include "HPrefs.h"
#include "TextUtils.h"
#include "HFileCaption.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HNewsWindow::HNewsWindow(BRect rect, const char* name)
			:BWindow(rect,name,B_DOCUMENT_WINDOW,B_ASYNCHRONOUS_CONTROLS)
{
	InitGUI();
	this->AddShortcut('/',0,new BMessage(B_ZOOM));
	this->AddShortcut('P',0,new BMessage(NEWS_POST_MESSAGE));
	float min_width,min_height,max_width,max_height;
	GetSizeLimits(&min_width,&max_width,&min_height,&max_height);
	min_width = 150;
	min_height = 150;
	SetSizeLimits(min_width,max_width,min_height,max_height);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HNewsWindow::~HNewsWindow()
{
	
}

/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
HNewsWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case NEWS_POST_MESSAGE:
	{
		HPostNewsDialog *dig = new HPostNewsDialog(RectUtils().CenterRect(300,200),"Post");
		dig->Show();
		break;
	}
	case H_RECEIVE_POST_NEWS:
	{
		const char* text = message->FindString("text");
		this->AddPosted(text);
		break;
	}
	case NEWS_SET_NEWS:
	{
		fCaption->SetLabel("");
		fCaption->StopBarberPole();
		const char* text = message->FindString("text");
		this->SetNews(text);
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
HNewsWindow::InitGUI()
{
	BRect rect = this->Bounds();
	rect.bottom = 30;
	rect.left -= 2;
	rect.right += 2;
	HToolbar *toolbar = new HToolbar(rect,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);

	ResourceUtils utils;
	toolbar->AddButton("postbtn",utils.GetBitmapResource('BBMP',"BMP:POST"),new BMessage(NEWS_POST_MESSAGE),"Post article");
	this->AddChild(toolbar);
	rect = Bounds();
	rect.top += 31;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT;
	//rect.bottom -= B_H_SCROLL_BAR_HEIGHT;
	rect.right -= B_V_SCROLL_BAR_WIDTH;
	URLTextView *textview = new URLTextView(rect,"textview",B_FOLLOW_ALL,B_WILL_DRAW);
	textview->MakeEditable(false);
	textview->SetWordWrap(true);
	uint32 encode;
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encode);
	if(encode == 0)
		textview->SetFontAndColor(be_fixed_font);
	else
		textview->SetFontAndColor(be_plain_font);
	BScrollView *scrollview = new BScrollView("scrollview",textview,B_FOLLOW_ALL,B_WILL_DRAW,false,true);
	this->AddChild(scrollview);
	
	/********** Caption ***********/
	BRect captionframe = Bounds();
	captionframe.bottom+=2;
	captionframe.top = captionframe.bottom - B_H_SCROLL_BAR_HEIGHT -1;
	captionframe.right = Bounds().Width() - B_V_SCROLL_BAR_WIDTH +2;
	captionframe.left-=2;
	BBox *bbox = new BBox(captionframe,NULL,B_FOLLOW_BOTTOM);
	captionframe.OffsetTo(B_ORIGIN);
	captionframe.top += 2;
	captionframe.bottom -= 2;
	captionframe.right -= 2;
	captionframe.left += 2;
	fCaption = new HFileCaption(captionframe,"caption",NULL);
	fCaption->SetLabel(_("Getting News…"));
	bbox->AddChild(fCaption);
	AddChild(bbox);
}

/***********************************************************
 * SetNews
 ***********************************************************/
void
HNewsWindow::SetNews(const char* text)
{
	BAutolock lock(this);
	char *news_buf = new char[strlen(text)+1];
	::strcpy(news_buf,text);
	
	int32 encoding = 0;
	TextUtils utils;
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
	if(encoding)
		utils.ConvertToUTF8(&news_buf,encoding-1);
	utils.ConvertReturnsToLF(news_buf);
	
	CTextView *view = cast_as(this->FindView("textview"),CTextView);
	if(view != NULL)
	{
		view->SetText(news_buf);
	}
	delete[] news_buf;
}

/***********************************************************
 * Add posted news.
 ***********************************************************/
void
HNewsWindow::AddPosted(const char* text)
{
	BAutolock lock(this);
	CTextView *view = cast_as(this->FindView("textview"),CTextView);
	
	char *news_buf = new char[strlen(text)+1];
	::strcpy(news_buf,text);
	
	int32 encoding = 0;
	TextUtils utils;
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
	if(encoding)
		utils.ConvertToUTF8(&news_buf,encoding-1);
	utils.ConvertReturnsToLF(news_buf);
	
	if(view != NULL)
	{
		const char* old = view->Text();
		BString t = news_buf;
		t << old;
		view->SetText(t.String());
	}
	delete[] news_buf;
}

/***********************************************************
 * QuitRequested
 ***********************************************************/
bool
HNewsWindow::QuitRequested()
{
	fTarget->PostMessage(NEWS_CLOSE_WINDOW);
	((HApp*)be_app)->SaveRect("news_rect",this->Frame());
	return BWindow::QuitRequested();
}