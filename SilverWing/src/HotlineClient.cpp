#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <Beep.h>
#include <Autolock.h>

#include "HotlineClient.h"
#include "xmalloc.h"
#include "TextUtils.h"
#include "HAgreeWindow.h"
#include "HApp.h"
#include "dhargs.h"
#include "RectUtils.h"
#include "MAlert.h"
#include "HTaskWindow.h"
#include "SoundUtils.h"
#include "AppUtils.h"
#include "HInfoWindow.h"
#include "HWindow.h"
#include "TextUtils.h"
#include "InvitationWindow.h"
#include "HTaskWindow.h"
#include "HFileWindow.h"
#include "HPrefs.h"

#include <Debug.h>
#include <sys/time.h>

//#define DEBUG

static void htont(unsigned char *text, unsigned long text_len);
static void ntoht(unsigned char *text, unsigned long text_len);
static const char *dirchar_basename (const char *str);
static void rd_wr (BNetEndpoint *rd_fd, int wr_fd, uint32 data_len);

/***********************************************************
 * Constructor.
 ***********************************************************/
HotlineClient::HotlineClient()
	:BLooper("HotlineSocket")
	,fEndpoint(NULL)
	,fRcvThread(-1)
	,fConnectThread(-1)
	,fConnected(false)
	,fServerName("")
	,fServerVersion(0)
	,fHxTrans(1)
	,fHxBuf(NULL)
	,fHxPos(0)
	,fHxLen(SIZEOF_HX_HDR)
	,fCancel(false)
	,fSilverWingMode(false)
	,fContinue(false)
{
	fPrvChatList.MakeEmpty();
	fNewsCategoryArray.MakeEmpty();

	fSocketLocker = new BLocker();
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HotlineClient::~HotlineClient()
{
	xfree(fHxBuf);
	int32 count = fPrvChatList.CountItems();
	for(int i = 0;i< count ;i++)
		delete fPrvChatList.ItemAt(i);
	fPrvChatList.MakeEmpty();
	count = fNewsTransArray.CountItems();
	for(int i = 0;i<count;i++)
		delete fNewsTransArray.ItemAt(i);
	fNewsTransArray.MakeEmpty();

	count = fNewsCategoryArray.CountItems();
	for(int i = 0;i<count;i++)
		delete fNewsCategoryArray.ItemAt(i);
	fNewsCategoryArray.MakeEmpty();

	count = fFileTransArray.CountItems();
	for(int i = 0;i<count;i++)
		delete fFileTransArray.ItemAt(i);
	fFileTransArray.MakeEmpty();

	count = fFileListArray.CountItems();
	for(int i = 0;i<count;i++)
		delete fFileListArray.ItemAt(i);
	fFileListArray.MakeEmpty();

	count = fTaskList.CountItems();
	for(int i = 0;i<count;i++)
		delete fTaskList.ItemAt(i);
	fTaskList.MakeEmpty();

	delete fSocketLocker;
}

/***********************************************************
 * Looper message received.
 ***********************************************************/
void
HotlineClient::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case H_PRVCHAT_TOPIC_CHANGE:
		{
			const char* topic;
			if(message->FindString("text",&topic) == B_OK)
			{
				uint32 pcref = message->FindInt32("pcref");
				SendPrvChatTopicChange(topic,pcref);
			}
			break;
		}
	case H_JOIN_CHAT:
		{
			uint32 	pcref = message->FindInt32("pcref");
			SendChatJoin(pcref);
			break;
		}
	case H_REFUSE_CHAT:
		{
			uint32 pcref = message->FindInt32("pcref");
			SendDeclineChat(pcref);
			break;
		}
	case H_FILE_MOVE:
	{
		const char* file_path;
		const char* filename;
		const char* dest_path;
		if(message->FindString("file_path",&file_path) == B_OK
			&& message->FindString("dest_path",&dest_path) == B_OK
			&& message->FindString("file_name",&filename) == B_OK)
		{
			SendFileMove(filename,file_path,dest_path);
		}
		break;
	}
	case H_CONNECT_TRACKER:
	{
		//const char* address = message->FindString("address");
		//GetTrackerList(address.c_str(),5498);
		break;
	}
	case H_PRV_CHAT_INVITE:
	{
		uint32 sock,pcref;
		message->FindInt32("pcref",(int32*)&pcref);
		message->FindInt32("sock",(int32*)&sock);
		SendChatInvite(pcref,sock);
		break;
	}
	case H_CHAT_INVITE:
	{
		uint32 sock,pcref;
		message->FindInt32("pcref",(int32*)&pcref);
		message->FindInt32("sock",(int32*)&sock);
		SendChatCreate(sock);
		break;
	}
	case H_SEND_PRV_CHAT_MSG:
	{
		const char* text;
		message->FindString("text",&text);
		uint32 pcref;
		message->FindInt32("pcref",(int32*)&pcref);
		SendChatChat(pcref,text,::strlen(text));
		break;
	}
	case H_NEWS_GET_FILE:
	{
		SendNewsGetFile();
		break;
	}
	case H_NEWS_POST_NEWS:
	{
		const char* text;
		message->FindString("text",&text);
		SendNewsPost(text);
		break;
	}
	case H_NEW_GET_CATEGORY:
	{
		const char* path = message->FindString("path");
		uint32 index = message->FindInt32("index");
		this->SendNewsDirList(path,index);
		break;
	}
	case H_THREAD_REQUESTED:
	{
		const char* category = message->FindString("category");
		int16 thread = message->FindInt16("thread");
		this->SendNewsGet(category,thread);
		PRINT(( "Category:%s\n", category ));
		break;
	}
	case H_POST_THREAD:
	{
		const char* msg = message->FindString("message");
		const char* category = message->FindString("category");
		const char* subject = message->FindString("subject");
		int16 reply = message->FindInt16("reply_thread");
		int16 parent = message->FindInt16("parent_thread");
		this->SendNewsPostThread(category, reply, subject, parent, msg);

		break;
	}
	case H_DELETE_THREAD:
	{
		const char* path = message->FindString("path");
		uint16 thread = message->FindInt16("thread");
		SendDeleteThread(path,thread);
		break;
	}
	case H_DELETE_CATEGORY:
	{
		const char* path = message->FindString("path");
		SendDeleteCategory(path);
		break;
	}
	case H_CREATE_FOLDER:
	{
		const char* path = message->FindString("path");
		const char* text = message->FindString("text");
		SendNewsCreateFolder(path,text);
		break;
	}
	case H_CREATE_CATEGORY:
	{
		const char* path = message->FindString("path");
		const char* text = message->FindString("text");
		this->SendNewsCreateCategory(path,text);
		break;
	}
	case H_NEWS_SEND_GET_ARTICLELIST:
	{
		const char* path = message->FindString("path");
		SendNewsCategory(path);
		break;
	}
	case H_CONNECT_REQUESTED:
		{
			Close();
			fCancel = false;
			fAddress = message->FindString("address");
			message->FindInt16("port",(int16*)&fPort);
			PRINT(( "Connect:%s %d\n", __FILE__,__LINE__));
			fConnectThread = ::spawn_thread(Connecting, "ConnectThread", B_NORMAL_PRIORITY, this);
			::resume_thread(fConnectThread);
			break;
		}
	case H_LOGIN_REQUESTED:
		{
			const char* login = message->FindString("login");
			const char* password = message->FindString("password");
			const char* nick = message->FindString("nick");
			int32 icon = message->FindInt32("icon");

			Login(login,password,nick,icon);
			fConnected = true;

			StartThread();

			fHxTrans--;
			fHxTrans++;
			break;
		}
	case H_SEND_MSG:
	{
		uint32 sock;
		message->FindInt32("sock",(int32*)&sock);
		const char* text = message->FindString("text");
		SendMessage(sock,text,::strlen(text));
		break;
	}
	case H_CLOSE_REQUESTED:
		{
			Close();
			fConnected = false;
			fCancel = false;
			break;
		}
	case MWIN_KICK_USER:
	{
		uint32 sock;
		message->FindInt32("sock",(int32*)&sock);
		SendUserKick(sock);
		break;
	}
	case MWIN_USER_INFO_MESSAGE:
	{
		uint32 sock;
		message->FindInt32("sock",(int32*)&sock);
		SendUserGetInfo(sock);
		break;
	}
	case H_CHAT_REQUESTED:
	{
		const char* text = message->FindString("text");
		SendChat(text,::strlen(text));
		break;
	}
	case H_FILE_REQUESTED:
	{
		const char* path = message->FindString("path");
		uint32 index;
		if(message->FindInt32("index",(int32*)&index) != B_OK)
			index = 0;
		SendFileList(path,index);
		break;
	}
	case H_CHAT_OUT:
	{
		uint32 pcref;
		message->FindInt32("pcref",(int32*)&pcref);
		SendChatLeave(pcref);
		break;
	}
	case FILE_GET_FILE_MSG:
	{
		const char* path= message->FindString("remotepath");
		const char* localpath = message->FindString("localpath");

		uint32 data_size = message->FindInt32("data_size");
		SendFileGet(path,localpath,data_size,0);
		break;
	}
	case FILE_PUT_FILE_MSG:
	{
		const char* remotepath = message->FindString("remotepath");
		const char* localpath = message->FindString("localpath");
		uint16 resume = message->FindInt16("resume");
		SendFilePut(remotepath,localpath,resume,0);
		break;
	}
	case H_FILE_DELETE:
	{
		const char* path = message->FindString("path");
		SendFileDelete(path);
		break;
	}
	case H_KICK_USER:
	{
		uint32 sock = message->FindInt32("sock");
		SendUserKick(sock);
		break;
	}
	case H_BAN_USER:
	{
		uint32 sock = message->FindInt32("sock");
		SendUserKickBan(sock);
		break;
	}
	case H_SEND_USER_CHAGED:
	{
		uint32 icon =message->FindInt32("icon");
		const char* nick = message->FindString("nick");

		SendUserChange(nick,icon);
		break;
	}
	case H_FILE_CREATE_FOLDER:
	{
		const char* path = message->FindString("path");
		const char* name = message->FindString("text");
		BString fullpath = path;
		if(fullpath.ByteAt(fullpath.Length()-1) != dir_char && strlen(path) > 0)
			fullpath << dir_char;
		fullpath << name;
		this->SendMkDir(fullpath.String());
		break;
	}
	case H_FILE_GET_INFO:
	{
		const char* path = message->FindString("path");
		this->SendGetFileInfo(path);
		break;
	}
	default:
		BLooper::MessageReceived(message);
	}
}

/***********************************************************
 * Connect to server.
 ***********************************************************/
