#include "TServer.h"
#include "hx_types.h"
#include "hl_magic.h"
#include "AppUtils.h"
#include "TextUtils.h"
#include "HApp.h"
#include "HPrvChat.h"
#include "HUserItem.h"
#include "dhargs.h"
#include "HLPacket.h"
#include "HFileTransThread.h"
#include "HPrefs.h"
#include "HUserTimer.h"

#include <Beep.h>
#include <Autolock.h>
#include <Debug.h>

#include <sys/time.h>

#define SLEEP_TIME 1000
#define USER_IDLE_TIME 600
#define ACCEPT_TIMEOUT 1000
#define UPLOADS_FOLDER_NAME "Uploads"

//#define DEBUG

/**************************************************************
 * Constructor.
 **************************************************************/
TServer::TServer(const char* address,uint32 port,uint32 maxConnection,BLooper *target)
		:BLooper(address)
		,fPort(port)
		,fMaxConnections(maxConnection)
		,fTarget(target)
		,fServerSocket(NULL)
		,fAddress(address)

{
	fBanList.MakeEmpty();
	InitValiables();
}

/**************************************************************
 * Destructor.
 **************************************************************/
TServer::~TServer()
{
	delete fServerSocket;
}

/**************************************************************
 * Initialize server valiables.
 **************************************************************/
void
TServer::InitValiables()
{
	BAutolock lock(this);
	fUserList.MakeEmpty();
	fSocketIndex = 1;
	fPrvChatIndex = 1;
	fServerSocket = NULL;
	fCurrentUsers = 0;
	fCurrentDownloads = 0;
	fCurrentUploads = 0;
	fFileTransRef = 1;
	fFileTransList.MakeEmpty();
}

/**************************************************************
 * start serving.
 **************************************************************/
void
TServer::Start()
{
	status_t err;
	//********  Init valiables **********//
	InitValiables();
	//********************************//
	delete fServerSocket;
	fServerSocket = new BNetEndpoint();
#ifdef DEBUG
	BNetDebug::Enable(true);
#endif
	// Bind server socket.
	if(fAddress.Length() > 0) // if specify ip address
	{
		BNetAddress addr(fAddress.String(),fPort);
		err = fServerSocket->Bind(addr);
	}else
		err = fServerSocket->Bind(fPort);
	fConnected = true;
	if(err == B_OK)
	{
		err = fServerSocket->Listen(5);
		if(err == B_OK)
		{
			// start accept thread.
			fHandleConnection = ::spawn_thread(HandleConnectionRequested,"HandleConnectionRequested",B_LOW_PRIORITY,this);
			::resume_thread(fHandleConnection);
			// start listen thread.
			fListenToClient = ::spawn_thread(ListenToClient,"ListenToClient",B_LOW_PRIORITY,this);
			::resume_thread(fListenToClient);
			// start file trans accept thread.
			fTransThread = ::spawn_thread(HandleFileTrans,"HandleFileTrans",B_LOW_PRIORITY,this);
			::resume_thread(fTransThread);

			BString log = "SilverWing server ";
			BNetAddress local_addr = fServerSocket->LocalAddr();
			char hostname[1024];
			::memset(hostname,0,1024);
			local_addr.GetAddr(hostname);

			log << APP_VERSION;
			log <<" started. ";
			log << "Address: " << hostname << "  Port: " << fPort << "\n";
			Log(log.String());
		}
	}else{
		/******* Failed to bind socket ********/
		BString log = "Could not bind socket...\n";
		log << "Address: " << fAddress.String() << "  Port: " << fPort<< "\n";
		Log(log.String(),T_ERROR_TYPE);
		this->PostMessage(B_QUIT_REQUESTED);
	}
	return;
}

/***********************************************************
 * Stop
 ***********************************************************/
void
TServer::Stop()
{
	BAutolock lock(this);
	status_t err;
	fConnected = false;

	::wait_for_thread(fHandleConnection,&err);
	::wait_for_thread(fTransThread,&err);
	::wait_for_thread(fListenToClient,&err);

	fServerSocket->Close();
	this->PostMessage(B_QUIT_REQUESTED);
}

/**************************************************************
 * MessageReceived.
 **************************************************************/
void
TServer::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case T_RESET_ACCOUNT:
	{
		int32 count;
		type_code type;
		message->GetInfo("account_name",&type,&count);
		for(int32 i = 0;i < count;i++)
		{
			const char* name;
			if(message->FindString("account_name",i,&name) == B_OK)
			{
				ResetAccount(name);
			}
		}
		break;
	}
	/*********** Grey out idle users **********/
	case H_USER_TIMER:
	{
		uint32 sock;
		if(message->FindInt32("sock",(int32*)&sock) == B_OK)
		{
			HUserItem *item = FindUser(sock);
			if(!item)
				return;
			uint16 color = item->Color();
			if(color%2 == 0)
			{
				item->SetColor(color+1);
				SendUserChange(sock,item->Nick(),item->Icon(),item->Color());
			}
		}
		break;
	}
	/*********** Broadcast message ***********/
	case T_BROADCAST_MSG:
	{
		const char* text;
		if(message->FindString("text",&text) == B_OK)
			this->SendBroadcastMessage(text);
		break;
	}
	/********** Add client to user list ************/
	case T_ADD_CLIENT:
	{
		this->AddClient(message);
		break;
	}
	/********** Delete client from user list ***********/
	case T_REMOVE_CLIENT:
	{
		HUserItem* user;
		if(message->FindPointer("pointer",(void**)&user) == B_OK)
			this->RemoveClient(user);
		break;
	}
	/**********　Start server **********/
	case T_START_SERVER:
	{
		Start();
		break;
	}
	/********* Stop server **********/
	case T_STOP_SERVER:
	{
		Stop();
		break;
	}
	/******** Remove File trans thread pointer **********/
	case T_REMOVE_TRANS:
	{
		void *data;
		if(message->FindPointer("pointer",&data) == B_OK)
		{
			HFileTransThread *trans = static_cast<HFileTransThread*>(data);
			if(trans)
				this->RemoveFileTrans(trans);
		}
		break;
	}
	default:
		BLooper::MessageReceived(message);
	}
}


/***********************************************************
 *  Handle connections.
 ***********************************************************/
int32
TServer::HandleConnectionRequested(void *data)
{
	TServer *serv = (TServer*)data;
	serv->Log("Handle connection thread was started\n");
	bigtime_t timeout = 30*1000000; // 30sec.
	while( serv->fConnected )
	{
		BNetEndpoint *clientSocket = serv->fServerSocket->Accept(ACCEPT_TIMEOUT);
		if(!serv->fConnected)
			break;
		if(!clientSocket)
		{
			::snooze(SLEEP_TIME);
			continue;
		}

		BNetBuffer buf;
		buf.InitCheck();
		if( serv->CheckBanList(clientSocket) )
		{
			clientSocket->Close();
			delete clientSocket;
			clientSocket = NULL;
			continue;
		}

		/******* Check hotline client ********/
		if( serv->fConnected && clientSocket->IsDataPending(timeout) )
		{
			PRINT(("Connection Accepted\n"));
			if( clientSocket->Receive(buf, HTLC_MAGIC_LEN) == HTLC_MAGIC_LEN)
			{
				char* text = new char[HTLC_MAGIC_LEN+1];
				buf.RemoveData(text,HTLC_MAGIC_LEN);
				text[HTLC_MAGIC_LEN] = '\0';
				if( ::strcmp(text, "TRTPHOTL") == 0)
				{
					//clientSocket->Send(HTLS_MAGIC,HTLS_MAGIC_LEN);
					// Login
					BNetBuffer buf;
					if(buf.InitCheck() != B_OK)
					{
						serv->Log("No memory...\n",T_ERROR_TYPE);
						clientSocket->Close();
						delete clientSocket;
						continue;
					}

					BString log;
					BNetAddress addr = clientSocket->RemoteAddr();
					char hostname[1024];
					::memset(hostname,0,1024);
					addr.GetAddr(hostname);
					log << "Connected from " << hostname << "\n";
					serv->Log(log.String());

					/********* Above max users ************/
					if(serv->fCurrentUsers >= serv->fMaxConnections)
					{

						HLPacket header,data;
						if(header.InitCheck() != B_OK || data.InitCheck() != B_OK)
						{
							serv->Log("Memory was exhausted\n",T_ERROR_TYPE);
							clientSocket->Close();
							delete clientSocket;
							continue;
						}
						data.AddString(HTLS_DATA_TASKERROR,"Too many users connected to server.");
						header.CreateHeader(HTLS_HDR_TASK,1,1,data.Size()+2,1);
						if(serv->Lock())
						{
							clientSocket->Send(header);
							clientSocket->Send(data);
							serv->Unlock();
						}
						clientSocket->Close();
						delete clientSocket;
						continue;
					}
					/***************************************/
					BNetBuffer sendBuf;
					if(sendBuf.InitCheck() != B_OK)
					{
						serv->Log("Memory was exhausted...\n",T_ERROR_TYPE);
						clientSocket->Close();
						delete clientSocket;
						continue;
					}
					sendBuf.AppendData(HTLS_MAGIC,HTLS_MAGIC_LEN);
					clientSocket->Send(sendBuf);
					/********* check login header ***********/
					BNetBuffer recvBuf;
					if(recvBuf.InitCheck() != B_OK)
					{
						serv->Log("Memory was exhausted...\n",T_ERROR_TYPE);
						clientSocket->Close();
						delete clientSocket;
						continue;
					}
					bool success = false;
					while(clientSocket->IsDataPending(timeout))
					{
						int32 rlen = 0;
						while( rlen < 22)
						{
						 	int32 r = clientSocket->Receive(recvBuf,22);
							if(r == B_ERROR || r == 0)
								break;
							rlen += r;
						}
						if(rlen == 22)
						{
							success = serv->ProcessLogin(recvBuf,clientSocket);
							if( success )
								break;
						}else
							break;
					}
					if(!success)
					{
						clientSocket->Close();
						delete clientSocket;
					}
				}
				delete[] text;
			}
		}else{ // failed IsDataPending()
				PRINT(( "IsDataPending failed:%s Line:%d\n", __FILE__,__LINE__  ));
				clientSocket->Close();
				delete clientSocket;
				clientSocket = NULL;
			}
		::snooze(SLEEP_TIME);
	}
	serv->Log("Handle connection thread was stopped\n");
	return 0;
}

/**************************************************************
 * Handle file transfers.
 **************************************************************/
