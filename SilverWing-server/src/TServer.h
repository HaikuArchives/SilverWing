#ifndef __TSERVER_H__
#define __TSERVER_H__

#include <Looper.h>
#include <String.h>
#include <NetworkKit.h>
#include <Message.h>
#include <string>

/******** MESSAGE TYPE ********/
enum MessageType{
	T_NORMAL_TYPE = 0x0000,
	T_LOGIN_TYPE = 0x0001,
	T_LOGOUT_TYPE = 0x0002,
	T_DOWNLOAD_TYPE = 0x0003,
	T_UPLOAD_TYPE = 0x0004,
	T_ERROR_TYPE = 0x0005,
	T_DOWNLOAD_END_TYPE = 0x0006,
	T_UPLOAD_END_TYPE = 0x0007,
};
/******** MESSAGES ********/
enum{
	T_ADD_CLIENT = 'TACL',
	T_REMOVE_CLIENT = 'TRCL',
	T_LOG_MESSAGE = 'TLOG',
	T_PROCESS_HEADER = 'TPRH',
	T_START_SERVER = 'TSTS',
	T_STOP_SERVER = 'TSTO',
	T_BROADCAST_MSG = 'TBOM',
	T_REMOVE_TRANS = 'TRET',
	T_RESET_ACCOUNT = 'TREA'
};

/******* Class definition *****/
class HPrvChat;
class HUserItem;
class HLPacket;
class HFileTransThread;
/******* File trans struct ********/
typedef struct {
	uint32 ref;
	string filename;
	bool	isDownload;
	uint32	data_pos;
	HFileTransThread *thread;
} FileTrans;



class TServer :public BLooper{
public:
						TServer(const char* address
								,uint32 port
								,uint32 maxConnection
								,BLooper *target);
						~TServer();
	 		void		Start();
 			void		Stop();
 			bool		IsServing() {return fConnected;}
 			uint32		Users() const {return fUserList.CountItems();}
 			void		Log(const char* text,MessageType type = T_NORMAL_TYPE) const;
 			void		SendUserChange(uint16 sock,const char* nick,uint16 icon,uint16 color = 0);
protected:
			void		Log2News(HUserItem *user,const char* text);
			void		ResetAccount(const char* name);
	static	int32		ListenToClient(void* data);
	static	int32		HandleConnectionRequested(void *data);
	static	int32		HandleFileTrans(void* data);
	virtual void		MessageReceived(BMessage *message);
	virtual bool		QuitRequested();
//**************** Managing client sockets ***********************//
			void		AddClient(BMessage *message);
			void		RemoveClient(HUserItem *user);
			HUserItem*	FindUser(uint32 sock);
			HPrvChat*	FindPrvChat(uint32 pcref);
			FileTrans*	FindTrans(uint32 ref);
			void		InitValiables();
			bool		CheckAccount(const char* login,const char* password);
			void		KickUser(uint32 sock);
			void		DeleteFile(const char* path,const char* name);
			void		CreateFolder(const char* path,const char* name);
			void		CreatePrvChat(HUserItem *user,uint32 sock,uint32 trans);
			void		AddPrvChatClient(HUserItem *user,uint32 pcref,uint32 trans);
			void		RemovePrvChatClient(HUserItem *user,uint32 pcref,uint32 trans);
			void		AddDownload(const char* filename,uint32 ref,uint32 data_pos);
			void		AddUpload(const char* filename,uint32 ref,uint32 data_pos);
			void		RemoveFileTrans(HFileTransThread *thread);
			void		AddBanList(BNetEndpoint *endpoint);
			bool		CheckBanList(BNetEndpoint *endpoint);
			void		ClearBanList();
			void		MoveFile(const char* filename,const char* old_path,const char* new_path);
			void		WriteArticle(const char* path,const char* username,const char* message);
			uint32		ReadData(BNetBuffer &buf,uint32 len,HUserItem *user);
//**************** Handle receive client message ******************************//
			bool		ProcessLogin(BNetBuffer &buf,BNetEndpoint *endpoint);
			void		ProcessHeader(BNetBuffer &buf,HUserItem *user);
		
