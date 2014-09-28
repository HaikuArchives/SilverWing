#include <TextControl.h>
#include <Button.h>
#include <ClassInfo.h>

#include "HDialog.h"
#include "Colors.h"
#include "TextUtils.h"
#include "HApp.h"

/**************************************************************
 * Constructor.
 **************************************************************/
HDialog::HDialog(BRect rect,const char* title,const char* textlabel,const char* buttonlabel)
	:BWindow(rect,title,B_FLOATING_WINDOW_LOOK,B_MODAL_APP_WINDOW_FEEL,B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
{
	InitGUI(textlabel, buttonlabel);
	BTextControl *control = cast_as(FindView("text"),BTextControl);
	control->MakeFocus(true);
}

/**************************************************************
 * Destructor.
 **************************************************************/
HDialog::~HDialog()
{
}

/**************************************************************
 *	InitGUI
 **************************************************************/void
HDialog::InitGUI(const char* textlabel,const char* buttonlabel)
{
	BView *bg = new BView(Bounds(),"bgview",B_FOLLOW_ALL,B_WILL_DRAW);
	bg->SetViewColor(BeBackgroundGrey);
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

/**************************************************************
 * MessageReceived.
 **************************************************************/
void
HDialog::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case OK_MESSAGE:
	{
		BTextControl *control = dynamic_cast<BTextControl*>(FindView("text"));
		if( strlen(control->Text()) > 0)
		{
			
			BMessage msg(OK_MESSAGE);
			msg.AddString("text",control->Text());
			if(fParent != NULL)
				fParent->PostMessage(&msg);
			this->PostMessage(B_QUIT_REQUESTED);
		}
		break;
	}	
	default:
		BWindow::MessageReceived(message);
	}
}