bool
HotlineClient::Connect(const char* address,uint16 port)
{
	BNetAddress addr;
	fHxTrans = 1;
	if(!fEndpoint)
	{
		fEndpoint = new BNetEndpoint();
		PRINT(("new socket was created\n"));
	}
	fServerName="";
	fSilverWingMode = false;
	fServerVersion = 0;
	fSWModeTask = 0;
	fEndpoint->SetTimeout(30*1000000);
	uint8 buf[HTLS_MAGIC_LEN];
	//
	((HApp*)be_app)->Prefs()->GetData("sock5",&fUseSock5);
	((HApp*)be_app)->TaskWindow()->PostMessage(M_REMOVE_ALL_TASK);

#ifdef DEBUG
	BNetDebug::Enable(true);
#endif
	AddTask(_("Connecting…"),fHxTrans);

	if(fEndpoint->InitCheck() != B_NO_ERROR)
	{
		PRINT(("INIT CONNECTION ERROR\n"));

		if(!fCancel)
		(new MAlert(_("SOCKET ERROR!"),_("Could not initialize socket."),_("OK"),NULL,NULL))->Go();
		return false;
	} else {
		//************ SOCKS5 Firewall or NOT *****************//
		if(fUseSock5)
		{
			// ファイアーウォールへ接続
			const char* firewall;
			((HApp*)be_app)->Prefs()->GetData("firewall",&firewall);
			uint32 firewall_port;
			((HApp*)be_app)->Prefs()->GetData("firewall_port",(int32*)&firewall_port);
			PRINT(("Firewall:%s %d",firewall ,firewall_port ));
			addr.SetTo(firewall,firewall_port);
			if(fEndpoint->Connect(addr) != B_OK)
			{
				if(!fCancel)
				(new MAlert(_("SOCKS5 Error"),_("Cannot connect to SOCKS5 server"),_("OK"),NULL,NULL))->Go();
				return false;
			}
			// ログイン
			bool auth;
			((HApp*)be_app)->Prefs()->GetData("auth",&auth);
			char buf[128];
			// authenticate with firewall
			buf[0] = 0x05; /* protocol version */
			buf[1] = 0x01; /* number of methods */
			int8 authType;
			if( auth )
				authType = 0x02; /* method username/password */
			else
				authType = 0x00; /* no authorization required */
			buf[2] = authType;

			SendData(buf,3);

			int result = ReceiveData(buf,2);
			if (result != 2 || buf[0] != 0x05 || (buf[1] != authType && buf[1] != 0x00))	// didn't read all; not SOCKS5; or won't accept method requested
			{
				if(!fCancel)
				(new MAlert("SOCKS5 Error","Firewall authentication method INVALID.","OK",NULL,NULL,B_STOP_ALERT))->Go();
				this->Close();
				return false;
			}
			if(auth)
			{
				// authenticate with firewall
				int offset = 0;
				buf[offset++] = 1; /* version of subnegotiation  - send username, password*/
				const char* user;
				const char* password;
				((HApp*)be_app)->Prefs()->GetData("firewall_user",&user);
				((HApp*)be_app)->Prefs()->GetData("firewall_password",&password);
				int8 size = strlen(user) + 1;
				buf[offset++] = (int8) size;
				memcpy(&buf[offset], user, size);		offset += size;
				size = strlen(password) + 1;
				buf[offset++] = (int8) size;
				memcpy(&buf[offset], password, size);		offset += size;

				SendData(buf, offset);

				result = ReceiveData(buf, 2);
				if (result != 2 || buf[0] != 1 || buf[1] != 0)
				{
					if(!fCancel)
					(new MAlert(_("SOCKS5 Error"),_("Firewall authentication FAILED."),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
					this->Close();
					return false;
				}
			}
			//********* ファイアーウォールを通してサーバーへ接続する ****************://
			buf[0] = 0x05; /* protocol version */
			buf[1] = 0x01; /* command TCP connect */
			buf[2] = 0x00; /* reserved */
			buf[3] = 0x01; /* address type IP v4 */
			uint32 ad = inet_addr(address);
			memcpy(&buf[4], &ad, 4);
			uint16 pt = htons(port);
			memcpy(&buf[8], &pt, 2);
			SendData(buf,10);
			result = ReceiveData(buf, 10);
			// check what firewall says
			if (result != 10 || buf[0] != 0x05 || buf[1] != 0x00)	// didn't read all; not SOCKS5; or rejected
			{
				if(!fCancel)
				(new MAlert(_("SOCKS5 Error"),_("Firewall connect FAILED."),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
				this->Close();
				return false;
			}
		//************* ノーマルな接続 ******************//
		}else{
			if(addr.SetTo(address, port) != B_OK)
			{
				if(!fCancel)
					(new MAlert(_("SOCKET ERROR!"),_("Could not resolve address"),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
				return false;
			}
			if(fEndpoint->Connect(addr) != B_OK)
			{
				if(!fCancel)
					(new MAlert(_("Error"),_("Could not connect to server"),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
				return false;
			}

		}
		UpdateTask(fHxTrans,4);

		//fEndpoint->SetNonBlocking(false);

		fHxTrans++;
		if(SendData(HTLC_MAGIC, HTLC_MAGIC_LEN) != HTLC_MAGIC_LEN)
		{
			PRINT(("send(HTLC_MAGIC, 0) failed:\n"));
			return false;
		}
		UpdateTask(fHxTrans-1,4);
		if (ReceiveData(buf, HTLS_MAGIC_LEN) != HTLS_MAGIC_LEN)
		{
			PRINT(("recv(HTLS_MAGIC) failed\n"));
			return false;
		}
		RemoveTask(fHxTrans-1);
		return true;
	}
	return false;
}


/***********************************************************
 * Connect to server and reply to app loop.
 ***********************************************************/
int32
HotlineClient::Connecting(void* data)
{
	PRINT(("Enter Connecting\n"));
	HotlineClient* client = (HotlineClient*)data;
	if( client->Connect(client->fAddress.String(),client->fPort) )
	{
		PRINT(("Connect sucess:%s\n",client->fAddress.String() ));
		if(!client->fCancel)
			be_app->PostMessage(new BMessage(H_CONNECT_SUCCESS));
	}else{
		// remove all task
		((HApp*)be_app)->TaskWindow()->PostMessage(M_REMOVE_ALL_TASK);
	}
	client->fConnectThread = -1;
	PRINT(("Exit Connecting\n"));
}

/***********************************************************
 * Login to server.
 ***********************************************************/
bool
HotlineClient::Login(const char *login, const char *pass,const char *nick, uint16 icon)
{
	uint8 buf[1024];

	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));
	uint16 llen, plen, nlen, hc = 1;
	uint32 tot_len = 8;
	fLoginTask = fHxTrans;
	AddTask(_("Logging in…"),fHxTrans);
	AddTaskList(fHxTrans,T_LOGIN_TASK);
	UpdateTask(fHxTrans,2);
	h->type = htonl(HTLC_HDR_LOGIN);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	dh->type = htons(HTLC_DATA_ICON);
	dh->len = htons(2);
	*((uint16 *)dh->data) = htons(icon);
	dh = (struct hx_data_hdr *)(&(buf[28]));

	if (login)
		llen = strlen(login);
	else
		llen = 0;
	if (pass)
		plen = strlen(pass);
	else
		plen = 0;
	if (nick)
		nlen = strlen(nick);
	else
		nlen = 0;

	if (llen) {
		register uint16 i;
		for (i = 0; i < llen; i++)
			dh->data[i] = (255 - login[i]);
		S16HTON(HTLC_DATA_LOGIN, &dh->type);
		S16HTON(llen, &dh->len);
		tot_len += 4 + llen;
		hc++;
		dh = (struct hx_data_hdr *)((uint8 *)dh + SIZEOF_HX_DATA_HDR + llen);
	}
	if (plen) {
		register uint16 i;
		for (i = 0; i < plen; i++)
			dh->data[i] = (255 - pass[i]);
		S16HTON(HTLC_DATA_PASSWORD, &dh->type);
		S16HTON(plen, &dh->len);
		tot_len += 4 + plen;
		hc++;
		dh = (struct hx_data_hdr *)((uint8 *)dh + SIZEOF_HX_DATA_HDR + plen);
	}
	if (nlen) {
		BString bnick = nick;
		int32 encoding;
		((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);

		TextUtils utils;
		if(encoding)
			utils.ConvertFromUTF8(bnick,encoding-1);
		nlen = bnick.Length();
		S16HTON(HTLC_DATA_NICK, &dh->type);
		S16HTON(nlen, &dh->len);
		memcpy(dh->data, bnick.String(), nlen);
		tot_len += 4 + nlen;
		hc++;
		dh = (struct hx_data_hdr *)((uint8 *)dh + SIZEOF_HX_DATA_HDR + nlen);
	}
	// Server Versionの設定
	S16HTON(HTLS_SERVER_VERSION, &dh->type);
	S16HTON(2,&dh->len);
	*((uint16 *)dh->data) = htons(151);
	tot_len += 6;
	hc++;

	h->hc = htons(hc);

	h->len = h->len2 = htonl(tot_len);
	SendData(buf, (unsigned)(20 + tot_len));
	UpdateTask(fHxTrans-1,7);
	fIdleTime = time(NULL);
}

/***********************************************************
 * Disconnect from server.
 ***********************************************************/
void
HotlineClient::Close()
{
	((HApp*)be_app)->TaskWindow()->PostMessage(M_REMOVE_ALL_TASK);
	RemoveAllUsers();
	fCancel = true;
	fContinue = false;
	BAutolock lock(fSocketLocker);
	status_t status;
	/*if(!isConnected())
	{
		if(fEndpoint != NULL)
		{
			PRINT(( "Waiting the end of connect thread\n"));
			fEndpoint->Close();
			if(fConnectThread >= 0)
				::wait_for_thread(fConnectThread,&status);
		}
	}else{
		if(fConnectThread>=0)
			::wait_for_thread(fConnectThread,&status);
		if(fRcvThread >=0)
		{
			PRINT(( "Waiting the end of rcv thread\n"));
			::wait_for_thread(fRcvThread,&status);
		}
	}*/
	if(fEndpoint)
		fEndpoint->Close();
	if(fConnectThread>=0)
	{
		PRINT(( "Waiting the end of connect thread\n"));
		::wait_for_thread(fConnectThread,&status);
	}
	if(fRcvThread >=0)
	{
		PRINT(( "Waiting the end of rcv thread\n"));
		::wait_for_thread(fRcvThread,&status);
	}

	delete fEndpoint;
	fEndpoint = NULL;
	fConnected = false;

	fRcvThread = -1;
	PRINT(( "Waiting the end of connect thread2\n"));

	hx_reset();
	PRINT(( "Close end:%s Line:%d\n",__FILE__,__LINE__ ));
}

/***********************************************************
 * is connected ?
 ***********************************************************/
bool
HotlineClient::isConnected()
{
	return fConnected;
}

/***********************************************************
 * Start receive thread.
 ***********************************************************/
status_t
HotlineClient::StartThread()
{
	fRcvThread = ::spawn_thread(ThreadEntry, "ReceiveThread", B_NORMAL_PRIORITY, this);
	PRINT(("SpawnRecvThread()\n"));
	return (::resume_thread(fRcvThread));
}

/***********************************************************
 * Receive thread function.
 ***********************************************************/
int32
HotlineClient::ThreadEntry(void *arg)
{
	HotlineClient *obj = (HotlineClient*)arg;
	return obj->ListenToServer();
}

/***********************************************************
 * Listen to server loop.
 ***********************************************************/
int32
HotlineClient::ListenToServer()
{
	int32 r;
	hx_reset();

	struct timeval timeout;
	timeout.tv_sec=30; timeout.tv_usec=0;
	bigtime_t btimeout = 50000; //00.5 sec
	fContinue = true;

	while(fContinue)
	{
		/*FD_ZERO(&readfds);
		FD_SET(fEndpoint->Socket(), &readfds);
		if(::select(fEndpoint->Socket() + 1, &readfds, NULL, NULL, &timeout)  > 0)
		*/
		BAutolock lock(fSocketLocker);
		if(fEndpoint->IsDataPending(btimeout))
		{
			//bool readyToRead  = FD_ISSET(fEndpoint->Socket(), &readfds);
			//if(!readyToRead)
			//	continue;

			r = fEndpoint->Receive(&fHxBuf[fHxPos],fHxLen);
			//r = ReceiveData(&fHxBuf[fHxPos],fHxLen);
			if (r == B_ERROR )
			{
				BString title = _("Connection was closed");
				title << "\n" << fEndpoint->ErrorStr() << " " << _("Type")<<": " << (long)fEndpoint->Error();
				beep();
				(new BAlert("",title.String(),_("OK"),NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT))->Go();
				fEndpoint->Close();
				fConnected = false;
				fContinue = false;
				delete fEndpoint;
				fEndpoint = NULL;
				((HApp*)be_app)->TaskWindow()->PostMessage(M_REMOVE_ALL_TASK);
				RemoveAllUsers();
				break;
			}
			fHxPos += r;
			fHxLen -= r;

			PRINT(( "fHxPos:%d fHxLen:%d rcv:%d\n",fHxPos,fHxLen,r));

			if (!fHxLen)
			{
				if(fHxFun >=0)
					Fook_Hx_Fun();
				else
					hx_reset();
			}/* else {
				hx_reset();
			}*/
			if(r == 0)
			{
			 	BString title = _("Server was stopped or you were kicked.");
				//title << fEndpoint->ErrorStr() << " Type: " << (long)fEndpoint->Error();
				beep();
				(new MAlert(_("Connection was closed…"),title.String(),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
				fEndpoint->Close();
				fConnected = false;
				delete fEndpoint;
				fEndpoint = NULL;
				((HApp*)be_app)->TaskWindow()->PostMessage(M_REMOVE_ALL_TASK);
				RemoveAllUsers();
				break;
			}
			bool keep;
			((HApp*)be_app)->Prefs()->GetData("keep_alive",&keep);
			if(keep)
				SendPing();
		}else{
			bool keep;
			((HApp*)be_app)->Prefs()->GetData("keep_alive",&keep);
			if(keep)
				SendPing();
		}
	}
	PRINT(("Listen thread quit...\n"));
	fRcvThread = -1;
	return 0;
}

/***********************************************************
 * Receive chat.
 ***********************************************************/
void
HotlineClient::ReceiveChat()
{
	//BAutolock lock(fBufferLocker);
	be_app->PostMessage(SOUND_CHAT_SND);
	uint32 pcref = 0;
	char *chat = NULL;
	uint16 len = 0;

	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
		switch (ntohs(dh->type)) {
			case HTLC_DATA_CHAT_REF:
				dh_getint(pcref);
				break;
			case HTLC_DATA_CHAT:
				len = ntohs(dh->len);
				chat = new char[len +1];
				memcpy(chat, dh->data, len);
				chat[len] = '\0';
				break;
		}
	dh_end()
	if (!len)
		goto ret;

	if (!pcref)
	{
		BMessage msg(H_RCV_CHAT);
		msg.AddString("text",chat);
		be_app->PostMessage(&msg);
	}else{
		BMessage msg(H_RCV_PRV_CHAT);
		msg.AddString("text",chat);
		msg.AddInt32("pcref",(int32)pcref);
		be_app->PostMessage(&msg);
	}
ret:
	delete[] chat;
	hx_reset();
}

/***********************************************************
 * ReceiveBroadcast
 ***********************************************************/
void
HotlineClient::ReceiveBroadcast()
{
	PRINT(( "Broadcast" ));
	hx_reset();
}

/***********************************************************
 * ReceiveSelfInfo
 ***********************************************************/
void
HotlineClient::ReceiveSelfInfo()
{
	PRINT(( "Selfinfo\n" ));
	hx_reset();
}

/***********************************************************
 * Receive agreement.
 ***********************************************************/
void
HotlineClient::ReceiveAgreement()
{
	//BAutolock lock(this);
	char *text = NULL;

	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)

	if (ntohs(dh->type) == HTLS_DATA_AGREEMENT)
	{
		if(ntohs(dh->len) > 0)
		{
			PRINT(("\nAgreement:\n%.*s", ntohs(dh->len), dh->data));
			text = new char[ntohs(dh->len)+1];
			::memset(text,0,ntohs(dh->len));
			::memcpy(text,dh->data,ntohs(dh->len));
			text[ntohs(dh->len)] = '\0';

			BMessage msg(H_RCV_AGREEMENT);
			msg.AddString("text",text);
			be_app->PostMessage(&msg);
		}
	}
	dh_end()

	delete[] text;
	hx_reset();
}

/***********************************************************
 * Receive message.
 ***********************************************************/
void
HotlineClient::ReceiveMessage()
{
	//BAutolock lock(this);
	uint16 sock = 0;
	uint16 msglen = 0, nlen = 0;
	char nick[32];
	char *msg = NULL;
	uint16 icon;
	BMessage *mesg = NULL;

	::memset(nick,0,32);

	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
		switch (ntohs(dh->type)) {
			case HTLC_DATA_SOCKET:
				dh_getint(sock);
				break;
			case HTLC_DATA_NICK:
				nlen = ntohs(dh->len) > 31 ? 31 : ntohs(dh->len);
				memcpy(nick, dh->data, nlen);
				break;
			case HTLC_DATA_MSG:
				msglen = ntohs(dh->len);
				msg = new char[ntohs(dh->len)+1];
				memcpy(msg, dh->data, msglen);
				msg[ntohs(dh->len)] = '\0';
				break;
			case HTLC_DATA_ICON:
				dh_getint(icon);
				break;
		}
	dh_end()

	PRINT(( "Message Len: %d\n" ,msglen ));

	if (!msglen)
		goto ret;

	if(sock == 0)
	{
		mesg = new BMessage(H_RCV_GLOBAL_MSG);
		mesg->AddString("text",msg);
	}else{
		mesg = new BMessage(H_RCV_MSG);
		mesg->AddString("text",msg);
		mesg->AddString("nick",nick);
		mesg->AddInt32("sock",(int32)sock);
	}
	be_app->PostMessage(mesg);
	delete mesg;
	delete[] msg;
ret:
	hx_reset();
}


/***********************************************************
 * Receive polite quit
 ***********************************************************/
void
HotlineClient::ReceivePliteQuit()
{
	uint16 msglen = 0;
	char *msg = NULL;
	BString result;
	TextUtils utils;
	int32 encoding = 0;

	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
		switch (ntohs(dh->type)) {
			case HTLC_DATA_MSG:
				msglen = ntohs(dh->len);
				msg = new char[msglen+1];
				::memcpy(msg, dh->data, msglen);
				break;
	}
	dh_end()
	result = _("Server will stop services");
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
	utils.ConvertReturnsToLF(msg);
	if(encoding)
		utils.ConvertToUTF8(&msg,encoding-1);
	result << "\n" << msg;
	(new MAlert(_("Message"),result.String(),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();

	delete[] msg;
	this->Close();
	hx_reset();
}

/***********************************************************
 * Receive task type packet.
 ***********************************************************/
void
HotlineClient::ReceiveTask()
{
	struct hx_hdr *h = (struct hx_hdr*)fHxBuf;

	uint32 trans = ntohl(h->trans);
	uint32 flag = ntohl(h->flag);
	//uint16 hc = ntohs(h->hc);

	PRINT(( "Receive Task:%d Flag:%d\n" ,trans , flag ));

	if(trans == fLoginTask)
	{
		fLoginTask = 0;
		if(flag == 0)
		{
			this->SendGetUsersList();
			this->SendSilverWingMode();
		}else{
			ReceiveTaskError();
			hx_reset();
			RemoveTask(trans);
			return;
		}
	}

	HLTASK *hltask = FindTaskList(trans);
	if(flag == 1 && trans != fSWModeTask)
	{
		ReceiveTaskError();
		hx_reset();
		RemoveTask(trans);
		if(hltask)
			RemoveTaskList(hltask);
		return;
	}
	if(!hltask)
	{
		hx_reset();
		return;
	}
	//
	switch(hltask->type)
	{
	case T_USER_LIST_TASK:
		{
			ReceiveUserList();
			break;
		}
	case T_FILE_LIST_TASK:
		{
			ReceiveFileList();
			break;
		}
	case T_PRV_USER_TASK:
		{
			ReceivePrvChatUserList();
			break;
		}
	case T_FILE_TRANS_TASK:
		{
			int32 count = fFileTransArray.CountItems();
			for(register int32 i = 0;i <count ;i++)
			{
				FileTrans *filetrans = (FileTrans*)fFileTransArray.ItemAt(i);
				if(filetrans->task == trans)
				{
					if(filetrans->isDownload)
						ReceiveFileGet();
					else
						ReceiveFilePut();
					break;
				}
			}
			break;
		}
	case T_FILE_INFO_TASK:
		{
			ReceiveFileInfo();
			break;
		}
	case T_USER_INFO_TASK:
		{
			ReceiveUserInfo();
			break;
		}
	// receive article list
	case T_NEWS_CATEGORY_TASK:
		{
			ReceiveNewsCategory();
			break;
		}
	// receive folder list
	case T_NEWS_FOLDER_ITEM_TASK:
		{
			ReceiveNewsFolderItem();
			break;
		}
	// get news article content
	case T_NEWS_DATA_TASK:
		{
			ReceiveNews();
			break;
		}
	// reply when create private chat
	case T_PRV_INVITE_TASK:
		{
			ReceiveChatInvite(true);
			break;
		}
	case T_NEWS_FILE_TASK:
		{
			ReceiveNewsFile();
			break;
		}
	case T_LOGIN_TASK:
		{
			be_app->PostMessage(SOUND_LOGGEDIN_SND);
			ReceiveServerVersion();
			break;
		}
	default:
		break;
	}

	RemoveTask(trans);
	RemoveTaskList(hltask);
	hx_reset();
}


/***********************************************************
 * Send make directory.
 ***********************************************************/
void
HotlineClient::SendMkDir(const char* path)
{
	char buf[SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));
	uint16 nlen;
	char const *p;

	h->type = htonl(HTLC_HDR_FILE_MKDIR);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	if (!path || !path[0] || !(p = dirchar_basename(path)))
		return;
	dh->type = htons(HTLC_DATA_FILE);
	dh->len = htons((nlen = strlen(p)));
	if (p != path) {
		dh = path_to_hldir(path, 1);
		h->len = h->len2 = htonl(2 + (SIZEOF_HX_DATA_HDR * 2) + ntohs(dh->len) + nlen);
		h->hc = htons(2);

		SendData(buf,26);
		SendData(p,nlen);
		SendData(dh,SIZEOF_HX_DATA_HDR + ntohs(dh->len));
		xfree(dh);
	} else {
		h->len = h->len2 = htonl(2 + SIZEOF_HX_DATA_HDR + nlen);
		h->hc = htons(1);

		SendData(buf,26);
		SendData(p,nlen);
	}
}


/***********************************************************
 * Send get file info command.
 ***********************************************************/
void
HotlineClient::SendGetFileInfo(const char* path)
{
	char buf[SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));
	uint16 nlen;
	char const *p;
	uint32 task = fHxTrans;

	h->type = htonl(HTLC_HDR_FILE_GETINFO);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	if (!path || !path[0] || !(p = dirchar_basename(path)))
		return;

	AddTask(_("Getting file infomation…"),task);
	AddTaskList(task,T_FILE_INFO_TASK);

	dh->type = htons(HTLC_DATA_FILE);
	dh->len = htons((nlen = strlen(p)));
	if (p != path) {
		dh = path_to_hldir(path, 1);
		h->len = h->len2 = htonl(2 + (SIZEOF_HX_DATA_HDR * 2) + ntohs(dh->len) + nlen);
		h->hc = htons(2);
		this->UpdateTask(task,3);

		SendData(buf,26);
		UpdateTask(task,5);
		SendData( p, nlen);
		UpdateTask(task,7);
		SendData( dh, SIZEOF_HX_DATA_HDR + ntohs(dh->len));
		this->UpdateTask(task,9);
		xfree(dh);
	} else {
		h->len = h->len2 = htonl(2 + SIZEOF_HX_DATA_HDR + nlen);
		h->hc = htons(1);
		this->UpdateTask(task,3);

		SendData( buf, 26);
		UpdateTask(task,5);
		SendData( p, nlen);
		this->UpdateTask(task,7);
	}
}


/***********************************************************
 * Send get article list.
 ***********************************************************/
void
HotlineClient::SendNewsCategory(const char* path)
{
	uint8 buf[SIZEOF_HX_HDR];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh;

	//this->AddTask("Getting article list ...",fHxTrans);
	AddTaskList(fHxTrans,T_NEWS_CATEGORY_TASK);
	h->type = htonl(HTLC_HDR_NEWSCATLIST);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	if (!path || !path[0] || (path[0] == dir_char && !path[1])) {
		h->len = h->len2 = htonl(2);
		h->hc = 0;

		SendData(buf, SIZEOF_HX_HDR);
		//this->UpdateTask(trans->trans,6);
		return;
	}
	dh = news_path_to_hldir(path, 0);
	h->len = h->len2 = htonl(2 + SIZEOF_HX_DATA_HDR + ntohs(dh->len));
	h->hc = htons(2);

	NewsCategoryTrans *task = new NewsCategoryTrans;
	task->trans = fHxTrans-1;
	task->path = path;

	if(this->Lock())
	{
		fNewsCategoryArray.AddItem(task);
		this->Unlock();
	}

	SendData(buf, SIZEOF_HX_HDR);
	this->UpdateTask(fHxTrans-1,4);
	SendData(dh, SIZEOF_HX_DATA_HDR + ntohs(dh->len));
	this->UpdateTask(fHxTrans-1,6);
	xfree(dh);
}


/***********************************************************
 * Send create category command.
 ***********************************************************/
void
HotlineClient::SendNewsCreateCategory(const char* path, const char* name)
{
	uint8 buf[22];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = NULL;
	struct hx_data_hdr *dh2;
	this->AddTask(_("Creating news category…"),fHxTrans);
	h->type = htonl(HTLC_HDR_MAKECATEGORY);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->hc = htons(1);

	dh2 = (hx_data_hdr*)malloc(SIZEOF_HX_DATA_HDR + strlen(name) );
	dh2->len  = htons(strlen(name));
	dh2->type = htons(HTLS_DATA_NEWS_CATEGORY);
	::memcpy(dh2->data, name,strlen(name));

	if( strlen(path) == 0)
	{
		h->len = h->len2 = htonl( 2 + SIZEOF_HX_DATA_HDR + ntohs(dh2->len) );

		SendData(h,22);
		SendData(dh2,SIZEOF_HX_DATA_HDR + ntohs(dh2->len));
		xfree(dh2);
		return;
	}
	h->hc = htons(2);
	dh = this->news_path_to_hldir(path,0);
	h->len = h->len2 = htonl( 2 + SIZEOF_HX_DATA_HDR*2 + ntohs(dh->len)+ ntohs(dh2->len) );
	this->UpdateTask(fHxTrans-1,2);

	SendData(  buf, 22);
	UpdateTask(fHxTrans-1,4);
	SendData(dh2,SIZEOF_HX_DATA_HDR + ntohs(dh2->len) );
	UpdateTask(fHxTrans-1,6);
	SendData(dh,SIZEOF_HX_DATA_HDR + ntohs(dh->len) );
	this->UpdateTask(fHxTrans-1,8);
	xfree(dh2);
	xfree(dh);
	this->RemoveTask(fHxTrans-1);
}


/***********************************************************
 * Send create news folder command.
 ***********************************************************/
void
HotlineClient::SendNewsCreateFolder(const char* path, const char* name)
{
	uint8 buf[22];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = NULL;
	struct hx_data_hdr *dh2;

	this->AddTask(_("Creating news folder…"),fHxTrans);
	h->type = htonl(HTLC_HDR_MAKENEWSDIR);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->hc = htons(1);

	dh2 = (hx_data_hdr*)malloc(SIZEOF_HX_DATA_HDR + strlen(name) );
	dh2->len  = htons(strlen(name));
	dh2->type = htons(HTLC_DATA_FILE);
	::memcpy(dh2->data, name,strlen(name));
	//AfxMessageBox(path);
	if( strlen(path) == 0)
	{
		h->len = h->len2 = htonl( 2 + SIZEOF_HX_DATA_HDR + ntohs(dh2->len) );

		SendData( h,22);
		SendData( dh2,SIZEOF_HX_DATA_HDR + ntohs(dh2->len));

		xfree(dh2);
		return;
	}
	h->hc = htons(2);
	dh = this->news_path_to_hldir(path,0);
	h->len = h->len2 = htonl( 2 + SIZEOF_HX_DATA_HDR*2 + ntohs(dh->len)+ ntohs(dh2->len) );
	this->UpdateTask(fHxTrans-1,2);
	SendData(  buf, 22);
	UpdateTask(fHxTrans-1,5);
	SendData(dh2,SIZEOF_HX_DATA_HDR + ntohs(dh2->len) );
	UpdateTask(fHxTrans-1,9);
	SendData(dh,SIZEOF_HX_DATA_HDR + ntohs(dh->len) );
	xfree(dh2);
	xfree(dh);
	this->RemoveTask(fHxTrans-1);
}


/***********************************************************
 * Send delete news thread command.
 ***********************************************************/
void
HotlineClient::SendDeleteThread(const char* category, uint16 thread)
{
	uint8 buf[22+6];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));
	struct hx_data_hdr *dh2;

	h->type = htonl(HTLC_HDR_DELETETHREAD);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;

	h->hc = htons(2);

	dh->type = htons(HTLS_DATA_NEWS_THREADID);
	dh->len = htons(2);
	S16HTON(thread, dh->data);

	dh2 = this->news_path_to_hldir(category,0);

	h->len = h->len2 = htonl( 2 +SIZEOF_HX_DATA_HDR + 6+ ntohs(dh2->len) );

	SendData(  buf, 28);
	SendData( dh2,SIZEOF_HX_DATA_HDR+ ntohs(dh2->len));
	xfree(dh2);
}

/***********************************************************
 * Send refuse private chat.
 ***********************************************************/
void
HotlineClient::SendDeclineChat(uint32 pcref)
{
	uint8 buf[30];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));

	h->type = htonl(HTLC_HDR_CHAT_DECLINE);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->len = h->len2 = htonl(10);
	h->hc = htons(1);
	dh->type = htons(HTLC_DATA_CHAT_REF);
	dh->len = htons(4);
	S32HTON(pcref, dh->data);

	SendData(buf, 30 );
}


/***********************************************************
 * Send delete category command.
 ***********************************************************/
void
HotlineClient::SendDeleteCategory(const char* category)
{
	uint8 buf[22];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh;

	h->type = htonl(HTLC_HDR_DELNEWSDIRCAT);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;

	h->hc = htons(2);

	dh = this->news_path_to_hldir(category,0);
	h->len = h->len2 = htonl( 2 + SIZEOF_HX_DATA_HDR + ntohs(dh->len) );

	SendData(  buf, 22);
	SendData(dh,SIZEOF_HX_DATA_HDR + ntohs(dh->len) );

	xfree(dh);
}


/***********************************************************
 * Get news category  list.
 ***********************************************************/
void
HotlineClient::ReceiveNewsFolderItem()
{
	struct hx_hdr *hdr = (hx_hdr*)fHxBuf;

	uint8	hc = ntohs(hdr->hc);
	uint32 trans = ntohl(hdr->trans);
	uint32 index = 0;
	BMessage msg(H_NEWS_RECEIVE_FOLDER);
	if(this->Lock())
	{
		int size = this->fNewsTransArray.CountItems();
		for(int i = 0; i < size ;i++)
		{
			NewsTrans *nt = (NewsTrans*)fNewsTransArray.ItemAt(i);
			if(nt->trans == trans)
			{
				index = nt->item_index;
				delete nt;
				fNewsTransArray.RemoveItem(i);
			}
		}

		struct folderitem {
			uint16 type;
			uint16 len;
			uint16 item_type;
			uint16 posted;
		};

		int offset = 22;
		//this->UpdateTask(trans,8);
		for(int j = 0;j < hc;j++)
		{
			struct folderitem *folder = (folderitem*)&fHxBuf[offset];
			uint16 type = ntohs(folder->item_type);
			uint16 nlen = ntohs(folder->len);
			uint16 posted = ntohs(folder->posted);

			uint8 slen;
			int name_offset = 0;
			if(type == 3 )
			{
				::memcpy(&slen ,&fHxBuf[ offset+ 32],1);
				name_offset = 33;
			}else{
				slen = nlen - 6;
				name_offset = 9;
			}
			char *name = new char[slen+ 1];
			::memset(name,0,slen + 1);
			::memcpy(name,&fHxBuf[ offset + name_offset ],slen);
			name[slen] = '\0';

			PRINT(( "Name: %s\n", name ));
			msg.AddString("name",name);
			delete[] name;
			msg.AddInt16("posted",posted);

			PRINT(( "Type : %d\n", type ));

			msg.AddInt16("type",type);

			PRINT(( "Index: %d\n" , index ));

			msg.AddInt32("index",index);
			offset += nlen + 6 - 2;
		}
		this->Unlock();
	}
	be_app->PostMessage(&msg);
}


/***********************************************************
 * Receive news article list.
 ***********************************************************/
void
HotlineClient::ReceiveNewsCategory()
{
	typedef struct{
	uint16 unknown;
	uint16 unknown2;
	uint16 index;
	uint16 base_year;
	uint16 pad;
	uint32 seconds;
	uint32 parent_id;
	uint32 newssize;
	}NewNewsItem;

	char *subject,*sender,*mime;
	uint16 index;
	struct hx_hdr *hd = (hx_hdr*)fHxBuf;
	uint32 trans = ntohl(hd->trans);
	NewsItemHdr *hdr = (NewsItemHdr*)&fHxBuf[SIZEOF_HX_HDR];
	BString date;
	//uint16 hdrtype = ntohs(hdr->type);
	uint32 numItems = ntohl(hdr->numItems);

	uint32 offset = SIZEOF_HX_HDR + 12;
	uint8  datasize = 0;
	BMessage msg(H_NEWS_RECEIVE_ARTICLELIST);
	BString category = "";
	if( this->Lock() )
	{
	int count = fNewsCategoryArray.CountItems();
	for(int i = 0;i < count;i++)
	{
		NewsCategoryTrans *task = (NewsCategoryTrans*)fNewsCategoryArray.ItemAt(i);
		if(task->trans == trans)
		{
			category = task->path;
			fNewsCategoryArray.RemoveItem(i);
			delete task;
		}
	}
	for(uint32 i = 0 ;i < numItems;i++)
	{
		NewNewsItem *item = (NewNewsItem*)&fHxBuf[offset];
		offset += 24;
		index = ntohs(item->index);
		uint16 base_year = ntohs(item->base_year);
		uint32 tmpsec;// = ntohl(item->seconds);

		::memcpy(&tmpsec,&fHxBuf[offset - 24 + 10],4);
		uint32 sec = ntohl(tmpsec);

		if(base_year == 1970)
		{
		}else if(base_year == 2000){
			sec += 946684800U - 32400;
		}else if(base_year == 1904){
			sec -= 2082844800U + 32400;
		}

		//uint32 newssize = ntohl(item->newssize);
		uint32 parent_id = ntohl(item->parent_id);

		NewsItemData* data = (NewsItemData*)&fHxBuf[offset];
		memcpy(&datasize,&data->len,1);

		offset += 1;
		subject = new char[datasize+1];
		memcpy(subject,&fHxBuf[offset],datasize);
		subject[datasize] = '\0';
		offset += datasize;
		data = (NewsItemData*)&fHxBuf[offset];
		offset += 1;
		memcpy(&datasize,data->len,1);

		sender = new char[datasize+1];
		memcpy(sender,&fHxBuf[offset],datasize);
		sender[datasize] = '\0';

		offset += datasize;

		data = (NewsItemData*)&fHxBuf[offset];
		offset += 1;
		memcpy(&datasize,data->len,1);
		mime = new char[datasize+1];
		memcpy(mime,&fHxBuf[offset],datasize);
		mime[datasize] = '\0';

		offset += datasize;
		// メッセージを作成して送信

		msg.AddString("subject",subject);
		msg.AddString("sender",sender);
		//msg.AddString("date",date.String());
		msg.AddInt32("time",sec);
		msg.AddInt32("parent_id",parent_id);
		msg.AddInt16("index",index);
		delete[] mime;
		delete[] subject;
		delete[] sender;

		PRINT(("Subject:%s\n", subject ));

	}
	this->Unlock();
	}
	msg.AddString("category",category.String());
	be_app->PostMessage(&msg);
}

/***********************************************************
 * Send move files.
 ***********************************************************/
void
HotlineClient::SendFileMove(const char* filename,const char* path ,const char* dest_path)
{
	// make old file path and file name.
	//	HTLC_HDR_FILE_MOVE
	/**************** HEADER ***********/
	char buf[SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));
	struct hx_data_hdr *dest;
	uint16 nlen;
	char const *p;

	PRINT(( "Name: %s\n" , filename ));
	PRINT(( "Move: %s\n", path ));
	PRINT(( "Dest: %s\n" , dest_path ));

	AddTask(_("Moving files…"),fHxTrans);

	h->type = htonl(HTLC_HDR_FILE_MOVE);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	/****************** dest PATH ****************/
	p = dirchar_basename(path);
	/*if (!path || !path[0] || !())
		return;*/
	dh->type = htons(HTLC_DATA_FILE);
	dh->len = htons((nlen = strlen(filename)));
	if (strlen(path) > 0) {
		dh = path_to_hldir(path, false);
		dest = path_to_hldir(dest_path,false);
		dest->type = htons(HTLC_DATA_DIR_RENAME);
		h->len = h->len2 = htonl(2 + (SIZEOF_HX_DATA_HDR * 3) + ntohs(dest->len)
									+ ntohs(dh->len) + nlen);
		h->hc = htons(3);

		UpdateTask(fHxTrans-1,4);
		SendData( buf, 26);
		UpdateTask(fHxTrans-1,5);
		SendData( filename, nlen);
		UpdateTask(fHxTrans-1,6);
		SendData( dh, SIZEOF_HX_DATA_HDR + ntohs(dh->len));
		UpdateTask(fHxTrans-1,7);
		SendData( dest, SIZEOF_HX_DATA_HDR + ntohs(dest->len));
		UpdateTask(fHxTrans-1,8);
		RemoveTask(fHxTrans-1);

		xfree(dh);
		xfree(dest);
	} else {
		dest = path_to_hldir(dest_path,false);
		dest->type = htons(HTLC_DATA_DIR_RENAME);
		h->len = h->len2 = htonl(2 + SIZEOF_HX_DATA_HDR *2+ ntohs(dest->len) + nlen);
		h->hc = htons(2);

		UpdateTask(fHxTrans-1,5);
		SendData( buf, 26);
		UpdateTask(fHxTrans-1,6);
		SendData( filename, nlen);
		UpdateTask(fHxTrans-1,7);
		SendData( dest,SIZEOF_HX_DATA_HDR + ntohs(dest->len) );
		UpdateTask(fHxTrans-1,9);
		RemoveTask(fHxTrans-1);

		xfree(dest);
	}
}

/***********************************************************
 * Receive file info.
 ***********************************************************/
void
HotlineClient::ReceiveFileInfo()
{
	uint16 ilen = 0;
	uint32 icon=0,size=0;
	uint32 sec;
	DateTime time;
	BString type = "";
	BString modified = "";
	BString created = "";
	BString creator = "";
	BString name = "";
	BString comment = "";
	time_t timet;
	char *tmp = NULL;
	//fHxTrans--;
	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
		switch (ntohs(dh->type)) {
			case HTLS_DATA_FILE_ICON:
				//icon = ntohs(dh->len);
				dh_getint(icon);
				break;
			case HTLS_DATA_FILE_TYPE:
				ilen = ntohs(dh->len);
				tmp = new char[ilen+1];
				::memcpy(tmp,dh->data,ilen);
				tmp[ilen] = '\0';
				type = tmp;
				delete[] tmp;
				break;
			case HTLS_DATA_FILE_CREATOR:
				ilen = ntohs(dh->len);
				tmp = new char[ilen+1];
				::memcpy(tmp,dh->data,ilen);
				tmp[ilen] = '\0';
				creator = tmp;
				delete[] tmp;
				break;
			case HTLS_DATA_FILE_SIZE:
				dh_getint(size);
				break;
			case HTLS_DATA_FILE_NAME:
				ilen = ntohs(dh->len);
				tmp = new char[ilen+1];
				::memcpy(tmp,dh->data,ilen);
				tmp[ilen] = '\0';
				name = tmp;
				delete[] tmp;
				break;
			case HTLS_DATA_FILE_COMMENT:
				ilen = ntohs(dh->len);
				tmp = new char[ilen+1];
				::memcpy(tmp,dh->data,ilen);
				tmp[ilen] = '\0';
				comment = tmp;
				delete[] tmp;
				break;
			case HTLS_DATA_FILE_CDATE:
				//dh_getint(cdate);
				::memcpy(&time,dh->data,ntohs(dh->len));
				sec = ntohl(time.seconds);
				timet = sec;

				if(ntohs(time.base_year) == 1970)
				{
					created=ctime(&timet);
				}else if(ntohs(time.base_year) == 2000){
					timet += 946684800U;
					created=ctime(&timet);
				}else if(ntohs(time.base_year) == 1904){
					timet -= 2082844800U ;
					created=ctime(&timet);
				}
				break;
			case HTLS_DATA_FILE_MDATE:
				::memcpy(&time,dh->data,ntohs(dh->len));
				sec = ntohl(time.seconds);
				timet = sec;
				if(ntohs(time.base_year) == 1970)
				{
					modified=ctime(&timet);
				}else if(ntohs(time.base_year) == 2000){
					timet += 946684800U - 32400;
					modified=ctime(&timet);
				}else if(ntohs(time.base_year) == 1904){
					timet -= 2082844800U + 32400;
					modified=ctime(&timet);
				}

				break;
	}
	dh_end()

	BString info="";
	info << "File name: ";
	info << name;
	info << "\n";
	info << "File type: ";
	info << type;
	info << "\n";
	info << "Creator: ";
	info << creator;
	info << "\n";
	info << "Size: ";
	info << size;
	info << "\n";
	info << "Created: ";
	info << created;
	info << "Modified: ";
	info << modified;
	info << "Comment:\n";
	info << comment;
	info << "\n";
	char* tmp2 = new char[info.Length()+1];
	::strcpy(tmp2,info.String());
	int32 encoding;
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
	if(encoding)
		TextUtils().ConvertToUTF8(&tmp2,encoding-1);
	info  = tmp2;
	delete[] tmp2;

	HInfoWindow *win = new HInfoWindow(RectUtils().CenterRect(300,300)
					,_("File infomation"),info.String());
	win->Show();
	hx_reset();
}


/***********************************************************
 * Send get news articles.
 ***********************************************************/
void
HotlineClient::SendNewsGet(const char* category, uint16 thread,const char* mime)
{
	struct hx_hdr hdr;
	struct hx_data_hdr *dh;

//	uint32 category_len = ::strlen(category);
	uint32 task = fHxTrans;
	int mime_len = ::strlen(mime);

	//AddTask("Getting a article ...",task);
	AddTaskList(task,T_NEWS_DATA_TASK);
	/******** hxhdr *******/
	hdr.type = htonl(HTLC_HDR_GETTHREAD);
	hdr.trans = htonl(fHxTrans++);
	hdr.flag = htonl(0);

	hdr.hc = htons(3);

	dh = news_path_to_hldir(category, 0);
	/******** New Hdr *********/

	NewsMidHdr midhdr;
	midhdr.type = htons(HTLS_DATA_NEWS_THREADID);
	midhdr.unknown = htons(2);
	midhdr.thread = htons(thread);

	NewsBottomHdr btmhdr;
	btmhdr.type = htons(HTLS_DATA_NEWS_NEWSTYPE);
	btmhdr.len = htons(mime_len);

	uint32 hlen = 2 + SIZEOF_HX_DATA_HDR + ntohs(dh->len) + 6+4+mime_len;
	hdr.len = htonl(hlen);
	hdr.len2 = htonl(hlen);

	UpdateTask(task,2);
	SendData(&hdr,22);
	UpdateTask(task,3);
	SendData(dh,SIZEOF_HX_DATA_HDR + ntohs(dh->len));
	UpdateTask(task,4);
	SendData(&midhdr,6);
	UpdateTask(task,5);
	SendData(&btmhdr,4);
	UpdateTask(task,6);
	SendData(mime, mime_len);
	UpdateTask(task,7);

	//this->UpdateTask(task,7);
	xfree(dh);
}


/***********************************************************
 * Receive news article
 ***********************************************************/
void
HotlineClient::ReceiveNews()
{
	uint32 sec = 0;
	uint16 base_year = 0, nlen = 0;
	BString nick;
	BString msg;
	BString subject;
	BString str;
	BString date;
	BString tmp;
	char *buf = NULL;
	time_t timet;

	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)

		switch (ntohs(dh->type)) {
			case HTLS_DATA_NEWS_NEWSDATE:
				DateTime time;
				nlen = ntohs(dh->len);
				::memcpy(&time,dh->data,nlen);
				base_year = ntohs(time.base_year);
				sec = ntohl(time.seconds);
				timet = sec;
				if(base_year == 1970)
				{
					date = ctime(&timet);
				}else if(base_year == 2000){
					timet += 946684800U - 32400;
					date = ctime(&timet);
				}else if(base_year == 1904){
					//timet -= 2082844800U ;
					timet -= 2082844800U + 32400;
					date = ctime(&timet);
				}
				break;
			case HTLS_DATA_NEWS_NEWSAUTHOR:
				nlen = ntohs(dh->len);
				buf = new char[nlen+1];
				memcpy(buf, dh->data, nlen);
				buf[nlen]= '\0';
				nick = buf;
				delete[] buf;
				break;
			case HTLS_DATA_NEWS_NEWSDATA:
				nlen = ntohs(dh->len);
				buf = new char[nlen+1];
				memcpy(buf, dh->data, nlen);
				buf[nlen] = '\0';
				msg =buf;
				delete[] buf;
				break;
			case HTLS_DATA_NEWS_NEWSSUBJECT:
				nlen = ntohs(dh->len);
				buf = new char[nlen+1];
				memcpy(buf, dh->data, nlen);
				buf[nlen] = '\0';
				subject = buf;
				delete[] buf;
				break;
	}
	dh_end()
	BMessage message(H_RECEIVE_THREAD);
	message.AddString("subject",subject.String());
	message.AddString("sender",nick.String());
	message.AddString("date",date.String());
	message.AddString("content",msg.String());
	be_app->PostMessage(&message);

	hx_reset();
}

/***********************************************************
 * Post news thread.
 ***********************************************************/
void
HotlineClient::SendNewsPostThread(const char* path, uint16 reply_thread, const char* name, uint16 parent_thread, const char* message, const char* mime)
{
	uint8 buf[22];
	uint8 buf2[6];
	uint8 buf3[6];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = NULL;
	struct hx_data_hdr *dh2 = (hx_data_hdr*)buf2;
	struct hx_data_hdr *dh3;
	struct hx_data_hdr *dh4 = (hx_data_hdr*)buf3;
	struct hx_data_hdr *dh5;
	struct hx_data_hdr *dh6;
	// Path
	h->type = htonl(HTLC_HDR_POSTTHREAD);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->hc = htons(6);
	dh = this->news_path_to_hldir(path,0);
	// Reply Thread
	dh2->type = htons(HTLS_DATA_NEWS_THREADID);
	dh2->len  = htons(2);
	S16HTON(reply_thread,dh2->data);
	//Subject
	dh3 = (hx_data_hdr*)malloc(SIZEOF_HX_DATA_HDR+strlen(name));
	dh3->type = htons(HTLS_DATA_NEWS_NEWSSUBJECT);
	dh3->len = htons(strlen(name));
	::memcpy(dh3->data,name,strlen(name));
	// Parent Thread
	dh4->type = htons(HTLS_DATA_NEWS_PARENTTHREAD);
	dh4->len  = htons(2);
	S16HTON(reply_thread,dh4->data);
	// Mime
	dh5 = (hx_data_hdr*)malloc(SIZEOF_HX_DATA_HDR+strlen(mime));
	dh5->type = htons(HTLS_DATA_NEWS_NEWSTYPE);
	dh5->len = htons(strlen(mime));
	::memcpy(dh5->data,mime,strlen(mime));
	// Message

	int meslen = strlen(message);
	dh6 = (hx_data_hdr*)malloc(SIZEOF_HX_DATA_HDR+meslen);
	dh6->type = htons(HTLS_DATA_NEWS_NEWSDATA);
	dh6->len = htons(meslen);
	::memcpy(dh6->data,message,meslen);

	h->len = h->len2 = htonl( 2 + SIZEOF_HX_DATA_HDR*6 + ntohs(dh->len)+ ntohs(dh2->len) +
		ntohs(dh3->len) + ntohs(dh4->len)+ntohs(dh5->len) + ntohs(dh6->len));

	SendData(  buf, 22);
	SendData(dh,SIZEOF_HX_DATA_HDR + ntohs(dh->len) );
	SendData(buf2,SIZEOF_HX_DATA_HDR + ntohs(dh2->len) );
	SendData(dh3,SIZEOF_HX_DATA_HDR+ ntohs(dh3->len) );
	SendData(buf3,SIZEOF_HX_DATA_HDR + ntohs(dh4->len) );
	SendData(dh5,SIZEOF_HX_DATA_HDR+ ntohs(dh5->len) );
	SendData(dh6,SIZEOF_HX_DATA_HDR+ ntohs(dh6->len) );

	xfree(dh);
	xfree(dh3);
	xfree(dh5);
	xfree(dh6);
}


/***********************************************************
 * Receive news( version 1.2.3)
 ***********************************************************/
void
HotlineClient::ReceiveNewsFile (/*void *do_cmd_news*/)
{
	//getting_news = 0;
	uint16 news_len;
	char	*news_buf = NULL;
	struct hx_data_hdr* dh = (hx_data_hdr*)&fHxBuf[22];
	BMessage msg(H_RECEIVE_NEWS_FILE);
	//dh_start_news(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
		if (ntohs(dh->type) == HTLS_DATA_NEWS) {
			news_len = ntohs(dh->len);
			news_buf = new char[news_len + 1];
			memcpy(news_buf, dh->data, news_len);
			news_buf[news_len] = '\0';
		}
	//dh_end()

	msg.AddString("news",news_buf);
	be_app->PostMessage(&msg);
	delete[] news_buf;
	hx_reset();
}


/***********************************************************
 * Receive posting news.(version 1.2.3)
 ***********************************************************/
void
HotlineClient::ReceivePostNews()
{
	char *news_buf = NULL;
	unsigned int news_len = 0;
	struct hx_data_hdr *dh = (hx_data_hdr*)&fHxBuf[22];

	//dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
	if (ntohs(dh->type) == HTLS_DATA_NEWS) {
		news_len = ntohs(dh->len);
		news_buf = new char[news_len + 1];
		memmove(&(news_buf[ntohs(dh->len)]), news_buf, news_len - ntohs(dh->len));
		memcpy(news_buf, dh->data, ntohs(dh->len));
		news_buf[news_len] = '\0';
	}
	//dh_end()
	BMessage msg(H_RECEIVE_POST_NEWS);
	msg.AddString("text",news_buf);
	((HApp*)be_app)->MainWindow()->PostMessage(&msg);
	be_app->PostMessage(SOUND_POST_NEWS_SND);
	delete[] news_buf;
	hx_reset();
}


/***********************************************************
 * Get error messages.
 ***********************************************************/
void
HotlineClient::ReceiveTaskError()
{
	struct hx_hdr *hdr = (hx_hdr*)fHxBuf;
	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
		if (ntohs(dh->type) == HTLS_DATA_TASKERROR) {
			CR2LF(dh->data, ntohs(dh->len));
			TextUtils utils;
			char *text =new char[ntohs(dh->len) +1];
			::memset(text,0,ntohs(dh->len)+1);
			::memcpy(text,dh->data,ntohs(dh->len));
			int32 encoding;
			((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
			if(encoding)
				utils.ConvertToUTF8(&text,encoding-1);

			(new MAlert(_("Error"),text,_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
			/*if( ::strcmp(text,"Incorrect login.") == 0)
			{
				PRINT(("LOGIN ERROR\n"));
				//PostMessage(H_CLOSE_REQUESTED);
				delete[] text;
				return;
			}*/
			delete[] text;
		}
	dh_end()
	uint32 task = ntohl(hdr->trans);
	this->RemoveTask(task);
}


/***********************************************************
 * Receive user changed.
 ***********************************************************/
void
HotlineClient::ReceiveUserChanged()
{
	int16 sock = 0;
	int16 icon = 0;
	int16 colour = 0;
	int16 user_nlen = 0;
	char  nick[32];
	uint32 pcref =0;

	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
	switch (ntohs(dh->type)) {
			case HTLS_DATA_SOCKET:

				PRINT(( "Change User\n" ));

				dh_getint(sock);
				break;
			case HTLS_DATA_ICON:
				dh_getint(icon);
				break;
			case HTLS_DATA_COLOUR:
				dh_getint(colour);
				break;
			case HTLS_DATA_NICK:
				user_nlen = ntohs(dh->len) > 31 ? 31 : ntohs(dh->len);
				memcpy(nick, dh->data, user_nlen);
				nick[user_nlen] = 0;
				break;
			case HTLS_DATA_CHAT_REF:
				dh_getint(pcref);
				break;
			}
	dh_end()

	ChangeUserItem(sock,icon,colour,nick);
	hx_reset();
}


/***********************************************************
 * Receive left user.
 ***********************************************************/
void
HotlineClient::ReceiveUserLeave()
{
 	uint16 sock = 0;

	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
		switch (ntohs(dh->type)) {
			case HTLS_DATA_SOCKET:
				dh_getint(sock);

				PRINT(( "Sock: %d User has left\n", sock ));

				break;
		}
	dh_end()
	hx_reset();
	RemoveUserItem(sock);
}


/***********************************************************
 * Receive file list.
 ***********************************************************/
void
HotlineClient::ReceiveFileList (void)
{

	char *buf = NULL;
	uint16 i, bpos;
	uint32 fnlen, filesize;
	//::memset(buf,0,4096);
	struct hx_hdr *hdr = (hx_hdr*)fHxBuf;
	uint32 trans = ntohl(hdr->trans);

	// find parent item index
	int32 count = fFileListArray.CountItems();
	uint32 index = 0;
	for(register int32 i = 0;i < count;i++)
	{
		FileListIndex *list_index = (FileListIndex*)fFileListArray.ItemAt(i);
		if(trans == list_index->task)
			index = list_index->parent_index;
	}
	if( index == 0 )    // ping action.
		return;
	BMessage msg(H_FILELIST_RECEIVED);
	msg.AddInt32("index",index);
	if(!fSilverWingMode)
	{
		struct hx_filelist_hdr *fh;
		dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
			i = ntohs(dh->type);
			if (i != HTLS_DATA_FILE_LIST)
				continue;
			fh = (struct hx_filelist_hdr *)dh;
			//L32NTOH(fnlen, &fh->fnlen);
			fnlen = ntohl(fh->fnlen);
			bpos = 0;
			buf = new char[fnlen+1];
			::memcpy(buf,fh->fname,fnlen);
			buf[fnlen] = '\0';
			filesize = ntohl(fh->fsize);
			char *tmp = new char[5];
			::memcpy(tmp,fh->fcreator,4);
			tmp[4] = '\0';
			msg.AddString("creator",tmp);
			delete[] tmp;
			tmp = new char[5];
			::memcpy(tmp,fh->ftype,4);
			tmp[4] = '\0';
			msg.AddString("type",tmp);
			delete[] tmp;
			msg.AddInt32("size",filesize);
			msg.AddString("name",buf);
			delete[] buf;
		dh_end()
	}else{
		struct hx_filelist_hdr_sw *fh;
		dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
			i = ntohs(dh->type);
			if (i != HTLS_DATA_FILE_LIST)
				continue;
			fh = (struct hx_filelist_hdr_sw *)dh;
			//L32NTOH(fnlen, &fh->fnlen);
			fnlen = ntohl(fh->fnlen);
			bpos = 0;
			uint32 modtime = ntohl(fh->modtime);

			buf = new char[fnlen+1];
			::memcpy(buf,fh->fname,fnlen);
			buf[fnlen] = '\0';
			filesize = ntohl(fh->fsize);
			char *tmp = new char[5];
			::memcpy(tmp,fh->fcreator,4);
			tmp[4] = '\0';
			msg.AddString("creator",tmp);
			delete[] tmp;
			tmp = new char[5];
			::memcpy(tmp,fh->ftype,4);
			tmp[4] = '\0';
			msg.AddString("type",tmp);
			delete[] tmp;
			msg.AddInt32("size",filesize);
			msg.AddString("name",buf);
			msg.AddInt32("modified",modtime);

			delete[] buf;
		dh_end()

	}
	//if( !msg.IsEmpty())
	be_app->PostMessage(&msg);

	hx_reset();
}


/***********************************************************
 * Send delete file command.
 ***********************************************************/
void
HotlineClient::SendFileDelete(const char* path)
{
	char buf[SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));
	uint16 nlen;
	char const *p;
	PRINT(( "Delete: %s\n" , path ));
	h->type = htonl(HTLC_HDR_FILE_DELETE);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	if (!path || !path[0] || !(p = dirchar_basename(path)))
		return;
	dh->type = htons(HTLC_DATA_FILE);
	dh->len = htons((nlen = strlen(p)));
	if (p != path) {
		dh = path_to_hldir(path, 1);
		h->len = h->len2 = htonl(2 + (SIZEOF_HX_DATA_HDR * 2) + ntohs(dh->len) + nlen);
		h->hc = htons(2);

		SendData( buf, 26);
		SendData( p, nlen);
		SendData( dh, SIZEOF_HX_DATA_HDR + ntohs(dh->len));

		xfree(dh);
	} else {
		h->len = h->len2 = htonl(2 + SIZEOF_HX_DATA_HDR + nlen);
		h->hc = htons(1);

		SendData( buf, 26 );
		SendData( p, nlen );

	}
}



/***********************************************************
 * Receive get file command reply.
 ***********************************************************/
void
HotlineClient::ReceiveFileGet ()
{
	PRINT(( "Enter Receive File Get\n" ));

	struct hx_hdr *h = (struct hx_hdr *)fHxBuf;

	uint32 task = ntohl(h->trans);
	uint32 ref = 0, size = 0, queue = 0;

	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
		switch (ntohs(dh->type)) {
			case HTLS_DATA_HTXF_SIZE:
				dh_getint(size);
				break;
			case HTLS_DATA_HTXF_REF:
				dh_getint(ref);
				break;
			case HTLS_DATA_FILE_QUEUE:
				dh_getint(queue);
				break;
		}
	dh_end()

	if(Lock())
	{
		int32 count = fFileTransArray.CountItems();
		for(register int32 i = 0;i <count ;i++)
		{
			FileTrans *trans = (FileTrans*)fFileTransArray.ItemAt(i);
			if(trans->task == task)
			{
				fFileTransArray.RemoveItem(i);
				//if(queue == 0)
				//{
					//BMessage msg(B_REFS_RECEIVED);
					BMessage msg(M_DOWNLOAD_TASK);
					msg.AddInt32("task",(int32)task);
					msg.AddInt32("ref",(int32)ref);
					msg.AddInt32("size",(int32)size);
					msg.AddString("localpath",trans->localpath.String());
					msg.AddString("remotepath",trans->remotepath.String());
					msg.AddString("address",fAddress.String());
					msg.AddInt16("port",(int16)fPort);
					msg.AddInt32("queue",queue);
					bool isDownload = true;
					msg.AddBool("download",isDownload);

					((HApp*)be_app)->TaskWindow()->PostMessage(&msg);
					delete trans;
				/*}else{
					(new MAlert("Caution!","Server side queued downloading is not supported yet","OK"))->Go();
				}*/
			break;
			}
		}
		Unlock();
	}
	hx_reset();
}

/***********************************************************
 * Receive put file command replay.
 ***********************************************************/
void
HotlineClient::ReceiveFilePut()
{
	uint32 ref = 0;
	uint8 buf[74];
	uint32 data_pos = 0;
	uint32 queue = 0;
	struct hx_hdr *hdr = (hx_hdr*)fHxBuf;
	uint32 task = ntohl(hdr->trans);

	dh_start( &(fHxBuf[SIZEOF_HX_HDR]),fHxPos - SIZEOF_HX_HDR)
		switch(ntohs(dh->type))
		{
		case HTLS_DATA_HTXF_REF:
		{
			dh_getint(ref);
			break;
		}
		case HTLC_DATA_RESUMEINFO:
		{
			memcpy(buf,dh->data,ntohs(dh->len));
			L32NTOH(data_pos,&buf[46]);
			break;
		}
		case HTLS_DATA_FILE_QUEUE:
		{
			dh_getint(queue);
			break;
		}
		}
	dh_end()
	if(Lock())
	{
	int count = fFileTransArray.CountItems();
	for(int i = 0;i <count ;i++)
	{
		FileTrans *trans = (FileTrans*)fFileTransArray.ItemAt(i);
		if(trans->task == task)
		{
			fFileTransArray.RemoveItem(i);

				//BMessage msg(B_REFS_RECEIVED);
				BMessage msg(M_UPLOAD_TASK);
				msg.AddInt32("task",(int32)task);
				msg.AddInt32("ref",(int32)ref);
				msg.AddInt32("size",(int32)data_pos);
				msg.AddString("localpath",trans->localpath.String());
				msg.AddString("remotepath",trans->remotepath.String());
				msg.AddString("address",fAddress.String());
				msg.AddInt16("port",(int16)fPort);
				msg.AddInt32("queue",queue);
				bool isDownload = false;
				msg.AddBool("download",isDownload);
				/*if(be_roster->IsRunning("application/x-vnd.takamatsu-hltransfer"))
				{
					team_id id = be_roster->TeamFor("application/x-vnd.takamatsu-hltransfer");
					BMessenger messenger("application/x-vnd.takamatsu-hltransfer",id);
					messenger.SendMessage(&msg);
				}else{
					be_roster->Launch("application/x-vnd.takamatsu-hltransfer",&msg);
				}*/
				((HApp*)be_app)->TaskWindow()->PostMessage(&msg);
				delete trans;
			break;
		}
	}
	Unlock();
	}
	hx_reset();
}


/***********************************************************
 * Send get user list command.
 ***********************************************************/
void
HotlineClient::SendGetUsersList()
{
	char fBuffer[22];
	hx_hdr *hdr;

	hdr = (hx_hdr *)fBuffer;
	hdr->type = htonl(HTLC_HDR_USER_GETLIST);

	AddTask(_("Getting user list…"),fHxTrans);
	AddTaskList(fHxTrans,T_USER_LIST_TASK);

	hdr->trans = htonl(fHxTrans++);
	hdr->flag = 0;
	hdr->len = hdr->len2 = htonl(2);
	hdr->hc =htons(0);
	//hdr->obj_count = htons(0);

	SendData(fBuffer, 22);
	UpdateTask(fHxTrans-1,5);

	return;
}

/***********************************************************
 * Receive invitation of private chat.
 ***********************************************************/
void
HotlineClient::ReceiveChatInvite (bool me)
{
	uint32 sock = 0, pcref = 0;
	char *nick = NULL;
	uint16 nlen = 0;

	//uint32 trans = ntohl(h->trans);

	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
		switch (ntohs(dh->type)) {
			case HTLS_DATA_SOCKET:
				dh_getint(sock);
				break;
			case HTLS_DATA_CHAT_REF:
				dh_getint(pcref);
				break;
			case HTLS_DATA_NICK:
				nlen = ntohs(dh->len);
				nick = new char[nlen+1];
				::memcpy(nick,dh->data,nlen);
				nick[nlen] = '\0';
				break;
		}
	dh_end()

	hx_reset();
	if(!me)
	{
		BString title = _("You are invited to chat with");
		title << ": \n";
		title << nick;
		time_t t = time(NULL);
		title << "\n";
		title << ctime(&t);
		bool which;
		((HApp*)be_app)->Prefs()->GetData("refusechat",&which);
		if(which)
		{
			this->SendDeclineChat(pcref);
		}else{
			be_app->PostMessage(SOUND_INVITE_SND);
			InvitationWindow *dialog = new InvitationWindow(RectUtils().CenterRect(300,80),
								_("Invitation"),nick,pcref,this);
			dialog->Show();
		}
	}else{
		//fHxTrans--;
		this->SendChatJoin(pcref);
	}
}


/***********************************************************
 * Receive server name and version.
 ***********************************************************/
void
HotlineClient::ReceiveServerVersion (void)
{
	uint32 version = 0;
	uint16 nlen = 0;
	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
		switch (ntohs(dh->type)) {
			case HTLS_SERVER_VERSION:
				dh_getint(version);
				fServerVersion = version;
				break;
			case HTLS_SERVER_NAME:
				nlen = ntohs(dh->len);
				char *name = new char[nlen+1];
				::memcpy(name,dh->data,nlen);
				name[nlen] = '\0';
				TextUtils utils;
				fServerName = name;
				delete[] name;
				int32 encoding;
				((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
				utils.ConvertToUTF8(fServerName,encoding-1);
				break;
		}
	dh_end()
	hx_reset();
	BMessage msg(H_SERVER_VERSION_RECEIVED);
	msg.AddString("name",fServerName.String());
	msg.AddInt32("version",fServerVersion);
	be_app->PostMessage(&msg);
}


/***********************************************************
 * Receive chat user changed.
 ***********************************************************/
void
HotlineClient::ReceiveChatUserChange()
{
	uint32 sock = 0, icon = 0, colour = 0, pcref = 0;
	uint16 nlen = 0;
	char *name = NULL;
	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
		switch (ntohs(dh->type)) {
			case HTLS_DATA_SOCKET:
				dh_getint(sock);
				break;
			case HTLS_DATA_ICON:
				dh_getint(icon);
				break;
			case HTLS_DATA_NICK:
				nlen = ntohs(dh->len) > 31 ? 31 : ntohs(dh->len);
				name = new char[nlen+1];
				::memcpy(name, dh->data, nlen);
				name[nlen] = '\0';
				break;
			case HTLS_DATA_COLOUR:
				dh_getint(colour);
				break;
			case HTLS_DATA_CHAT_REF:
				dh_getint(pcref);
				break;
		}
	dh_end()

	BMessage msg(H_CHAT_USER_CHANGE);
	msg.AddInt32("pcref",(int32)pcref);
	msg.AddInt16("sock",sock);
	msg.AddInt16("icon",icon);
	msg.AddInt16("color",colour);
	msg.AddString("nick",name);
	be_app->PostMessage(&msg);
	delete[] name;
	hx_reset();
}


/***********************************************************
 * Recieve private chat user changed.
 ***********************************************************/
void
HotlineClient::ReceiveChatUserLeave (void)
{
	uint32 sock = 0, pcref = 0;

	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
		switch (ntohs(dh->type)) {
			case HTLS_DATA_SOCKET:
				dh_getint(sock);
				break;
			case HTLS_DATA_CHAT_REF:
				dh_getint(pcref);
				break;
		}
	dh_end()
	BMessage msg(H_CHAT_USER_LEAVE);
	msg.AddInt32("pcref",(int32)pcref);
	msg.AddInt32("sock",(int32)sock);
	be_app->PostMessage(&msg);
	hx_reset();
}


/***********************************************************
 * Send join private chat.
 ***********************************************************/
void
HotlineClient::SendChatJoin (uint32 pcref)
{
	uint8 buf[32];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));

	h->type = htonl(HTLC_HDR_CHAT_JOIN);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->len = h->len2 = htonl(10);
	h->hc = htons(1);
	dh->type = htons(HTLC_DATA_CHAT_REF);
	dh->len = htons(4);
	S32HTON(pcref, dh->data);

	AddPrvChat(pcref,fHxTrans-1);
	AddTaskList(fHxTrans-1,T_PRV_USER_TASK);
	PRINT(("JOIN TASK:%d\n" , fHxTrans-1 ));
	BMessage msg(H_RCV_JOIN_CHAT);
	msg.AddInt32("pcref",pcref);
	be_app->PostMessage(&msg);

	SendData(buf, 30);
}


/***********************************************************
 * Send leave private chat.
 ***********************************************************/
void
HotlineClient::SendChatLeave (uint32 pcref)
{
	uint8 buf[30];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));

	h->type = htonl(HTLC_HDR_CHAT_LEAVE);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->len = h->len2 = htonl(10);
	h->hc = htons(1);
	dh->type = htons(HTLC_DATA_CHAT_REF);
	dh->len = htons(4);
	S32HTON(pcref, dh->data);

	SendData(buf, 30);
}


/***********************************************************
 * Send invite private chat.
 ***********************************************************/
void
HotlineClient::SendChatInvite (uint32 pcref, uint32 sock)
{
	uint8 buf[38];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));

	h->type = htonl(HTLC_HDR_CHAT_INVITE);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->len = h->len2 = htonl(18);
	h->hc = htons(2);
	dh->type = htons(HTLC_DATA_CHAT_REF);
	dh->len = htons(4);
	S32HTON(pcref, dh->data);
	dh = (struct hx_data_hdr *)(&(buf[30]));
	dh->type = htons(HTLC_DATA_SOCKET);
	dh->len = htons(4);
	S32HTON(sock, dh->data);

	SendData(  buf, 38);
}


/***********************************************************
 * Send private chat.
 ***********************************************************/
void
HotlineClient::SendChatChat (uint32 pcref, const void *data, uint16 len)
{
	uint8 buf[SIZEOF_HX_HDR + (SIZEOF_HX_DATA_HDR * 2) + 4];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));

	h->type = htonl(HTLC_HDR_CHAT);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->len = h->len2 = htonl(14 + len);
	h->hc = htons(2);
	dh->type = htons(HTLC_DATA_CHAT);
	dh->len = htons(len);
	dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR]));
	dh->type = htons(HTLC_DATA_CHAT_REF);
	dh->len = htons(4);
	S32HTON(pcref, dh->data);

	SendData( buf, SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR);
	SendData( data, len);
	SendData( &(buf[SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR]), SIZEOF_HX_DATA_HDR + 4);
}


