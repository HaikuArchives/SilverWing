#ifndef __HOTLINECLIENT_H__
#define __HOTLINECLIENT_H__
/*********** SYSTEM HEADER ***********/
#include <Looper.h>
#include <Message.h>
#include <String.h>
#include <NetworkKit.h>
/*********** PROJECT HEADER **********/
#include "hl_magic.h"
#include "hx_types.h"
#include "HTaskWindow.h"

class HTaskWindow;

typedef struct  {
	uint16 type, len;
	uint16 sock, icon, colour, nlen;
	char nick[64];
}BHTLC_USER;

struct ChatData{
	uint16 len;
	char* string;
};

struct PrvChatRef{
	uint16 len;
	uint32 pcref;
};

struct HLObject{
	uint16 type;
	union {
		PrvChatRef	prvChatRef;
		ChatData chatData;
	}data;
} ;

typedef struct{
	uint32 pcref;
	uint32 trans;
} PrvChatTask;

typedef struct{
	uint32 trans;
	uint32 item_index;
}NewsTrans;

typedef struct{
	uint32 trans;
	BString path;
}NewsCategoryTrans;

typedef struct{
	uint16 type;
	uint16 len;
	uint32 unknown;
	uint32 numItems;
}NewsItemHdr;

typedef struct{
	uint16 unknown;
	uint16 unknown2;
	uint16 index;
	uint16 base_year;
	uint16 pad;
	uint32 seconds;
	uint32 parent_id;
	uint32 newssize;
}NewsItem;

typedef struct{
	uint8 len[1];
	uint8 data[1];
}NewsItemData;

typedef struct{
	uint16 type;
	uint16 unknown;
	uint16 thread;
}NewsMidHdr;

typedef struct{
	uint16 type;
	uint16 len;
}NewsBottomHdr;

typedef struct{
	uint16 base_year;
	uint16 pad;
	uint32 seconds;
}DateTime;

typedef struct{
	uint32 task;
	BString localpath;
	BString remotepath;
	bool	isDownload;
}FileTrans;

typedef struct{
	uint32 task;
	uint32 parent_index;
}FileListIndex;

struct HLHDR {
	uint32 type;
	uint32 trans;
	uint32 flag;
	uint32 len;
	uint32 len2;
	uint16 hc;
	HLObject **object;
};

enum TaskType{
	T_FILE_LIST_TASK = 0,
	T_USER_LIST_TASK = 1,
	T_PRV_USER_TASK = 2,
	T_FILE_TRANS_TASK = 3,
	T_ERROR_TASK = 4,
	T_FILE_INFO_TASK = 5,
	T_USER_INFO_TASK = 6,
	T_NEWS_CATEGORY_TASK =7,
	T_NEWS_FOLDER_ITEM_TASK = 8,
	T_NEWS_DATA_TASK = 9,
	T_PRV_INVITE_TASK = 10,
	T_LOGIN_TASK = 11,
	T_NEWS_FILE_TASK = 12
};

typedef struct {
	uint32 task;
	TaskType type;
}HLTASK;



