#include "HAgreeWindow.h"
#include "RectUtils.h"
#include "TextUtils.h"
#include "HApp.h"
#include "HPrefs.h"
#include <Autolock.h>
#include <Button.h>
#include <ScrollView.h>

/***********************************************************
 * Constructor.
 ***********************************************************/
HAgreeWindow::HAgreeWindow(BRect rect,const char *name,const char* text)
	:BWindow(rect,name,B_DOCUMENT_WINDOW,B_ASYNCHRONOUS_CONTROLS|B_NOT_CLOSABLE)
{
	InitGUI(text);
	
	float min_width,min_height,max_width,max_height;
	GetSizeLimits(&min_width,&max_width,&min_height,&max_height);
	min_width = 200;
	min_height = 100;
	SetSizeLimits(min_width,max_width,min_height,max_height);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HAgreeWindow::~HAgreeWindow()
{
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HAgreeWindow::InitGUI(const char* text)
{
	BRect rect = Bounds();
	rect.OffsetTo(B_ORIGIN);
	BView *bg = new BView(rect,"bg",B_FOLLOW_ALL,0);
	bg->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	rect.left += 5;
	rect.right -= 5+B_V_SCROLL_BAR_WIDTH;
	rect.top += 5;
	rect.bottom -= 55;
	BRect textrect = rect;
	textrect.OffsetTo(B_ORIGIN);
	agreeview = new URLTextView(rect,"agreeview",B_FOLLOW_ALL,B_WILL_DRAW);
	agreeview->MakeEditable(false);
	agreeview->SetWordWrap(true);
	uint32 encode;
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encode);
	if(encode == 0)
		agreeview->SetFontAndColor(be_fixed_font);
	else
		agreeview->SetFontAndColor(be_plain_font);
	BScrollView *scrollView = new BScrollView("scrollview",agreeview,B_FOLLOW_ALL,
													B_WILL_DRAW,true,true,B_FANCY_BORDER);
	bg->AddChild(scrollView);
	rect.top = rect.bottom + 10+B_V_SCROLL_BAR_WIDTH;
	rect.bottom = rect.top + 20;
	
	rect.left = rect.right - 100;
	BButton *btn = new BButton(rect,"ok",_("OK"),new BMessage(B_QUIT_REQUESTED),B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT);
	btn->MakeDefault(true);
	bg->AddChild(btn);
	AddChild(bg);
/********* Agreement Insertion ****************/	
	InsertAgreeMessage(text);
}

/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
HAgreeWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{

	default:
		BWindow::MessageReceived(message);
	}
}

/***********************************************************
 * Insert agreement to view.
 ***********************************************************/
void
HAgreeWindow::InsertAgreeMessage(const char* text)
{
	char *t = new char[strlen(text)+1];
	::memset(t,0,strlen(text)+1);
	::memcpy(t,text,strlen(text));
	int32 encoding;
	TextUtils utils;
	utils.ConvertReturnsToLF(t);
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
	if(encoding)
		utils.ConvertToUTF8(&t,encoding-1);
	BAutolock lock(this);
	if( lock.IsLocked() )
	{
		agreeview->Insert(t);
		agreeview->Insert("\n");
	}
	delete[] t;
}

/***********************************************************
 * QuitRequested
 ***********************************************************/
bool
HAgreeWindow::QuitRequested()
{
	return BWindow::QuitRequested();
}