/***********************************************************
 * Analyze hotline header packet.
 ***********************************************************/
void
HotlineClient::ReceiveHeader(void)
{
	struct hx_hdr *h = (struct hx_hdr *)fHxBuf;
	register uint32 type = ntohl(h->type), len = ntohl(h->len);
#ifdef DEBUG
	uint32 trans = ntohl(h->trans);
#endif
	PRINT(("rcv_hdr: %ld  Trans: %ld\n",type,trans));

	if (len >= 2)
		len -= 2;
	switch (type) {
		case HTLS_HDR_CHAT:
			hx_set(1, len);
			break;
		case HTLS_HDR_MSG:
			hx_set(2, len);
			break;
		case HTLS_HDR_USER_CHANGE:
			hx_set(3, len);
			break;
		case HTLS_HDR_USER_LEAVE:
			hx_set(4, len);
			break;
		case HTLS_HDR_TASK:
			hx_set(5, len);
			break;
		case HTLS_HDR_NEWS_POST:
			hx_set(6, len);
			break;
		case HTLS_HDR_CHAT_USER_CHANGE:
			hx_set(7, len);
			break;
		case HTLS_HDR_CHAT_USER_LEAVE:
			hx_set(8, len);
			break;
		case HTLS_HDR_CHAT_SUBJECT:
			hx_set(9, len);
			break;
		case HTLS_HDR_CHAT_INVITE:
			hx_set(10, len);
			break;
		case HTLS_HDR_AGREEMENT:
			hx_set(11, len);
			break;
		case HTLS_HDR_POLITEQUIT:
			hx_set(12,len);
			break;
		case HLTS_HDR_SERVERQUEUE:
			hx_set(13,len);
			break;
		case HTLS_HDR_USER_SELFINFO:
			hx_set(14,len);
			break;
		case HTLS_HDR_MSG_BROADCAST:
			hx_set(15,len);
			break;
		case HTLS_HDR_SILVERWING_MODE:
			hx_set(16,len);
			break;
		default:
			PRINT(("unknown header type 0x%lx\n", (unsigned long)type));
		//	hx_set(0, len);
			fHxFun = -1;
			break;
	}
	if (!len) {
		if (fHxFun)
			Fook_Hx_Fun();
		else
			hx_reset();
	}
}


