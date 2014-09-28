#include <iostream>
#include <String.h>
#include <ClassInfo.h>
#include <Autolock.h>
#include <Beep.h>
#include <media/PlaySound.h>
#include <MediaFiles.h>
#include <MediaRoster.h> 
#include <Debug.h>

#include "HApp.h"
#include "HWindow.h"
#include "HConnectWindow.h"
#include "HInfoWindow.h"
#include "HPrvChatWindow.h"
#include "HSendMsgWindow.h"
#include "HAgreeWindow.h"
#include "HPostNewsDialog.h"
#include "HArticleList.h"
#include "HPostThreadWindow.h"
#include "HAboutWindow.h"
#include "HSettingWindow.h"
#include "HTaskWindow.h"
#include "HWindow.h"
#include "HotlineClient.h"
#include "HPrefs.h"
#include "HFileWindow.h"
#include "HPrvChatList.h"
#include "AppUtils.h"
#include "HMsgWindow.h"
#include "HMsgChatWindow.h"
#include "RectUtils.h"

#include "hldat.h"

#define kMovieEventType "Films"

#define kChatEvent 		"SilverWing Chat"
#define kPostEvent 		"SilverWing Post News"
#define kMsgEvent   	"SilverWing Message"
#define kInviteEvent 	"SilverWing Invitation"
#define kLoginEvent	 	"SilverWing Login"
#define kLogoutEvent 	"SilverWing Logout"
#define kJoinEvent 		"SilverWing Join"
#define kFileDoneEvent 	"SilverWing Transfer Done"
#define kNotifyEvent	"SilverWing Chat contain nick"

#define kChatSound		"chat.aiff"
#define kPostSound		"newspost.aiff"
#define kMsgSound		"srvmsg.aiff"
#define kInviteSound	"invitation.aiff"
#define kLoginSound		"user_login.aiff"
#define kLogoutSound	"user_logout.aiff"
#define kJoinSound		"logged-in.aiff"
#define kFileDoneSound	"File transfer done.aiff"
#define kNotifySound	"invitation.aiff"

/***********************************************************
 * Constructor.
 ***********************************************************/
HApp::HApp():BApplication("application/x-vnd.takamatsu-silverwing")
	,fTaskWindow(NULL)
	,fMainWindow(NULL)
	,client(NULL)
	,fPrefs(NULL)
	,fIconResource(NULL)
	,fSoundPath("")
	,fHLDat(NULL)
	,fIsBadmoon(false)
{
	/********* Load preference *********/
	fPrefs = new HPrefs("SilverWing-Prefs");
	fPrefs->LoadPrefs();
	fPrefs->GetData("sound",&fEnableSound);

	OpenResource();

	/**** Start client looper *****/
	client = new HotlineClient();
	client->Run();
	/**** Show window ******/	
	BRect rect;
	fPrefs->GetData("task_rect",&rect);
	fTaskWindow = new HTaskWindow(rect);
	fTaskWindow->Show();
	bool iconfy;
	fPrefs->GetData("taskiconfy",&iconfy);
	if(iconfy)
		fTaskWindow->Hide();
	fPrefs->GetData("window_rect",&rect);
	fMainWindow = new HWindow(rect,"SilverWing");
	fMainWindow->Show();
	
	fServerVersion = 0;
	
}

/***********************************************************
 * Destructor
 ***********************************************************/
HApp::~HApp()
{
	int count = prvchatlist.CountItems();
	for(int i = 0;i < count;i++)
		delete prvchatlist.ItemAt(i);
		
	fPrefs->StorePrefs();	
	delete fPrefs;
	CloseResource();
}

/***********************************************************
 * Message Received
 ***********************************************************/
