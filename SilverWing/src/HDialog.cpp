#include "HDialog.h"
#include "Colors.h"
#include "TextUtils.h"
#include "HApp.h"
#include "HPrefs.h"
#include "HotlineClient.h"

#include <TextControl.h>
#include <ClassInfo.h>
#include <Button.h>

/***********************************************************
 * Constructor
 ***********************************************************/
HDialog::HDialog(BRect rect
				,const char* title
				,BMessage *message
				,const char* textlabel
				,const char* buttonlabel)
		:BWindow(rect
				,title
				,B_FLOATING_WINDOW_LOOK
				,B_MODAL_APP_WINDOW_FEEL
				,B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
	,fMessage(message)
{
	AddShortcut('W',0,new BMessage(B_QUIT_REQUESTED));
	AddShortcut(B_RETURN,0,new BMessage(OK_MESSAGE));
	InitGUI(textlabel, buttonlabel);
	BTextControl *control = cast_as(FindView("text"),BTextControl);
	control->MakeFocus(true);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HDialog::~HDialog()
{
	delete fMessage;
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HDialog::InitGUI(const char* textlabel,const char* buttonlabel)
{
	BView *bg = new BView(Bounds(),"bgview",B_FOLLOW_ALL,B_WILL_DRAW);
	bg->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect rect = Bounds();
	rect.top += 10;
	rect.left += 5;
	rect.right -= 5;
	rect.bottom = rect.top + 15;
	
	BTextControl *control = new BTextControl(rect,"text",textlabel,"",NULL);
	control->SetDivider(bg->StringWidth(textlabel) + 2);
	bg->AddChild(control);	
	rect.top = rect.bottom + 15;
	rect.left = rect.right - 80;
	rect.bottom = rect.top + 20;
	BButton *button = new BButton(rect,"button",buttonlabel,new BMessage(OK_MESSAGE));
	bg->AddChild(button);

	this->AddChild(bg);
}

/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
HDialog::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case OK_MESSAGE:
	{
		BTextControl *control = cast_as(FindView("text"),BTextControl);
		if( strlen(control->Text()) > 0)
		{
			char* text = new char[strlen(control->Text()) +1];
			strcpy(text,control->Text());
			int32 encoding;
			((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
			if(encoding)
				TextUtils().ConvertFromUTF8(&text,encoding-1);
			fMessage->AddString("text",text);
			delete[] text;
			((HApp*)be_app)->Client()->PostMessage(fMessage);
			this->PostMessage(B_QUIT_REQUESTED);
		}
		break;
	}	
	default:
		BWindow::MessageReceived(message);
	}
}