/***********************************************************
 * Send get file command.
 ***********************************************************/
void
HotlineClient::SendFileGet (const char *path,const char* localpath, uint32 data_size, uint32 rsrc_size)
{
	char buf[SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR + 74];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));
	uint16 nlen;
	const char *p;

	PRINT(("RemotePath:%s\n",  path ));

	if(Lock())
	{
		FileTrans *trans = new FileTrans;
		trans->localpath = localpath;
		trans->remotepath = path;
		trans->task = fHxTrans;
		trans->isDownload = true;
		fFileTransArray.AddItem(trans);
		Unlock();
	}
	AddTask(_("Downloading file…"),fHxTrans);
	AddTaskList(fHxTrans,T_FILE_TRANS_TASK);
	UpdateTask(fHxTrans,2);

	h->type = htonl(HTLC_HDR_FILE_GET);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	if (!path || !path[0] || !(p = dirchar_basename(path)))
		return;
	dh->type = htons(HTLC_DATA_FILE);
	dh->len = htons((nlen = strlen(p)));
	if (p != path) {
		dh = path_to_hldir(path, 1);
		h->len = h->len2 = htonl(74 + 2 + (SIZEOF_HX_DATA_HDR * 3) + ntohs(dh->len) + nlen);
		h->hc = htons(3);

		SendData( buf, 26);
		UpdateTask(fHxTrans-1,3);
		SendData( p, nlen);
		UpdateTask(fHxTrans-1,5);
		SendData( dh, SIZEOF_HX_DATA_HDR + ntohs(dh->len));
		UpdateTask(fHxTrans-1,6);

		xfree(dh);
	} else {
		h->len = h->len2 = htonl(74 + 2 + (SIZEOF_HX_DATA_HDR * 2) + nlen);
		h->hc = htons(2);

		SendData( buf, 26);
		UpdateTask(fHxTrans-1,4);
		SendData( p , nlen);
		UpdateTask(fHxTrans-1,6);
	}
	dh = (struct hx_data_hdr *)buf;
	dh->type = htons(HTLC_DATA_RESUMEINFO);
	dh->len = htons(74);
	memcpy(&(buf[4]),
		"\x52\x46\x4c\x54\0\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\x44"
		"\x41\x54\x41\0\0\0\0\0\0\0\0\0\0\0\0\x4d\x41\x43\x52"
		"\0\0\0\0\0\0\0\0\0\0\0\0",
	74);
	S32HTON(data_size, &buf[4 + 46]);
	S32HTON(rsrc_size, &buf[4 + 62]);

	SendData( buf, 78);
	UpdateTask(fHxTrans-1,8);
}



