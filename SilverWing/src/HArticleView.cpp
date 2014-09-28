#include <Autolock.h>
#include <ClassInfo.h>
#include <StringView.h>
#include <ScrollView.h>

#include "HArticleView.h"
#include "Colors.h"
#include "URLTextView.h"
#include "TextUtils.h"
#include "HPrefs.h"
#include "HApp.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HArticleView::HArticleView(BRect rect,const char* name)
			:BView(rect,name,B_FOLLOW_ALL,B_WILL_DRAW)
{
	InitGUI();
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HArticleView::~HArticleView()
{
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HArticleView::InitGUI()
{
	this->SetViewColor(	ui_color(B_PANEL_BACKGROUND_COLOR));	
	BRect rect= Bounds();
	rect.left += 5;
	rect.right -= 5;
	rect.top += 5;
	rect.bottom = rect.top + 15;
	BRect labelRect = rect;
	labelRect.right = labelRect.left + 40;
	rect.left = labelRect.right + 3;
	BStringView *label = new BStringView(labelRect,"label",_("Subject:"));
	label->SetAlignment(B_ALIGN_RIGHT);
	AddChild(label);
	BStringView *control = new BStringView(rect,"subject","",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	control->SetViewColor(White);
	this->AddChild(control);
	rect.OffsetBy(0,20);
	labelRect.OffsetBy(0,20);
	label = new BStringView(labelRect,"label",_("Sender:"));
	label->SetAlignment(B_ALIGN_RIGHT);
	control = new BStringView(rect,"sender","",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	control->SetViewColor(White);
	this->AddChild(label);
	this->AddChild(control);
	rect.OffsetBy(0,20);
	labelRect.OffsetBy(0,20);
	label = new BStringView(labelRect,"label",_("Date:"));
	label->SetAlignment(B_ALIGN_RIGHT);
	control = new BStringView(rect,"date","",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	control->SetViewColor(White);
	this->AddChild(control);
	this->AddChild(label);	
	//rect.OffsetBy(0,20);
	
	BRect textRect = this->Bounds();
	textRect.right -= B_V_SCROLL_BAR_WIDTH+5;
	textRect.left += 5;
	textRect.top = rect.bottom + 5;
	URLTextView *textview = new URLTextView(textRect,"textview",B_FOLLOW_ALL,B_WILL_DRAW);
	textview->MakeEditable(false);
	textview->SetWordWrap(true);
	BScrollView *scroller = new BScrollView("Scroller",textview,B_FOLLOW_ALL,0,false,true);
	this->AddChild(scroller);

}

/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
HArticleView::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
	default:
		BView::MessageReceived(message);
	}
}

/***********************************************************
 * SetSender
 ***********************************************************/
void
HArticleView::SetSender(const char* text)
{
	BStringView *view = cast_as(FindView("sender"),BStringView);
	TextUtils utils;
	char *t = new char[strlen(text)+1];
	strcpy(t,text);
	int32 encoding;
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
	if(encoding)
		utils.ConvertToUTF8(&t,encoding-1);
	BString str=t;
	BAutolock lock(Window());
	if(lock.IsLocked())
	{
		view->SetText(str.String());
	}
	delete[] t;
}	

/***********************************************************
 * SetSubject
 ***********************************************************/
void
HArticleView::SetSubject(const char* text)
{
	BStringView *view = cast_as(FindView("subject"),BStringView);
	TextUtils utils;
	char *t = new char[strlen(text)+1];
	strcpy(t,text);
	int32 encoding;
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
	if(encoding)
		utils.ConvertToUTF8(&t,encoding-1);
	utils.ConvertReturnsToLF(t);
	
	BAutolock lock(Window());
	if(lock.IsLocked())
	{
		view->SetText(t);
	}
	delete[] t;
}	

/***********************************************************
 * SetDate
 ***********************************************************/
void
HArticleView::SetDate(const char* text)
{
	BStringView *view = cast_as(FindView("date"),BStringView);
	
	BAutolock lock(Window());
	if(lock.IsLocked())
	{
		view->SetText(text);
	}
}	

/***********************************************************
 * SetArticle
 ***********************************************************/
void
HArticleView::SetArticle(const char* text)
{
	CTextView *view = cast_as(FindView("textview"),CTextView);
	TextUtils utils;
	char *t = new char[strlen(text)+1];
	::strcpy(t,text);
	utils.ConvertReturnsToLF(t);
	int32 encoding;
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
	if(encoding)
		utils.ConvertToUTF8(&t,encoding-1);
	BString str = t;
	
	BAutolock lock(Window());
	view->SetText(str.String());
	
	delete[] t;
}	

/***********************************************************
 * ResetAll
 ***********************************************************/
void
HArticleView::ResetAll()
{
	this->SetArticle("");
	this->SetSender("");
	this->SetSubject("");
	this->SetDate("");
}