void
HApp::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	/**** Connect ****/
	case CONNECT_CONNECT_REQUESTED:
		{
			fMainWindow->PostMessage(MWIN_CLOSE_WINDDOWS);
	
			fLogin = fPassword = fAddress = "";
			fLogin = message->FindString("login");
			fPassword = message->FindString("password");
			fAddress = message->FindString("address");
			int16 port;
			message->FindInt16("port",(int16*)&port);
			BMessage msg(H_CONNECT_REQUESTED);
			msg.AddString("address",fAddress.String());
			msg.AddInt16("port",port);
			client->PostMessage(&msg);
			fServerVersion = 0;
			BTextView *chatview = cast_as(fMainWindow->FindView("chatview"),BTextView);
			if(fMainWindow->Lock())
			{
				fMainWindow->SetTitle("SilverWing");
				chatview->SetText("");
				fMainWindow->Unlock();
			}
			if(fIsBadmoon)
				fHLDat->Reset();
			break;
		}
	/**** Disconnect　*****/
	case MWIN_DISCONNECT:
		{	
			//client->PostMessage(H_CLOSE_REQUESTED);
			client->Close();
			break;
		}
	/**** Login ****/
	case H_CONNECT_SUCCESS:
		{
			
			BMessage msg(H_LOGIN_REQUESTED);
			msg.AddString("login",fLogin.String());
			msg.AddString("password",fPassword.String());
			msg.AddString("nick",this->Nick());
			msg.AddInt32("icon",this->Icon());
			client->PostMessage(&msg);
			break;
		}
	/**** Recv Global Chat****/
	case H_RCV_CHAT:
	{
			fMainWindow->PostMessage(message);
			break;
	}
	/**** Message chat *****/
	case MESSAGE_CHAT_MSG:
	{
		fMainWindow->PostMessage(message);
		break;
	}
	
	case H_CHANGE_USER:
	{
		fMainWindow->PostMessage(message);
		for(int i = 0 ; i < prvchatlist.CountItems();i++)
		{
			HPrvChatWindow* win = static_cast<HPrvChatWindow*>( prvchatlist.ItemAt(i) );
			if(win)	
				win->PostMessage(message);
		}
		break;
	}
	/****** Recv Private Chat ******/
	case H_RCV_PRV_CHAT:
	{
		uint32 pcref;
		message->FindInt32("pcref",(int32*)&pcref);
		//const char* text = message->FindString("text");
		message->what = PRVCHAT_RECEIVE_MSG;
		for(int i = 0 ; i < prvchatlist.CountItems();i++)
		{
			if( ((HPrvChatWindow*)prvchatlist.ItemAt(i))->Pcref() == pcref)
			{
				((HPrvChatWindow*)prvchatlist.ItemAt(i))->PostMessage(message);
				break;
			}
		}
		break;
	}
	/******* Join Private Chat ********/
	case H_RCV_JOIN_CHAT:
	{
		uint32 pcref;
		message->FindInt32("pcref",(int32*)&pcref);
		BRect rect;
		fPrefs->GetData("prvchat_window_rect",&rect);
		HPrvChatWindow *win = new HPrvChatWindow(rect
							,_("Private Chat"),pcref);
		win->Show();
		prvchatlist.AddItem(win);
		break;
	}
	/****** Change Private Chat User Info *********/
	case H_CHAT_USER_CHANGE:
	{
		uint32 pcref;
		uint16 sock,icon,color;
		HPrvChatWindow *win;
		message->FindInt32("pcref",(int32*)&pcref);
		message->FindInt16("sock",(int16*)&sock);
		message->FindInt16("icon",(int16*)&icon);
		message->FindInt16("color",(int16*)&color);
		const char* nick;
		if( message->FindString("nick",&nick) != B_OK)
			nick = "";
		
		for(int i = 0;i < prvchatlist.CountItems() ; i++)
		{
			win = static_cast<HPrvChatWindow*>(prvchatlist.ItemAt(i));
			if(win)
				win->PostMessage(message);
			//win->ChangeUserItem(sock,icon,color,nick);
		}
		
		break;
	}
	/******* Left Private Chat User ********/
	case H_CHAT_USER_LEAVE:
	{
		uint32 sock,pcref;
		message->FindInt32("pcref",(int32*)&pcref);
		message->FindInt32("sock",(int32*)&sock);
		HPrvChatWindow *win;
		this->PostMessage(SOUND_LOGOUT_SND);
		for(int i = 0;i < prvchatlist.CountItems() ; i++)
		{
			win = (HPrvChatWindow*)prvchatlist.ItemAt(i);
			if(win->Pcref() == pcref){
				win->RemoveUserItem(sock);
				break;
			}
		}
		break;
	}
	/******* Add Private Chat User *********/
	case H_PRV_CHAT_USER_ADD:
	{
		uint32 pcref;
		uint16 sock,icon,color;
		HPrvChatWindow *win = NULL;
		message->FindInt32("pcref",(int32*)&pcref);
		
		for(int i = 0;i < prvchatlist.CountItems() ; i++)
		{
			HPrvChatWindow *tmp = (HPrvChatWindow*)prvchatlist.ItemAt(i);
			if(tmp->Pcref() == pcref){
				win = tmp;
				break;
			}
		}
		if(win)
		{
			int32 count;
			type_code type;
			message->GetInfo("sock",&type,&count);
			for(register int32 i = 0;i < count;i++)
			{
				message->FindInt16("sock",i,(int16*)&sock);
				message->FindInt16("icon",i,(int16*)&icon);
				message->FindInt16("color",i,(int16*)&color);
				BString nick = "";
				if(message->HasString("nick",i) )
					nick = message->FindString("nick",i);
				win->AddUserItem(sock,icon,color,nick.String());
			}
			
			if(message->HasString("topic") )
			{
				BString topic = message->FindString("topic");
				
				BMessage msg(H_TOPIC_CHANGED);
				msg.AddString("text",topic.String());
				msg.AddInt32("pcref",pcref);
				win->PostMessage(&msg);
			}
		}
		break;
	}
	/******* top changed *********/
	case H_TOPIC_CHANGED:
	{
		uint32 pcref;
		const char* topic;
		pcref = message->FindInt32("pcref");
		HPrvChatWindow *win = NULL;
		if(message->FindString("text",&topic) == B_OK)
		{
			for(int i = 0;i < prvchatlist.CountItems() ; i++)
			{
				win = (HPrvChatWindow*)prvchatlist.ItemAt(i);
				if(win->Pcref() == pcref){
					win->PostMessage(message);
					break;
				}
			}
		}
		break;
	}
	/******* Send Kick User　*******/
	case MWIN_KICK_USER:
	{
		if(client->isConnected() == true)
			client->PostMessage(message);
		break;
	}
	/******* Get UserInfo ********/
	case MWIN_USER_INFO_MESSAGE:
	{
		if(client->isConnected() == true)
			client->PostMessage(message);
		break;
	}
	/******* Recv UserInfo　*******/
	case H_RCV_INFO:
	{
		const char* info = message->FindString("info");
		BString nick = _("User Info");
		nick << ": ";
		nick << message->FindString("nick");

		HInfoWindow *win = new HInfoWindow(RectUtils().CenterRect(300,300)
						,nick.String(),info);
		win->Show();
		break;
	}
	/********* Delete Private Chat　*********/
	case PRVCHAT_PRVCHAT_DELETE:
	{
		HPrvChatWindow *win;
		uint32 pcref;
		message->FindPointer("window",(void**)&win);
		BMessage msg(H_CHAT_OUT);
		pcref = win->Pcref();
		msg.AddInt32("pcref",(int32)pcref);
		client->PostMessage(&msg);
		prvchatlist.RemoveItem(win);
		break;
	}
	/******* Recv Agreement **********/
	case H_RCV_AGREEMENT:
	{
		const char* text = message->FindString("text");
		(new HAgreeWindow(RectUtils().CenterRect(350,300),_("Agreement"),text))->Show();
		break;
	}
	/******* Recv Global Message　**********/
	case H_RCV_GLOBAL_MSG:
	{
		const char* text = message->FindString("text");
		HAgreeWindow *win = new HAgreeWindow( RectUtils().CenterRect(300,200),_("Administrator message"),text);
		win->Show();
		be_app->PostMessage(SOUND_SRVMSG_SND);
		break;
	}
	case M_CLOSE_MSG_CHAT_WINDOW:
	/******* Recv Message *********/
	case H_RCV_MSG:
	{
		fMainWindow->PostMessage(message);
		break;
	}
	/************ Send request to get file list *****************/
	case FILE_FILE_REQUEST:
	{
		message->what = H_FILE_REQUESTED;
		//win->SetTrans(client->Trans());
		if(client->isConnected() == true)
			client->PostMessage(message);
		break;
	}
	/********** Recv File List *****************/
	case H_FILELIST_RECEIVED:
	{
		BWindow *win = fMainWindow->FileWindow();
		if(win != NULL)
			win->PostMessage(message);
		break;
	}
	/********* Send Chat **********/
	case MWIN_INVOKE_CHAT:
	{
		const char* text = message->FindString("text");
		BMessage msg(H_CHAT_REQUESTED);
		msg.AddString("text",text);
		if(client->isConnected() == true)
			client->PostMessage(&msg);
		break;
	}
	/******* Send Message **********/
	case SENDMSG_SEND_MSG:
	{
		uint32 sock;
		message->FindInt32("sock",(int32*)&sock);
		const char* text = message->FindString("text");
		BMessage msg(H_SEND_MSG);
		msg.AddString("text",text);
		msg.AddInt32("sock",(int32)sock);
		if(client->isConnected() == true)
			client->PostMessage(&msg);
		break;
	}
	/******** Recv News File *********/
	case MWIN_SEND_GET_NEWS_FILE:
	{
		if(client->isConnected() == true)
			client->PostMessage(H_NEWS_GET_FILE);
			//client->SendNewsDirList("");
		break;
	}
	/******** Recv News File **************/
	case H_RECEIVE_NEWS_FILE:
	{
		fMainWindow->PostMessage(message);
		break;
	}
	/******** Old news' posting  ***************/
	case POST_NEWS_POST:
	{
		const char* text = message->FindString("text");
		BMessage msg(H_NEWS_POST_NEWS);
		msg.AddString("text",text);
		if(client->isConnected() == true)
			client->PostMessage(&msg);
		break;
	}
	/******* Send request to get category list *********/
	case MWIN_NEWS_GET_CATEGORY:
	{
		const char* path = message->FindString("path");
		BMessage msg(H_NEW_GET_CATEGORY);
		msg.AddString("path",path);
		msg.AddInt32("index",message->FindInt32("index"));
		if(client->isConnected() == true)
			client->PostMessage(&msg);
		break;
	}
	/******** Recv category **************/
	case H_NEWS_RECEIVE_FOLDER:
	{
		fMainWindow->PostMessage(message);
		break;
	}
	/******* Article List Request ************/
	case H_NEWS_SEND_GET_ARTICLELIST:
	{
		if(client->isConnected() ==true)
			client->PostMessage(message);
		break;
	}
	/******* Article List Recieve *************/
	case H_NEWS_RECEIVE_ARTICLELIST:
	{
		fMainWindow->PostMessage(message);
		break;
	}
	/******* Send request to get an article ***********/
	case ARTICLE_REQUESTED:
	{
		const char* category = message->FindString("category");
		int16 thread = message->FindInt16("thread");
		BMessage msg(H_THREAD_REQUESTED);
		msg.AddString("category",category);
		msg.AddInt16("thread",thread);
		if(client->isConnected()== true)
			client->PostMessage(&msg);
		break;
	}
	/******* Recv an article **************/
	case H_RECEIVE_THREAD:
	{
		fMainWindow->PostMessage(message);
		break;
	}
	/****** Post an article ************/
	case POST_THREAD_MESSAGE:
	{
		message->what=H_POST_THREAD;
		if(client->isConnected() == true)
			client->PostMessage(message);
		break;
	}
	/******** Send Private Chat Message *****/
	case PRVCHAT_PRVCHAT_SENDMSG:
	{
		uint32 pcref;
		message->FindInt32("pcref",(int32*)&pcref);
		const char* text = message->FindString("text");
		BMessage msg(H_SEND_PRV_CHAT_MSG);
		msg.AddString("text",text);
		msg.AddInt32("pcref",(int32)pcref);
		client->PostMessage(&msg);
		break;
	}
	/****** Create Private Chat ********/
	case MWIN_PRV_CREATE_MESSAGE:
	{
		uint32 sock,pcref;
		message->FindInt32("pcref",(int32*)&pcref);
		message->FindInt32("sock",(int32*)&sock);
		BMessage msg(H_CHAT_INVITE);
		msg.AddInt32("pcref",pcref);
		msg.AddInt32("sock",sock);
		client->PostMessage(&msg);
		break;
	}
	/******* Private Chat invitation **********/
	case PRVLIST_CHAR_INVITE:
	{
		uint32 sock,pcref;
		message->FindInt32("pcref",(int32*)&pcref);
		message->FindInt32("sock",(int32*)&sock);
		BMessage msg(H_PRV_CHAT_INVITE);
		msg.AddInt32("pcref",pcref);
		msg.AddInt32("sock",sock);
		client->PostMessage(&msg);
		break;
	}
	/*********** Change nick name **************/	
	case M_SETTING_CHANGED:
	{
		const char* nick = message->FindString("nick");
		uint32 icon = message->FindInt32("icon");
		
		this->SetNick(nick);
		this->SetIcon(icon);
		
		if(client->isConnected())
		{
			BMessage msg(H_SEND_USER_CHAGED);
			msg.AddString("nick",nick);
			msg.AddInt32("icon",icon);
			client->PostMessage(&msg);
		}
			
		break;
	}
	/*********** Change Chat font *************/
	case M_SET_CHAT_FONT_MSG:
	{
		this->MainWindow()->PostMessage(message);
		int32 count = prvchatlist.CountItems();
		for(register int32 i = 0;i < count;i++)
		{
			HPrvChatWindow *win = (HPrvChatWindow*)prvchatlist.ItemAt(i);
			win->PostMessage(message);
		}
		break;
	}
	/******* ServerVersion **********/
	case H_SERVER_VERSION_RECEIVED:
	{
		const char* name = message->FindString("name");
		if(strlen(name) != 0)
		{
			BString title = "SilverWing";
			title << " < ";
			title << name;
			title << " >";
			fMainWindow->SetTitle(title.String());
		}
		message->FindInt32("version",(int32*)&fServerVersion);

		PRINT(("Server Version: %d\n" , fServerVersion ));
		break;
	}
	/******** Free resource *********/
	case M_RESET_RESOURCE:
	{
		if(fIsBadmoon)
			fHLDat->Reset();
		else{
			CloseResource();
			OpenResource();
		}
		PRINT(("Reset resources\n"));
		break;
	}
	/****** Play Sound **********/
	case SOUND_CHAT_SND:
	case SOUND_SRVMSG_SND:
	case SOUND_LOGGEDIN_SND:
	case SOUND_LOGIN_SND:
	case SOUND_LOGOUT_SND:
	case SOUND_INVITE_SND:
	case SOUND_POST_NEWS_SND:
	case SOUND_FILE_DOWN_SND:
	case SOUND_NOTIFY_SND:
	{
		//soundplayer->PostMessage(message);
		if(fEnableSound)
			this->PlaySound(message->what);
		break;
	}
	default:
		BApplication::MessageReceived(message);
	}
}