int32
TServer::HandleFileTrans(void*data)
{
	TServer *serv = (TServer*)data;
	BNetEndpoint serverSocket;
	serverSocket.InitCheck();
	status_t error = B_OK;

	bigtime_t timeout = 30*1000000; // 30sec.
	// Bind server socket.
	if(serv->fAddress.Length() > 0) // if specify ip address
	{
		BNetAddress addr(serv->fAddress.String(),serv->fPort+1);
		error = serverSocket.Bind(addr);
	}else
		error = serverSocket.Bind(serv->fPort+1);
	if(error != B_OK)
	{
		serv->Log("Could not create file transfer socket.\n",T_ERROR_TYPE);
		return -1;
	}
	serverSocket.Listen(5);

	serv->Log("Handle file transfer thread was started\n");

	while( serv->fConnected )
	{
		BNetEndpoint *clientSocket = serverSocket.Accept(ACCEPT_TIMEOUT);

		if(!serv->fConnected)
			break;
		if(!clientSocket)
		{
			::snooze(SLEEP_TIME);
			continue;
		}
		clientSocket->SetReuseAddr(true);
		BNetBuffer buf;
		buf.InitCheck();
		PRINT(("File trans accept.\n"));

		if(clientSocket->IsDataPending(timeout))
		{
			if(serv->Lock())
			{
				if(clientSocket->Receive(buf,16) != 16)
				{
					PRINT(("Unknown packet:%s  %s\n",__FILE__,__LINE__));
					clientSocket->Close();
					delete clientSocket;
					continue;
				}
				serv->Unlock();
			}
		}else{
			clientSocket->Close();
			delete clientSocket;
			clientSocket = NULL;
			continue;
		}
		/****** check magic. ******/
		uint32 dlen,dtype,ref;
		char *magic = new char[4+1];
		buf.RemoveData(magic,4);
		magic[4] = '\0';
		if( ::strcmp(magic,"HTXF") != 0)
		{
			PRINT(("Unknown packet:%s  %s\n",__FILE__,__LINE__));
			continue;
		}
		delete[] magic;
		buf.RemoveUint32(ref);
		buf.RemoveUint32(dtype);
		buf.RemoveUint32(dlen);

		FileTrans *fileTrans = serv->FindTrans(ref);
		if(!fileTrans)
		{
			clientSocket->Close();
			delete clientSocket;
			continue;
		}
		if(fileTrans->isDownload)
		{
		char *sendBuf = new char[0xf000 + 512];
		::memcpy(sendBuf, "\
FILP\0\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\
\2INFO\0\0\0\0\0\0\0\0\0\0\0^AMAC\
TYPECREA\
\0\0\0\0\0\0\1\0\0\0\0\0\0\0\0\0\0\0\0\0\
\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\
\7\160\0\0\0\0\0\0\7\160\0\0\0\0\0\0\0\0\0\3hxd", 115);

		sendBuf[39] = 65 + 12;
		::memcpy(&sendBuf[117], "DATA\0\0\0\0\0\0\0\0", 12);
		S32HTON((fileTrans->data_pos), &sendBuf[129]);

		if(serv->Lock())
		{
				clientSocket->Send(sendBuf,133);
				serv->Unlock();
		}
		delete[] sendBuf;
		}
		if(fileTrans->isDownload)
				serv->fCurrentDownloads++;
			else
				serv->fCurrentUploads++;

		/****** Start thread *********/
		fileTrans->thread = new HFileTransThread(fileTrans->filename.c_str()
									,clientSocket
									,fileTrans->data_pos
									,fileTrans->isDownload
									,serv);
		fileTrans->thread ->Start();
		/*****************************/
		::snooze(SLEEP_TIME);
	}
	serv->Log("Handle file transfer thread was stopped\n");
	serverSocket.Close();
	return 0;
}

/**************************************************************
 * Pick up file transfers.
 **************************************************************/
FileTrans*
TServer::FindTrans(uint32 ref)
{
	FileTrans *trans = NULL;

	if(this->Lock())
	{
		int32 count = fFileTransList.CountItems();
		for(register int32 i = 0;i < count ;i++)
		{
			FileTrans* tmp = (FileTrans*)fFileTransList.ItemAt(i);
			if(!tmp)
				continue;
			if(tmp->ref == ref)
			{
				trans = tmp;
				break;
			}
		}
		this->Unlock();
	}
	return trans;
}

/**************************************************************
 * Add client IP to Ban list.
 **************************************************************/
void
TServer::AddBanList(BNetEndpoint *endpoint)
{
	BAutolock lock(this);
	if(lock.IsLocked())
	{
		BNetAddress addr = endpoint->RemoteAddr();
		char host[255];
		addr.GetAddr(host);
		fBanList.AddString("IP",host);
		PRINT(("Added new Ban IP: %s\n",host ));
	}
}

/**************************************************************
 * Check ban list.
 **************************************************************/
bool
TServer::CheckBanList(BNetEndpoint *endpoint)
{
	bool rc = false;
	type_code type;
	int32 count;
	fBanList.GetInfo("IP",&type,&count);

	// Get users host.
	BNetAddress addr = endpoint->RemoteAddr();
	char host[255];
	addr.GetAddr(host);

	for(register int32 i = 0;i <count;i++)
	{
		const char* ip;
		if( fBanList.FindString("IP",i,&ip) == B_OK)
		{
			if( ::strcmp(ip,host) == 0)
				rc = true;
		}
	}
	return rc;
}

/**************************************************************
 * Clear ban list.
 **************************************************************/
void
TServer::ClearBanList()
{
	fBanList.MakeEmpty();
}

/**************************************************************
 * Listen to client.
 **************************************************************/
int32
TServer::ListenToClient(void *data)
{
	TServer *serv = (TServer*)data;
	struct fd_set readfds;
	struct timeval 		tv; // Blocking time out
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	serv->Log("Listen thread was started\n");
	while(serv->fConnected == true)
	{
		int32 sock_count = serv->Users();
		if(sock_count == 0)
		{
			::snooze(SLEEP_TIME);
			continue;
		}
		FD_ZERO(&readfds);
		for (int j = 0; j < sock_count; j++)
		{
			HUserItem *user = static_cast<HUserItem*>(serv->fUserList.ItemAt(j));
			if(!user )
				continue;
            BNetEndpoint *client = user->Socket();
        	FD_SET(client->Socket(), &readfds);
        }
       	if( ::select(FD_SETSIZE, &readfds, NULL, NULL, &tv)  > 0)
       	//if(client->IsDataPending(1000))
       	{
        	for(int i = 0;i < sock_count;i++)
			{
				HUserItem *user = static_cast<HUserItem*>(serv->fUserList.ItemAt(i));
				if(!user)
					continue;
				BNetEndpoint *client = user->Socket();
				if ( FD_ISSET(client->Socket(), &readfds) )
				{
					BNetBuffer buf;
					buf.InitCheck();
					string str;
					int size = 0,j = 0;
					while(size < 22)
					{
						if(serv->Lock())
						{
							j = client->Receive(buf,22);
							serv->Unlock();
						}
						size += j;

						if(j <= 0)
						{
							serv->RemoveClient(user);
							break;
						}
					}
					if(size == 22) // exclude unknown big packet.
						serv->ProcessHeader(buf,user);
				}
			}
		}
		//}
		::snooze(SLEEP_TIME);
	}// end while
	serv->Log("Listen thread was stopped.\n");
	return 0;
}

/**************************************************************
 * Add client to user list.
 **************************************************************/
void
TServer::AddClient(BMessage *message)
{
	BAutolock lock(this);

	BNetEndpoint *endpoint;
	message->FindPointer("pointer",(void**)&endpoint);
	const char* nick;
	if(message->FindString("nick",&nick) != B_OK)
		nick = "";
	const char* login;
	if(message->FindString("login",&login) != B_OK)
		login = "";
	uint32 trans = message->FindInt32("trans");
	uint16 icon = message->FindInt16("icon");

	HUserItem *user = new HUserItem(nick
								,icon
								,2
								,new HUserTimer(USER_IDLE_TIME,fSocketIndex,this)
								,this
								,endpoint);
	user->SetAccount(login);
	user->SetIndex(fSocketIndex);
	user->SetTrans(trans);
	/********** Send new user to everyone **********/
	this->SendUserChange(fSocketIndex++,nick,icon);
	/************************************************/
	fUserList.AddItem(user);
	/********* Create Log ************/
	BString log="";
	BNetAddress addr = endpoint->RemoteAddr();
	char host[255];
	addr.GetAddr(host);
	log << nick << " was logged in. " << "Host: "<< host << "\n";
	Log(log.String(),T_LOGIN_TYPE);
	/***********************************/

	PRINT(("New user added. User count: %d\n",fCurrentUsers+1 ));
	fCurrentUsers++;
}


/**************************************************************
 * Check user account.
 **************************************************************/
bool
TServer::CheckAccount(const char* login,const char* password)
{
	BAutolock lock(this);
	bool result = false;
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Accounts");
	if(strlen(login) == 0) // if data don't have login, look as guest.
		path.Append("guest");
	else
		path.Append(login);

	BFile file(path.Path(),B_READ_ONLY);
	if(file.InitCheck() == B_OK)
	{
		BMessage msg;
		msg.Unflatten(&file);
		const char* pass;
		if(msg.FindString("password",&pass) == B_OK)
		{
			if( ::strcmp(pass,password) == 0 )
			{
				result = true;
			}
		}
	}
	return result;
}


/**************************************************************
 * Find user by index.
 **************************************************************/
HUserItem*
TServer::FindUser(uint32 index)
{
	HUserItem *item = NULL;

	if(this->Lock())
	{
		uint32 count = fUserList.CountItems();
		for(register uint32 i = 0;i < count ;i++)
		{
			HUserItem* user = static_cast<HUserItem*>(fUserList.ItemAt(i));
			if(!user)
				continue;
			if(user->Index() == index)
			{
				item = user;
				break;
			}
		}
		this->Unlock();
	}
	return item;
}

/***********************************************************
 * Find private chat by private chat reference
 ***********************************************************/
HPrvChat*
TServer::FindPrvChat(uint32 pcref)
{
	HPrvChat *result = NULL;
	if(this->Lock())
	{
		uint32 count = fPrvChatList.CountItems();
		for(register uint32 i = 0;i < count ;i++)
		{
			HPrvChat* tmp = static_cast<HPrvChat*>(fPrvChatList.ItemAt(i));
			if(!tmp)
				continue;
			if(tmp->Pcref() == pcref)
			{
				result = tmp;
				break;
			}
		}
		this->Unlock();
	}
	return result;
}

/**************************************************************
 * Delete client from user list.
 **************************************************************/
void
TServer::RemoveClient(HUserItem *user)
{
	BAutolock lock(this);
	BString title;
	title << user->Nick() << " was logged out.\n";
	if(fUserList.RemoveItem(user))
	{
		this->Log(title.String(),T_LOGOUT_TYPE);
	}
	this->SendUserLeave(user->Index());
	delete user;
	PRINT(("User removed\n"));
	fCurrentUsers--;
}

/**************************************************************
 * Process login.
 **************************************************************/
