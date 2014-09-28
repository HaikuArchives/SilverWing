#include <TextControl.h>
#include <Button.h>
#include <ClassInfo.h>

#include "HFindWindow.h"
#include "HApp.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HFindWindow::HFindWindow(BRect rect,const char* name)
	:BWindow(rect,name,B_FLOATING_WINDOW_LOOK,B_NORMAL_WINDOW_FEEL,B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_ASYNCHRONOUS_CONTROLS)
{
	InitGUI();
	BTextControl *control = cast_as(FindView("text"),BTextControl);
	control->MakeFocus(true);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HFindWindow::~HFindWindow()
{
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HFindWindow::InitGUI()
{
	BRect rect = Bounds();
	BView *view = new BView(rect,"bgview",B_FOLLOW_ALL,B_WILL_DRAW);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	rect.left += 10;
	rect.right -= 10;
	rect.top += 10;
	rect.bottom = rect.top + 15;
	BTextControl *control = new BTextControl(rect,"text","Keyword:","",NULL);
	control->SetDivider(control->StringWidth("Keyword:")+2);
	view->AddChild(control);
	
	rect.top = rect.bottom + 10;
	rect.bottom = rect.top + 20;
	rect.left = rect.right - 70;
	BButton *btn = new BButton(rect,"searchBtn",_("Search"),new BMessage(M_SEARCH_MSG));
	btn->MakeDefault(true);
	view->AddChild(btn);
	this->AddChild(view);
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HFindWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case M_SEARCH_MSG:
	{
		const char* text = this->SearchText();
		message->AddString("text",text);
		fTarget->PostMessage(message);	
		this->PostMessage(B_QUIT_REQUESTED);
		break;
	}
	default:
		BWindow::MessageReceived(message);
	}
}

/***********************************************************
 * Set search text.
 ***********************************************************/
void
HFindWindow::SetSearchText(const char* text)
{
	BTextControl *control = cast_as(FindView("text"),BTextControl);
	if(control != NULL)
	{
		control->SetText(text);
	}
}

/***********************************************************
 * Return search text.
 ***********************************************************/
const char*
HFindWindow::SearchText()
{
	BTextControl *control = cast_as(FindView("text"),BTextControl);
	return control->Text();
}