/***** Messages *******/
enum{
H_CONNECT_REQUESTED = 0x101L,
H_CLOSE_REQUESTED = 0x1020L,
H_CONNECT_SUCCESS = 0x103L,
H_LOGIN_REQUESTED = 0x104L,
H_CHAT_REQUESTED = 0x109L,
H_FILE_REQUESTED = 0x110L,
H_RCV_CHAT = 0x105L,
H_RCV_MSG = 0x106L,
H_RCV_GLOBAL_MSG = 0x109L,
H_RCV_PRV_CHAT = 0x107L,
H_RCV_AGREEMENT = 0x108L,
H_TRACKER_RCV = 0x111L,
H_SEND_MSG = 0x112L,
H_RCV_JOIN_CHAT = 0x113L,
H_PRV_CHAT_USER_ADD = 0x114L,
H_SEND_PRV_CHAT_MSG=0x115L,
H_CHAT_INVITE = 0x116L,
H_CONNECT_TRACKER = 0x117L,
H_RCV_INFO = 0x118L,
H_CHAT_USER_CHANGE = 0x119L,
H_CHAT_USER_LEAVE = 0x120L,
H_CHAT_DECLINE = 0x121L,
H_CHAT_OUT = 0x122L,
H_PRV_CHAT_INVITE = 0x123L,
H_SERVER_VERSION_RECEIVED = 0x124L,
H_FILELIST_RECEIVED = 0x125L,
H_NEWS_GET_FILE = 0x126L,
H_NEWS_POST_NEWS = 0x127L,
H_RECEIVE_NEWS_FILE = 0x128L,
H_NEW_GET_CATEGORY = 0x129L,
H_NEWS_RECEIVE_FOLDER = 0x130L,
H_NEWS_SEND_GET_ARTICLELIST =0x131L,
H_NEWS_RECEIVE_ARTICLELIST = 0x132L,
H_THREAD_REQUESTED = 0x133L,
H_RECEIVE_THREAD = 0x134L,
H_POST_THREAD = 0x135L,
H_CREATE_FOLDER = 0x136L,
H_CREATE_CATEGORY = 0x137L,
H_DELETE_THREAD = 0x138L,
H_DELETE_CATEGORY = 0x139L,
H_KICK_USER = 0x140L,
H_BAN_USER = 0x141L,
H_SEND_USER_CHAGED = 0x142L,
H_FILE_CREATE_FOLDER = 0x143L,
H_FILE_GET_INFO = 0x144L,
H_FILE_DELETE = 0x145L,
H_RECEIVE_POST_NEWS = 0x146L,
H_ADD_USER = 0x147L,
H_CHANGE_USER = 0x149L,
H_REMOVE_USER = 0x150L,
H_UPDATE_QUEUE = 0x151L,
H_FILE_MOVE = 0x152L,
H_JOIN_CHAT = 0x153L,
H_REFUSE_CHAT = 0x154L,
H_PRVCHAT_TOPIC_CHANGE = 0x155L,
H_TOPIC_CHANGED = 0x156L
};
/************************/

class HotlineClient :public BLooper{
public:

