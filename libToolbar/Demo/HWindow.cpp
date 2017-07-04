#include "HWindow.h"
#include "HApp.h"
#include "HToolbar.h"
#include "HToolbarButton.h"

HWindow::HWindow(BRect rect ,const char* name)
	:BWindow(rect,name,B_DOCUMENT_WINDOW,0)
{
	InitMenu();
	InitGUI();
}

HWindow::~HWindow()
{

}

void
HWindow::InitGUI()
{
/****** Construct toolbar *********/
	BRect toolrect = this->Bounds();
	toolrect.top += (KeyMenuBar()->Bounds()).Height();
	// toolbar's height is always 30.
	toolrect.bottom = toolrect.top + 50;
	// extend both side.
	toolrect.right += 2;
	toolrect.left -= 2;
	
	HToolbar *toolbar = new HToolbar(toolrect,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	toolbar->UseLabel(true);
	toolbar->AddButton("test1",this->GetBitmapResource('BBMP',"BMP:POORMAN"),new BMessage(M_TEST1_MESSAGE),"This is a TEST1 button");
	toolbar->AddButton("test2",this->GetBitmapResource('BBMP',"BMP:MAIL"),new BMessage(M_TEST2_MESSAGE),"This is a TEST2 button");
	toolbar->AddSpace();
	toolbar->AddButton("test3",this->GetBitmapResource('BBMP',"BMP:CODYCAM"),new BMessage(M_TEST3_MESSAGE),"This is a TEST3 button");
	toolbar->AddButton("test4",this->GetBitmapResource('BBMP',"BMP:CAMERA"),new BMessage(M_TEST4_MESSAGE),"");	
	this->AddChild(toolbar);
	
	toolrect.OffsetBy(0,50);
	toolrect.bottom = toolrect.top + 30;
	toolbar = new HToolbar(toolrect,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	toolbar->AddButton("test1",this->GetBitmapResource('BBMP',"BMP:POORMAN"),new BMessage(M_TEST1_MESSAGE),"This is a TEST1 button");
	toolbar->AddButton("test2",this->GetBitmapResource('BBMP',"BMP:MAIL"),new BMessage(M_TEST2_MESSAGE),"This is a TEST2 button");
	toolbar->AddSpace();
	toolbar->AddButton("test3",this->GetBitmapResource('BBMP',"BMP:CODYCAM"),new BMessage(M_TEST3_MESSAGE),"This is a TEST3 button");
	toolbar->AddButton("test4",this->GetBitmapResource('BBMP',"BMP:CAMERA"),new BMessage(M_TEST4_MESSAGE),"");	
	this->AddChild(toolbar);
/***********************************/
	BRect rect = Bounds();
	rect.top += (KeyMenuBar()->Bounds()).Height()+80;
	fStringView = new BStringView(rect,"stringview","");
	this->AddChild(fStringView);
}

void
HWindow::InitMenu()
{
	BMenuBar *menuBar = new BMenuBar(Bounds(),"MENUBAR");
	BMenu *menu;
	BMenuItem *item;
	
	menu = new BMenu("File");
	item = new BMenuItem("Quit",new BMessage(B_QUIT_REQUESTED),'Q',0);
	item->SetTarget(this);
	menu->AddItem(item);
	menuBar->AddItem(menu);
	this->AddChild(menuBar);
}

void
HWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	// the test1 button was pressed.
	case M_TEST1_MESSAGE:
	{
		this->SetText("TEST1 button");
		break;
	}
	// the test2 button was pressed.
	case M_TEST2_MESSAGE:
	{
		this->SetText("TEST2 button");
		break;
	}
	// the test3 button was pressed.
	case M_TEST3_MESSAGE:
	{
		this->SetText("TEST3 button");
		break;
	}
	// the test4 button was pressed.
	case M_TEST4_MESSAGE:
	{
		this->SetText("TEST4 button");
		break;
	}
	// Update toolbar buttons.
	case M_UPDATE_TOOLBUTTON:
	{
		this->UpdateToolbarButtons(message);
		break;
	}
	default:
		BWindow::MessageReceived(message);
	}
}

void
HWindow::SetText(const char* text)
{
	BAutolock lock(this);
	if(lock.IsLocked())
		fStringView->SetText(text);
}

void
HWindow::UpdateToolbarButtons(BMessage *message)
{
	const char* button_name = message->FindString("name");
	void* data;
	message->FindPointer("pointer",&data);
	HToolbarButton *button = static_cast<HToolbarButton*>(data);
	if(::strcmp(button_name,"test3") == 0)
	{
		button->SetEnabled(false);	
	}
}

bool
HWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

BBitmap*
HWindow::GetBitmapResource(type_code type,const char* name)
{
	size_t len = 0;
	BResources *rsrc = BApplication::AppResources();
	const void *data = rsrc->LoadResource(type, name, &len);

	if (data == NULL) {
		return NULL;
	}
	
	BMemoryIO stream(data, len);
	
	// Try to read as an archived bitmap.
	stream.Seek(0, SEEK_SET);
	BMessage archive;
	status_t err = archive.Unflatten(&stream);
	if (err != B_OK)
		return NULL;

	BBitmap* out = new BBitmap(&archive);
	if (!out)
		return NULL;

	err = (out)->InitCheck();
	if (err != B_OK) {
		delete out;
		out = NULL;
	}
	
	return out;
}