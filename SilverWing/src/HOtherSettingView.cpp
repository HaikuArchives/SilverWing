#include <ClassInfo.h>
#include <TextControl.h>
#include <CheckBox.h>
#include <Button.h>
#include <Roster.h>
#include <Beep.h>

#include "HPrefs.h"
#include "NumberControl.h"
#include "HOtherSettingView.h"
#include "HApp.h"
#include "FolderPanel.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HOtherSettingView::HOtherSettingView(BRect rect)
	:BView(rect,"othersetting",B_FOLLOW_ALL,B_WILL_DRAW)
	,fFilePanel(NULL)
{
	InitGUI();
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HOtherSettingView::~HOtherSettingView()
{
	delete fFilePanel;
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HOtherSettingView::InitGUI()
{
	this->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect rect = Bounds();
	rect.left += 10;
	rect.right -= 10;
	rect.top += 10;
	rect.bottom = rect.top + 20;
	BCheckBox *checkBox = new BCheckBox(rect,"chatlog",_("Show leave&join chat"),NULL);
	bool which;
	((HApp*)be_app)->Prefs()->GetData("showlogin",&which);
	checkBox->SetValue(which);
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,25);
	checkBox = new BCheckBox(rect,"preload",_("Preload all icons into memory"),NULL);
	((HApp*)be_app)->Prefs()->GetData("preload",&which);
	checkBox->SetValue(which);
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,25);
	BRect tmp_rect = rect;
	tmp_rect.right = tmp_rect.left + 130;
	checkBox = new BCheckBox(tmp_rect,"sound",_("Enable sound"),NULL);
	((HApp*)be_app)->Prefs()->GetData("sound",&which);
	checkBox->SetValue(which);
	this->AddChild(checkBox);
	tmp_rect.OffsetBy(140,0);
	BButton *button = new BButton(tmp_rect,"config",_("Configure Sounds"),new BMessage(M_CONFIG_SOUND));
	AddChild(button);
	
	rect.OffsetBy(0,25);
	checkBox = new BCheckBox(rect,"refusechat",_("Refuse private chat invitation"),NULL);
	((HApp*)be_app)->Prefs()->GetData("refusechat",&which);
	checkBox->SetValue(which);
	this->AddChild(checkBox);
	
	/*rect.OffsetBy(0,25);
	checkBox = new BCheckBox(rect,"queue_download",_("Queue file transfers"),NULL);
	((HApp*)be_app)->Prefs()->GetData("queue_download",&which);
	checkBox->SetValue(which);
	this->AddChild(checkBox);
	*/
	rect.OffsetBy(0,25);
	checkBox = new BCheckBox(rect,"time_stamp",_("Timestamp in chat"),NULL);
	((HApp*)be_app)->Prefs()->GetData("timestamp",&which);
	checkBox->SetValue(which);
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,25);
	checkBox = new BCheckBox(rect,"taskiconfy",_("Auto show & hide task window"),NULL);
	((HApp*)be_app)->Prefs()->GetData("taskiconfy",&which);
	checkBox->SetValue(which);
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,25);
	checkBox = new BCheckBox(rect,"msgchat",_("Use message chat as default"),NULL);
	((HApp*)be_app)->Prefs()->GetData("msgchat",&which);
	checkBox->SetValue(which);
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,25);
	checkBox = new BCheckBox(rect,"filewindow",_("Use old style file window"),NULL);
	((HApp*)be_app)->Prefs()->GetData("single_window",&which);
	checkBox->SetValue(which);
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,25);
	BRect textRect = rect;
	textRect.right = textRect.left + StringWidth("Keep alive") + 80;
	checkBox = new BCheckBox(textRect,"keepalive",_("Keep alive"),new BMessage(M_KEEP_ALIVE_CHANGED));
	((HApp*)be_app)->Prefs()->GetData("keep_alive",&which);
	checkBox->SetValue(which);
	this->AddChild(checkBox);
	textRect.OffsetBy(textRect.Width(),0);
	textRect.right = textRect.left + 130;
	int32 interval;
	((HApp*)be_app)->Prefs()->GetData("interval",&interval);
	
	NumberControl *numControl = new NumberControl(textRect,"interval",_("Ping interval (min):"),interval,NULL);
	numControl->SetDivider(StringWidth(_("Ping interval (min):"))+5);
	this->AddChild(numControl);
	numControl->SetEnabled(which);
	rect.OffsetBy(0,25);
	rect.right -= 70;
	BTextControl *control = new BTextControl(rect,"download_path",_("Download folder:"),"",NULL);
	const char* download_path;
	((HApp*)be_app)->Prefs()->GetData("download_path",&download_path);
	control->SetDivider(StringWidth(_("Download folder:"))+5);
	control->SetText(download_path);
	this->AddChild(control);
	rect.left = rect.right + 5;
	rect.right = Bounds().right - 5;
	button = new BButton(rect,"select",_("Select"),new BMessage(M_SELECT_FOLDER));
	AddChild(button);
	button->SetTarget(this);
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HOtherSettingView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case M_KEEP_ALIVE_CHANGED:
	{
		BCheckBox *checkBox = cast_as(FindView("keepalive"),BCheckBox);
		BTextControl *textControl = cast_as(FindView("interval"),BTextControl);
		bool keep = checkBox->Value();
		if(keep)
			textControl->SetEnabled(true);
		else
			textControl->SetEnabled(false);
		break;
	}
	case M_CONFIG_SOUND:
	{
		entry_ref ref;
		if(be_roster->FindApp("application/x-vnd.takamatsu.sounds+",&ref) == B_OK)
			be_roster->Launch("application/x-vnd.takamatsu.sounds+");
		else
			be_roster->Launch("application/x-vnd.Be.SoundsPrefs");
		break;
	}
	case M_SELECT_FOLDER:
	{
		Window()->SetFeel(B_NORMAL_WINDOW_FEEL);
		BMessenger messenger(this,Window());
		if(!fFilePanel)
			fFilePanel = new FolderPanel(B_OPEN_PANEL,&messenger);
		fFilePanel->Show();
		break;
	}
	case B_REFS_RECEIVED:
	{
		Window()->SetFeel(B_MODAL_APP_WINDOW_FEEL);
		entry_ref ref;
		if(message->FindRef("refs",&ref) == B_OK)
		{
			BPath path(&ref);
			BTextControl *control = cast_as(FindView("download_path"),BTextControl);
			if(control)
				control->SetText(path.Path());
		}
		break;
	}
	default:
		BView::MessageReceived(message);
	}
}