/***********************************************************
 * Send kick user command.
 ***********************************************************/
void
HotlineClient::SendUserKick (uint32 sock)
{
	uint8 buf[32];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));

	h->type = htonl(HTLC_HDR_USER_KICK);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->len = h->len2 = htonl(10);
	h->hc = htons(1);
	dh->type = htons(HTLC_DATA_SOCKET);
	dh->len = htons(4);
	S32HTON(sock, dh->data);

	SendData(buf, 30);
}


/***********************************************************
 * Send get user info command.
 ***********************************************************/
void
HotlineClient::SendUserGetInfo (uint32 sock)
{
	uint8 buf[32];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));

	h->type = htonl(HTLC_HDR_USER_GETINFO);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->len = h->len2 = htonl(10);
	h->hc = htons(1);
	dh->type = htons(HTLC_DATA_SOCKET);
	dh->len = htons(4);
	S32HTON(sock, dh->data);

	AddTask(_("Getting user infomation…"),fHxTrans-1);
	AddTaskList(fHxTrans-1,T_USER_INFO_TASK);
	UpdateTask(fHxTrans-1,5);

	SendData(buf, 30);
}

/*
 * Send get news list.

void
HotlineClient::SendNewsDirList(const char* path)
{
	uint8 buf[SIZEOF_HX_HDR];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh;

	h->type = htonl(HTLC_HDR_NEWSDIRLIST);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	if (!path || !path[0] || (path[0] == '/' && !path[1])) {
		h->len = h->len2 = htonl(2);
		h->hc = 0;
		if(this->Lock() )
		{
			fEndpoint->Send(buf, SIZEOF_HX_HDR);
			this->Unlock();
		}
		return;
	}
	dh = path_to_hldir(path, 0);
	h->len = h->len2 = htonl(2 + SIZEOF_HX_DATA_HDR + ntohs(dh->len));
	h->hc = htons(1);
	if(this->Lock())
	{
		fEndpoint->Send(buf, SIZEOF_HX_HDR);
		fEndpoint->Send(dh, SIZEOF_HX_DATA_HDR + ntohs(dh->len));
		this->Unlock();
	}
	xfree(dh);
}
*/