			bool		ReceiveLogin(BNetEndpoint *endpoint,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveChat(HUserItem *user,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveMessage(HUserItem *user,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveUserList(HUserItem *user,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveNews(HUserItem *user ,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveNewsPost(HUserItem *user ,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveUnknown(HUserItem* user,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveFileGetList(HUserItem *user,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveFileDelete(HUserItem *user,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveFolderCreate(HUserItem *user,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveUserChange(HUserItem *user,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveUserInfo(HUserItem* user,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveUserKick(HUserItem *user,uint32 len,uint16 hc,uint32 trans);
			void		ReceivePrvChatCreate(HUserItem *user,uint32 len,uint16 hc,uint32 trans);
			void		ReceivePrvChatJoin(HUserItem *user,uint32 len,uint16 hc,uint32 trans);
			void		ReceivePrvChatInvite(HUserItem *user,uint32 len,uint16 hc,uint32 trans);
			void		ReceivePrvChatLeave(HUserItem *user,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveFileGet(HUserItem *user,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveFilePut(HUserItem* user,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveFileInfo(HUserItem *user,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveFileMove(HUserItem* user,uint32 len,uint16 hc,uint32 trans);
			void		ReceivePrvChatTopicChanged(HUserItem* user,uint32 len,uint16 hc,uint32 trans);
			void		ReceiveSilverWingMode(HUserItem *user,uint32 len,uint16 hc,uint32 trans);
//**************** Send to clients*************************//
			void		SendServerVersion(BNetEndpoint *endpoint,uint32 trans);
			void		SendAgreement(BNetEndpoint *endpoint,uint32 trans);
			void		SendUserList(BNetEndpoint *endpoint,uint32 trans);

			void		SendToAllUsers( HLPacket &data,uint32 type,uint32 data_size,uint32 data_count);	
			void		SendToAllUsers(void* data,uint32 length);
			void		SendError(HUserItem *user,uint32 trans,const char* text);
			void		SendNews(HUserItem *user,uint32 trans);
			void		SendNewsCategory(HUserItem *user,uint32 trans,const char* category);
			void		SendNewsPost(const char* text);
			void		SendFileList(HUserItem *user,uint32 trans,const char* path);
			void		SendMessage(HUserItem* sender,HUserItem* recver,uint32 trans,const char* text);
			void		SendTaskEnd(HUserItem *user ,uint32 trans);
			void		SendUserInfo(HUserItem *user,uint32 sock,uint32 trans);
			void		SendUserLeave(uint16 index);
			void		SendBroadcastMessage(const char* message);
			void		SendPrvChatInvite(HUserItem *user,uint32 sock,uint32 pcref,uint32 trans);
			void		SendFileInfo(HUserItem *user,uint32 trans,const char* filename,const char* path);
			void		SendPrvChatCreate(HUserItem *user,uint32 pcref,uint32 trans);
			void		SendSilverWingMode(HUserItem *user,uint32 trans);
private:
			uint32 			fPort; // Server port.
			uint32			fMaxConnections; // Total connection.
			uint32			fCurrentUsers;  // Current connected users.
			BLooper 		*fTarget;       // Log target.
			BNetEndpoint 	*fServerSocket; // server main socket
			thread_id		fHandleConnection; // Accept thread
			thread_id		fListenToClient; // Liten thread
			thread_id		fTransThread; // file trans thread
			BString 		fAddress;     // Server address.
			BList			fUserList;    // Connected user list stored user data and socket. 
			BList			fPrvChatList;  // private chat list
			bool			fConnected;    // Which server is running or not.
			uint32			fSocketIndex;  // user's sock index
			uint32			fPrvChatIndex; // private chat ref
			uint32			fFileTransRef;  // Reference file trans index. 
			uint32 			fCurrentDownloads;  // current working downloads.
			uint32			fCurrentUploads;   // current working uploads.
			BList			fFileTransList;    // Stored file trans reference and info.
			BMessage		fBanList;          // ban IP list. (list of const char*)
};
#endif