/***********************************************************
 * RefsReceived
 ***********************************************************/
void
HApp::RefsReceived(BMessage *message)
{	
	// Connect request from Tracker
	if(!message->HasRef("refs"))
	{
		message->what = CONNECT_CONNECT_REQUESTED;
		this->PostMessage(message);
	}else{
		entry_ref ref;
 		message->FindRef("refs",&ref);
		BFile file(&ref,B_READ_ONLY);
		if(file.InitCheck() == B_OK)
		{
			BString port= "";
			int16 iport;
			BMessage msg;
			msg.Unflatten(&file);
			const char* host = msg.FindString("address");
			const char* login = msg.FindString("login");	
			const char* pass = msg.FindString("password");
			iport = msg.FindInt16("port");
			int32 d = iport;
			port << d;

			BMessage pmsg(CONNECT_CONNECT_REQUESTED);
			pmsg.AddString("address",host);
			pmsg.AddString("login",login);
			pmsg.AddString("password",pass);
			pmsg.AddInt16("port",(int16)iport);
			this->PostMessage(&pmsg);
		}
	}
}

/***********************************************************
 * IsConnected
 ***********************************************************/
bool
HApp::IsConnected() const
{
	return client->isConnected();
}


/***********************************************************
 * QuitRequested
 ***********************************************************/
