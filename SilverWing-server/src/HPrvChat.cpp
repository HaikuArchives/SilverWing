#include <Autolock.h>
#include <NetDebug.h>
#include <Debug.h>

#include "HPrvChat.h"
#include "hl_magic.h"
#include "HUserItem.h"
#include "TServer.h"
#include "HLPacket.h"

/**************************************************************
 * Constructor.
 **************************************************************/
HPrvChat::HPrvChat(uint32 pcref,TServer *server)
	:BLocker()
	,fServer(server)
	,fTopic("")
	,fPcref(pcref)
{
	fUserList.MakeEmpty();
	BNetDebug::Enable(true);
}

/**************************************************************
 * Destructor.
 **************************************************************/
HPrvChat::~HPrvChat()
{
	
}

/**************************************************************
 * Add new client to private chat.
 **************************************************************/
void
HPrvChat::AddClient(HUserItem *user,bool send_to_all)
{
	BAutolock lock(this);
	
	if(send_to_all)
	{
		HLPacket data;
		if(data.InitCheck() != B_OK)
		{
			fServer->Log("Memory was exhausted\n",T_ERROR_TYPE);
			return;
		}
		PRINT(("PRVCHAT_USER_CHANGED: %s\n",user->Nick() ));
		data.AddUint16(HTLS_DATA_SOCKET,user->Index());
		data.AddUint16(HTLS_DATA_ICON,user->Icon());
		data.AddString(HTLS_DATA_NICK,user->Nick());
		data.AddUint16(HTLS_DATA_COLOUR,user->Color());
		data.AddUint32(HTLS_DATA_CHAT_REF ,fPcref);
		SendToAllUsers(data,HTLS_HDR_CHAT_USER_CHANGE,data.Size()+2,5);
	}
	fUserList.AddItem(user);
}

/**************************************************************
 * return how many users in this private chat.
 **************************************************************/
uint32
HPrvChat::Users() const
{
	return fUserList.CountItems();
}

/**************************************************************
 * Remove client from private chat.
 **************************************************************/
void
HPrvChat::RemoveClient(HUserItem* user)
{
	BAutolock lock(this);
	fUserList.RemoveItem(user);
	if(fUserList.CountItems() > 0)
	{
		HLPacket header,data;
		if( header.InitCheck() != B_OK || data.InitCheck() != B_OK)
		{
			fServer->Log("Memory was exhausted\n",T_ERROR_TYPE);
			return;
		}
	
		data.AddUint16(HTLS_DATA_SOCKET,user->Index());
		data.AddUint16(HTLS_DATA_CHAT_REF,this->Pcref());
		SendToAllUsers(data,HTLS_HDR_CHAT_USER_LEAVE,data.Size()+2,2);
	}
}

/**************************************************************
 * Send data to all users in private chat.
 **************************************************************/
void
HPrvChat::SendToAllUsers(HLPacket &data,uint32 type,uint32 data_size,uint32 hc)
{
	BAutolock lock(this);
	
	int32 count = fUserList.CountItems();
	
	for(register int32 i = 0;i < count;i++)
	{
		HUserItem *user = static_cast<HUserItem*>(fUserList.ItemAt(i) );
		if(!user)
			continue;
		uint32 trans = user->Trans();
		HLPacket header;
		if(header.InitCheck() != B_OK)
		{
			fServer->Log("No memory... Could not send prvchat\n",T_ERROR_TYPE);
			return;
		}
		header.CreateHeader(type,trans,0,data_size,hc);
		user->SetTrans(++trans);
		if(user->Lock())
		{
			user->Socket()->Send(header);
			user->Socket()->Send(data);
			user->Unlock();
		}
	}
}

/**************************************************************
 * Send private chat user list.
 **************************************************************/
void
HPrvChat::SendPrvUserList(HUserItem *user,uint32 sock)
{
	BAutolock lock(this);
	HLPacket header,data;
	if( header.InitCheck() != B_OK || data.InitCheck() != B_OK)
	{
		fServer->Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	int32 count = fUserList.CountItems();
	if(header.InitCheck() != B_OK || data.InitCheck() != B_OK)
	{
		fServer->Log("No memory... Could not send user list.\n",T_ERROR_TYPE);
		return;
	}
	for(register int32 i = 0;i < count;i++)
	{
		HUserItem *item = static_cast<HUserItem*>(this->fUserList.ItemAt(i));
		if(!item)
			continue;
		data.AddUser(HTLS_DATA_USER_LIST,item->Index(),item->Icon(),item->Color(),item->Nick());
	}
	
	header.CreateHeader(HTLS_HDR_TASK,user->Trans(),0,data.Size()+2,count);

	if(user->Lock())
	{
		user->Socket()->Send(header);
		if(count != 0)
			user->Socket()->Send(data);
		user->Unlock();
	}
}

/***********************************************************
 * Find user
 ***********************************************************/
bool
HPrvChat::FindUser(uint32 sock)
{
	BAutolock lock(this);
	bool result = false;
	int32 count = fUserList.CountItems();
	
	for(register int32 i = 0;i < count;i++)
	{
		HUserItem *user = static_cast<HUserItem*>(fUserList.ItemAt(i));
		if(!user)
			continue;
		if(user->Index() == sock)
		{
			result = true;
			break;	
		}
	}
	return result;
}

/***********************************************************
 * Set topic
 ***********************************************************/
void
HPrvChat::SetTopic(const char* topic)
{
	fTopic = topic;
	SendTopicChanged();
}


/***********************************************************
 * Send topic changed
 ***********************************************************/
void
HPrvChat::SendTopicChanged()
{
	HLPacket data;
	if( data.InitCheck() != B_OK)
	{
		fServer->Log("Memory was exhausted\n",T_ERROR_TYPE);
		return;
	}
	data.AddUint32(HTLS_DATA_CHAT_REF,fPcref);
	data.AddString(HTLS_DATA_CHAT_SUBJECT,fTopic.String());
	SendToAllUsers(data,HTLS_HDR_CHAT_SUBJECT,data.Size()+2,2);
}