/***********************************************************
 * Send chat
 ***********************************************************/
void
HotlineClient::SendChat(const void *data, uint16 len)
{
	uint8 buf[SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));

	h->type = htonl(HTLC_HDR_CHAT);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->len = h->len2 = htonl(6 + len);
	h->hc = htons(1);
	dh->type = htons(HTLC_DATA_CHAT);
	dh->len = htons(len);

	SendData(buf, SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR);
	SendData(data, len);
	fIdleTime = time(NULL);
}

/***********************************************************
 * b_read
 ***********************************************************/
int
HotlineClient::b_read (BNetEndpoint *endpoint, void *bufp, size_t len)
{
	register uint8 *buf = (uint8 *)bufp;
	register int r, pos = 0;

	while (len) {
		if ((r = endpoint->Receive(&(buf[pos]), len)) <= 0)
			return -1;
		pos += r;
		len -= r;
	}

	return pos;
}


/***********************************************************
 * Receive user info.
 ***********************************************************/
void
HotlineClient::ReceiveUserInfo (void)
{
	uint16 ilen = 0;
	uint8 info[4096], nick[32];
	uint16 nlen = 0;
	TextUtils utils;
	nick[0] = 0;
	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
		switch (ntohs(dh->type)) {
			case HTLS_DATA_USER_INFO:
				ilen = ntohs(dh->len);
				memcpy(info, dh->data, ilen);
				info[ilen] = '\0';
				break;
			case HTLS_DATA_NICK:
				nlen = ntohs(dh->len) > 31 ? 31 : ntohs(dh->len);
				memcpy(nick, dh->data, nlen);
				nick[nlen] = '\0';
				break;
		}
	dh_end()
	if (ilen) {
		char *t = new char[nlen+1];
		::memset(t,0,nlen+1);
		::memcpy(t,nick,nlen);
		BString n = t;
		delete[] t;
		t = new char[ilen+1];
		::memset(t,0,ilen);
		::memcpy(t,info,ilen);
		t[ilen] = '\0';
		utils.ConvertReturnsToLF(t);
		BString in = t;
		delete[] t;
		BMessage msg(H_RCV_INFO);
		msg.AddString("nick",n.String());
		msg.AddString("info",in.String());
		be_app->PostMessage(&msg);
	}
}