bool
HApp::QuitRequested()
{
	fTaskWindow->PostMessage(B_QUIT_REQUESTED);
	client->Close();
	client->PostMessage(B_QUIT_REQUESTED);
	fPrefs->StorePrefs();
	return BApplication::QuitRequested();
}


/***********************************************************
 * Play sound. play SE by type.
 ***********************************************************/
void
HApp::PlaySound(uint32 type)
{
	switch(type)
	{
	case SOUND_CHAT_SND:
		system_beep(kChatEvent);
		break;
	case SOUND_LOGGEDIN_SND:
		system_beep(kJoinEvent);
		break;
	case SOUND_LOGIN_SND:
		system_beep(kLoginEvent);
		break;	
	case SOUND_LOGOUT_SND:
		system_beep(kLogoutEvent);
		break;
	case SOUND_INVITE_SND:
		system_beep(kInviteEvent);
		break;
	case SOUND_SRVMSG_SND:
		system_beep(kMsgEvent);
		break;
	case SOUND_POST_NEWS_SND:
		system_beep(kPostEvent);
		break;
	case SOUND_FILE_DOWN_SND:
		system_beep(kFileDoneEvent);
		break;
	case SOUND_NOTIFY_SND:
		system_beep(kNotifyEvent);
		break;
	}
}


