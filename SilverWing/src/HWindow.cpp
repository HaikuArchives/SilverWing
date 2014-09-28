#include <NodeMonitor.h>
#include <Autolock.h>
#include <ClassInfo.h>

#include "HWindow.h"
#include "RectUtils.h"
#include "Colors.h"
#include "HConnectWindow.h"
#include "HotlineClient.h"
#include "HSendMsgWindow.h"
#include "HApp.h"
#include "TextUtils.h"
#include "IconMenuItem.h"
#include "ResourceUtils.h"
#include "HToolbar.h"
#include "HToolbarButton.h"
#include "HCaption.h"
#include "HSettingWindow.h"
#include "HUserList.h"
#include "HUserItem.h"
#include "CTextView.h"
#include "HFileWindow.h"
#include "HNewsWindow.h"
#include "HNews15Window.h"
#include "MLTextControl.h"
#include "SplitPane.h"
#include "ChatLogView.h"
#include "HMsgWindow.h"
#include "HMsgChatWindow.h"
#include "HPrefs.h"
#include "HSingleFileWindow.h"
#include "TrackerUtils.h"
#include "MenuUtils.h"
#include "AppUtils.h"
#include "MAlert.h"

#define kSplitterPad 1

/***********************************************************
 * Constructor.
 ***********************************************************/
