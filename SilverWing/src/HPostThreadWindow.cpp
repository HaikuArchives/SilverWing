#include <TextControl.h>
#include <ClassInfo.h>
#include <ScrollView.h>
#include <Button.h>

#include "HPostThreadWindow.h"
#include <santa/Colors.h>
#include "CTextView.h"
#include "HApp.h"
#include "HPrefs.h"
#include "TextUtils.h"

/***********************************************************
 * Contructor.
 ***********************************************************/
HPostThreadWindow::HPostThreadWindow(BRect rect,const char* name,const char* category,const char* subject,uint16 reply,uint16 parent)
	:BWindow(rect,name,B_TITLED_WINDOW,B_ASYNCHRONOUS_CONTROLS),fReply(reply),fParent(parent),fCategory(category)
{
	InitGUI();
	BTextControl *textcontrol = cast_as(FindView("subject"),BTextControl);
	textcontrol->SetText(subject);
	textcontrol->MakeFocus(true);
	this->AddShortcut(B_RETURN,0,new BMessage(POST_MESSAGE));
	this->AddShortcut(B_ESCAPE,0,new BMessage(B_QUIT_REQUESTED));
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HPostThreadWindow::~HPostThreadWindow()
{

}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HPostThreadWindow::InitGUI()
{
	BView *bg = new BView(Bounds(),"bgview",B_FOLLOW_ALL,B_WILL_DRAW);
	bg->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect rect = Bounds();
	rect.top += 10;
	rect.left += 5;
	rect.right -= 5;
	rect.bottom = rect.top + 15;
	BTextControl *textcontrol = new BTextControl(rect,"subject",_("Subject:"),"",NULL,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	textcontrol->SetDivider(bg->StringWidth(_("Subject:")) + 2);
	bg->AddChild(textcontrol);
	rect.top = rect.bottom + 10;
	rect.right -= B_V_SCROLL_BAR_WIDTH;
	rect.bottom = Bounds().bottom - 40;
	CTextView *textview = new CTextView(rect,"textview",B_FOLLOW_ALL,B_WILL_DRAW|B_NAVIGABLE);
	BScrollView *scrollview = new BScrollView("scrollview",textview,B_FOLLOW_ALL,B_WILL_DRAW,false,true);
	bg->AddChild(scrollview);
	rect.left = rect.right - 80;
	rect.top = rect.bottom + 10;
	rect.bottom = Bounds().bottom - 5;
	BButton *button = new BButton(rect,"post",_("Post"),new BMessage(POST_MESSAGE),B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM);
	bg->AddChild(button);
	this->AddChild(bg);
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HPostThreadWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case POST_MESSAGE:
	{
		CTextView *view = (CTextView*)FindView("textview");
		BTextControl *control = (BTextControl*)FindView("subject");
		if(view->TextLength() > 0 && strlen(control->Text()) > 0)
		{
			TextUtils utils;;
			char* subject = new char[strlen(control->Text())+1];
			::strcpy(subject,control->Text());
			int32 encoding;
			((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
			if(!encoding)
				utils.ConvertFromUTF8(&subject,encoding-1);

			char* message = new char[view->TextLength()+1];
			::strcpy(message,view->Text());
			if(encoding)
				utils.ConvertFromUTF8(&message,encoding-1);
			utils.ConvertReturnsToCR(message);
			BMessage msg(POST_THREAD_MESSAGE);
			msg.AddString("message",message);
			msg.AddString("category",fCategory.String());
			msg.AddString("subject",subject);
			msg.AddInt16("reply_thread",fReply);
			msg.AddInt16("parent_thread",fParent);
			be_app->PostMessage(&msg);
			delete[] subject;
			delete[] message;

			this->PostMessage(B_QUIT_REQUESTED);
		}
	}
	default:
		BWindow::MessageReceived(message);
	}
}