/***********************************************************
 * Log user's login.
 ***********************************************************/
bool
HOtherSettingView::LogLogin()
{
	BCheckBox *checkBox = cast_as(FindView("chatlog"),BCheckBox);
	return checkBox->Value();
}

/***********************************************************
 * Preload user icons.
 ***********************************************************/
bool
HOtherSettingView::Preload()
{
	BCheckBox *checkBox = cast_as(FindView("preload"),BCheckBox);
	return checkBox->Value();
}

/***********************************************************
 * Refuse private chat invitation.
 ***********************************************************/
bool
HOtherSettingView::RefuseChat()
{
	BCheckBox *checkBox = cast_as(FindView("refusechat"),BCheckBox);
	return checkBox->Value();
}

/***********************************************************
 * Enable sound.
 ***********************************************************/
bool
HOtherSettingView::EnableSound()
{
	BCheckBox *checkBox = cast_as(FindView("sound"),BCheckBox);
	return checkBox->Value();
}

/***********************************************************
 * Queue download.
 ***********************************************************/
bool
HOtherSettingView::QueueDownload()
{
	BCheckBox *checkBox = cast_as(FindView("queue_download"),BCheckBox);
	return checkBox->Value();
}

/***********************************************************
 * Time stamp.
 ***********************************************************/
bool
HOtherSettingView::TimeStamp()
{
	BCheckBox *checkBox = cast_as(FindView("time_stamp"),BCheckBox);
	return checkBox->Value();
}

/***********************************************************
 * Task window iconfy.
 ***********************************************************/
bool
HOtherSettingView::TaskIconfy()
{
	BCheckBox *checkBox = cast_as(FindView("taskiconfy"),BCheckBox);
	return checkBox->Value();
}

/***********************************************************
 * Keep alive.
 ***********************************************************/
bool
HOtherSettingView::KeepAlive()
{
	BCheckBox *checkBox = cast_as(FindView("keepalive"),BCheckBox);
	return checkBox->Value();
}

/***********************************************************
 * Interval
 ***********************************************************/
int32
HOtherSettingView::Interval()
{
	NumberControl *numberControl = cast_as(FindView("interval"),NumberControl);
	return numberControl->Value();
}

/***********************************************************
 * Download path
 ***********************************************************/
const char*
HOtherSettingView::DownloadPath()
{
	BTextControl *control = cast_as(FindView("download_path"),BTextControl);
	return control->Text();
}

/***********************************************************
 * Message chat
 ***********************************************************/
bool
HOtherSettingView::MessageChat()
{
	BCheckBox *checkBox = cast_as(FindView("msgchat"),BCheckBox);
	return checkBox->Value();
}

/***********************************************************
 * Single window
 ***********************************************************/
bool
HOtherSettingView::SingleWindow()
{
	BCheckBox *checkBox = cast_as(FindView("filewindow"),BCheckBox);
	return checkBox->Value();
}