bool
TServer::ProcessLogin(BNetBuffer &buf,BNetEndpoint *endpoint)
{
	uint32 type,trans,flag,len,len2;
	uint16 hc;
	buf.RemoveUint32(type);
	buf.RemoveUint32(trans);
	buf.RemoveUint32(flag);
	buf.RemoveUint32(len);
	buf.RemoveUint32(len2);
	buf.RemoveUint16(hc);
	bool rc = false;

	switch(type)
	{
		case HTLC_HDR_LOGIN:
		{
			rc = ReceiveLogin(endpoint,len2,hc,trans);
 			break;
		}
	}
	return rc;
}

/**************************************************************
 * Process hotline headers.
 **************************************************************/
void
TServer::ProcessHeader(BNetBuffer &buf,HUserItem *user)
{
	BNetBuffer trash;
	uint32 type,trans,flag,len,len2;
	uint16 hc;
	buf.RemoveUint32(type);
	buf.RemoveUint32(trans);
	buf.RemoveUint32(flag);
	buf.RemoveUint32(len);
	buf.RemoveUint32(len2);
	buf.RemoveUint16(hc);
	user->SetTrans(trans);
	//reset user timer
	user->ResetIdleTime();
	//
	switch(type)
	{
		/******* Private chat topic *******/
		case HTLC_HDR_CHAT_SUBJECT:
		{
			ReceivePrvChatTopicChanged(user,len,hc,trans);
			break;
		}
		/******* Receive file info ******/
		case HTLC_HDR_FILE_GETINFO:
		{
			ReceiveFileInfo(user,len,hc,trans);
			break;
		}
		/******* Receive uploads ******/
		case HTLC_HDR_FILE_PUT:
		{
			ReceiveFilePut(user,len,hc,trans);
			break;
		}
		/******* Receive downloads ******/
		case HTLC_HDR_FILE_GET:
		{
			ReceiveFileGet(user,len,hc,trans);
			break;
		}
		/******* Leave private chat ******/
		case HTLC_HDR_CHAT_LEAVE:
		{
			ReceivePrvChatLeave(user,len,hc,trans);
			break;
		}
		/******* Join private chat *******/
		case HTLC_HDR_CHAT_JOIN:
		{
			ReceivePrvChatJoin(user,len,hc,trans);
			break;
		}
		case HTLS_HDR_CHAT_INVITE:
		{
			ReceivePrvChatInvite(user,len,hc,trans);
			break;
		}
		/******* Create Private Chat *******/
		case HTLC_HDR_CHAT_CREATE:
		{
			ReceivePrvChatCreate(user,len,hc,trans);
			break;
		}
		/******* Create folder ******/
		case HTLC_HDR_FILE_MKDIR:
		{
			ReceiveFolderCreate(user,len,hc,trans);
			break;
		}
		/******* Delete files *******/
		case HTLC_HDR_FILE_DELETE:
		{
			ReceiveFileDelete(user,len,hc,trans);
			break;
		}
		/******* Move files *********/
		case HTLC_HDR_FILE_MOVE:
		{
			ReceiveFileMove(user,len,hc,trans);
			break;
		}
		/******* Chat *******/
		case HTLC_HDR_CHAT:
		{
			ReceiveChat(user,len,hc,trans);
			break;
		}
		/****** Message ******/
		case HTLC_HDR_MSG:
		{
			ReceiveMessage(user,len,hc,trans);
			break;
		}
		/****** User list ******/
		case HTLC_HDR_USER_GETLIST:
		{
			ReceiveUserList(user,len,hc,trans);
			break;
		}
		/****** Change user info ******/
		case HTLC_HDR_USER_CHANGE:
		{
			ReceiveUserChange(user,len,hc,trans);
			break;
		}
		/****** Get user info *******/
		case HTLC_HDR_USER_GETINFO:
		{
			ReceiveUserInfo(user,len,hc,trans);
			break;
		}
		/****** Kick user [including Ban] ********/
		case HTLC_HDR_USER_KICK:
		{
			ReceiveUserKick(user,len,hc,trans);
			break;
		}
		/******　News *******/
		case HTLC_HDR_NEWS_GETFILE:
		{
			ReceiveNews(user,len,hc,trans);
			break;
		}
		/****** Post news ******/
		case HTLC_HDR_NEWS_POST:
		{
			this->ReceiveNewsPost(user,len,hc,trans);
			break;
		}
		/****** View File list ******/
		case HTLC_HDR_FILE_LIST:
		{
			this->ReceiveFileGetList(user,len,hc,trans);
			break;
		}
		/****** SilverWing mode ******/
		case HTLC_HDR_SILVERWING_MODE:
		{
			this->ReceiveSilverWingMode(user,len,hc,trans);
			break;
		}
		/****** Unkown type *******/
		default:
			PRINT(( "Unknown Type: 0x%x", type ));
			ReceiveUnknown(user,len,hc,trans);
			break;
	}
}

/**************************************************************
 * Receive get file message.
 **************************************************************/
void
TServer::ReceiveFileGet(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	HLPacket buf;

	len-=2;

	if(ReadData(buf,len,user)!=len)
		return;

	BString path = "";
	BString filename = "";
	uint32 data_pos = 0,rsrc_pos = 0;
	uint32 preview = 0;

	uint16 dtype = 0,dlen = 0,dir_count = 0;

	for(register int16 i = 0;i < hc ;i++)
	{
		buf.RemoveUint16(dtype);
		buf.RemoveUint16(dlen);
		switch(dtype)
		{
		case HTLC_DATA_DIR:
		{
			uint16 enc;
			uint8  nlen;
			buf.RemoveUint16(dir_count);
			for(register int i = 0;i < dir_count;i++)
			{
				buf.RemoveUint16(enc);
				buf.RemoveUint8(nlen);
				char *tmp = new char[nlen+1];
				buf.RemoveData(tmp,nlen);
				tmp[nlen] = '\0';
				path << tmp;
				path << "/";
				delete[] tmp;
			}
			break;
		}
		case HTLC_DATA_RESUMEINFO:
		{
			char* data = new char[dlen+1];
			buf.RemoveData(data,dlen);
			uint32 dataLen;
			if(dlen > 70)
			{
				::memcpy(&dataLen,&data[46],4);
				data_pos = ntohl(dataLen);
				::memcpy(&dataLen,&data[62],4);
				rsrc_pos = ntohl(dataLen);
			}
			delete[] data;
			break;
		}
		case HTLC_DATA_FILE:
		{
			char* name = new char[dlen+1];
			buf.RemoveData(name,dlen);
			name[dlen] = '\0';
			filename = name;
			delete[] name;
			break;
		}
		case HTLC_DATA_FILE_PREVIEW:
		{
			preview = buf.GetUint(dlen);
			break;
		}
		default:
		{
			char* tmp = new char[dlen+1];
			buf.RemoveData(tmp,dlen);
			delete[] tmp;
			break;
		}
		}
	}
	path << filename.String();

	/******* not supported file preview *********/
	if(preview != 0)
	{
		this->SendError(user,trans,"Could not preview files.\r\n Not yet implemented.");
		return;
	}
	/********** Check sim downloads ***********/
	uint32 sim;
	((HApp*)be_app)->Prefs()->GetData("sim_download",(int32*)&sim);
	if(fCurrentDownloads >= sim)
	{
		this->SendError(user,trans,"Too many users are downloading now. Try again later.");
		return;
	}
	/**********************************************/
	BPath filePath = AppUtils().GetAppDirPath(be_app);
	filePath.Append("Files");
	filePath.Append(path.String());

	BFile file(filePath.Path(),B_READ_ONLY);
	if(file.InitCheck() == B_OK && file.IsDirectory() == false)
	{
		off_t file_size;
		file.GetSize(&file_size);
		uint32 fSize = file_size;
		fSize -= data_pos;

		HLPacket header,data;
		if(header.InitCheck() != B_OK||data.InitCheck() != B_OK)
		{
			Log("Memory was exhausted\n",T_ERROR_TYPE);
			return;
		}
		data.AddUint32(HTLS_DATA_HTXF_REF,fFileTransRef);
		data.AddUint32(HTLS_DATA_HTXF_SIZE,fSize);

		header.CreateHeader(HTLS_HDR_TASK,trans,0,data.Size()+2,2);

		if(user->CanDownload())
		{
			this->AddDownload(filePath.Path(),fFileTransRef++,fSize);
			BString log;
			log<<user->Nick() << " is downloading files.\n";
			Log(log.String(),T_DOWNLOAD_TYPE);
			if(user->Lock())
			{
				user->Socket()->Send(header);
				user->Socket()->Send(data);
				user->Unlock();
			}
		}else{
			this->SendError(user,trans,"You are not allowed to download files.");
		}
	}else{
		this->SendError(user,trans,"Could not find such a file.");
	}
}

/**************************************************************
 * Receive put file message.
 **************************************************************/