/***********************************************************
 * Get nick name from preference.
 ***********************************************************/
const char*
HApp::Nick() const
{
	const char* nick;
	fPrefs->GetData("nick",&nick);
	return nick;
}


/***********************************************************
 * Unused functions. 
 ***********************************************************/
BBitmap*
HApp::GetIcon(int16 icon)
{
	BAutolock lock((BLooper*)this);

	if(!fIsBadmoon)
	{
		if(!fIconResource)
			return NULL;
		size_t length;
		if( !fIconResource->HasResource(B_RAW_TYPE,icon) )
			if( fIconResource->HasResource(B_RAW_TYPE,128) )
				icon = 128;
	
		const char* data = (const char*)fIconResource->LoadResource(B_RAW_TYPE,icon,&length);
	
		if(data != NULL)
		{
			BMessage* theMesg = new BMessage();
			BArchivable* theObj = NULL;
			if(theMesg->Unflatten(data) == B_OK) {
				theObj = instantiate_object(theMesg);
			}
			delete theMesg;
			return cast_as(theObj,BBitmap);
		}
		return NULL;
	}else{
		BBitmap *bitmap = fHLDat->GetBitmap(icon);
		if(!bitmap)
			bitmap = fHLDat->GetBitmap(128);
		return bitmap;
	}
}	