/***********************************************************
 * Receive user list.
 ***********************************************************/
void
HotlineClient::ReceiveUserList()
{
	int16 sock;
	int16 icon;
	int16 colour;
	int16 user_nlen;
	char  nick[64];
	struct hx_userlist_hdr *uh;

	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)

	switch (ntohs(dh->type))
	{
		case HTLS_DATA_USER_LIST:

		uh = (struct hx_userlist_hdr *)dh;
		sock = (uint16)ntohs(uh->sock);
		icon = (uint16)ntohs(uh->icon);
		colour = (uint16)ntohs(uh->colour);
		user_nlen = (uint8)(ntohs(uh->nlen) > 31 ? 31 : ntohs(uh->nlen));
		memcpy(nick, uh->nick, user_nlen);
		nick[user_nlen] = '\0';
		AddUserItem(sock,icon,colour,nick);
		break;
	}
	dh_end()
}

/***********************************************************
 * Receive private user list.
 ***********************************************************/
void
HotlineClient::ReceivePrvChatUserList()
{
	int16 sock;
	int16 icon;
	int16 colour;
	int16 user_nlen;
	char  nick[64];
	char* chat_subject = NULL;
	struct hx_hdr *h = (struct hx_hdr*)fHxBuf;
	uint32 trans = ntohl(h->trans);
	struct hx_userlist_hdr *uh;
	BMessage mes(H_PRV_CHAT_USER_ADD);

	int count = this->fPrvChatList.CountItems();
	uint32 pcref = 0;

	for(register int32 k = 0;k< count;k++)
	{
		PrvChatTask *prvtask = (PrvChatTask*)this->fPrvChatList.ItemAt(k);
		if(prvtask->trans == trans)
		{
			pcref = prvtask->pcref;
			fPrvChatList.RemoveItem(prvtask);
			delete prvtask;
			break;
		}
	}



	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)

	switch (ntohs(dh->type))
	{
		case HTLS_DATA_USER_LIST:
		{
			uh = (struct hx_userlist_hdr *)dh;
			sock = (uint16)ntohs(uh->sock);
			icon = (uint16)ntohs(uh->icon);
			colour = (uint16)ntohs(uh->colour);
			user_nlen = (uint8)(ntohs(uh->nlen) > 31 ? 31 : ntohs(uh->nlen));
			memcpy(nick, uh->nick, user_nlen);
			nick[user_nlen] = '\0';


			mes.AddInt32("pcref",pcref);
			mes.AddInt16("sock",sock);
			mes.AddInt16("icon",icon);
			mes.AddInt16("color",colour);
			mes.AddString("nick",nick);

			break;
		}
		case HTLS_DATA_CHAT_SUBJECT:
		{
			int16 len = ntohs(dh->len);
			chat_subject = new char[len +1];
			memcpy(chat_subject, dh->data, len);
			chat_subject[len] = '\0';
			mes.AddString("topic",chat_subject);
			break;
		}
	}
	dh_end()
	be_app->PostMessage(&mes);
	delete[] chat_subject;
}


/***********************************************************
 * Send create private chat command.
 ***********************************************************/
void
HotlineClient::SendChatCreate (uint32 sock)
{
	uint8 buf[30];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));

	AddTaskList(fHxTrans,T_PRV_INVITE_TASK);

	h->type = htonl(HTLC_HDR_CHAT_CREATE);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->len = h->len2 = htonl(10);
	h->hc = htons(1);
	dh->type = htons(HTLC_DATA_SOCKET);
	dh->len = htons(4);
	S32HTON(sock, dh->data);

	SendData(buf, 30);
}


/***********************************************************
 * Get news. (version 1.2.3)
 ***********************************************************/
void
HotlineClient::SendNewsGetFile (void)
{
	struct hx_hdr h;
	//AddTask("Getting news ...",fHxTrans);
	AddTaskList(fHxTrans,T_NEWS_FILE_TASK);
	h.type = htonl(HTLC_HDR_NEWS_GETFILE);
	h.trans = htonl(fHxTrans++);
	h.flag = 0;
	h.len = h.len2 = htonl(2);
	h.hc = 0;

	SendData(&h, SIZEOF_HX_HDR);
	UpdateTask(fHxTrans-1,4);
}


/***********************************************************
 * Post news (version 1.2.3)
 ***********************************************************/
void
HotlineClient::SendNewsPost(const char* text)
{
	uint8 buf[SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));

	int len = strlen(text);
	h->type = htonl(HTLC_HDR_NEWS_POST);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->len = h->len2 = htonl(2 + SIZEOF_HX_DATA_HDR + len);
	h->hc = htons(1);
	dh->type = htons(HTLC_DATA_NEWS_POST);
	dh->len = htons(len);
	SendData( buf, SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR );
	SendData( text, len );
}


/***********************************************************
 * Old function.(currently unused)
 ***********************************************************/
void
HotlineClient::SendCategory()
{
	uint8 buf[SIZEOF_HX_HDR];
	struct hx_hdr *h = (struct hx_hdr *)buf;
//	struct hx_news_snd_data_hdr *dh;

	h->type = htonl(HTLC_HDR_NEWS_GETFILE+1);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;

	BNetBuffer bbuf;// = new BNetBuffer();
	bbuf.AppendInt16(htonl(HTLC_DATA_NEWS));
	bbuf.AppendInt16(htonl(0x000a));
	bbuf.AppendInt16(htonl(0x0001));
	bbuf.AppendUint8(0x0);
	bbuf.AppendInt16(htonl(0x0008));
	bbuf.AppendString("TEST.hnz");
	h->len = h->len2 = htonl(2 + 8 + 9);
	h->hc = htons(1);

	//SendData( buf, SIZEOF_HX_HDR);
	//SendData( bbuf);
	//xfree(dh);
}


/***********************************************************
 * Send private message.
 ***********************************************************/
void
HotlineClient::SendMessage (uint32 sock, const void *data, uint16 len)
{
	uint8 buf[34];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));

	h->type = htonl(HTLC_HDR_MSG);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->len = h->len2 = htonl(12 + len);
	h->hc = htons(2);
	dh->type = htons(HTLC_DATA_SOCKET);
	dh->len = htons(2);
	uint16 sock16 = sock;
	S16HTON(sock16, dh->data);
	dh = (struct hx_data_hdr *)(&(buf[28]));
	dh->type = htons(HTLC_DATA_MSG);
	dh->len = htons(len);

	SendData(buf, 32);
	SendData( data, len);
}


/***********************************************************
 * Send kick users.
 ***********************************************************/
void
HotlineClient::SendUserKickBan (uint32 sock)
{
	uint8 buf[36];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));

	h->type = htonl(HTLC_HDR_USER_KICK);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->len = h->len2 = htonl(16);
	h->hc = htons(2);
	dh->type = htons(HTLC_DATA_BAN);
	dh->len = htons(2);
	*((uint16 *)dh->data) = htons(1);
	dh = (struct hx_data_hdr *)(&(buf[28]));
	dh->type = htons(HTLC_DATA_SOCKET);
	dh->len = htons(4);
	*((uint32 *)dh->data) = htonl(sock);

	SendData( buf, 36);
}


/***********************************************************
 * Send change user info.
 ***********************************************************/
void
HotlineClient::SendUserChange(const char* nick, uint16 icon)
{
	uint8 buf[32];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));
	uint16 nlen = strlen(nick);
	h->type = htonl(HTLC_HDR_USER_CHANGE);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->len = h->len2 = htonl(12 + nlen);
	h->hc = htons(2);
	dh->type = htons(HTLC_DATA_ICON);
	dh->len = htons(2);
	*((uint16 *)dh->data) = htons(icon);
	dh = (struct hx_data_hdr *)(&(buf[28]));
	dh->type = htons(HTLC_DATA_NICK);
	dh->len = htons(nlen);

	SendData(buf, 32 );

	BString bnick = nick;
	TextUtils utils;
	int32 encoding;
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
	if(encoding)
		utils.ConvertFromUTF8(bnick,encoding-1);
	SendData(bnick.String(), nlen );
}

/***********************************************************
 * Send SilverWing mode
 ***********************************************************/
void
HotlineClient::SendSilverWingMode()
{
	uint8 buf[SIZEOF_HX_HDR];
	struct hx_hdr *h = (struct hx_hdr *)buf;

	h->type = htonl(HTLC_HDR_SILVERWING_MODE);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->len = h->len2 = htonl(2);
	h->hc = htons(0);
	fSWModeTask = fHxTrans-1;
	SendData(buf, SIZEOF_HX_HDR);
	fIdleTime = time(NULL);
}

/***********************************************************
 * Receive SilverWing mode
 ***********************************************************/
void
HotlineClient::ReceiveSilverWingMode()
{
	//BAutolock lock(this);
	fSilverWingMode = true;
	//(new MAlert("","Enter SilverWing mode" ,"OK"))->Go();
	hx_reset();
}

/***********************************************************
 * Add task to task window.
 ***********************************************************/
void
HotlineClient::AddTask(const char* name,uint32 task,uint32 type)
{
	BMessage msg(M_ADD_TASK);
	msg.AddString("label",name);
	msg.AddInt32("task",task);
	msg.AddInt32("type",type);
	((HApp*)be_app)->TaskWindow()->PostMessage(&msg);
}


/***********************************************************
 * Update task.
 ***********************************************************/
void
HotlineClient::UpdateTask(uint32 task,uint32 update)
{
	BMessage msg(M_UPDATE_TASK);
	msg.AddInt32("task",task);
	msg.AddInt32("update",update);
	((HApp*)be_app)->TaskWindow()->PostMessage(&msg);
}

/***********************************************************
 * Remove task.
 ***********************************************************/
void
HotlineClient::RemoveTask(uint32 task_id)
{
	BMessage msg(M_REMOVE_TASK);
	msg.AddInt32("task",task_id);
	((HApp*)be_app)->TaskWindow()->PostMessage(&msg);
}

/***********************************************************
 * Set task max value.
 ***********************************************************/
void
HotlineClient::SetTaskMax(uint32 task,uint32 max_value)
{
	BMessage msg(M_SET_MAX_TASK);
	msg.AddInt32("task",task);
	msg.AddInt32("max_value",max_value);
	((HApp*)be_app)->TaskWindow()->PostMessage(&msg);
}

/***********************************************************
 * Add logging users.
 ***********************************************************/
void
HotlineClient::AddUserItem(uint16 sock,uint16 icon,uint16 color,const char* nick)
{
	BMessage msg(H_ADD_USER);
	msg.AddInt16("sock",sock);
	msg.AddInt16("icon",icon);
	msg.AddInt16("color",color);
	msg.AddString("nick",nick);
	((HWindow*)((HApp*)be_app)->MainWindow())->PostMessage(&msg);
}


/***********************************************************
 * Remove left users.
 ***********************************************************/
void
HotlineClient::RemoveUserItem(uint16 sock)
{
	BMessage msg(H_REMOVE_USER);
	msg.AddInt16("sock",sock);

	((HWindow*)((HApp*)be_app)->MainWindow())->PostMessage(&msg);
}

/***********************************************************
 * Change logged user info.
 ***********************************************************/
void
HotlineClient::ChangeUserItem(uint16 sock,uint16 icon,uint16 color,const char* nick)
{
	BMessage msg(H_CHANGE_USER);
	msg.AddInt16("sock",sock);
	msg.AddInt16("icon",icon);
	msg.AddInt16("color",color);
	msg.AddString("nick",nick);
	be_app->PostMessage(&msg);
}


/***********************************************************
 * Remove all users.
 ***********************************************************/
void
HotlineClient::RemoveAllUsers()
{
	((HWindow*)((HApp*)be_app)->MainWindow())->PostMessage(MWIN_REMOVE_ALL_USERS);
}

/***********************************************************
 * Set the next packet buffer that followed hotline header packet.
 ***********************************************************/
void
HotlineClient::hx_set (int32 fun, uint32 len)
{
	fHxLen += len;
	fHxBuf = (uint8*)xrealloc(fHxBuf, 8 + fHxLen + fHxPos);
	fHxFun = fun;
}

/***********************************************************
 * Reset network buffer and position.
 ***********************************************************/
void
HotlineClient::hx_reset (void)
{
	fHxBuf = (uint8*)xrealloc(fHxBuf, 8 + SIZEOF_HX_HDR);
	fHxPos = 0;
	fHxLen = SIZEOF_HX_HDR;
	fHxFun = 0;
}

/***********************************************************
 * Fook function that change functino by hotline packet header.
 ***********************************************************/
void
HotlineClient::Fook_Hx_Fun()
{
	switch(fHxFun)
	{
	case 0:
		ReceiveHeader();
		break;
	case 1:
		ReceiveChat();
		break;
	case 2:
		ReceiveMessage();
		break;
	case 3:
		ReceiveUserChanged();
		break;
	case 4:
		ReceiveUserLeave();
		break;
	case 5:
		ReceiveTask();
		break;
	case 6:
		ReceivePostNews();
		break;
	case 7:
		ReceiveChatUserChange();
		break;
	case 8:
		ReceiveChatUserLeave();
		break;
	case 9:
		ReceivePrvChatTopicChanged();
		break;
	case 10:
		ReceiveChatInvite ();
		break;
	case 11:
		ReceiveAgreement();
		break;
	case 12:
		ReceivePliteQuit();
		break;
	case 13:
		ReceiveUpdateQueue();
		break;
	case 14:
		ReceiveSelfInfo();
		break;
	case 15:
		ReceiveBroadcast();
		break;
	case 16:
		ReceiveSilverWingMode();
		break;
	default:
		hx_reset();
	}
}


/***********************************************************
 * Send get file command.
 ***********************************************************/
