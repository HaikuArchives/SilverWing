#ifndef __HWINDOW_H__
#define __HWINDOW_H__
/*********** SYSTEM HEADER ***********/
#include <NetworkKit.h>
#include <Window.h>
#include <iostream>


class HUserList;
class HUserItem;
class CTextView;
class HFileWindow;
class HNewsWindow;
class HNews15Window;
class MLTextControl;
class SplitPane;
class ChatLogView;
class HMsgChatWindow;
class HSingleFileWindow;

enum{
MWIN_CONNECT = 'MCON',
MWIN_DISCONNECT = 'MDIC',
//MWIN_KICK_MSG = 'MKIC',
MWIN_INVOKE_CHAT = 'CHIC',
MWIN_SEND_MESSAGE = 'CHSM',
MWIN_RECEIVE_MSG = 'CHRM',
USERWIN_INVOKE_CHAT = 'UWIC',
USERWIN_SENDCHAT_MSG = 'UWSM',
USERWIN_RECEIVE_MSG = 'UWRM',
USERWIN_CHAT_INVITE= 'UWCI',
USERWIN_GET_INFO = 'UWGI',
USERWIN_KICK_MSG = 'UWKM',
MWIN_FILE_REQUESTED = 'UWFQ',
MWIN_TRACKER = 'UWTR',
MWIN_PRV_CREATE_MESSAGE = 'UWCC',
MWIN_USER_INFO_MESSAGE = 'UWGI',
MWIN_PREFERENCE = 'UWPR',
MWIN_ADDBOOKMARKS = 'MWAB',
MWIN_NEWSMESSAGE = 'MNEW',
MWIN_SEND_GET_NEWS_FILE = 'MSEN',
MWIN_NEWS_GET_CATEGORY = 'NEGC',
MWIN_KICK_USER = 'MKIU',
MWIN_BAN_USER = 'MBAN',
MWIN_CLOSE_WINDDOWS = 'MCLW',
MWIN_FILE_TRANSFER = 'MFTR',
MWIN_RESCANBOOKMARKS = 'MREC',
MWIN_OPEN_DOWNLOAD_FOLDER = 'MODF',
MWIN_REMOVE_ALL_USERS = 'MREA'
};

class HWindow :public BWindow {
public:
					HWindow(BRect rect,const char* name);
virtual				~HWindow();
		void		InsertUser(uint16 sock,uint16 icon,uint16 color,const char* nick);
		void		RemoveUser(uint16 sock);
		void		ChangeUser(uint16 sock,uint16 icon,uint16 color,const char* nick);
		void		RemoveAllUsers();
		void 		InsertChatMessage(const char* text);
		BWindow* 	FileWindow();
		BString		CheckCPU();
	ChatLogView*	ChatView() {return chatview;}
	HMsgChatWindow*	FindMsgChat(int32 sock);
/************ PROTECTED ***********/
protected:
virtual void		MessageReceived(BMessage *message);
virtual bool		QuitRequested();
virtual void		MenusBeginning();
		void		InitGUI();
		void		InitMenu();
		void		InitBookmarks();
		void		StartWatchingBookmarks();
		void		RescanBookmarks();
		void		MakeMessageChat(uint32 sock
								,uint32 icon
								,const char* nick
								,const char* text = NULL);
		bool		FindUser(uint32 sock,uint32 &out_icon,BString &out_nick);
/*********** PRIVATE ***********/
private:
		HUserList* 			listview;
		MLTextControl* 		chatmsg;
		ChatLogView*		chatview;
		HFileWindow*		fFileWindow;
		HNewsWindow* 		fNewsWindow;
		SplitPane*			fVSplitter;
		SplitPane*			fHSplitter;
		HNews15Window*		fNews15Window;
		rgb_color			fChatColor;
		rgb_color			fNickColor;
		BList				fMsgChatList;
		HSingleFileWindow*	fSingleFileWindow;
		bool				fSingleWndMode;
};
#endif