/***********************************************************
 * Get user icon from preference.
 ***********************************************************/
int32
HApp::Icon() const
{
	int32 icon;
	fPrefs->GetData("icon",&icon);
	return icon;
}

/***********************************************************
 * Save window rect.
 ***********************************************************/
void
HApp::SaveRect(const char* name,BRect rect)
{
	fPrefs->SetData(name,rect);
	fPrefs->StorePrefs();
}

/***********************************************************
 * OpenResource
 ***********************************************************/
void
HApp::OpenResource()
{
	//====== Open Resource ======
	//if(fIconResource)
	//	return;
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("UserIcons");
	BString filename;
	path.Append("hotline.rsrc");
	BFile file(path.Path(),B_READ_ONLY);
	if(file.InitCheck() != B_OK)
		return;
	fIconResource = new BResources(&file);
	size_t len;
	int32  id;
	if( fIconResource->GetResourceInfo(B_INT32_TYPE,"numicons",&id,&len) )
	{
		bool which;
		fPrefs->GetData("preload",&which);
		if(which)
			fIconResource->PreloadResourceType(B_RAW_TYPE);
	}else{
		fHLDat = new HLDat(path.Path());
		fHLDat->Init();
		fIsBadmoon = true;
	}
}

/***********************************************************
 * CloseResource
 ***********************************************************/
void
HApp::CloseResource()
{
	delete fIconResource;
	fIconResource = NULL;
	delete fHLDat;
}


/***********************************************************
 * Load window rect from preference.
 ***********************************************************/