void
TServer::ReceiveFilePut(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	HLPacket buf;

	len-=2;

	if( ReadData(buf,len,user) != len)
		return;

	uint16 dtype,dlen;
	BString filename = "";
	BString path = "";
	uint32 filesize = 0;
	uint32 data_pos =0,rsrc_pos = 0;
	uint32 resume_flag = 0;
	for(register int16 i = 0;i < hc ;i++)
	{
		buf.RemoveUint16(dtype);
		buf.RemoveUint16(dlen);
		PRINT(("Data Type: 0x%x\n",dtype ));
		switch(dtype)
		{
			case HTLC_DATA_FILE:
			{
				char *name = new char[dlen +1 ];
				buf.RemoveData(name,dlen);
				name[dlen] = '\0';
				filename = name;
				delete[] name;
				break;
			}
			case HTLC_DATA_RESUMEFLAG:
			{
				char* data = new char[dlen+1];
				buf.RemoveData(data,dlen);
				data[dlen] = '\0';
				uint32 dataLen;

				if(dlen > 70)
				{
					::memcpy(&dataLen,&data[46],4);
					data_pos = ntohl(dataLen);
					::memcpy(&dataLen,&data[62],4);
					rsrc_pos = ntohl(dataLen);
					resume_flag = 1;
					PRINT(("Resume: %d\n", resume_flag ));
				}else{
					resume_flag = buf.GetUint(dlen);
					PRINT(("Resume: %d\n", resume_flag));
				}
				delete[] data;
				break;
			}
			case HTLC_DATA_DIR:
			{
				uint16 enc;
				uint8  nlen;
				uint16 dir_count;
				buf.RemoveUint16(dir_count);
				for(register int i = 0;i < dir_count;i++)
				{
					buf.RemoveUint16(enc);
					buf.RemoveUint8(nlen);
					char *tmp = new char[nlen+1];
					buf.RemoveData(tmp,nlen);
					tmp[nlen] = '\0';
					path << tmp;
					path << "/";
					delete[] tmp;
				}
				break;
			}
			case HTLC_DATA_HTXF_SIZE:
			{
				filesize = buf.GetUint(dlen);
				break;
			}
			default:
			{
				char *tmp = new char[dlen+1];
				buf.RemoveData(tmp,dlen);
				delete[] tmp;
			}
		}
	}


	BPath filePath = AppUtils().GetAppDirPath(be_app);
	filePath.Append("Files");
	filePath.Append(path.String());
	filePath.Append(filename.String());
	HLPacket header,data;
	if(header.InitCheck() != B_OK||data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	BFile file(filePath.Path(),B_READ_ONLY);
	status_t err = file.InitCheck();
	/********** Check sim downloads ***********/
	uint32 sim;
	((HApp*)be_app)->Prefs()->GetData("sim_upload",(int32*)&sim);
	if(fCurrentUploads >= sim)
	{
		this->SendError(user,trans,"Too many users are uploading now. Try again later…");
		return;
	}

	/*******************************************/
	if( err == B_OK && resume_flag == 1)
	{
		//Upload with resume.
		if(user->CanUpload())
		{
			data.AddUint32(HTLS_DATA_HTXF_REF,fFileTransRef);
			data.AddUint32(HTLC_DATA_RESUMEINFO,data_pos);
			this->AddUpload(filePath.Path(),fFileTransRef++,data_pos);
			header.CreateHeader(HTLS_HDR_TASK,trans,0,data.Size()+2,3);
			if(user->Lock())
			{
				user->Socket()->Send(header);
				user->Socket()->Send(data);
				user->Unlock();
			}
			//this->SendError(user,trans,"Could not resume uploading. \n Not yet implemented.");
		}else{
			this->SendError(user,trans,"You are not allowed to upload files.");
		}
	}else if(err == B_OK && resume_flag == 0){
		this->SendError(user,trans,"This file has been uploaded.");
	}else if( err != B_OK && resume_flag == 1){
		this->SendError(user,trans,"Could not find such a file.\n You can't resume upload.");
		return;
	}else{
		// Upload without resume.
		data.AddUint32(HTLS_DATA_HTXF_REF,fFileTransRef);
		header.CreateHeader(HTLS_HDR_TASK,trans,0,data.Size()+2,2);
		// check uploads folder
		bool uploads = false;
		int32 i = path.FindFirst("/");
		if(i != B_ERROR)
		{
			BString tmp;
			path.CopyInto(tmp,0,i);
			if(tmp.Compare(UPLOADS_FOLDER_NAME) == 0)
				uploads = true;
		}

		if(user->CanUpload() || uploads == true)
		{
			BString log;
			log<<user->Nick() << " is uploading files.\n";
			Log(log.String(),T_UPLOAD_TYPE);
			this->AddUpload(filePath.Path(),fFileTransRef++,data_pos);
			if(user->Lock())
			{
				user->Socket()->Send(header);
				user->Socket()->Send(data);
				user->Unlock();
			}
			/*********** This is test function **********/
			BString str;
			str << filename << " was uploaded by " << user->Nick();
			Log2News(user,str.String());
			/********************************************/
		}else{
			this->SendError(user,trans,"You are not allowed to upload files.");
		}
	}

}


/**************************************************************
 * Receive creating private chat.
 **************************************************************/
void
TServer::ReceivePrvChatCreate(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	HLPacket buf;

	len-=2;

	BString log;
	log<<user->Nick() << " is creating private chat.\n";
	Log(log.String(),T_NORMAL_TYPE);

	if(ReadData(buf,len,user)!=len)
		return;

	uint16 dtype,dlen;
	buf.RemoveUint16(dtype);
	buf.RemoveUint16(dlen);
	if(dtype == HTLC_DATA_SOCKET)
	{
		uint32 sock = buf.GetUint(dlen);
		if(user->CanCreatePrvChat())
			this->CreatePrvChat(user,sock,trans);
		else
			this->SendError(user,trans,"You are not allowed to create private chat.");
	}
}

/**************************************************************
 * Receive join private chat.
 **************************************************************/
void
TServer::ReceivePrvChatJoin(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	HLPacket buf;
	len-=2;

	if(ReadData(buf,len,user) != len)
		return;

	uint16 dtype,dlen;
	buf.RemoveUint16(dtype);
	buf.RemoveUint16(dlen);
	if(dtype == HTLC_DATA_CHAT_REF)
	{
		uint32 pcref = buf.GetUint(dlen);
		this->AddPrvChatClient(user,pcref,trans);
	}
}

/***********************************************************
 * Receive private chat invitation
 ***********************************************************/
void
TServer::ReceivePrvChatInvite(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	HLPacket buf;

	len-=2;
	uint32 pcref = 0;
	uint32 sock = 0;

	if( ReadData(buf,len,user) != len)
		return;

	for(register int i = 0;i < hc ;i++)
	{
		uint16 dtype,dlen;
		buf.RemoveUint16(dtype);
		buf.RemoveUint16(dlen);
		switch(dtype)
		{
		case HTLC_DATA_CHAT_REF:
		{
			pcref = buf.GetUint(dlen);
			break;
		}
		case HTLC_DATA_SOCKET:
		{
			sock = buf.GetUint(dlen);
			break;
		}
		default:
		{
			char *tmp = new char[dlen+1];
			buf.RemoveData(tmp,dlen);
			delete[] tmp;
		}
		}
	}
	HPrvChat *prvchat = FindPrvChat(pcref);
	if(!prvchat)
		SendError(user,trans,"Cound not find such a private chat room");
	else{
		if(prvchat->FindUser(sock )) // already joined
			SendError(user,trans,"User have already joined this private chat room");
		else
			SendPrvChatInvite(user,sock,pcref,trans);
	}
}
/**************************************************************
 * Receive leave private chat.
 **************************************************************/
void
TServer::ReceivePrvChatLeave(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	HLPacket buf;
	len-=2;

	if(ReadData(buf,len,user)!=len)
		return;

	uint16 dtype,dlen;
	buf.RemoveUint16(dtype);
	buf.RemoveUint16(dlen);
	if(dtype == HTLC_DATA_CHAT_REF)
	{
		uint32 sock = buf.GetUint(dlen);
		this->RemovePrvChatClient(user,sock,trans);
	}
}

/***********************************************************
 * Receive private chat topic changed
 ***********************************************************/
void
TServer::ReceivePrvChatTopicChanged(HUserItem* user,uint32 len,uint16 hc,uint32 trans)
{
	HLPacket buf;
	len-=2;
	uint32 pcref = 0;
	char *topic = NULL;

	if( ReadData(buf,len,user) != len)
		return;

	for(register int i = 0;i < hc ;i++)
	{
		uint16 dtype,dlen;
		buf.RemoveUint16(dtype);
		buf.RemoveUint16(dlen);
		switch(dtype)
		{
		case HTLC_DATA_CHAT_REF:
		{
			pcref = buf.GetUint(dlen);
			break;
		}
		case HTLC_DATA_CHAT_SUBJECT:
		{
			topic = new char[dlen+1];
			buf.RemoveData(topic,dlen);
			topic[dlen] = '\0';
			break;
		}
		default:
		{
			char *tmp = new char[dlen+1];
			buf.RemoveData(tmp,dlen);
			delete[] tmp;
		}
		}
	}

	if(topic)
	{
		HPrvChat *prvchat = FindPrvChat(pcref);
		if(prvchat)
		{
			prvchat->SetTopic(topic);
		}
	}
	delete[] topic;
}

/**************************************************************
 * Receive unknown or not supported hotline headers.
 **************************************************************/
void
TServer::ReceiveUnknown(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	BNetBuffer buf;

	len-=2;
	if( ReadData(buf,len,user) != len)
		return;
	this->SendError(user,trans,"Unknown packet or not implemented function.");
}

/**************************************************************
 * Receive get user list message.
 **************************************************************/
void
TServer::ReceiveUserList(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	BNetBuffer buf;
	len-=2;

	if(ReadData(buf,len,user) != len)
		return;

	this->SendUserList(user->Socket(),trans);
	return;
}

/**************************************************************
 * Receive get user info message.
 **************************************************************/
void
TServer::ReceiveUserInfo(HUserItem* user,uint32 len,uint16 hc,uint32 trans)
{
	HLPacket buf;

	len -= 2;

	BString log;
	log<<user->Nick() << " is getting user info.\n";
	Log(log.String(),T_NORMAL_TYPE);

	if( ReadData(buf,len,user) != len)
		return;

	uint16 dtype,dlen;
	buf.RemoveUint16(dtype);
	buf.RemoveUint16(dlen);
	if(dtype == HTLC_DATA_SOCKET)
	{
		uint32 sock = buf.GetUint(dlen);
		if(user->CanGetInfo())
			this->SendUserInfo(user,sock,trans);
		else
			this->SendError(user,trans,"You are not allowed to get user info.");
	}
}

/**************************************************************
 * Receive kick or ban users.
 **************************************************************/
void
TServer::ReceiveUserKick(HUserItem* user,uint32 len,uint16 hc,uint32 trans)
{
	HLPacket buf;

	uint32 sock = 0;
	uint32 isBan = 0;
	len -= 2;

	if( ReadData(buf,len,user) != len)
		return;

	for(register int i = 0;i < hc ;i++)
	{
		uint16 dtype,dlen;
		buf.RemoveUint16(dtype);
		buf.RemoveUint16(dlen);
		switch(dtype)
		{
		case HTLC_DATA_SOCKET:
		{
			sock = buf.GetUint(dlen);
			break;
		}
		case HTLC_DATA_BAN:
		{
			isBan = buf.GetUint(dlen);
			break;
		}
		default:
		{
			char *tmp = new char[dlen+1];
			buf.RemoveData(tmp,dlen);
			delete[] tmp;
		}
		}
	}
	HUserItem *target = FindUser(sock);
	if(!target)
		return;
	// If kick yourself.
	if(user->Index() == sock)
	{
		this->SendError(user,trans,"You can not kick yourself.");
		return;
	}

	if(target->CanKickUser()||target->CanBanUser())
	{
		this->SendError(user,trans,"You are not allowed to kick this user.");
	}
	if(isBan == 0)
	{
		if(user->CanKickUser())
			this->KickUser(sock);
		else
			this->SendError(user,trans,"You are not allowed to kick users.");
	}else{
		if(user->CanBanUser())
		{
			this->KickUser(sock);
			this->AddBanList(user->Socket());
		}
		else
			this->SendError(user,trans,"You are not allowed to ban users.");
	}
}

/**************************************************************
 * Receive login.
 **************************************************************/
bool
TServer::ReceiveLogin(BNetEndpoint *endpoint,uint32 len,uint16 hc,uint32 trans)
{
	bigtime_t timeout = 60*1000000; //60 secs

	HLPacket buf;
	if(buf.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return false;
	}
	uint32 size = 0;
	int32 j = 0;
	BString nick = "";
	BString login = "";
	BString password = "";
	char* tmp = NULL;
	uint16 version = 0;
	uint16 icon = 0;
	len-=2;

	while(size < len)
	{
		if(endpoint->IsDataPending(timeout))
		{
			j = endpoint->Receive(buf,len);
			if(j == B_ERROR)
				return false;
			size += j;
		}else
			return false;
	}

	for(register int i = 0;i<hc ;i++)
	{
		uint16 dtype,dlen;
		buf.RemoveUint16(dtype);
		buf.RemoveUint16(dlen);

		switch(dtype)
		{
		case HTLC_DATA_NICK:
			tmp = new char[dlen+1];
			buf.RemoveData(tmp,dlen);
			tmp[dlen] = '\0';
			nick = tmp;
			delete[] tmp;
			break;
		case HTLC_DATA_ICON:
			buf.RemoveUint16(icon);
			break;
		case HTLC_DATA_LOGIN:
			tmp = new char[dlen+1];
			buf.RemoveData(tmp,dlen);
			tmp[dlen] = '\0';
			for (register int16 i = 0; i < dlen; i++)
				login << (char)(255 - tmp[i]);
			delete[] tmp;
			break;
		case HTLC_DATA_PASSWORD:
		{
			tmp = new char[dlen+1];
			buf.RemoveData(tmp,dlen);
			tmp[dlen] = '\0';
			for (register int16 i = 0; i < dlen; i++)
				password << (char)(255 - tmp[i]);
			delete[] tmp;
		 	break;
		}
		 case HTLS_SERVER_VERSION:
		 	buf.RemoveUint16(version);
		 	break;
		 default:
		 {
		 	char* trash = new char[dlen+1];
		 	buf.RemoveData(trash,dlen);
		 	delete[] trash;
		 	break;
		 }
		}
	}

	PRINT(("login:%s\n",login.String() ));
	PRINT(("password:%s\n",password.String() ));
	PRINT(("version:%d\n", version ));
	/*********** Too many users logged in *********/
	if( fMaxConnections <= fCurrentUsers)
	{
		HLPacket header,data;
		if(header.InitCheck() != B_OK || data.InitCheck() != B_OK)
		{
			Log("Memory was exhausted.\n",T_ERROR_TYPE);
			return false;
		}
		data.AddString(HTLS_DATA_TASKERROR,"Login incorrect. Too many users logged in now.");
		header.CreateHeader(HTLS_HDR_TASK,trans,1,data.Size()+2,1);
		if(this->Lock())
		{
			endpoint->Send(header);
			endpoint->Send(data);
			this->Unlock();
		}
		return false;
	}
	/************ Check Account ***********/
	if( CheckAccount(login.String(),password.String()) )
	{
		this->SendServerVersion(endpoint,trans);
		this->SendAgreement(endpoint,trans);
		BMessage msg(T_ADD_CLIENT);
		msg.AddString("nick",nick.String());
		msg.AddString("nick","");
		msg.AddString("password",password.String());
		msg.AddString("login",login.String());
		msg.AddInt16("icon",icon);
		msg.AddPointer("pointer",endpoint);
		msg.AddInt32("trans",(int32)trans);
		this->PostMessage(&msg);
		return true;
	}else{
		HLPacket header,data;
		if(header.InitCheck() != B_OK || data.InitCheck() != B_OK)
		{
			Log("Memory was exhausted.\n",T_ERROR_TYPE);
			return false;
		}
		data.AddString(HTLS_DATA_TASKERROR,"Login incorrect.");
		header.CreateHeader(HTLS_HDR_TASK,trans,1,data.Size()+2,1);
		if(this->Lock())
		{
			endpoint->Send(header);
			endpoint->Send(data);
			this->Unlock();
		}
	}
	return false;
}

/**************************************************************
 * Receive chat message.
 **************************************************************/
void
TServer::ReceiveChat(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	HLPacket buf;

	uint16 type;
	uint16 dlen;
	uint32 pcref = 0;
	char* chatmsg = NULL;

	len -= 2;

	if( ReadData(buf,len,user) != len)
		return;

	for(int16 i = 0;i < hc;i++)
	{
		buf.RemoveUint16(type);
		buf.RemoveUint16(dlen);
		switch( type )
		{
		case HTLC_DATA_CHAT:
		{
			chatmsg = new char[dlen+1];
			::memset(chatmsg,0,dlen+1);
			buf.RemoveData(chatmsg,dlen);
			chatmsg[dlen] = '\0';
			break;
		}
		case HTLC_DATA_CHAT_REF:
		{
			pcref = buf.GetUint(dlen);
			break;
		}
		}
	}

	HLPacket data;
	if( data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	BString msg;
	char nick[17];
	::memset(nick,0x20,13);
	int nlen = strlen(user->Nick());
	int pos = 13 - nlen;
	if(pos >= 0)
		memcpy(&nick[pos],user->Nick(),nlen);
	else
		memcpy(nick,user->Nick(),13);
	nick[13] = ':';
	nick[14] = 0x20;
	nick[15] = 0x20;
	nick[16] = '\0';
	int32 chat_len = strlen(chatmsg);
	msg << "\r" << nick;
	for(register int32 i = 0;i < chat_len;i++)
	{
		if(chatmsg[i] != '\r')
			msg << chatmsg[i];
		else{
			msg << chatmsg[i];
			msg << nick;
		}
	}

	if(data.InitCheck() != B_OK)
	{
		Log("No memory... Could not send chat\n",T_ERROR_TYPE);
		delete[] chatmsg;
		return;
	}

	data.AddString(HTLC_DATA_CHAT,msg.String());

	if(pcref == 0)
		this->SendToAllUsers(data,HTLS_HDR_CHAT,data.Size()+2,1);
	else{
		HPrvChat* chat = FindPrvChat(pcref);
		if(chat)
		{

			PRINT(("prvchat: %s\n",msg.String() ));

			data.AddUint32(HTLC_DATA_CHAT_REF,pcref);
			chat->SendToAllUsers(data,HTLS_HDR_CHAT,data.Size()+2,2);
		}
	}
	delete[] chatmsg;
}

/**************************************************************
 * Receive view news.
 **************************************************************/
void
TServer::ReceiveNews(HUserItem *user ,uint32 len,uint16 hc,uint32 trans)
{
	BNetBuffer buf;

	len -= 2;

	if( ReadData(buf,len,user) != len)
		return;

	if(user->CanReadNews())
	{
		BString log;
		log<<user->Nick() << " is loading news.\n";
		Log(log.String(),T_NORMAL_TYPE);
		this->SendNews(user,trans);
	}else
		this->SendError(user,trans,"You are not allowed to view news.");
}

/**************************************************************
 * Receive get file list message.
 **************************************************************/
void
TServer::ReceiveFileGetList(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	BNetBuffer buf;

	BString str;
	len -= 2;

	uint32 size = ReadData(buf,len,user);
	if(size != len)
		return;
	BString path = "";
	uint16 dtype = 0,dlen = 0,dir_count = 0;
	if(size != 0)
	{
		buf.RemoveUint16(dtype);
		buf.RemoveUint16(dlen);
		buf.RemoveUint16(dir_count);
	}

	if(dtype == HTLC_DATA_DIR)
	{
		uint16 enc;
		uint8  nlen;
		for(register int i = 0;i < dir_count;i++)
		{
			buf.RemoveUint16(enc);
			buf.RemoveUint8(nlen);
			char *tmp = new char[nlen+1];
			buf.RemoveData(tmp,nlen);
			tmp[nlen] = '\0';
			path << tmp;
			path << "/";
			delete[] tmp;
		}

	}
	if(user->CanViewFile())
	{
		BString log;
		log<<user->Nick() << " is loading file list.\n";
		Log(log.String(),T_NORMAL_TYPE);
		this->SendFileList(user,trans,path.String());
	}else
		this->SendError(user,trans,"You are not allowed to view files.");
}

/**************************************************************
 * Receive delete file message.
 **************************************************************/
void
TServer::ReceiveFileDelete(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	BNetBuffer buf;

	BString str;
	BString dir_path = "";
	BString filename;
	len -= 2;

	if( ReadData(buf,len,user) != len)
		return;

	for(register int i = 0;i < hc;i++)
	{
		uint16 dtype,dlen;
		buf.RemoveUint16(dtype);
		buf.RemoveUint16(dlen);
		switch(dtype)
		{
		case HTLC_DATA_FILE:
		{
			char *name = new char[dlen +1 ];
			buf.RemoveData(name,dlen);
			name[dlen] = '\0';
			filename = name;
			delete[] name;
			break;
		}
		case HTLC_DATA_DIR:
		{
			uint16 dc; // directory count
			uint8 nlen;
			buf.RemoveUint16(dc);
			for(int j = 0;j < dc ;j++)
			{
				uint16 pad;
				buf.RemoveUint16(pad);
				buf.RemoveUint8(nlen);
				char* dir = new char[nlen+1];
				buf.RemoveData(dir,nlen);
				dir[nlen] = '\0';

				dir_path << dir << "/";
				delete[] dir;
			}
			break;
		}
		default:
		{
			char* tmp = new char[dlen+1];
			buf.RemoveData(tmp,dlen);
			delete[] tmp;
		}
		}
	}
	if(user->CanDeleteFile())
	{
		BString log;
		log<<user->Nick() << " is deleting files.\n";
		Log(log.String(),T_NORMAL_TYPE);
		DeleteFile(dir_path.String(),filename.String());
		SendTaskEnd(user,trans);
	}else
		SendError(user,trans,"You are not allowed to delete files.");
}

/***********************************************************
 * Receive file info.
 ***********************************************************/
void
TServer::ReceiveFileInfo(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	BNetBuffer buf;

	BString str;
	BString dir_path = "";
	BString filename = "";
	len -= 2;

	if( ReadData(buf,len,user) != len)
		return;

	for(register int i = 0;i < hc;i++)
	{
		uint16 dtype,dlen;
		buf.RemoveUint16(dtype);
		buf.RemoveUint16(dlen);
		switch(dtype)
		{
		case HTLC_DATA_FILE:
		{
			char *name = new char[dlen +1 ];
			buf.RemoveData(name,dlen);
			name[dlen] = '\0';
			filename = name;
			delete[] name;
			break;
		}
		case HTLC_DATA_DIR:
		{
			uint16 dc; // directory count
			uint8 nlen;
			buf.RemoveUint16(dc);
			for(int j = 0;j < dc ;j++)
			{
				uint16 pad;
				buf.RemoveUint16(pad);
				buf.RemoveUint8(nlen);
				char* dir = new char[nlen+1];
				buf.RemoveData(dir,nlen);
				dir[nlen] = '\0';

				dir_path << dir << "/";
				delete[] dir;
			}
			break;
		}
		default:
		{
			char* tmp = new char[dlen+1];
			buf.RemoveData(tmp,dlen);
			delete[] tmp;
		}
		}
	}
	if(filename.Length() == 0)
		return;

	BString log;
	log<<user->Nick() << " is getting file info.\n";
	Log(log.String(),T_NORMAL_TYPE);

	SendFileInfo(user,trans,filename.String(),dir_path.String());;
}


/***********************************************************
 * Receive Move files.
 ***********************************************************/
void
TServer::ReceiveFileMove(HUserItem* user,uint32 len,uint16 hc,uint32 trans)
{
	BNetBuffer buf;

	BString str;
	BString dir_path = "";
	BString filename = "";
	BString dest_dir = "";
	len -= 2;

	if( ReadData(buf,len,user) != len)
		return;

	for(register int i = 0;i < hc;i++)
	{
		uint16 dtype,dlen;
		buf.RemoveUint16(dtype);
		buf.RemoveUint16(dlen);
		switch(dtype)
		{
		case HTLC_DATA_FILE:
		{
			char *name = new char[dlen +1 ];
			buf.RemoveData(name,dlen);
			name[dlen] = '\0';
			filename = name;
			delete[] name;
			break;
		}
		case HTLC_DATA_DIR:
		{
			uint16 dc; // directory count
			uint8 nlen;
			buf.RemoveUint16(dc);
			for(int j = 0;j < dc ;j++)
			{
				uint16 pad;
				buf.RemoveUint16(pad);
				buf.RemoveUint8(nlen);
				char* dir = new char[nlen+1];
				buf.RemoveData(dir,nlen);
				dir[nlen] = '\0';
				dir_path << dir << "/";
				delete[] dir;
			}
			break;
		}
		case HTLC_DATA_DIR_RENAME:
		{
			uint16 dc; // directory count
			uint8 nlen;
			buf.RemoveUint16(dc);
			for(int j = 0;j < dc ;j++)
			{
				uint16 pad;
				buf.RemoveUint16(pad);
				buf.RemoveUint8(nlen);
				char* dir = new char[nlen+1];
				buf.RemoveData(dir,nlen);
				dir[nlen] = '\0';
				dest_dir << dir << "/";
				delete[] dir;
			}
			break;
		}
		default:
		{
			char* tmp = new char[dlen+1];
			buf.RemoveData(tmp,dlen);
			delete[] tmp;
		}
		}
	}
	if(filename.Length() == 0)
		return;
	if(user->CanMoveFile())
	{
		BString log;
		log<<user->Nick() << " is moving files.\n";
		Log(log.String(),T_NORMAL_TYPE);
		MoveFile(filename.String(),dir_path.String(),dest_dir.String());
	}else
		SendError(user,trans,"You are not allowed to move files.");
}

/**************************************************************
 * Receive creating folder.
 **************************************************************/
void
TServer::ReceiveFolderCreate(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	BNetBuffer buf;

	BString str;
	BString filename = "",dir_path = "";
	len -= 2;
	BString log;
	log<<user->Nick() << " is creating a folder.\n";
	Log(log.String(),T_NORMAL_TYPE);

	if( ReadData(buf,len,user) != len)
		return;

	for(register int i = 0;i < hc;i++)
	{
		uint16 dtype,dlen;
		buf.RemoveUint16(dtype);
		buf.RemoveUint16(dlen);
		switch(dtype)
		{
		case HTLC_DATA_FILE:
		{
			char *name = new char[dlen +1 ];
			buf.RemoveData(name,dlen);
			name[dlen] = '\0';
			filename = name;
			delete[] name;
			break;
		}
		case HTLC_DATA_DIR:
		{
			uint16 dc; // directory count
			uint8 nlen;
			buf.RemoveUint16(dc);
			for(int j = 0;j < dc ;j++)
			{
				uint16 pad;
				buf.RemoveUint16(pad);
				buf.RemoveUint8(nlen);
				char* dir = new char[nlen+1];
				buf.RemoveData(dir,nlen);
				dir[nlen] = '\0';
				dir_path << dir <<"/";
				delete[] dir;
			}
			break;
		}
		default:
		{
			char* tmp = new char[dlen+1];
			buf.RemoveData(tmp,dlen);
			delete[] tmp;
		}
		}
	}

	if(user->CanCreateFolder())
	{
		this->CreateFolder(dir_path.String(),filename.String());
		this->SendTaskEnd(user,trans);
	}else{
		this->SendError(user,trans,"You are not allowed to create folders.");
	}
}


/**************************************************************
 * Receive post news.
 **************************************************************/
void
TServer::ReceiveNewsPost(HUserItem *user ,uint32 len,uint16 hc,uint32 trans)
{
	BNetBuffer buf;

	BString str;
	len -= 2;

	if(ReadData(buf,len,user) != len)
		return;

	uint16 dtype,dlen;
	buf.RemoveUint16(dtype);
	buf.RemoveUint16(dlen);

	if(dtype == HTLC_DATA_NEWS_POST)
	{
		if(user->CanPostNews())
		{
			char *news = new char[dlen+1];
			::memset(news,0,dlen+1);
			buf.RemoveData(news,dlen);
			news[dlen] = '\0';

			time_t timet = time(NULL);
			const char* time = ctime(&timet);
			char* tmp = new char[strlen(time)+1];
			::strcpy(tmp,time);
			tmp[strlen(time)-1] = '\0';
			str << "From " << user->Nick() << " (" << tmp << "):\r"
				<< news
				<<	"\r_________________________________________________________\r";
			delete[] tmp;

			BString log;
			log<<user->Nick() << " is posting news.\n";
			Log(log.String(),T_NORMAL_TYPE);
			this->SendTaskEnd(user,trans);
			this->SendNewsPost(str.String());
			delete[] news;
		}else
			this->SendError(user,trans,"You are not allowed to post news.");
	}
}


/**************************************************************
 * Receive user changing.
 **************************************************************/
void
TServer::ReceiveUserChange(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	HLPacket buf;

	BString msg="";
	uint16 sock = user->Index();

	len -= 2;

	if(ReadData(buf,len,user) != len)
		return;

	BString nick = user->Nick();
	uint16 icon = user->Icon();
	uint16 color = user->Color();

	for(register int i = 0;i < hc;i++)
	{
		uint16 dtype,dlen;
		buf.RemoveUint16(dtype);
		buf.RemoveUint16(dlen);

		switch(dtype)
		{
		case HTLC_DATA_NICK:
		{
			char *tmp = new char[dlen+1];
			buf.RemoveData(tmp,dlen);
			tmp[dlen] = '\0';

			if( ::strcmp(nick.String(),tmp) != 0)
			{
				BString log = nick;
				log << " is changing nickname to " << tmp << "\n";
				this->Log(log.String(),T_NORMAL_TYPE);

				nick = tmp;
			}
			delete[] tmp;
			break;
		}
		case HTLC_DATA_ICON:
		{
			icon = buf.GetUint(dlen);
			break;
		}
		case HTLS_DATA_COLOUR:
		{
			color = buf.GetUint(dlen);
			break;
		}
		default:
		{
			char *tmp = new char[dlen+1];
			buf.RemoveData(tmp,dlen);
			delete[] tmp;
		}
		}
	}

	user->SetNick(nick.String());
	user->SetIcon(icon);
	user->SetColor(color);
	this->SendUserChange(sock,nick.String(),icon,color);
	// Private chat user change

}


/**************************************************************
 * Receive message.
 **************************************************************/
void
TServer::ReceiveMessage(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	HLPacket buf;

	BString msg="";
	uint32 sock = 0;

	bool	isValid = false;

	len -= 2;

	BString log;
	log<<user->Nick() << " sent message.\n";
	Log(log.String(),T_NORMAL_TYPE);

	if(ReadData(buf,len,user) != len)
		return;

	for(register int i = 0;i < hc;i++)
	{
		uint16 dtype,dlen;
		buf.RemoveUint16(dtype);
		buf.RemoveUint16(dlen);

		switch(dtype)
		{
		case HTLC_DATA_MSG:
		{
			char* str = new char[dlen+1];
			buf.RemoveData(str,dlen);
			str[dlen] = '\0';
			msg = str;
			delete[] str;
			break;
		}
		case HTLC_DATA_SOCKET:
		{
			sock = buf.GetUint(dlen);
			isValid = true;
			break;
		}
		default:
		{
			char *tmp = new char[dlen+1];
			buf.RemoveData(tmp,dlen);
			delete[] tmp;
		}
		}
	}
	if(!isValid)
		return;

	HUserItem *receiver = this->FindUser(sock);
	if(!receiver)
		return;
	this->SendMessage(user,receiver,trans,msg.String());
	this->SendTaskEnd(user,trans);
}


/**************************************************************
 * Send data to all users.
 **************************************************************/
void
TServer::SendToAllUsers(void* data,uint32 length)
{
	int32 socket_count = fUserList.CountItems();
	for(int i = 0;i < socket_count;i++)
	{
		HUserItem *user = static_cast<HUserItem*>(fUserList.ItemAt(i));
		if( !user )
			continue;
		if(user->Socket()->Send(data,length) < 0)
		{
			// 	Delete client when could not send data.
			BMessage msg(T_REMOVE_CLIENT);
			msg.AddPointer("pointer",user);
			this->PostMessage(&msg);
		}
	}
}

/**************************************************************
 * Send data to all users.
 **************************************************************/
void
TServer::SendToAllUsers(HLPacket &data,uint32 type,uint32 data_size,uint32 hc)
{
	int32 count = fUserList.CountItems();
	for(register int32 i = 0;i < count;i++)
	{
		HUserItem *user = static_cast<HUserItem*>(fUserList.ItemAt(i));
		if(!user)
			continue;
		uint32 trans = user->Trans();
		HLPacket header;
		if(header.InitCheck() != B_OK)
		{
			Log("Memory was exhauseted\n",T_ERROR_TYPE);
			return;
		}
		header.CreateHeader(type,++trans,0,data_size,hc);
		user->SetTrans(trans);
		if(user->Lock())
		{
			user->Socket()->Send(header);
			user->Socket()->Send(data);
			user->Unlock();
		}
	}
}

/**************************************************************
 * Send error message to client.
 **************************************************************/
void
TServer::SendError(HUserItem* user,uint32 trans,const char* text)
{
	HLPacket header,data;
	if(header.InitCheck() != B_OK || data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	data.AddString(HTLS_DATA_TASKERROR,text);
	header.CreateHeader(HTLS_HDR_TASK,trans,1,data.Size()+2,1);
	if(user->Lock())
	{
		user->Socket()->Send(header);
		user->Socket()->Send(data);
		user->Unlock();
	}
}

/**************************************************************
 * Send agreement.
 **************************************************************/
void
TServer::SendAgreement(BNetEndpoint *endpoint,uint32 trans)
{
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Agreement.txt");

	BFile file(path.Path(),B_READ_ONLY);
	if(file.InitCheck() != B_OK)
		return;
	off_t size;
	file.GetSize(&size);
	char *agree = new char[size+1];
	::memset(agree,0,size+1);
	file.Read(agree,size);
	TextUtils().ConvertReturnsToCR(agree);
	HLPacket header,data;
	if( header.InitCheck() != B_OK|| data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	data.AddString(HTLS_DATA_AGREEMENT,agree);
	header.CreateHeader(HTLS_HDR_AGREEMENT,trans,0,data.Size()+2,1);
	if(this->Lock())
	{
		endpoint->Send(header);
		endpoint->Send(data);
		this->Unlock();
	}
	delete[] agree;
}


/**************************************************************
 * Send server version and info.
 **************************************************************/
void
TServer::SendServerVersion(BNetEndpoint *endpoint,uint32 trans)
{
	HLPacket header,data;
	if(header.InitCheck() != B_OK || data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	/*********** Get server name ***********/
	const char* name;
	((HApp*)be_app)->Prefs()->GetData("server_name",&name);
	data.AddUint16(HTLS_SERVER_VERSION,123);
	data.AddString(HTLS_SERVER_NAME,name);
	header.CreateHeader(HTLS_HDR_TASK,trans,0,data.Size()+2,2);
	if(this->Lock())
	{
		endpoint->Send(header);
		endpoint->Send(data);
		this->Unlock();
	}
}

/**************************************************************
 * Send user list.
 **************************************************************/
void
TServer::SendUserList(BNetEndpoint *endpoint,uint32 trans)
{
	HLPacket header,data;
	uint16 count = fUserList.CountItems();
	if(header.InitCheck() != B_OK || data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	for(register int16 i = 0;i < count;i++)
	{
		HUserItem *item = static_cast<HUserItem*>(fUserList.ItemAt(i));
		if(!item)
			break;
		data.AddUser(HTLS_DATA_USER_LIST,item->Index(),item->Icon(),item->Color(),item->Nick());
	}

	header.CreateHeader(HTLS_HDR_TASK,trans,0,data.Size()+2,count);

	if(this->Lock())
	{
		endpoint->Send(header);
		if(count != 0)
			endpoint->Send(data);
		this->Unlock();
	}
}

/**************************************************************
 * Send left user.
 **************************************************************/
void
TServer::SendUserLeave(uint16 sock)
{
	HLPacket data;
	if( data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	data.AddUint16(HTLS_DATA_SOCKET,sock);

	this->SendToAllUsers(data,HTLS_HDR_USER_LEAVE,data.Size()+2,1);
}

/**************************************************************
 * Send end of task?.
 **************************************************************/
void
TServer::SendTaskEnd(HUserItem *user ,uint32 trans)
{
	HLPacket header;
	if(header.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	header.CreateHeader(HTLS_HDR_TASK,trans,0,2,0);
	if(user->Lock())
	{
		user->Socket()->Send(header);
		user->Unlock();
	}
}

/**************************************************************
 * Send admin broadcast message.
 **************************************************************/
void
TServer::SendBroadcastMessage(const char* text)
{
	HLPacket header,data;
	if(header.InitCheck() != B_OK||data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	data.AddString(HTLC_DATA_MSG,text);
	this->SendToAllUsers(data,HTLS_HDR_MSG,data.Size()+2,1);
}

/**************************************************************
 * Send message to user.
 **************************************************************/
void
TServer::SendMessage(HUserItem *sender,HUserItem* recver,uint32 trans,const char* text)
{
	HLPacket header,data;
	if( header.InitCheck() != B_OK||data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	uint16 sock = sender->Index();
	data.AddUint16(HTLC_DATA_SOCKET,sock);
	data.AddString(HTLC_DATA_NICK,sender->Nick());
	data.AddString(HTLC_DATA_MSG,text);

	header.CreateHeader(HTLS_HDR_MSG,trans+1,0,data.Size()+2,3);
	if(recver->Lock())
	{
		recver->Socket()->Send(header);
		recver->Socket()->Send(data);
		recver->Unlock();
	}
}

/**************************************************************
 * Send news.
 **************************************************************/
void
TServer::SendNews(HUserItem *user,uint32 trans)
{
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("News.txt");
	BFile file(path.Path(),B_READ_ONLY|B_CREATE_FILE);
	if(file.InitCheck() == B_OK)
	{
		file.Lock();
		off_t size;
		file.GetSize(&size);
		char *news = new char[size+1];
		::memset(news,0,size+1);
		file.Read(news,size);
		HLPacket header,data;
		if( header.InitCheck() != B_OK || data.InitCheck() != B_OK)
		{
			Log("Memory was exhausted\n",T_ERROR_TYPE);
			return;
		}
		data.AddString(HTLS_DATA_NEWS,news);
		delete[] news;
		header.CreateHeader(HTLS_HDR_TASK,trans,0,data.Size()+2,1);
		if(user->Lock())
		{
			user->Socket()->Send(header);
			user->Socket()->Send(data);
			user->Unlock();
		}
		file.Unlock();
		file.Unset();
	}else{
		this->SendError(user,trans,"Could not read news.");
	}
}


/***********************************************************
 * Send news categories. ( threaded news version )
 ***********************************************************/
void
TServer::SendNewsCategory(HUserItem *user,uint32 trans,const char* category)
{
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("News");
	path.Append(category);

	HLPacket header,data;
	if( header.InitCheck() != B_OK || data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}

	uint32 num_items = data.CreateCategoryList(path.Path());
	header.CreateHeader(HTLS_HDR_TASK,trans,0,data.Size()+2,num_items);
	if(user->Lock())
	{
		user->Socket()->Send(header);
		user->Socket()->Send(data);
		this->Unlock();
	}
}

/**************************************************************
 * Send news posted.
 **************************************************************/
void
TServer::SendNewsPost(const char* text)
{
	HLPacket data;
	if(data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	/************ Write to News file *****************/
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("News.txt");
	BFile file(path.Path(),B_READ_WRITE|B_CREATE_FILE);
	if(file.InitCheck() == B_OK)
	{
		file.Lock();
		off_t size;
		file.GetSize(&size);
		char* old = new char[size+1];
		::memset(old,0,size+1);
		file.Read(old,size);
		BString s;
		s << text << old;
		file.WriteAt(0,s.String(),s.Length());
		file.SetSize(s.Length());
		file.Unlock();
		file.Unset();
		delete[] old;
	}
	/**************************************************/
	data.AddString(HTLS_DATA_NEWS,text);
	int32 count = fUserList.CountItems();
	for(register int32 i = 0;i < count;i++)
	{
		HLPacket header;
		if( header.InitCheck() != B_OK)
		{
			Log("Memory was exhausted\n",T_ERROR_TYPE);
			return;
		}
		HUserItem *user = static_cast<HUserItem*>(fUserList.ItemAt(i));
		if(!user)
			continue;
		uint32 trans = user->Trans();
		header.CreateHeader(HTLS_HDR_NEWS_POST,++trans,0,data.Size()+2,1);
		user->SetTrans(trans);
		if(user->Lock())
		{
			user->Socket()->Send(header);
			user->Socket()->Send(data);
			user->Unlock();
		}
	}
}


/**************************************************************
 * Send user change.
 **************************************************************/
void
TServer::SendUserChange(uint16 sock,const char* nick,uint16 icon,uint16 color)
{
	HLPacket header,data;
	if(header.InitCheck() != B_OK || data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	data.AddUint16(HTLS_DATA_SOCKET,sock);
	data.AddUint16(HTLS_DATA_ICON,icon);
	data.AddString(HTLS_DATA_NICK,nick);
	data.AddUint16(HTLS_DATA_COLOUR,color);

	this->SendToAllUsers(data,HTLS_HDR_USER_CHANGE,data.Size()+2,4);
}


/**************************************************************
 * Send user info.
 **************************************************************/
void
TServer::SendUserInfo(HUserItem *user,uint32 sock,uint32 trans)
{
	HLPacket header,data;
	if( header.InitCheck() != B_OK || data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	HUserItem *target = this->FindUser(sock);
	if(target != NULL)
	{
		BString info;
		char host[1024];
		::memset(host,0,124);
		BNetEndpoint *endpoint = target->Socket();
		BNetAddress addr = endpoint->RemoteAddr();
		addr.GetAddr(host);
		info << "Nickname: " << target->Nick() << "\r"
			<< "Host: " << host << "\r";
		data.AddString(HTLS_DATA_USER_INFO,info.String());
		data.AddString(HTLS_DATA_NICK,target->Nick());

		header.CreateHeader(HTLS_HDR_TASK,trans,0,data.Size()+2,2);

		if(user->Lock())
		{
			user->Socket()->Send(header);
			user->Socket()->Send(data);
			user->Unlock();
		}
	}else{
		this->SendError(user,trans,"Could not find such a user");
	}
}

/**************************************************************
 * Receive file list.
 **************************************************************/
void
TServer::SendFileList(HUserItem* user,uint32 trans,const char* path)
{
	BPath dirpath = AppUtils().GetAppDirPath(be_app);
	dirpath.Append("Files");
	dirpath.Append(path);

	HLPacket header,data;
	if(header.InitCheck() != B_OK || data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	BDirectory dir( dirpath.Path() );
   	status_t err = B_NO_ERROR;
   	BEntry entry;
   	uint16 hc = 0;
	bool IsSilverWing = user->IsSilverWing();

	while( err == B_NO_ERROR )
	{
		err = dir.GetNextEntry( &entry, true );
		if( entry.InitCheck() != B_NO_ERROR )
			break;
		BPath filepath;
		if(entry.GetPath(&filepath) != B_OK)
		{
			break;
		}
		data.AddFile(filepath.Path(),IsSilverWing);
		hc++;
	}

	header.CreateHeader(HTLS_HDR_TASK,trans,0,data.Size()+2,hc);
	if(user->Lock())
	{
		user->Socket()->Send(header);
		if(data.Size() != 0)
			user->Socket()->Send(data);
		user->Unlock();
	}

}

/***********************************************************
 * Send file info.
 ***********************************************************/
void
TServer::SendFileInfo(HUserItem *user,uint32 trans,const char* filename,const char* dir_path)
{
	// get file info
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Files");
	path.Append(dir_path);
	path.Append(filename);

	BEntry entry(path.Path());
	if(entry.InitCheck() != B_OK)
	{
		SendError(user,trans,"Could not find such a file");
		return;
	}
	time_t ctime;
	time_t mtime;
	off_t size;
	entry.GetCreationTime(&ctime);
	entry.GetModificationTime(&mtime);
	entry.GetSize(&size);
	// create packet
	HLPacket header,data;
	if( header.InitCheck() != B_OK || data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	data.AddUint16(HTLS_DATA_FILE_ICON,0);
	data.AddDate(HTLS_DATA_FILE_MDATE,ctime);
	data.AddDate(HTLS_DATA_FILE_CDATE,mtime);
	data.AddString(HTLS_DATA_FILE_COMMENT,"");
	data.AddString(HTLS_DATA_FILE_NAME,filename);
	data.AddUint32(HTLS_DATA_FILE_SIZE,size);
	data.AddString(HTLS_DATA_FILE_CREATOR,"????");
	data.AddString(HTLS_DATA_FILE_TYPE,"????");

	header.CreateHeader(HTLS_HDR_TASK,trans,0,data.Size()+2,7);
	if(user->Lock())
	{
		user->Socket()->Send(header);
		user->Socket()->Send(data);
		user->Unlock();
	}
}

/***********************************************************
 * Reply creating private chat
 ***********************************************************/
void
TServer::SendPrvChatCreate(HUserItem *user,uint32 pcref,uint32 trans)
{
	HLPacket header,data;
	if( header.InitCheck() != B_OK || data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	if(user->Lock())
	{
		data.AddUint16(HTLS_DATA_CHAT_REF,pcref);
		header.CreateHeader(HTLS_HDR_TASK,trans,0,data.Size()+2,1);
		user->Socket()->Send(header);
		user->Socket()->Send(data);
		user->Unlock();
	}
}

/**************************************************************
 * Send invite private chat.
 **************************************************************/
void
TServer::SendPrvChatInvite(HUserItem *user,uint32 sock,uint32 pcref,uint32 trans)
{
	HLPacket header,data;
	if( header.InitCheck() != B_OK || data.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	if(this->Lock())
	{
		data.AddUint16(HTLS_DATA_SOCKET,user->Index());
		data.AddUint16(HTLS_DATA_CHAT_REF,pcref);
		data.AddString(HTLS_DATA_NICK,user->Nick());

		HUserItem *target = static_cast<HUserItem*>(this->FindUser(sock));
		trans = target->Trans();
		header.CreateHeader(HTLS_HDR_CHAT_INVITE,trans++,0,data.Size()+2,3);
		target->SetTrans(trans);

		target->Socket()->Send(header);
		target->Socket()->Send(data);
		this->Unlock();
	}
}

/***********************************************************
 * Receive SilverWing Mode
 ***********************************************************/
void
TServer::ReceiveSilverWingMode(HUserItem *user,uint32 len,uint16 hc,uint32 trans)
{
	HLPacket buf;
	len -= 2;
	ReadData(buf,len,user);

	this->SendSilverWingMode(user,trans);
}

/***********************************************************
 * Send SilverWing mode
 ***********************************************************/
void
TServer::SendSilverWingMode(HUserItem* user,uint32 trans)
{
	HLPacket header;
	if( header.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	if(user->Lock())
	{
		header.CreateHeader(HTLS_HDR_SILVERWING_MODE,trans,0,2,1);
		user->Socket()->Send(header);
		user->Unlock();
	}
	user->SetSilverWing(true);
}

/************************************************************************/
/*							Other functions								*/
/************************************************************************/
/***********************************************************
 * Write article.
 ***********************************************************/
void
TServer::WriteArticle(const char* path,const char* username,const char* message)
{
}

/***********************************************************
 * Read all pending data
 ***********************************************************/
uint32
TServer::ReadData(BNetBuffer &buf,uint32 len,HUserItem *user)
{
	uint32 size = 0;
	int32 r = 0;
	BNetEndpoint *endpoint = user->Socket();
	bigtime_t timeout = 60*1000000; //60 secs

	if( buf.InitCheck() != B_OK)
	{
		Log("Memory was exhausted\n",T_ERROR_TYPE);
		return 0;
	}

	while(len)
	{
		if(endpoint->IsDataPending(timeout) )
		{
			if(user->Lock())
			{
				r = endpoint->Receive(buf,len);
				user->Unlock();
			}
			if(r == B_ERROR || r == 0)
			{
				this->RemoveClient(user);
				break;
			}
			len -= r;
			size += r;
		}else
			break;
	}
	return size;
}


/***********************************************************
 * Move files
 ***********************************************************/
void
TServer::MoveFile(const char* filename,const char* old_path,const char* new_path)
{
	BPath path = AppUtils().GetAppDirPath(be_app);

	path.Append("Files");

	BPath dest_path = path;
	path.Append(old_path);
	path.Append(filename);

	dest_path.Append(new_path);

	BString cmd = "mv \"";
	cmd << path.Path() << "\" \"" << dest_path.Path() << "\"";
	::system(cmd.String());
}


/**************************************************************
 * Add upload.
 **************************************************************/
void
TServer::AddUpload(const char* filename,uint32 ref,uint32 data_pos)
{
	FileTrans *theFile = new FileTrans;
	theFile->ref = ref;
	theFile->filename = filename;
	theFile->isDownload = false;
	theFile->data_pos = data_pos;
	theFile->thread = NULL;
	if(this->Lock())
	{
		fFileTransList.AddItem(theFile);
		this->Unlock();
	}
}

/**************************************************************
 * Add download.
 **************************************************************/
void
TServer::AddDownload(const char* filename,uint32 ref,uint32 data_pos)
{
	FileTrans *theFile = new FileTrans;
	theFile->ref = ref;
	theFile->filename = filename;
	theFile->isDownload = true;
	theFile->data_pos = data_pos;
	theFile->thread = NULL;
	if(this->Lock())
	{
		fFileTransList.AddItem(theFile);
		this->Unlock();
	}
}

/**************************************************************
 * Remove file transfer.
 **************************************************************/
void
TServer::RemoveFileTrans(HFileTransThread *thread)
{
	if(this->Lock())
	{
		int32 count = fFileTransList.CountItems();
		for(int32 i = 0;i < count ;i++)
		{
			FileTrans *trans = static_cast<FileTrans*>(fFileTransList.ItemAt(i));
			if(trans->thread == thread)
			{
				fFileTransList.RemoveItem(trans);
				if(trans->isDownload)
				{
					fCurrentDownloads--;
					this->Log("End download\n",T_DOWNLOAD_END_TYPE);
				}else{
					fCurrentUploads--;
					this->Log("End upload\n",T_UPLOAD_END_TYPE);
				}
				delete trans->thread;
				delete trans;
			}
		}
		this->Unlock();
	}
}

/**************************************************************
 * Remove client from private chat.
 **************************************************************/
void
TServer::RemovePrvChatClient(HUserItem *user,uint32 pcref,uint32 trans)
{
	if(this->Lock())
	{
		HPrvChat *chat = FindPrvChat(pcref);
		if(chat)
		{
			chat->RemoveClient(user);
			if( chat->Users() == 0)
			{
				fPrvChatList.RemoveItem(chat);
				PRINT(("Private chat was removed\n" ));

				delete chat;
			}
		}
		this->Unlock();
	}
}

/**************************************************************
 * Add client to private chat.
 **************************************************************/
void
TServer::AddPrvChatClient(HUserItem *user,uint32 pcref,uint32 trans)
{
	HPrvChat *chat = FindPrvChat(pcref);
	if(chat)
	{
		chat->AddClient(user,true);
		chat->SendPrvUserList(user,trans);
	}
}

/**************************************************************
 * Create private chat.
 **************************************************************/
void
TServer::CreatePrvChat(HUserItem *user ,uint32 sock,uint32 trans)
{
	if(this->Lock())
	{
		HPrvChat *prvchat = new HPrvChat(fPrvChatIndex,this);
		//if(sock != user->Index())
		//	prvchat->AddClient(user);
		fPrvChatList.AddItem(prvchat);
		this->Unlock();
	}

	SendPrvChatCreate(user,fPrvChatIndex,trans);
	if(user->Index() != sock)
		SendPrvChatInvite(user,sock,fPrvChatIndex++,trans);
}

/**************************************************************
 * Create folder.
 **************************************************************/
void
TServer::CreateFolder(const char* inPath,const char* name)
{
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Files");
	path.Append(inPath);
	path.Append(name);

	BDirectory dir(path.Path());
	if(dir.CreateDirectory(path.Path(),&dir) != B_OK)
		PRINT(("Could not create directory\n" ));
}

/**************************************************************
 * Delete files.
 **************************************************************/
void
TServer::DeleteFile(const char* inPath,const char* name)
{
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Files");
	path.Append(inPath);
	path.Append(name);

	BNode node(path.Path());
	if(node.InitCheck() == B_OK)
	{
		BEntry entry(path.Path());
		entry_ref ref;
		entry.GetRef(&ref);
		// Send move to trash message to Tracker
		BMessenger tracker("application/x-vnd.Be-TRAK" );
		BMessage msg( B_DELETE_PROPERTY ) ;

	    BMessage specifier( 'sref' ) ;
	    specifier.AddRef( "refs", &ref ) ;
	    specifier.AddString( "property", "Entry" ) ;
	    msg.AddSpecifier( &specifier ) ;

    	msg.AddSpecifier( "Poses" ) ;
    	msg.AddSpecifier( "Window", 1 ) ;
		BMessage reply ;
    	tracker.SendMessage( &msg, &reply );
	}
}

/**************************************************************
 * Kick user.
 **************************************************************/
void
TServer::KickUser(uint32 sock)
{
	HUserItem *user = FindUser(sock);
	user->Socket()->Close();
	BMessage msg(T_REMOVE_CLIENT);
	msg.AddPointer("pointer",user);
	this->PostMessage(&msg);
}

/***********************************************************
 * Log to news file
 ***********************************************************/
void
TServer::Log2News(HUserItem *user,const char* text)
{
	BString str;
	time_t timet = time(NULL);
	const char* time = ctime(&timet);
	char* tmp = new char[strlen(time)+1];
	::strcpy(tmp,time);
	tmp[strlen(time)-1] = '\0';
	str << "From " << user->Nick() << " (" << tmp << "):\r"
		<< text
		<<	"\r_________________________________________________________\r";
	delete[] tmp;

	SendNewsPost(str.String());
}
/**************************************************************
 * Send log to main window.
 **************************************************************/
void
TServer::Log(const char* text,MessageType type) const
{
	BMessage msg(T_LOG_MESSAGE);
	BString log;
	time_t t = time(NULL);
	const char *tmp = ctime(&t);
	char *time = new char[strlen(tmp)+1];
	strcpy(time,tmp);
	time[strlen(tmp)-1] = '\0';
	log << time << "     " << text;
	delete[] time;
	msg.AddString("log",log.String());
	msg.AddInt32("type",(int32)type);
	fTarget->PostMessage(&msg);
}

/***********************************************************
 * ResetAccount
 ***********************************************************/
void
TServer::ResetAccount(const char* name)
{
	int32 count = fUserList.CountItems();
	for(int32 i = 0;i < count;i++)
	{
		HUserItem* item = static_cast<HUserItem*>(fUserList.ItemAt(i));

		if(!item)
			continue;

		if(::strcmp(item->Account(),name) == 0)
		{
			item->ResetAccount();
		}
	}
}

/**************************************************************
 * QuitRequested.
 **************************************************************/
bool
TServer::QuitRequested()
{
	fConnected = false;
	status_t	status;

	::wait_for_thread(fHandleConnection,&status);
	::wait_for_thread(fListenToClient,&status);
	::wait_for_thread(fTransThread,&status);
	Log("Server was stopped...\n");
	fServerSocket->Close();
	return BLooper::QuitRequested();
}
