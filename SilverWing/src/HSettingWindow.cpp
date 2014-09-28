#include "HSettingWindow.h"
#include "HApp.h"
#include "HotlineClient.h"
#include "FontMenuItem.h"
#include "HUserSettingView.h"
#include "HOtherSettingView.h"
#include "HFontSettingView.h"
#include "HFireWallView.h"
#include "HPrefs.h"
#include "MAlert.h"
#include <TabView.h>
#include <Button.h>

/***********************************************************
 * Constructor.
 ***********************************************************/
HSettingWindow::HSettingWindow(BRect rect)
	:BWindow(rect,_("Preferences"),B_TITLED_WINDOW_LOOK,
		B_MODAL_APP_WINDOW_FEEL,
		B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_ASYNCHRONOUS_CONTROLS)
{
	InitGUI();
	this->AddShortcut(B_RETURN,0,new BMessage(M_APPLY_MESSAGE));	
	usersetting->SetNick( ((HApp*)be_app)->Nick() );
	usersetting->SetIcon( ((HApp*)be_app)->Icon() );
	this->AddShortcut('W',0,new BMessage(B_QUIT_REQUESTED));
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HSettingWindow::~HSettingWindow()
{

}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HSettingWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case M_APPLY_MESSAGE:
	{
		//********** UserSetting **********//
		const char* nick = usersetting->Nick();
		
		uint32 icon = usersetting->Icon();
		if(strlen(nick) > 0 && icon > 0)
		{
			BMessage msg(M_SETTING_CHANGED);
			msg.AddString("nick",nick);
			msg.AddInt32("icon",icon);
			be_app->PostMessage(&msg);
		}
		//******* OtherSetting **********//
		bool which = othersetting->LogLogin();
		((HApp*)be_app)->Prefs()->SetData("showlogin",which);
		which = othersetting->Preload();
		((HApp*)be_app)->Prefs()->SetData("preload",which);
		which = othersetting->RefuseChat();
		((HApp*)be_app)->Prefs()->SetData("refusechat",which);
		//which = othersetting->QueueDownload();
		//((HApp*)be_app)->Prefs()->SetData("queue_download",which);
		which = othersetting->EnableSound();
		((HApp*)be_app)->Prefs()->SetData("sound",which);
		((HApp*)be_app)->EnableSound(which);
		which = othersetting->TimeStamp();
		((HApp*)be_app)->Prefs()->SetData("timestamp",which);
		which = othersetting->TaskIconfy();
		((HApp*)be_app)->Prefs()->SetData("taskiconfy",which);
		which = othersetting->KeepAlive();
		((HApp*)be_app)->Prefs()->SetData("keep_alive",which);
		which = othersetting->MessageChat();
		((HApp*)be_app)->Prefs()->SetData("msgchat",which);
		which = othersetting->SingleWindow();
		((HApp*)be_app)->Prefs()->SetData("single_window",which);
		int32 n = othersetting->Interval();
		((HApp*)be_app)->Prefs()->SetData("interval",n);
		const char* download_path = othersetting->DownloadPath();
		BNode node(download_path);
		if(node.InitCheck() != B_OK)
		{
			(new MAlert(_("Error"),_("Download folder is invalid."),_("OK"),NULL
				,NULL,B_STOP_ALERT))->Go();
			break;
		}
		((HApp*)be_app)->Prefs()->SetData("download_path",download_path);
		//******* Font Setting **********//
		const char* family = fontsetting->FontFamily();
		const char* style = fontsetting->FontStyle();
		int32 size = fontsetting->FontSize();
		uint32 color = fontsetting->FontColor();
		uint32 bcolor = fontsetting->BackColor();
		uint32 nickcolor = fontsetting->NickColor();
		uint32 encoding = fontsetting->Encoding();
		uint32 urlcolor = fontsetting->URLColor();
		((HApp*)be_app)->Prefs()->SetData("font_family",family);
		((HApp*)be_app)->Prefs()->SetData("font_style",style);
		((HApp*)be_app)->Prefs()->SetData("font_size",size);
		((HApp*)be_app)->Prefs()->SetData("font_color",(int32)color);
		((HApp*)be_app)->Prefs()->SetData("back_color",(int32)bcolor);
		((HApp*)be_app)->Prefs()->SetData("encoding",(int32)encoding);
		((HApp*)be_app)->Prefs()->SetData("nick_color",(int32)nickcolor);
		((HApp*)be_app)->Prefs()->SetData("url_color",(int32)urlcolor);
		BMessage msg(M_SET_CHAT_FONT_MSG);
		msg.AddString("font_family",family);
		msg.AddString("font_style",style);
		msg.AddInt32("font_size",size);
		msg.AddInt32("font_color",color);
		msg.AddInt32("back_color",bcolor);
		msg.AddInt32("nick_color",nickcolor);
		//((HApp*)be_app)->MainWindow()->PostMessage(&msg);
		be_app->PostMessage(&msg);
		//******** Firewall setting **********//
		bool useSock5 = firewallsetting->UseFirewall();
		if(useSock5)
		{
			const char* server = firewallsetting->Server();
			uint32 port = firewallsetting->Port();
			((HApp*)be_app)->Prefs()->SetData("sock5",useSock5);
			((HApp*)be_app)->Prefs()->SetData("firewall",server);
			((HApp*)be_app)->Prefs()->SetData("firewall_port",(int32)port);
			bool auth = firewallsetting->Auth();
			if(auth)
			{
				const char* username = firewallsetting->Username();
				const char* password = firewallsetting->Password();
				((HApp*)be_app)->Prefs()->SetData("auth",auth);
				((HApp*)be_app)->Prefs()->SetData("firewall_user",username);
				((HApp*)be_app)->Prefs()->SetData("firewall_password",password);			
			}
		}else{
			((HApp*)be_app)->Prefs()->SetData("sock5",false);
			((HApp*)be_app)->Prefs()->SetData("auth",false);
		}	
		//this->PostMessage(B_QUIT_REQUESTED);
		break;
	}
	case M_GATHER_ICON_MSG:
	case M_LIST_CLICKED:
	{
		PostMessage(message,usersetting);
		break;
	}
	case k_font_menu_msg_type:
	{
		PostMessage(message,fontsetting);
		break;
	}
	case M_USE_SOCK5:
	case M_AUTH_REQ:
	{
		PostMessage(message,firewallsetting);
		break;
	}
	case M_KEEP_ALIVE_CHANGED:
	case M_CONFIG_SOUND:
	case M_SELECT_FOLDER:
	{
		PostMessage(message,othersetting);
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
HSettingWindow::InitGUI()
{
	BRect rect = Bounds();
	rect.bottom -= 35;
	BTabView *tabview = new BTabView(rect,"tabview");
	tabview->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BTab *tab;
	BRect frame = tabview->Bounds();
//*********** User Setting ******************/
	tab = new BTab();
	tabview->AddTab(usersetting = new HUserSettingView(frame),tab);
	tab->SetLabel(_("User"));
//********** Font Setting ******************/
	tab = new BTab();
	tabview->AddTab(fontsetting = new HFontSettingView(frame),tab);	
	tab->SetLabel(_("Font & Color"));
//********** Firewall Setting **************/
	tab = new BTab();
	tabview->AddTab(firewallsetting = new HFireWallView(frame),tab);
	tab->SetLabel(_("Firewall"));
//********** Others Setting ****************/	
	tab = new BTab();
	tabview->AddTab(othersetting = new HOtherSettingView(frame),tab);
	tab->SetLabel(_("Others"));
	this->AddChild(tabview);

	BRect bgrect = Bounds();
	bgrect.top = bgrect.bottom - 35;
	BView *bgview = new BView(bgrect,"bgview",B_FOLLOW_ALL,B_WILL_DRAW);
	bgview->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	bgrect.OffsetTo(B_ORIGIN);
	bgrect.top += 5;
	bgrect.right -= 10;
	bgrect.left = bgrect.right - 80;
	bgrect.bottom -= 5;

	BButton *button = new BButton(bgrect,"apply",_("Apply"),new BMessage(M_APPLY_MESSAGE));
	bgview->AddChild(button);
	this->AddChild(bgview);
}

/***********************************************************
 * QuitRequested
 ***********************************************************/
bool
HSettingWindow::QuitRequested()
{
	usersetting->RemoveAll();
	return BWindow::QuitRequested();
}