							HotlineClient();
				virtual		~HotlineClient();
		virtual void		MessageReceived(BMessage* inMessage);
//----------------- Connection related ------------------------------
				bool		Connect(const char* address,uint16 port);
				bool		isConnected();
				bool		Login(const char *login, const char *pass,const char *nick, uint16 icon);
				void		Close();
				int32		ListenToServer();
		static	int32		ThreadEntry(void *arg);
				status_t	StartThread();
		static  int32 		Connecting(void *arg);
				void		Fook_Hx_Fun();
				void		hx_set(int32, uint32 len);
				void		hx_reset();		
				bool		IsSilverWingMode()const {return fSilverWingMode;}
//----------------- Receive related　------------------------------
				void		ReceiveHeader();
				void		ReceiveTask();
				void		ReceiveTaskError();
				void		ReceiveAgreement();
				void		ReceiveMessage();
				void		ReceiveUserInfo ();
				void		ReceiveUserLeave();
				void		ReceiveUserChanged();
				void		ReceiveFileList();
				void		ReceiveFileGet ();
				void		ReceiveFilePut();
				void		ReceiveFileInfo();
				void		ReceivePliteQuit();	
				void		ReceiveServerVersion();
				void		ReceiveChatInvite(bool me = false);
				void		ReceiveChatUserChange();
				void		ReceiveChatUserLeave();
				void		ReceiveChat();
				void		ReceiveNewsCategory();
				void		ReceiveNewsFolderItem();
				void		ReceiveNews();
				void		ReceiveNewsFile();
				void		ReceivePostNews();
				void		ReceiveUpdateQueue();
				void		ReceivePrvChatTopicChanged();
				void		ReceiveUserList();
				void		ReceivePrvChatUserList();
				void		ReceiveSelfInfo();
				void		ReceiveBroadcast();
				void		ReceiveSilverWingMode();
//----------------- Sending related ---------------------------------------
				void		SendGetUsersList();		
				void		SendChat(const void *data, uint16 len);
				void		SendChatJoin (uint32 pcref);
				void		SendChatCreate (uint32 sock);
				void		SendUserGetInfo (uint32 sock);
				void		SendFileList (const char *path,uint32 index,bool use_task = true);
				void		SendNewsGetFile (void);
				void		SendMessage (uint32 sock, const void *data, uint16 len);
				void		SendUserKickBan (uint32 sock);
				void		SendUserKick (uint32 sock);
				void		SendFileGet (const char *path,const char* localpath, uint32 data_size, uint32 rsrc_size);
				void		SendGetFileInfo(const char* path);
				void		SendCategory();
				void		SendFilePut (const char *remotepath,const char* localpath, uint32 resume_flag, uint32 rsrc_size);
				void		SendChatLeave (uint32 pcref);
				void		SendChatInvite (uint32 pcref, uint32 sock);
				void		SendChatChat (uint32 pcref, const void *data, uint16 len);
				void		SendNewsPost(const char* text);
				void		SendMkDir(const char* path);
				void		SendNewsDirList(const char* path);
				void		SendNewsDirList(const char* path,uint32 index);
				void		SendNewsCategory(const char* path);
				void 		SendNewsPostThread(const char* path, uint16 reply_thread, const char* name, uint16 parent_thread, const char* message, const char* mime= "text/plain");
				void		SendNewsGet(const char* category, uint16 thread,const char* mime="text/plain");
				void 		SendNewsCreateFolder(const char* path, const char* name);
				void		SendNewsCreateCategory(const char* path, const char* name);
				void		SendDeleteCategory(const char* category);
				void		SendDeleteThread(const char* category, uint16 thread);
				void		SendUserChange(const char* nick, uint16 icon);
				void		SendDeclineChat(uint32 pcref);
				void		SendFileDelete(const char* path);
				void		SendFileMove(const char* filename,
									const char* file_path,
									const char* dest_path);
				void		SendPrvChatTopicChange(const char* topic,uint32 pcref);
				void		SendSilverWingMode();
/********************** Others　********************/
				void		SetSock5Firewall(bool which) {fUseSock5 = which;}
			const char*		Address() {return fAddress.String();}
				uint16		Port() {return fPort;}
				uint32		Trans() {return fHxTrans;}
struct hx_data_hdr*			path_to_hldir (const char *path, int is_file);
struct hx_data_hdr* 		news_path_to_hldir(const char *path, int is_file);				
			const char*		LoginName() {return fLogin.String();}
			const char*		Password() {return fPassword.String();}
			const char*		ServerName() {return fServerName.String();}
				int			b_read (BNetEndpoint *endpoint, void *bufp, size_t len);
/********************** User ***********************/
		static	void		AddUserItem(uint16 sock,uint16 icon,uint16 color,const char* nick);
		static	void		ChangeUserItem(uint16 sock,uint16 icon,uint16 color,const char* nick);
		static	void		RemoveUserItem(uint16 sock);
		static 	void		RemoveAllUsers();
/********************* TASK **********************/
				void		AddPrvChat(uint32 pcref ,uint32 task);
				void		FindPrvChat(uint32 task);
				void		AddTask(const char* name ,uint32 task,uint32 type = T_NORMAL_TYPE);
				void		RemoveTask(uint32 task_id);
				void		UpdateTask(uint32 task,uint32 update);
				void		SetTaskMax(uint32 task,uint32 max_value);
protected:
				void		SendPing();
				void		AddTaskList(uint32 task,TaskType type);
				HLTASK*	FindTaskList(uint32 task);
				void		RemoveTaskList(HLTASK *task);
				int32		SendData(const void* data,size_t len);
				int32		ReceiveData( void* buffer,size_t size,bigtime_t timeout = 10*1000000);
private:
		BList		fPrvChatList; 			// private chat list
		BNetEndpoint *fEndpoint;  			// main socket
		thread_id	fRcvThread; 			// Listen to server thread id
		thread_id   fConnectThread;			// connect  thread id
		bool		fConnected;				
		uint32		fLoginTask;			// login task id
		char 		fBuffer[0x7FFF];			// main buffer for fHxBuf
		BString 	fAddress;				// server address
		BString		fLogin;					// login name
		BString		fPassword;				// password
		uint16		fPort;					// server port
		BString		fServerName;			// server name
		uint32		fServerVersion;			// server version	
		uint32 		fHxTrans;				// transfer id
		uint8		*fHxBuf;				// hotline buffer
	    uint32		fHxPos;					// hotline buffer position
        ssize_t 	fHxLen;					// buffer length
		int32		fHxFun;					// fook function id
		BList		fNewsTransArray;		// News task list
		BList		fNewsCategoryArray;		// Category task list
		BList		fFileTransArray;		// File transfer task list
		BList		fFileListArray;			// Getting file list task list
		bool		fUseSock5;				// SOCKS5 flag
		bool		fCancel;				// Cancel flag
		time_t		fIdleTime;				// idle time
		BList		fTaskList;
		bool		fSilverWingMode;
		uint32 		fSWModeTask;
		bool		fContinue;
		
		BLocker		*fSocketLocker;
};
#endif