HWindow::HWindow(BRect rect,const char* name)	
			:BWindow(rect,name,B_DOCUMENT_WINDOW,B_ASYNCHRONOUS_CONTROLS)
			,fFileWindow(NULL)
			,fNewsWindow(NULL)
			,fNews15Window(NULL)
			,fSingleFileWindow(NULL)
			,fSingleWndMode(true)
{
	InitMenu();
	InitGUI();
	
	((HApp*)be_app)->Prefs()->GetData("single_window",&fSingleWndMode);
	
	StartWatchingBookmarks();
	AddShortcut('/',0,new BMessage(B_ZOOM));
	
	float min_width,min_height,max_width,max_height;
	GetSizeLimits(&min_width,&max_width,&min_height,&max_height);
	min_width = 300;
	min_height = 150;
	SetSizeLimits(min_width,max_width,min_height,max_height);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HWindow::~HWindow()
{
	::stop_watching(this);
	listview->SetInvocationMessage(NULL);
}


/***********************************************************
 * Setup GUIs.
 ***********************************************************/
void
HWindow::InitGUI()
{
	
	BRect rect = Bounds();
	BView *bg = new BView(rect,"bg",B_FOLLOW_ALL,B_WILL_DRAW);
	bg->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	rect.right -= 20+B_V_SCROLL_BAR_WIDTH;
	rect.left = rect.right - 140;
	rect.top += 55;
	rect.bottom -= 30;
	// Userlist
	listview = new HUserList(rect,"userlist");
	BScrollView *scrollView = new BScrollView("scrollview",listview,B_FOLLOW_RIGHT|B_FOLLOW_TOP_BOTTOM,
													B_WILL_DRAW|B_FRAME_EVENTS,false,true,B_FANCY_BORDER);
	listview->SetInvocationMessage(new BMessage(MWIN_SEND_MESSAGE));
	//ChatView
	BRect textrect = this->Bounds();
	textrect.top += 2;
	textrect.left += 2;
	textrect.right -= 2 + B_V_SCROLL_BAR_WIDTH;
	textrect.bottom -= 2 + 50;
	chatview = new ChatLogView(textrect,"chatview",B_FOLLOW_ALL,B_WILL_DRAW);
	chatview->SetStylable(true);
	// Font 
	BFont chatfont;
	int32 font_size;
	const char* font_family;
	const char* font_style;
	uint32 indexColor;
	uint8 col;
	((HApp*)be_app)->Prefs()->GetData("font_color",(int32*)&indexColor);
	col = indexColor;
	fChatColor.blue = col;
	col = indexColor >> 8;
	fChatColor.green = col;
	col = indexColor >> 16;
	fChatColor.red = col;
	
	((HApp*)be_app)->Prefs()->GetData("nick_color",(int32*)&indexColor);
	col = indexColor;
	fNickColor.blue = col;
	col = indexColor >> 8;
	fNickColor.green = col;
	col = indexColor >> 16;
	fNickColor.red = col;
	
	((HApp*)be_app)->Prefs()->GetData("font_family",&font_family);
	((HApp*)be_app)->Prefs()->GetData("font_style",&font_style);
	((HApp*)be_app)->Prefs()->GetData("font_size",&font_size);
	chatfont.SetSize(font_size);
	chatfont.SetFamilyAndStyle(font_family,font_style);
	//chatfont.SetFace(chatfont.Face()|B_UNDERSCORE_FACE);
	chatview->SetFontAndColor(&chatfont,B_FONT_ALL,&fChatColor);
	// Color
	((HApp*)be_app)->Prefs()->GetData("back_color",(int32*)&indexColor);
	rgb_color backColor;
	col = indexColor;
	backColor.blue = col;
	col = indexColor >> 8;
	backColor.green = col;
	col = indexColor >> 16;
	backColor.red = col;
	chatview->SetColorSpace(B_RGB32);
	chatview->SetViewColor(backColor);
	//
	chatview->MakeEditable(false);
	
	BScrollView* scrollView2 = new BScrollView("scrollview",chatview,B_FOLLOW_ALL,
												B_WILL_DRAW|B_FRAME_EVENTS,false,true,B_FANCY_BORDER);
//	chatbgview->AddChild(scrollView2);
		
	
	textrect.bottom = Bounds().bottom-2;
	textrect.top = textrect.bottom - 40;
	textrect.right = Bounds().right -2;
	BBox *backbox = new BBox(textrect,NULL,B_FOLLOW_TOP_BOTTOM|B_FOLLOW_LEFT_RIGHT);
	textrect.InsetBy(2,2);
	textrect.OffsetTo(B_ORIGIN);
	textrect.OffsetBy(1,1);
	
	chatmsg = new MLTextControl(textrect,"textview",MWIN_INVOKE_CHAT,B_FOLLOW_TOP_BOTTOM|B_FOLLOW_LEFT_RIGHT);
	backbox->AddChild(chatmsg);
//	chatbgview->AddChild(backbox);

	BRect rightrect = bg->Bounds();
	rightrect.top += (KeyMenuBar()->Bounds()).Height() + 30;
	//rightrect.left += 202+ B_V_SCROLL_BAR_WIDTH; 
	rightrect.bottom -= B_H_SCROLL_BAR_HEIGHT;
	fHSplitter = new SplitPane(rightrect,scrollView2,backbox,B_FOLLOW_ALL);
	fHSplitter->SetBarThickness(BPoint(0,6));
	fHSplitter->SetViewInsetBy(BPoint(kSplitterPad,kSplitterPad));
	fHSplitter->SetAlignment(B_HORIZONTAL);
	fHSplitter->SetBarAlignmentLocked(true);
	fHSplitter->SetResizeViewOne(true, true);
	int32 pos;
	((HApp*)be_app)->Prefs()->GetData("main_hbar_pos",&pos);
	fHSplitter->SetBarPosition(BPoint(0,pos));
	fVSplitter = new SplitPane(rightrect,fHSplitter,scrollView,B_FOLLOW_ALL);
	fVSplitter->SetBarThickness(BPoint(7,0));
	fVSplitter->SetAlignment(B_VERTICAL);
	fVSplitter->SetBarAlignmentLocked(true);
	fVSplitter->SetResizeViewOne(true, true);
	fVSplitter->SetViewInsetBy(BPoint(kSplitterPad,kSplitterPad));
	((HApp*)be_app)->Prefs()->GetData("main_vbar_pos",&pos);
	fVSplitter->SetBarPosition(BPoint(pos,0));
	bg->AddChild(fVSplitter);
	/****************** Toolbar **********************/
	BRect toolrect = Bounds();
	toolrect.top += (KeyMenuBar()->Bounds()).Height();
	toolrect.bottom = toolrect.top + 30;
	toolrect.right += 2;
	toolrect.left -= 2;

	ResourceUtils utils;
	
	HToolbar *toolbox = new HToolbar(toolrect,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	toolbox->AddButton("connectbtn",utils.GetBitmapResource('BBMP',"BMP:CONNECT"),new BMessage(MWIN_CONNECT),_("Connect"));
	toolbox->AddButton("disconnectbtn",utils.GetBitmapResource('BBMP',"BMP:DISCONNECT"),new BMessage(MWIN_DISCONNECT),_("Disconnect"));
	toolbox->AddSpace();
	toolbox->AddButton("msgbtn",utils.GetBitmapResource('BBMP',"BMP:MESSAGE"),new BMessage(MWIN_SEND_MESSAGE),_("Send message"));
	toolbox->AddButton("prvchatbtn",utils.GetBitmapResource('BBMP',"BMP:PRVCHAT"),new BMessage(MWIN_PRV_CREATE_MESSAGE),_("Create private chat"));	
	toolbox->AddButton("newsbtn",utils.GetBitmapResource('BBMP',"BMP:NEWS"),new BMessage(MWIN_NEWSMESSAGE),_("Show News"));	
	toolbox->AddButton("filebtn",utils.GetBitmapResource('BBMP',"BMP:FOLDER"),new BMessage(MWIN_FILE_REQUESTED),_("Show Files"));
	toolbox->AddButton("filetransbtn",utils.GetBitmapResource('BBMP',"BMP:TRANS"),new BMessage(MWIN_FILE_TRANSFER),_("Show Tasks"));		
	toolbox->AddButton("trackerbtn",utils.GetBitmapResource('BBMP',"BMP:TRACKER"),new BMessage(MWIN_TRACKER),_("Show Tracker"));	
	toolbox->AddSpace();
	toolbox->AddButton("infobtn",utils.GetBitmapResource('BBMP',"BMP:INFO"),new BMessage(MWIN_USER_INFO_MESSAGE),_("Get user infomation"));		
	toolbox->AddSpace();
	toolbox->AddButton("kickbtn",utils.GetBitmapResource('BBMP',"BMP:KICK"),new BMessage(MWIN_KICK_USER),_("Kick"));
	toolbox->AddButton("banbtn",utils.GetBitmapResource('BBMP',"BMP:BAN"),new BMessage(MWIN_BAN_USER),_("Ban"));
	bg->AddChild(toolbox);
	/****************** StatusBar ***********************/
	BRect statusrect = this->Bounds();
	statusrect.bottom += 2;
	statusrect.top = statusrect.bottom - B_H_SCROLL_BAR_HEIGHT -1;
	statusrect.right -= B_V_SCROLL_BAR_WIDTH-2;
	statusrect.left--;
	BBox *box = new BBox(statusrect,"status",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM,B_WILL_DRAW);

	statusrect.OffsetTo(B_ORIGIN);
	statusrect.top +=2;
	statusrect.bottom -= 1;
	statusrect.left += 7;
	statusrect.right -= 7;

	HCaption *view = new HCaption(statusrect,"info",listview);
	box->AddChild(view);
	bg->AddChild(box);	
	AddChild(bg);
}

/***********************************************************
 * Set up menus.
 ***********************************************************/
void
HWindow::InitMenu()
{
	BRect frame;
	frame = Bounds();
	frame.bottom = frame.top + 15;
	frame.OffsetTo(B_ORIGIN);
	BMenuBar *menuBar = new BMenuBar(frame,"MENUBAR");
	BMenu* aMenu; 
    
    ResourceUtils rutils;
    MenuUtils *utils = new MenuUtils();
	//------------------------ File Menu ---------------------------------
	 aMenu = new BMenu(_("File")); 
	 	utils->AddMenuItem(aMenu,_("Open Downloads Folder"),MWIN_OPEN_DOWNLOAD_FOLDER,this,this,0,0);
	 	utils->AddMenuItem(aMenu,_("Preferences…"),MWIN_PREFERENCE,this,this,'P',0);
   		aMenu->AddSeparatorItem();
   		utils->AddMenuItem(aMenu,_("About SilverWing…"),B_ABOUT_REQUESTED,be_app,be_app);
   		aMenu->AddSeparatorItem();
  		utils->AddMenuItem(aMenu,_("Quit"),B_QUIT_REQUESTED,this,this,'Q');

    menuBar->AddItem( aMenu );
	
    aMenu = new BMenu(_("Command")); 
   		utils->AddMenuItem(aMenu,_("Connect…"),MWIN_CONNECT,this,this,'K',0,
    		rutils.GetBitmapResource('BBMP',"BMP:CONNECT"));
   		utils->AddMenuItem(aMenu,_("Disconnect"),MWIN_DISCONNECT,this,this,0,0,
    		rutils.GetBitmapResource('BBMP',"BMP:DISCONNECT"));
   		aMenu->AddSeparatorItem();
   		utils->AddMenuItem(aMenu,_("Send Message"),MWIN_SEND_MESSAGE,this,this,0,0,
    		rutils.GetBitmapResource('BBMP',"BMP:MESSAGE"));
   		utils->AddMenuItem(aMenu,_("Private Chat"),MWIN_PRV_CREATE_MESSAGE,this,this,0,0,
    		rutils.GetBitmapResource('BBMP',"BMP:PRVCHAT"));
   		utils->AddMenuItem(aMenu,_("Get User Infomation"),MWIN_USER_INFO_MESSAGE,this,this,'I',0,
    		rutils.GetBitmapResource('BBMP',"BMP:INFO"));
    	
    	aMenu->AddSeparatorItem();
   		utils->AddMenuItem(aMenu,_("Kick User"),MWIN_KICK_USER,this,this,0,0,
    		rutils.GetBitmapResource('BBMP',"BMP:KICK"));
   		utils->AddMenuItem(aMenu,_("Ban User"),MWIN_BAN_USER,this,this,0,0,
    		rutils.GetBitmapResource('BBMP',"BMP:BAN"));
    		
   		aMenu->AddSeparatorItem();
   		utils->AddMenuItem(aMenu,_("News"),MWIN_NEWSMESSAGE,this,this,'N',0,
   			rutils.GetBitmapResource('BBMP',"BMP:NEWS"));
   		utils->AddMenuItem(aMenu,_("Files"),MWIN_FILE_REQUESTED,this,this,'F',0,
    		rutils.GetBitmapResource('BBMP',"BMP:FOLDER"));
    	utils->AddMenuItem(aMenu,_("Tasks"),MWIN_FILE_TRANSFER,this,this,0,0,
    		rutils.GetBitmapResource('BBMP',"BMP:TRANS"));
   		utils->AddMenuItem(aMenu,_("Tracker"),MWIN_TRACKER,this,this,'T',0,
    		rutils.GetBitmapResource('BBMP',"BMP:TRACKER"));
   		
   		
    menuBar->AddItem( aMenu );
    aMenu = new BMenu(_("Bookmarks"));
    	utils->AddMenuItem(aMenu,_("Add to Bookmarks…"),MWIN_ADDBOOKMARKS,this,this);
		utils->AddMenuItem(aMenu,_("Rescan Bookmarks"),MWIN_RESCANBOOKMARKS,this,this);
   		aMenu->AddSeparatorItem();
    menuBar->AddItem(aMenu);
    	
    this->AddChild(menuBar); 
    delete utils;
    InitBookmarks();
}

/***********************************************************
 * Set up bookmarks.
 ***********************************************************/
void
HWindow::InitBookmarks()
{
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Bookmarks");
	BDirectory dir( path.Path() );
	BEntry entry;
	BMenuBar* menuBar = this->KeyMenuBar();
	BMenu *menu = menuBar->SubmenuAt(2);
	status_t err = B_NO_ERROR;
	BResources *rsrc = BApplication::AppResources();
	ResourceUtils utils(rsrc);

	while( err == B_NO_ERROR )
	{
		err = dir.GetNextEntry( (BEntry*)&entry, TRUE );	
			
		if( entry.InitCheck() != B_NO_ERROR )
			break;
		if( entry.IsFile() )
		{
			char name[B_FILE_NAME_LENGTH+1];
			entry.GetName(name);
			BMessage *msg = new BMessage(CONNECT_BOOKMARK_MSG);
			msg->AddString("name",name);
			IconMenuItem *item = new IconMenuItem(name,msg,0,0,
    			utils.GetBitmapResource('BBMP',"BMP:CONNECT"));
			menu->AddItem(item);
		}
	}
}

/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
HWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	/********* Message chat *******/
	case MESSAGE_CHAT_MSG:
	{
		uint32 sock,icon;
		if(message->FindInt32("sock",(int32*)&sock) == B_OK)
		{
			BString nick;
			if(FindUser(sock,icon,nick))
			{
				const char* text;
				if(message->FindString("text",&text) == B_OK)
					MakeMessageChat(sock,icon,nick.String(),text);
				else
					MakeMessageChat(sock,icon,nick.String());
			}
		}
		break;
	}
	/********** Msg chat close **********/
	case M_CLOSE_MSG_CHAT_WINDOW:
	{
		void *data;
		if(message->FindPointer("pointer",&data) == B_OK)
		{
			HMsgChatWindow *win = static_cast<HMsgChatWindow*>(data);
			if(win)
				fMsgChatList.RemoveItem(win);
		}
		break;
	}
	/*********** Add User list ***************/
	case H_ADD_USER:
	{
		uint16 sock,icon,color;
		message->FindInt16("sock",(int16*)&sock);
		message->FindInt16("icon",(int16*)&icon);
		message->FindInt16("color",(int16*)&color);
		const char* nick;
		if(message->FindString("nick",&nick) != B_OK)
			nick = "";
		this->InsertUser(sock,icon,color,nick);
		break;
	}
	/****** Show connect window *******/
	case MWIN_CONNECT:
	{
		(new HConnectWindow(RectUtils().CenterRect(300.0,180.0),_("Connect"),false))->Show();
		break;	
	}
	/****** Open download folder *********/
	case MWIN_OPEN_DOWNLOAD_FOLDER:
	{
		const char* path;
		((HApp*)be_app)->Prefs()->GetData("download_path",&path);
		entry_ref ref;
		::get_ref_for_path(path,&ref);
		
		TrackerUtils().OpenFolder(ref);
		break;
	}
	/******* Close All Windows ***********/
	case MWIN_CLOSE_WINDDOWS:
	{
		if (fFileWindow != NULL)
			fFileWindow->PostMessage(B_QUIT_REQUESTED);
		if (fSingleFileWindow != NULL)
			fSingleFileWindow->PostMessage(B_QUIT_REQUESTED);	
		if (fNewsWindow != NULL)
			fNewsWindow->PostMessage(B_QUIT_REQUESTED);
		if (fNews15Window != NULL)
			fNews15Window->PostMessage(B_QUIT_REQUESTED);
		break;
	}
	/**************** Disconnect　***************/
	case MWIN_DISCONNECT:
	{
		this->PostMessage(MWIN_CLOSE_WINDDOWS);
		be_app->PostMessage(message);
		break;
	}
	/****** Send Chat Message ********/
	case MWIN_INVOKE_CHAT:
	{
		int32 len = strlen(chatmsg->Text());
		if(len > 0 && ((HApp*)be_app)->IsConnected() == true)
		{
			char *text = new char[len+1];
			::memset(text,0,len+1);
			::sprintf(text,"%s",chatmsg->Text());
			int32 encoding;
			TextUtils utils;
			
			((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
			if(encoding)
				utils.ConvertFromUTF8(&text,encoding-1);
			utils.ConvertReturnsToCR(text);
			
			if(strlen(text) != 0)
			{
				BMessage msg(MWIN_INVOKE_CHAT);
				msg.AddString("text",text);
				be_app->PostMessage(&msg);
				chatmsg->SetText("");
			}
			delete[] text;
		}
		break;
	}
	/****** Recv Chat Message　*******/
	case H_RCV_CHAT:
	{
		const char* text = message->FindString("text");
		char *chat = new char[strlen(text)+1];
		::strcpy(chat,text);
		TextUtils utils;
		utils.ConvertReturnsToLF(chat);
		int32 encoding;
		((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
		if(encoding)
			utils.ConvertToUTF8(&chat,encoding-1);
		int32 len = strlen(chat);
		
		BString str;
		for(register int32 i = 0;i < len;i++)
		{
			str << chat[i];
			if(chat[i+1] == '\n')
			{
				InsertChatMessage(str.String());
				str = "";	
			}
			if(len == i+1)
				InsertChatMessage(str.String());
		}
		delete[] chat;
		break;
	}
	/****** Send Message　**********/
	case MWIN_SEND_MESSAGE:
	{
		if( ((HApp*)be_app)->Client()->isConnected() != true)
			break;
		BRect rect = RectUtils().CenterRect(SEND_MESSAGE_WIDTH,SEND_MESSAGE_HEIGHT);
		int32 sel = listview->CurrentSelection();
		if(sel < 0)
			break;
		
		HUserItem *item = cast_as(listview->ItemAt(sel),HUserItem);
		if(item != NULL)
		{
			uint32 sock = item->Sock();
			bool msgchat;
			((HApp*)be_app)->Prefs()->GetData("msgchat",&msgchat);
			if(msgchat)
			{
				HMsgChatWindow *win = FindMsgChat(sock);
				if(win)
				{
					if(win->IsMinimized())
						win->Minimize(false);
					if(win->IsHidden())
						win->Show();
					if(!win->IsActive())
						win->Activate(true);	
				}else{
					BMessage msg(MESSAGE_CHAT_MSG);
					msg.AddInt32("sock",sock);
					this->PostMessage(&msg);
				}
			}else{
				BString title = _("Send message to");
				title << " " << item->Nick();
				HSendMsgWindow *win = new HSendMsgWindow(rect,title.String(),sock);
				win->Show();
			}
		}	
		break;
	}
	/********** Create Private Chat　************/
	case MWIN_PRV_CREATE_MESSAGE:
	{
		if( ((HApp*)be_app)->Client()->isConnected() != true)
			break;
		uint32 pcref;
		uint32 sock;
		int32 sel = listview->CurrentSelection();
		if(sel < 0)
			break;
		HUserItem *item = cast_as( (listview->ItemAt(sel)) ,HUserItem);
		if(item != NULL)
		{
			sock = item->Sock();
			pcref = 0x0;
			message->AddInt32("sock",sock);
			message->AddInt32("pcref",pcref);
			be_app->PostMessage(message);
		}
		break;
	}
	/********** User Infomation **********/
	case MWIN_USER_INFO_MESSAGE:
	{
		if( ((HApp*)be_app)->Client()->isConnected() != true)
			break;
		uint32 sock;
		int32 sel = listview->CurrentSelection();
		if(sel < 0)
			break;
			
		HUserItem *item = cast_as(listview->ItemAt(sel),HUserItem);
		if(item != NULL)
		{
			sock = item->Sock();
			message->AddInt32("sock",sock);
			be_app->PostMessage(message);
		}
		break;
	}
	/******** Show FileWindow **********/
	case MWIN_FILE_REQUESTED:
	{
		if( ((HApp*)be_app)->Client()->isConnected() != true)
			break;
		bool IsSilverWing = ((HApp*)be_app)->Client()->IsSilverWingMode();
		if(!fSingleWndMode)
		{
			if(fFileWindow == NULL)
			{			
				BRect rect;
				((HApp*)be_app)->LoadRect("file_rect",&rect);
				fFileWindow = new HFileWindow( rect,_("Files") ,IsSilverWing);
				fFileWindow->SetParent(this);
				fFileWindow->Show();
				BMessage msg(FILE_FILE_REQUEST);
				msg.AddString("path","");
				msg.AddInt32("index",1);
				be_app->PostMessage(&msg);

			} else {
				fFileWindow->Activate(true);
			}
		}else{
			if(fSingleFileWindow == NULL)
			{			
				BRect rect;
				((HApp*)be_app)->LoadRect("file_rect",&rect);
				fSingleFileWindow = new HSingleFileWindow( rect,_("Files") ,IsSilverWing);
				fSingleFileWindow->SetParent(this);
				fSingleFileWindow->Show();
				BMessage msg(FILE_FILE_REQUEST);
				msg.AddString("path","");
				msg.AddInt32("index",1);
				be_app->PostMessage(&msg);

			} else {
				fSingleFileWindow->Activate(true);
			}
		}
		break;
	}
	/******* Recv Message *********/
	case H_RCV_MSG:
	{
		uint32 sock;
		message->FindInt32("sock",(int32*)&sock);
		bool msgchat;
		((HApp*)be_app)->Prefs()->GetData("msgchat",&msgchat);
		
		HMsgChatWindow *win = FindMsgChat(sock);
		if(win)
		{
			win->PostMessage(message);
		}else{
			const char* text = message->FindString("text");
			if(msgchat)
			{
				BMessage msg(MESSAGE_CHAT_MSG);
				msg.AddInt32("sock",sock);
				msg.AddString("text",text);
				this->PostMessage(&msg);
				be_app->PostMessage(SOUND_SRVMSG_SND);
			}else{
				uint16 icon = listview->FindUserIcon(sock);
				HMsgWindow *win = new HMsgWindow(RectUtils().CenterRect(300,200)
								,message->FindString("nick"),sock,icon,text);
				win->SetTime();
				win->Show();
			}
		}
		break;
	}	
	/********* Close FileWindow ********/
	case FILE_REMOVE_POINTER:
	{
		if(fSingleWndMode)
			fSingleFileWindow = NULL;
		else
			fFileWindow = NULL;
		break;		
	}
	/************ Tracker Reader***************/
	case MWIN_TRACKER:
	{
		be_roster->Launch("application/x-vnd.takamatsu-hltrackerreader");
		break;
	}
	/************ Show Task Window　*************/
	case MWIN_FILE_TRANSFER:
	{
		HApp *app = cast_as(be_app,HApp);
		BAutolock lock(app->TaskWindow());
		if(app->TaskWindow()->IsMinimized())
		{
			app->TaskWindow()->Minimize(false);
			app->TaskWindow()->Show();
		}else if(app->TaskWindow()->IsHidden())
		{
			app->TaskWindow()->Show();
		}else{
			
			app->TaskWindow()->Hide();
		}
		break;
	}
	/************ News ************/
	case MWIN_NEWSMESSAGE:
	{
		if( ((HApp*)be_app)->Client()->isConnected() != true)
			break;
		//******************** Old News ************************
		if( ((HApp*)be_app)->ServerVersion() < 151)
		{	
			BRect rect;
			((HApp*)be_app)->LoadRect("news_rect",&rect);
			if(fNewsWindow == NULL)
			{
				fNewsWindow = new HNewsWindow(rect,_("News"));
				fNewsWindow->SetTarget(this);
				be_app->PostMessage(MWIN_SEND_GET_NEWS_FILE);
				fNewsWindow->Show();
			}else{
				fNewsWindow->Activate(true);
			}
		}else{
		//******************** News 1.5 ************************
			if(fNews15Window == NULL)
			{
				BRect rect;
				((HApp*)be_app)->LoadRect("news15_rect",&rect);
				fNews15Window = new HNews15Window(rect,_("News"));
				fNews15Window->SetTarget(this);
				BMessage msg(MWIN_NEWS_GET_CATEGORY);
				msg.AddString("path","/");
				msg.AddInt32("index",0);
				be_app->PostMessage(&msg);
				fNews15Window->Show();
			}else{
				fNews15Window->Activate(true);
			}
		}
		break;
	}
	/******** Article List Receive **************/
	case H_NEWS_RECEIVE_ARTICLELIST:	
	/*************** Recv News15 Category　****************/
	case H_NEWS_RECEIVE_FOLDER:
	{
		if(fNews15Window != NULL)
		{
			fNews15Window->PostMessage(message);
		}
		break;
	}
	/******** Close Old News ***********/
	case NEWS_CLOSE_WINDOW:
	{
		fNewsWindow = NULL;
		break;
	}
	/********* Close News 15 Window ************/
	case NEWS15_CLOSE_WINDOW:
	{
		fNews15Window = NULL;
		break;
	}
	/******* Recv Article **************/
	case H_RECEIVE_THREAD:
	{
		if(fNews15Window != NULL)
		{
			fNews15Window->PostMessage(message);
		}
		break;
	}
	/// Connect from Bookmark
	case CONNECT_BOOKMARK_MSG:
	{	
		BString address,login,password,port;
		AppUtils utils;
		BPath path = utils.GetAppDirPath(be_app);
		path.Append("Bookmarks");
		path.Append(message->FindString("name"));
		
		BFile file(path.Path(),B_READ_ONLY);
		if(file.InitCheck() == B_OK)
		{
			BString address,login,password;
			BMessage msg;
			uint16 port;
			
			msg.Unflatten(&file);
			if(msg.HasString("address")&&msg.HasInt16("port"))
			{
			address = msg.FindString("address");
			login = msg.FindString("login");
			port = msg.FindInt16("port");
			password = msg.FindString("password");
			
			BMessage cntMsg(CONNECT_CONNECT_REQUESTED);
			cntMsg.AddString("address",address.String());
			cntMsg.AddString("login",login.String());
			cntMsg.AddString("password",password.String());
			cntMsg.AddInt16("port",(int16)port);
			be_app->PostMessage(&cntMsg);
			}else
				(new MAlert(_("Error"),_("Bad bookmark file…"),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
		}
		break;
	}	
	/*********** Bookmark *********/
	case MWIN_ADDBOOKMARKS:
	{
		RectUtils utils;
		BRect rect = utils.CenterRect(300.0,180.0);
		HConnectWindow *win = new HConnectWindow(rect,_("Add to Bookmarks"),true);
		if( ((HApp*)be_app)->IsConnected())
		{
			win->SetName(((HApp*)be_app)->Client()->ServerName() );
			win->SetAddress( ((HApp*)be_app)->Client()->Address());
			BString str = "";
			int32 iport = ((HApp*)be_app)->Client()->Port();
			str << iport;
			win->SetPort( str.String() );
			win->SetLogin(((HApp*)be_app)->Client()->LoginName());
			win->SetPassword(((HApp*)be_app)->Client()->Password());
		}
		win->Show();
		break;
	}
	/*********** Recv News **********/
	case H_RECEIVE_NEWS_FILE:
	{
		const char* text = message->FindString("news");
		BMessage msg(NEWS_SET_NEWS);
		msg.AddString("text",text);
		if(fNewsWindow != NULL)
			fNewsWindow->PostMessage(&msg);
		break;
	}
	/************ User info changed ***********/
	case H_CHANGE_USER:
	{
		uint16 sock,icon,color;
		message->FindInt16("sock",(int16*)&sock);
		message->FindInt16("icon",(int16*)&icon);
		message->FindInt16("color",(int16*)&color);
		const char* nick;
		if( message->FindString("nick",&nick) != B_OK)
			nick = "";
		this->ChangeUser(sock,icon,color,nick);
		break;
	}
	/********** User Leave ***********/
	case H_REMOVE_USER:
	{
		uint16 sock;
		message->FindInt16("sock",(int16*)&sock);
		RemoveUser(sock);
		break;
	}
	/********** Receive Post News **********/
	case H_RECEIVE_POST_NEWS:
	{
		if(fNewsWindow != NULL)
			fNewsWindow->PostMessage(message);
		break;
	}
	/************ Preference ************/
	case MWIN_PREFERENCE:
	{
		HSettingWindow* win = new HSettingWindow(RectUtils().CenterRect(430,350));
		win->Show();
		break;
	}
	/*********** Kick User *************/
	case MWIN_KICK_USER:
	{
		if( ((HApp*)be_app)->Client()->isConnected() != true)
			break;		
		uint32 sock;
		BMessage msg(H_KICK_USER);
		
		HUserItem *item = (HUserItem*)listview->ItemAt(listview->CurrentSelection());
		if(item != NULL)
		{
			sock = item->Sock();
			msg.AddInt32("sock",sock);
			((HApp*)be_app)->Client()->PostMessage(&msg);
		}	
		break;
	}
	/*********** Ban User **************/
	case MWIN_BAN_USER:
	{
		if( ((HApp*)be_app)->Client()->isConnected() != true)
			break;
		uint32 sock;
		int32 sel = listview->CurrentSelection();
		if(sel < 0)
			break;
		BMessage msg(H_BAN_USER);
		
		HUserItem *item = cast_as(listview->ItemAt(sel),HUserItem);
		if(item != NULL)
		{
			sock = item->Sock();
			msg.AddInt32("sock",sock);
			((HApp*)be_app)->Client()->PostMessage(&msg);
		}	
		break;
	}
	/******** Drop from Tracker ************/
	case B_SIMPLE_DATA:
	{
		message->PrintToStream();
		if(message->HasString("address") && message->HasInt16("port"))
		{
			const char* address = message->FindString("address");
			uint16 port = message->FindInt16("port");
			BMessage msg(CONNECT_CONNECT_REQUESTED);
			msg.AddString("address",address);
			msg.AddString("login","");
			msg.AddString("password","");
			msg.AddInt16("port",(int16)port);
			be_app->PostMessage(&msg);
		}else if(message->HasRef("refs"))
		{
			entry_ref ref;
			if(message->FindRef("refs",&ref) != B_OK)
				return;
			BFile file(&ref,B_READ_ONLY);
			BMessage msg;
			msg.Unflatten(&file);
			BMessage sndMsg(CONNECT_CONNECT_REQUESTED);
			if(msg.HasString("address") && msg.HasInt16("port"))
			{
				const char* address = msg.FindString("address");
				const char* login  = NULL;
				if(msg.HasString("login"))
					login = msg.FindString("login");
				const char* password = NULL;
				if(msg.HasString("password"))
					password = msg.FindString("password");
				uint16 port = msg.FindInt16("port");
				sndMsg.AddString("address",address);
				sndMsg.AddString("login",login);
				sndMsg.AddString("password",password);
				sndMsg.AddInt16("port",port);
				be_app->PostMessage(&sndMsg);
			}
		}
		break;
	}
	//****** Change Chat font **************//
	case M_SET_CHAT_FONT_MSG:
	{
		const char* family = message->FindString("font_family");
		const char* style = message->FindString("font_style");
		int32 size = message->FindInt32("font_size");
		uint32 font_color = message->FindInt32("font_color");
		uint32 back_color = message->FindInt32("back_color");
		uint32 nick_color = message->FindInt32("nick_color");
		
		uint8 col;
		col = font_color;
		fChatColor.blue = col;
		col = font_color >> 8;
		fChatColor.green = col;
		col = font_color >> 16;
		fChatColor.red = col;
		
		col = nick_color;
		fNickColor.blue = col;
		col = nick_color >> 8;
		fNickColor.green = col;
		col = nick_color >> 16;
		fNickColor.red = col;
		
		BFont font;
		font.SetSize(size);
		font.SetFamilyAndStyle(family,style);
		chatview->SetFontAndColor(0,strlen(chatview->Text()),&font);
		rgb_color color;
		col = back_color;
		color.blue = col;
		col = back_color >> 8;
		color.green = col;
		col = back_color >> 16;
		color.red = col;
		chatview->SetViewColor(color);
		chatview->Invalidate();
		break;
	}
	case B_NODE_MONITOR:
	case MWIN_RESCANBOOKMARKS:
	{
		this->RescanBookmarks();
		break;
	}
	case MWIN_REMOVE_ALL_USERS:
	{
		RemoveAllUsers();
		break;
	}
	/******* Update Toolbar button *********/
	case M_UPDATE_TOOLBUTTON:
	{
		const char* name = message->FindString("name");
		void *pointer;
		message->FindPointer("pointer",&pointer);
		HToolbarButton *btn = static_cast<HToolbarButton*>(pointer);
		if(btn == NULL)
			break;
					
		if(::strcmp(name,"infobtn") == 0
			||::strcmp(name,"kickbtn") == 0
			||::strcmp(name,"banbtn") == 0
			||::strcmp(name,"msgbtn") == 0
			||::strcmp(name,"prvchatbtn") == 0)
		{
			int32 sel = listview->CurrentSelection();
			if(sel >= 0)
			{
				btn->SetEnabled(true);
			}else{
				btn->SetEnabled(false);
			}	
		}else if(::strcmp(name,"newsbtn") == 0
			||::strcmp(name,"filebtn") == 0)
		{
			if( (cast_as(be_app,HApp))->IsConnected() )
			{
				btn->SetEnabled(true);
			}else{
				btn->SetEnabled(false);
			}
		}
		break;
	}
	default:
		BWindow::MessageReceived(message);
	}
}

/***********************************************************
 * Insert user to user list.
 ***********************************************************/
void
HWindow::InsertUser(uint16 sock,uint16 icon,uint16 color ,const char* nick)
{
	listview->AddUserItem(new HUserItem(sock,icon,color,nick));
}

/***********************************************************
 * Remove user from user list.
 ***********************************************************/
void
HWindow::RemoveUser(uint16 sock)
{
	int32 i = listview->CountItems();
	
	BString nick = "";
	
	for(int32 j= 0;j < i ; j++)
	{
		HUserItem *item = cast_as(listview->ItemAt(j),HUserItem);
		if(item->Sock() == sock){
			item = listview->DeleteItem(j);
			nick = item->Nick();
			bool which;
			((HApp*)be_app)->Prefs()->GetData("showlogin",&which);
			if(which )
			{
				BString title = "\n<<<  ";
				title << item->Nick();
				title << " " << _("has left.") << " ";
				time_t t = time(NULL);
				struct tm *stm = localtime(&t);
				char *buf = new char[1024];
				::memset(buf,0,1024);
				::sprintf(buf,"(%.2d:%.2d:%.2d)",stm->tm_hour,stm->tm_min,stm->tm_sec);
				title << buf;
				delete[] buf;
				title << "   >>>";
				this->InsertChatMessage(title.String());
			}
			delete item;
			be_app->PostMessage(SOUND_LOGOUT_SND);
			break;
			}
	}
	
	HMsgChatWindow *win = FindMsgChat(sock);
	if(win)
	{
		BMessage msg(M_LOGOUT_MESSAGE);
		msg.AddString("nick",nick);
		win->PostMessage(&msg);
	}
}


/***********************************************************
 * Change user info in the user list.
 ***********************************************************/
void
HWindow::ChangeUser(uint16 sock,uint16 icon,uint16 color,const char* nick)
{
	int32 i = listview->CountItems();
	bool rc = false;
	HUserItem *item = NULL;
	int32 itemIndex = -1;

	for(int32 j= 0;j < i ; j++)
	{
		item = cast_as(listview->ItemAt(j),HUserItem);
		if(item->Sock() == sock){
			rc = true;
			itemIndex = j;
			break;
			}
	}

	if(rc == true)
	{
		item->ChangeUser(sock,icon,color,nick);
		listview->InvalidateItem(itemIndex);
	}else{	
		bool which;
		((HApp*)be_app)->Prefs()->GetData("showlogin",&which);
		if(which )
		{
			BString title = "\n<<<  ";
			title << nick;
			title << " " << _("has joined.") << " ";
			time_t t = time(NULL);
			struct tm *stm = localtime(&t);
			char *buf = new char[1024];
			::memset(buf,0,1024);
			::sprintf(buf,"(%.2d:%.2d:%.2d)",stm->tm_hour,stm->tm_min,stm->tm_sec);
			title << buf;
			delete[] buf;
			title << "   >>>";
			this->InsertChatMessage(title.String());
		}
		be_app->PostMessage(SOUND_LOGIN_SND);
		InsertUser(sock,icon,color,nick);
	}
}

/***********************************************************
 * Remove all users from user list.
 ***********************************************************/
void
HWindow::RemoveAllUsers()
{
	if(listview->CountItems())
		listview->RemoveAll();	
}

/***********************************************************
 * Insert chat message into the chat view.
 ***********************************************************/
void
HWindow::InsertChatMessage(const char* text)
{
	int32 sel = strlen(chatview->Text());
	chatview->Select(sel,sel);
	BFont font;
	rgb_color color;
	chatview->GetFontAndColor(0,&font,&color);
	HCaption *caption = cast_as(FindView("info"),HCaption);
	if(caption)
		caption->SetTime(time(NULL));
	// time stamp
	int time_offset = 0;
	bool timestamp;
	((HApp*)be_app)->Prefs()->GetData("timestamp",&timestamp);
	if(timestamp)
		time_offset = 8;
	//
	//
	BString tmp = text;
	const char *nick;
	((HApp*)be_app)->Prefs()->GetData("nick",&nick);
	if( tmp.IFindFirst(nick,14)  != B_ERROR)
		be_app->PostMessage(SOUND_NOTIFY_SND);
	//
	if(strlen(text) >= 15 && text[14] == ':') // chat
	{
		text_run	run1;
		run1.offset = 0;
		run1.font = font;
		run1.color = fNickColor;
	
		text_run	run2;
		run2.offset = 15 + time_offset;
		run2.font = font;
		run2.color = fChatColor;
	
		text_run_array	array;	
		array.count = 2;
		array.runs[0] = run1;
		array.runs[1] = run2;
		BString str(text);
		if(timestamp)
		{
			time_t timet = time(NULL);
			struct tm* t = localtime(&timet);
			char *tmp = new char[10];
			::sprintf(tmp,"[%.2d:%.2d]",t->tm_hour,t->tm_min);
			str.Insert(tmp,1);
			delete[] tmp;
		}
		chatview->Insert(str.String(),&array);

		/*******************************/
		
	}else{
		text_run	run;
		run.offset = 0;
		run.font = font;
		run.color = fChatColor;
		
		text_run_array	array;
		array.count = 1;
		array.runs[0] = run;
		chatview->Insert(text,&array);
	}

	float min,max;
	(chatview->ScrollBar(B_VERTICAL))->GetRange(&min,&max);
	(chatview->ScrollBar(B_VERTICAL))->SetValue(max);
}

/***********************************************************
 * Find user
 ***********************************************************/
bool
HWindow::FindUser(uint32 sock,uint32 &out_icon,BString &out_nick)
{
	bool found = false;
	int32 count = listview->CountItems();
	while(count > 0)
	{
		HUserItem *item = cast_as(listview->ItemAt(--count),HUserItem);
		if(item->Sock() == sock)
		{
			out_icon = item->Icon();
			out_nick = item->Nick();
			found = true;
			break;
		}	
	}
	return found;
}


/***********************************************************
 * QuitRequested.
 ***********************************************************/
bool
HWindow::QuitRequested()
{
	/**** Save Window Rect *******/
	BPoint pos = fVSplitter->GetBarPosition();
	int32 x = static_cast<int32>(pos.x);
	((HApp*)be_app)->Prefs()->SetData("main_vbar_pos",x);
	pos = fHSplitter->GetBarPosition();
	x = static_cast<int32>(pos.y);
	((HApp*)be_app)->Prefs()->SetData("main_hbar_pos",x);
	((HApp*)be_app)->SaveRect("window_rect",this->Frame());
	if(fFileWindow != NULL)
	{
		fFileWindow->PostMessage(B_QUIT_REQUESTED);
	}
	if(fNewsWindow != NULL)
	{
		fNewsWindow->PostMessage(B_QUIT_REQUESTED);
	}
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}

/***********************************************************
 *   Bookmarks node monitor
 ***********************************************************/
void
HWindow::StartWatchingBookmarks()
{
	BPath AppPath = AppUtils().GetAppDirPath(be_app);
	AppPath.Append("Bookmarks");
	BEntry entry(AppPath.Path());
	node_ref nref;
	entry.GetNodeRef(&nref);

	::watch_node(&nref,B_WATCH_NAME|B_WATCH_DIRECTORY,this);
}

/***********************************************************
 * Rescan bookmarks.
 ***********************************************************/
void
HWindow::RescanBookmarks()
{
	// Remove All Bookmarks
	BMenuBar* menuBar = this->KeyMenuBar();
	BMenu *menu = menuBar->SubmenuAt(2);
	
	BMenuItem *item = menu->ItemAt(3);
	while(item != NULL)
	{
		menu->RemoveItem(item);
		delete item;
		item = menu->ItemAt(3);
	}	
	InitBookmarks();
}

/***********************************************************
 * MenusBegginging.
 ***********************************************************/
void
HWindow::MenusBeginning()
{
	HUserList *list = cast_as(FindView("userlist"),HUserList);
	
	if(((HApp*)be_app)->IsConnected() == false)
	{
		KeyMenuBar()->FindItem(MWIN_FILE_REQUESTED)->SetEnabled(false);
		KeyMenuBar()->FindItem(MWIN_NEWSMESSAGE)->SetEnabled(false);
		KeyMenuBar()->FindItem(MWIN_DISCONNECT)->SetEnabled(false);
		KeyMenuBar()->FindItem(MWIN_SEND_MESSAGE)->SetEnabled(false);
		KeyMenuBar()->FindItem(MWIN_PRV_CREATE_MESSAGE)->SetEnabled(false);
		KeyMenuBar()->FindItem(MWIN_USER_INFO_MESSAGE)->SetEnabled(false);
		KeyMenuBar()->FindItem(MWIN_KICK_USER)->SetEnabled(false);
		KeyMenuBar()->FindItem(MWIN_BAN_USER)->SetEnabled(false);
	}else{
		KeyMenuBar()->FindItem(MWIN_FILE_REQUESTED)->SetEnabled(true);
		KeyMenuBar()->FindItem(MWIN_NEWSMESSAGE)->SetEnabled(true);
		KeyMenuBar()->FindItem(MWIN_DISCONNECT)->SetEnabled(true);
		int32 sel = list->CurrentSelection();
		if(sel >= 0)
		{
			KeyMenuBar()->FindItem(MWIN_SEND_MESSAGE)->SetEnabled(true);
			KeyMenuBar()->FindItem(MWIN_PRV_CREATE_MESSAGE)->SetEnabled(true);
			KeyMenuBar()->FindItem(MWIN_USER_INFO_MESSAGE)->SetEnabled(true);
			KeyMenuBar()->FindItem(MWIN_KICK_USER)->SetEnabled(true);
			KeyMenuBar()->FindItem(MWIN_BAN_USER)->SetEnabled(true);
		}else{
			KeyMenuBar()->FindItem(MWIN_SEND_MESSAGE)->SetEnabled(false);
			KeyMenuBar()->FindItem(MWIN_PRV_CREATE_MESSAGE)->SetEnabled(false);
			KeyMenuBar()->FindItem(MWIN_USER_INFO_MESSAGE)->SetEnabled(false);
			KeyMenuBar()->FindItem(MWIN_KICK_USER)->SetEnabled(false);
			KeyMenuBar()->FindItem(MWIN_BAN_USER)->SetEnabled(false);
		}
	}
	
}	

/***********************************************************
 * MakeMessageChat
 ***********************************************************/
void
HWindow::MakeMessageChat(uint32 sock,uint32 icon,const char* nick,const char* text)
{
	BRect rect = RectUtils().CenterRect(SEND_MESSAGE_WIDTH,SEND_MESSAGE_HEIGHT);
	HMsgChatWindow *window = new HMsgChatWindow(rect,nick,sock,icon);
	fMsgChatList.AddItem(window);
	if(text)
		window->InsertChatMessage(text);
	window->Show();
}

/***********************************************************
 * Find message chat
 ***********************************************************/
HMsgChatWindow*
HWindow::FindMsgChat(int32 sock)
{
	int32 count = fMsgChatList.CountItems();
	HMsgChatWindow *result = NULL;
	while(count>0)
	{
		HMsgChatWindow *win = static_cast<HMsgChatWindow*>(fMsgChatList.ItemAt(--count));
		if(win->Sock() == sock)
		{
			result = win;
			break;
		}
	}
	return result;
}

/***********************************************************
 * FileWindow
 ***********************************************************/
BWindow*
HWindow::FileWindow()
{
	BWindow* window = NULL;
	if(fSingleWndMode)
		window = cast_as(fSingleFileWindow,BWindow);
	else
		window = cast_as(fFileWindow,BWindow);
	return window;
}

/***********************************************************
 * Check CPU type and clock.
 ***********************************************************/
BString
HWindow::CheckCPU()
{
	system_info sysinfo;
	BString str = "Unknown";
	get_system_info(&sysinfo);
	
	switch(sysinfo.cpu_type)
	{
	case B_CPU_PPC_601:
		str = "PowerPC 601";
		break;
	case B_CPU_PPC_603:
		str = "PowerPC 603";
		break;
	case B_CPU_PPC_603e:
		str = "PowerPC 603e";
		break;
	case B_CPU_PPC_604:
		str = "PowerPC 604";
		break;
	case B_CPU_PPC_604e:
		str = "PowerPC 604e";
		break;
	case B_CPU_PPC_750:
		str = "PowerPC G3";
		break;
	case B_CPU_INTEL_X86:
		str = "Intel x86";
		break;	
	case B_CPU_INTEL_PENTIUM:
	case B_CPU_INTEL_PENTIUM75:
		str = "Intel Pentium";
		break;
	case B_CPU_INTEL_PENTIUM_486_OVERDRIVE:
	case B_CPU_INTEL_PENTIUM75_486_OVERDRIVE:	
		str = "Intel Pentium OverDrive";
		break;
	case B_CPU_INTEL_PENTIUM_MMX:
//	case B_CPU_INTEL_PENTIUM_MMX_MODEL_4:
	case B_CPU_INTEL_PENTIUM_MMX_MODEL_8:	
		str = "Intel Pentium MMX";
		break;	
	case B_CPU_INTEL_PENTIUM_PRO:
		str = "Intel Pentium Pro";
		break;
	case B_CPU_INTEL_PENTIUM_II:
//	case B_CPU_INTEL_PENTIUM_II_MODEL_3:
	case B_CPU_INTEL_PENTIUM_II_MODEL_5:
		str = "Intel Pentium II";
		break;
	case B_CPU_INTEL_CELERON:
		str = "Intel Celeron";
		break;
	case B_CPU_INTEL_PENTIUM_III:
#ifdef B_BEOS_VERSION_5
	case B_CPU_INTEL_PENTIUM_III_MODEL_8:
#endif
		str = "Intel Pentium III";
		break;
#ifdef B_BEOS_VERSION_5
	case B_CPU_AMD_ATHLON_MODEL1:
		str = "AMD ATHLON";
		break;
#endif
	case B_CPU_AMD_X86:
		str = "AMD x86";
	 	break;
	case B_CPU_AMD_K5_MODEL0:
	case B_CPU_AMD_K5_MODEL1:
	case B_CPU_AMD_K5_MODEL2:
	case B_CPU_AMD_K5_MODEL3:
		str = "AMD K5";
		break;
	case B_CPU_AMD_K6_MODEL6:
	case B_CPU_AMD_K6_MODEL7:
	case B_CPU_AMD_K6_MODEL8:
//	case B_CPU_AMD_K6_2:
	case B_CPU_AMD_K6_MODEL9:
		str = "AMD K6 III";
		break;
	case B_CPU_CYRIX_X86:
		str = "Cyrix x86";
		break;
	case B_CPU_CYRIX_GXm:
		str = "Cyrix GXm";
		break;
	case B_CPU_CYRIX_6x86MX:
		str = "Cyrix 6x86MX";
		break;
	case B_CPU_RISE_X86:
		str = "RISE x86";
		break;
	case B_CPU_RISE_mP6:
		str = "RISE mP6";
		break;
	default:
		str = "Unknown";
	}	
	int64 cpu_speed;
	cpu_speed = sysinfo.cpu_clock_speed;
	/******* for PPC 604e ********/
	if(str.Compare("PowerPC 604e") == 0)
		cpu_speed *= 2;
	int target = cpu_speed / 1000000;
	int frac = target % 100;
	int delta = -frac;
	int at = 0;
	int freqs[] = { 100, 50, 25, 75, 33, 67, 20, 40, 60, 80, 10, 30, 70, 90 };

	for (uint x = 0; x < sizeof(freqs) / sizeof(freqs[0]); x++) {
		int ndelta = freqs[x] - frac;
		if (abs(ndelta) < abs(delta)) {
			at = freqs[x];
			delta = ndelta;
		}
	}
	
	BString clock;

	clock << (long)(target + delta) << " MHz";

	str << " (";
	str << clock.String();
	str << ")";
	return str;
}