void
HApp::LoadRect(const char* name,BRect *rect)
{
	fPrefs->GetData(name,rect);
}


/***********************************************************
 * Set user nick name to preference.
 ***********************************************************/
void
HApp::SetNick(const char* nick)
{
	fPrefs->SetData("nick",nick);
	fPrefs->StorePrefs();
}


/***********************************************************
 * Set user icon to preference.
 ***********************************************************/
void
HApp::SetIcon(int32 icon)
{
	fPrefs->SetData("icon",icon);
	fPrefs->StorePrefs();
}


/***********************************************************
 * Application about window.
 ***********************************************************/
void
HApp::AboutRequested()
{
	(new HAboutWindow("SilverWing",
			__DATE__,
			"Created by Atsushi Takamatsu @ Sapporo,Japan.\n\nSpecial thanks to bedevcity.com members.\n",
			"http://hp.vector.co.jp/authors/VA013465/garden_e.html",
			"E-Mail: atsushi@io.ocn.ne.jp"))->Show();
}

/***********************************************************
 * Install bookmark mime.
 ***********************************************************/
void
HApp::InitMimes()
{
	//********** Bookmark Mime ****************//
	BMimeType bookmarks("text/hotline-bookmarks");
	if( bookmarks.InitCheck() != B_OK)
	{
		PRINT(("Could not initialize mime type\n"));
		return;
	}
	if(!bookmarks.IsInstalled())
	{
		bookmarks.SetPreferredApp("application/x-vnd.takamatsu-silverwing");
		bookmarks.SetShortDescription("Hotline Bookmarks");
		bookmarks.Install();	
	}
}

/***********************************************************
 * Add system sound events
 ***********************************************************/
void
HApp::AddSoundEvents()
{
	BPath app_path = AppUtils().GetAppDirPath(be_app);
	BMediaFiles mfiles;
	entry_ref ref;
	
	const char *kEvents[] = {kChatEvent
							,kPostEvent
							,kMsgEvent
							,kInviteEvent
							,kLoginEvent
							,kLogoutEvent
							,kJoinEvent
							,kFileDoneEvent
							,kNotifyEvent};
	const char *kSounds[] = {kChatSound
							,kPostSound
							,kMsgSound
							,kInviteSound
							,kLoginSound
							,kLogoutSound
							,kJoinSound
							,kFileDoneSound
							,kNotifySound};
	const int32 kCountEvents = 9;
	// sounds
	for(int32 i = 0;i < kCountEvents ;i++)
	{
		if( mfiles.GetRefFor(BMediaFiles::B_SOUNDS,kEvents[i],&ref) == B_ENTRY_NOT_FOUND)
		{
			::add_system_beep_event(kEvents[i]);
			BPath path = app_path;
			path.Append("Sounds");
			path.Append(kSounds[i]);
		
			if(path.InitCheck() != B_OK)
			{
				PRINT(("Not Found\n" ));
			}else{
				PRINT(( "Sound event added:%s\n" , path.Path() ));
				::get_ref_for_path(path.Path(),&ref);
			
				mfiles.SetRefFor(BMediaFiles::B_SOUNDS,kEvents[i],ref);
			}
		}
	}	
/*	BPath path("/boot/optional/movies/Mime1");
	::get_ref_for_path(path.Path(),&ref);
	if(mfiles.GetRefFor(kMovieEventType,"SilverWing Message Movie",&ref) == B_ENTRY_NOT_FOUND)
	{
		mfiles.SetRefFor(kMovieEventType,"SilverWing Message Movie",ref);
	}*/
}

/***********************************************************
 * Get movie
 ***********************************************************/
status_t
HApp::GetMovie(const char* event,BPath &out)
{
	BMediaFiles mfiles;
	entry_ref ref;
	
	status_t err = mfiles.GetRefFor(kMovieEventType,event,&ref);
	if(err == B_OK)
		out.SetTo(&ref);
	return err;
}

/***********************************************************
 * ReadyToRun
 ***********************************************************/
void
HApp::ReadyToRun()
{
	AddSoundEvents();
	InitMimes();	
}
/***********************************************************
 * main
 ***********************************************************/
int main()
{
	HApp app;
	app.Run();
	return 0;
}