void
HotlineClient::SendFilePut (const char *remotepath,const char* localpath, uint32 resume_flag, uint32 rsrc_size)
{
	char buf[SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR + 74];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));
	uint16 nlen;
	const char  *p;
	if(Lock())
	{
		FileTrans *trans = new FileTrans;
		trans->remotepath = remotepath;
		trans->localpath = localpath;
		trans->task = fHxTrans;
		trans->isDownload = false;
		fFileTransArray.AddItem(trans);
		Unlock();
	}
	AddTask(_("Uploading file…"),fHxTrans);
	AddTaskList(fHxTrans,T_FILE_TRANS_TASK);
	UpdateTask(fHxTrans,2);

	h->type = htonl(HTLC_HDR_FILE_PUT);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	if (!remotepath || !remotepath[0] || !(p = dirchar_basename(remotepath)))
		return;
	dh->type = htons(HTLC_DATA_FILE);
	dh->len = htons((nlen = strlen(p)));
	if (p != remotepath) {
		dh = path_to_hldir(remotepath, 1);
		h->len = h->len2 = htonl(2 + 2 + (SIZEOF_HX_DATA_HDR * 3) + ntohs(dh->len) + nlen );
		h->hc = htons(3);

		SendData(buf, 26);
		UpdateTask(fHxTrans-1,4);
		SendData( p, nlen);
		UpdateTask(fHxTrans-1,6);
		SendData( dh, SIZEOF_HX_DATA_HDR + ntohs(dh->len));
		UpdateTask(fHxTrans-1,7);

		xfree(dh);
	} else {
		h->len = h->len2 = htonl(2 + 2 + (SIZEOF_HX_DATA_HDR * 2) + nlen);
		h->hc = htons(2);

		SendData( buf, 26);
		UpdateTask(fHxTrans-1,5);
		SendData( p, nlen);
		UpdateTask(fHxTrans-1,7);

	}
	dh = (struct hx_data_hdr *)buf;
	dh->type = htons(HTLC_DATA_RESUMEFLAG);
	dh->len = htons(2);
	uint16 resume = 0;
	if(resume_flag == 1)
		resume = 1;
	S16HTON(resume,dh->data);

	PRINT(( "Resume Flag: %s\n" , resume_flag ));

	/*memcpy(&(buf[4]),
		"\x52\x46\x4c\x54\0\x01\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\x02\x44\x41\x54\x41\0\0\0\0\0\0\0\0\0\0\0\0\x4d"
		"\x41\x43\x52\0\0\0\0\0\0\0\0\0\0\0\0",
	74);
	S32HTON(data_size, &buf[4 + 46]);
	S32HTON(rsrc_size, &buf[4 + 62]);*/
	SendData( buf, 6 );

	this->UpdateTask(fHxTrans-1,9);
}


/***********************************************************
 * Send get file list command.
 ***********************************************************/
void
HotlineClient::SendFileList (const char *path,uint32 index,bool use_task)
{
	uint8 buf[SIZEOF_HX_HDR];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh;

	/********* Add file list index *********/
	FileListIndex *file_index = new FileListIndex;
	file_index->task = fHxTrans;
	file_index->parent_index = index;
	fFileListArray.AddItem(file_index);
	//if(use_task)
	//	this->AddTask("Getting file list...",fHxTrans);
	AddTaskList(fHxTrans,T_FILE_LIST_TASK);
	h->type = htonl(HTLC_HDR_FILE_LIST);
	h->trans = htonl(fHxTrans++);
	h->flag = htonl(0);
	if (!path || !path[0] || (path[0] == dir_char && !path[1])) {
		h->len = h->len2 = htonl(2);
		h->hc = htons(0);

		SendData(buf, SIZEOF_HX_HDR);
		return;
	}
	dh = path_to_hldir(path, 0);
	h->len = h->len2 = htonl(2 + SIZEOF_HX_DATA_HDR + ntohs(dh->len));
	h->hc = htons(1);
	//if(use_task)
	//	this->UpdateTask(fHxTrans-1,2);
	SendData(buf, SIZEOF_HX_HDR);
	SendData(dh, SIZEOF_HX_DATA_HDR + ntohs(dh->len));
	//if(use_task)
	//	this->UpdateTask(fHxTrans-1,4);

	xfree(dh);
}

/***********************************************************
 * BeOS path to hotline path (for files);
 ***********************************************************/
struct hx_data_hdr *
HotlineClient::path_to_hldir (const char *path, int is_file)
{
	struct hx_data_hdr *dh;
	struct x_fhdr *fh;
	register char const *p, *p2;
	uint16 pos = 2, dc = 0;
	register uint16 nlen;

	dh = (hx_data_hdr *)xmalloc(SIZEOF_HX_DATA_HDR + 2);
	p = path;
	while ((p2 = strchr(p, dir_char))) {
		if (!(p2 - p)) {
			p++;
			continue;
		}
		nlen = (uint8)(p2 - p);
		pos += 3 + nlen;
		dh =(hx_data_hdr *) xrealloc(dh, SIZEOF_HX_DATA_HDR + pos);
		fh = (struct x_fhdr *)(&(dh->data[pos - (3 + nlen)]));
		memset(&fh->enc, 0, 2);
		fh->len = nlen;
		memcpy(fh->name, p, nlen);
		dc++;
		p = &(p2[1]);
	}
	if (!is_file && *p) {
		nlen = (uint8)strlen(p);
		pos += 3 + nlen;
		dh = (hx_data_hdr *)xrealloc(dh, SIZEOF_HX_DATA_HDR + pos);
		fh = (struct x_fhdr *)(&(dh->data[pos - (3 + nlen)]));
		memset(&fh->enc, 0, 2);
		fh->len = nlen;
		memcpy(fh->name, p, nlen);
		dc++;
	}
	dh->type = htons(HTLC_DATA_DIR);
	dh->len = htons(pos);
	*((uint16 *)dh->data) = htons(dc);

	return dh;
}


/***************************************************************************************/
void ntoht(unsigned char *text, unsigned long text_len)
{
	unsigned long i;
	for (i = 0; i < text_len; i++)
	{
		if (text[i] == 0xD)
			text[i] = 0xA;
	}
	return;
}

void htont(unsigned char *text, unsigned long text_len)
{
	unsigned long i;
	for (i = 0; i < text_len; i++)
	{
		if (text[i] == 0xA)
			text[i] = 0xD;
	}
	return;
}

/***********************************************************
 * Get dir char basename.
 ***********************************************************/
static const char *
dirchar_basename (const char *str)
{
	register int len;

	if (!str)
		return 0;
	len = strlen(str);
	while (len) {
		if (str[len] == dir_char)
			return &(str[len + 1]);
		len--;
	}

	return str;
}

/***********************************************************
 * rd_wr
 ***********************************************************/
void
rd_wr (BNetEndpoint *rd_fd, int wr_fd, uint32 data_len)
{
	register int r;
	register uint32 pos, len;
#ifdef __MWERKS__
	uint8 buf[16384];
#else
	uint8 buf[0xffff];
#endif

	while (data_len) {
		if ((len = rd_fd->Receive(buf, sizeof buf < data_len ? sizeof buf : data_len)) < 1)
			return;
		pos = 0;
		while (len) {
			if ((r = write(wr_fd, &(buf[pos]), len)) < 1)
				return;
			pos += r;
			len -= r;
		}
		data_len -= pos;
	}
}

/***********************************************************
 * beos path to hotline path ( for news).
 ***********************************************************/
struct hx_data_hdr*
HotlineClient::news_path_to_hldir(const char *path, int is_file)
{
	struct hx_data_hdr *dh;
	struct x_fhdr *fh;
	register char const *p, *p2;
	uint16 pos = 2, dc = 0;
	register uint16 nlen;

	dh = (hx_data_hdr *)xmalloc(SIZEOF_HX_DATA_HDR + 2);
	p = path;
	while ((p2 = strchr(p, '/'))) {
		if (!(p2 - p)) {
			p++;
			continue;
		}
		nlen = (uint8)(p2 - p);
		pos += 3 + nlen;
		dh =(hx_data_hdr *) xrealloc(dh, SIZEOF_HX_DATA_HDR + pos);
		fh = (struct x_fhdr *)(&(dh->data[pos - (3 + nlen)]));
		memset(&fh->enc, 0, 2);
		fh->len = nlen;
		memcpy(fh->name, p, nlen);
		dc++;
		p = &(p2[1]);
	}
	if (!is_file && *p) {
		nlen = (uint8)strlen(p);
		pos += 3 + nlen;
		dh = (hx_data_hdr *)xrealloc(dh, SIZEOF_HX_DATA_HDR + pos);
		fh = (struct x_fhdr *)(&(dh->data[pos - (3 + nlen)]));
		memset(&fh->enc, 0, 2);
		fh->len = nlen;
		memcpy(fh->name, p, nlen);
		dc++;
	}
	dh->type = htons(HTLS_DATA_NEWS_NEWSPATH);
	dh->len = htons(pos);
	*((uint16 *)dh->data) = htons(dc);

	return dh;
}


/***********************************************************
 * Send get news category list.
 ***********************************************************/
void
HotlineClient::SendNewsDirList(const char* path,uint32 index)
{
	uint8 buf[SIZEOF_HX_HDR];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh;

	//AddTask("Getting news categories ...",fHxTrans);
	AddTaskList(fHxTrans,T_NEWS_FOLDER_ITEM_TASK);

	if(Lock())
	{
		NewsTrans *trans = new NewsTrans;
		trans->trans = fHxTrans;
		trans->item_index = index;
		this->fNewsTransArray.AddItem(trans);
		Unlock();
	}
	h->type = htonl(HTLC_HDR_NEWSDIRLIST);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	if (!path || !path[0] || (path[0] == '/' && !path[1])) {
		h->len = h->len2 = htonl(2);
		h->hc = 0;

		SendData(buf, SIZEOF_HX_HDR);
		UpdateTask(fHxTrans-1,6);
		return;
	}
	dh = news_path_to_hldir(path, 0);
	h->len = h->len2 = htonl(2 + SIZEOF_HX_DATA_HDR + ntohs(dh->len));
	h->hc = htons(2);

	SendData(buf, SIZEOF_HX_HDR);
	UpdateTask(fHxTrans-1,4);
	SendData(dh, SIZEOF_HX_DATA_HDR + ntohs(dh->len));
	UpdateTask(fHxTrans-1,7);

	xfree(dh);
}


/***********************************************************
 * Add private chat.
 ***********************************************************/
void
HotlineClient::AddPrvChat(uint32 pcref,uint32 task)
{
	BAutolock lock(this);

	PrvChatTask *prvtask = new PrvChatTask;
	prvtask->pcref = pcref;
	prvtask->trans = task;
	this->fPrvChatList.AddItem(prvtask);
}



/***********************************************************
 * ReceiveUpdateQueue
 ***********************************************************/
void
HotlineClient::ReceiveUpdateQueue()
{
	PRINT(( "Receive update queue\n" ));
	uint32 queue = 0,ref = 0;
	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
		switch (ntohs(dh->type)) {
			case HTLS_DATA_HTXF_REF:
				dh_getint(ref);
				break;
			case HTLS_DATA_FILE_QUEUE:
				dh_getint(queue);
				break;
			default:
				PRINT(( "Type: 0x%x" , ntohs(dh->type) ));
		}
	dh_end()

	BMessage msg(M_RECEIVE_UPDATE_QUEUE);
	msg.AddInt32("queue",queue);
	msg.AddInt32("ref",ref);
	((HApp*)be_app)->TaskWindow()->PostMessage(&msg);
	hx_reset();
}

/***********************************************************
 * Topic change
 ***********************************************************/
void
HotlineClient::SendPrvChatTopicChange(const char* topic,uint32 pcref)
{
	uint8 buf[SIZEOF_HX_HDR + (SIZEOF_HX_DATA_HDR * 2) + 4];
	struct hx_hdr *h = (struct hx_hdr *)buf;
	struct hx_data_hdr *dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR]));
	int16 len = strlen(topic);

	h->type = htonl(HTLC_HDR_CHAT_SUBJECT);
	h->trans = htonl(fHxTrans++);
	h->flag = 0;
	h->len = h->len2 = htonl(14 + len);
	h->hc = htons(2);
	dh->type = htons(HTLC_DATA_CHAT_SUBJECT);
	dh->len = htons(len);
	dh = (struct hx_data_hdr *)(&(buf[SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR]));
	dh->type = htons(HTLC_DATA_CHAT_REF);
	dh->len = htons(4);
	S32HTON(pcref, dh->data);

	SendData( buf, SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR );
	SendData( topic, len );
	SendData( &(buf[SIZEOF_HX_HDR + SIZEOF_HX_DATA_HDR]), SIZEOF_HX_DATA_HDR + 4 );
}

/***********************************************************
 * Topic changed
 ***********************************************************/
void
HotlineClient::ReceivePrvChatTopicChanged()
{
	char *topic = NULL;
	uint32 pcref = 0;
	uint16 len = 0;

	dh_start(&(fHxBuf[SIZEOF_HX_HDR]), fHxPos - SIZEOF_HX_HDR)
		switch (ntohs(dh->type)) {
			case HTLS_DATA_CHAT_REF:
				dh_getint(pcref);
				break;
			case HTLS_DATA_CHAT_SUBJECT:
				len = ntohs(dh->len);
				topic = new char[len +1];
				memcpy(topic, dh->data, len);
				topic[len] = '\0';
				break;
		}
	dh_end()
	if (!len)
		goto ret;

	if (pcref)
	{
		BMessage msg(H_TOPIC_CHANGED);
		msg.AddString("text",topic);
		msg.AddInt32("pcref",pcref);
		be_app->PostMessage(&msg);
	}
ret:
	delete[] topic;
	hx_reset();
}

/***********************************************************
 * Ping
 ***********************************************************/
void
HotlineClient::SendPing()
{
	//BAutolock lock(this);
	time_t timet = time(NULL);
	int32 min;
	((HApp*)be_app)->Prefs()->GetData("interval",&min);
	if(abs(timet - fIdleTime) >= 60*min)
	{
		fIdleTime = timet;
		SendFileList("",0,false);
		PRINT(( "PING\n" ));
	}
}

/***********************************************************
 * Send Data
 ***********************************************************/
int32
HotlineClient::SendData(const void* data,size_t size)
{
	BAutolock lock(fSocketLocker);
	fIdleTime = time(NULL);
	if(!fEndpoint)
	{
		PRINT(("SendData:Socket is NULL\n"));
		return -1;
	}
	return fEndpoint->Send( data, size, 0);
}

/***********************************************************
 * Receive Data
 ***********************************************************/
int32
HotlineClient::ReceiveData(void* data,size_t size,bigtime_t timeout)
{
	BAutolock lock(fSocketLocker);
	int32 r = 0;
	int32 len = 0;
	while(size)
	{
		if(!fEndpoint)
			break;
		if(fEndpoint->IsDataPending(timeout))
		{
			r = fEndpoint->Receive(data,size);
			size -= r;
			len += r;
		}else
			break;
	}
	return len;
}

/***********************************************************
 * Add Task
 ***********************************************************/
void
HotlineClient::AddTaskList(uint32 task,TaskType type)
{
	BAutolock lock(this);

	HLTASK* hltask = new HLTASK;
	hltask->task = task;
	hltask->type = type;
	fTaskList.AddItem(hltask);
}

/***********************************************************
 * Find task
 ***********************************************************/
HLTASK*
HotlineClient::FindTaskList(uint32 task)
{
	BAutolock lock(this);

	register int32 count = fTaskList.CountItems();
	HLTASK *result = NULL;

	while(count > 0)
	{
		HLTASK* hltask = static_cast<HLTASK*>(fTaskList.ItemAt(--count));
		if(hltask)
		{
			if(hltask->task == task)
			{
				result = hltask;
				break;
			}
		}
	}
	return result;
}

/***********************************************************
 * Remove task
 ***********************************************************/
void
HotlineClient::RemoveTaskList(HLTASK *hltask)
{
	BAutolock lock(this);

	fTaskList.RemoveItem(hltask);
	delete hltask;
}
